#include "Select.h"
#if defined(PHYPHOX_BLE_USE_ARDUINOBLE)

#include <Arduino.h>
#if defined(PHYPHOX_BLE_STM32DUINOBLE)
  #include <STM32duinoBLE.h>
#else
  #include <ArduinoBLE.h>
#endif
#if defined(__has_include)
  #if __has_include(<utility/ATT.h>)
    #include <utility/ATT.h>
    #define PHYPHOX_BLE_HAS_ATT 1
  #endif
#endif
#include "ArduinoBleTransport.h"
#include <string.h>
#include <stdlib.h>

namespace phyphox {
namespace {

const char* const kSuffix = "-30f7-4671-8b43-5e40ba53514a";

struct Entry {
    char uuid[37];
    CharId id;
    BLECharacteristic* ch;
};

Entry* entries = nullptr;
uint8_t entryCount = 0;
BLEService* expService = nullptr;
BLEService* dataService = nullptr;
TransportListener* listener = nullptr;
bool isConnected = false;

void hex2(char* out, uint8_t v) { const char* h = "0123456789abcdef"; out[0] = h[v >> 4]; out[1] = h[v & 15]; }

/// "cddf" + two hex digits of `group` + two of `index` + the suffix.
void makeUuid(char* out, uint8_t group, uint8_t index) {
    memcpy(out, "cddf", 4); hex2(out + 4, group); hex2(out + 6, index); strcpy(out + 8, kSuffix);
}

Entry* find(const char* uuid) {
    for (uint8_t i = 0; i < entryCount; ++i) if (!strcasecmp(entries[i].uuid, uuid)) return &entries[i];
    return nullptr;
}
Entry* find(CharId id) {
    for (uint8_t i = 0; i < entryCount; ++i) if (entries[i].id.kind == id.kind && entries[i].id.index == id.index) return &entries[i];
    return nullptr;
}

void onWritten(BLEDevice, BLECharacteristic c) {
    Entry* e = find(c.uuid());
    if (e && listener) listener->onWrite(e->id, c.value(), (uint16_t)c.valueLength());
}
void onSubscribed(BLEDevice, BLECharacteristic c) { Entry* e = find(c.uuid()); if (e && listener) listener->onSubscribe(e->id, true); }
void onUnsubscribed(BLEDevice, BLECharacteristic c) { Entry* e = find(c.uuid()); if (e && listener) listener->onSubscribe(e->id, false); }
void onConnected(BLEDevice) { isConnected = true; if (listener) listener->onConnect(); }
void onDisconnected(BLEDevice) { isConnected = false; if (listener) listener->onDisconnect(); }

Entry& add(BLEService& svc, uint8_t group, uint8_t index, CharId id, uint16_t props, int size) {
    Entry& e = entries[entryCount++];
    makeUuid(e.uuid, group, index);
    e.id = id;
    e.ch = new BLECharacteristic(e.uuid, props, size, false);
    svc.addCharacteristic(*e.ch);
    if (props & (BLEWrite | BLEWriteWithoutResponse)) e.ch->setEventHandler(BLEWritten, onWritten);
    if (props & BLENotify) { e.ch->setEventHandler(BLESubscribed, onSubscribed); e.ch->setEventHandler(BLEUnsubscribed, onUnsubscribed); }
    return e;
}

} // namespace

ArduinoBleTransport& ArduinoBleTransport::instance() { static ArduinoBleTransport t; return t; }

bool ArduinoBleTransport::begin(const GattLayout& layout, TransportListener& li) {
    listener = &li;
    if (!BLE.begin()) return false;
    BLE.setLocalName(layout.deviceName);
    BLE.setDeviceName(layout.deviceName);

    uint8_t total = 4 + layout.inputChannels + layout.sensors + (layout.legacyConfig ? 1 : 0);
    entries = (Entry*)calloc(total, sizeof(Entry));
    entryCount = 0;
    expService = new BLEService("cddf0001-30f7-4671-8b43-5e40ba53514a");
    dataService = new BLEService("cddf1001-30f7-4671-8b43-5e40ba53514a");
    // ArduinoBLE gives no access to the negotiated MTU, so the transfer runs in 20-byte packets
    add(*expService, 0x00, 0x02, CharId{CH_EXPERIMENT, 0}, BLERead | BLENotify, 20);
    add(*expService, 0x00, 0x03, CharId{CH_CONTROL, 0}, BLERead | BLEWrite | BLEWriteWithoutResponse, 20);
    add(*expService, 0x00, 0x04, CharId{CH_EVENT, 0}, BLERead | BLEWrite | BLEWriteWithoutResponse, 20);
    add(*dataService, 0x10, 0x02, CharId{CH_DATA, 0}, BLERead | BLENotify, layout.dataValueSize < 20 ? 20 : layout.dataValueSize);
    for (uint8_t k = 1; k <= layout.inputChannels; ++k)
        if (layout.inputChannelMask & (1u << k))
            add(*dataService, 0x20, k, CharId{CH_INPUT, k}, BLERead | BLEWrite | BLEWriteWithoutResponse, 4);
    for (uint8_t s = 1; s <= layout.sensors; ++s)
        add(*dataService, 0x30, s, CharId{CH_SENSOR, s}, BLERead | BLEWrite | BLEWriteWithoutResponse, layout.sensorValueSize[s - 1]);
    if (layout.legacyConfig)
        add(*dataService, 0x10, 0x03, CharId{CH_LEGACY_CONFIG, 0}, BLERead | BLEWrite | BLEWriteWithoutResponse, 20);

    BLE.addService(*expService);
    BLE.addService(*dataService);
    BLE.setAdvertisedService(*expService);
    BLE.setEventHandler(BLEConnected, onConnected);
    BLE.setEventHandler(BLEDisconnected, onDisconnected);
    return BLE.advertise() != 0;
}

bool ArduinoBleTransport::notify(CharId id, const uint8_t* data, uint16_t len) {
    Entry* e = find(id);
    if (!e) return false;
    // writeValue() blocks until the controller has a free buffer, so it is never refused
    return e->ch->writeValue(data, len) != 0;
}

void ArduinoBleTransport::setValue(CharId id, const uint8_t* data, uint16_t len) {
    Entry* e = find(id);
    if (e) e->ch->writeValue(data, len);
}

void ArduinoBleTransport::requestMtu(uint16_t payload) {
#if defined(PHYPHOX_BLE_HAS_ATT)
    ATT.setMaxMtu(payload + 3);     // lets the phone negotiate up to this for data notifications
#else
    (void)payload;
#endif
}

bool ArduinoBleTransport::connected() const { return isConnected; }
uint16_t ArduinoBleTransport::mtuPayload() const { return PHYPHOX_BLE_DEFAULT_MTU; }

void ArduinoBleTransport::requestConnectionParameters(uint16_t minInterval, uint16_t maxInterval, uint16_t, uint16_t timeout) {
    BLE.setConnectionInterval(minInterval, maxInterval);
    BLE.setSupervisionTimeout(timeout);
}

void ArduinoBleTransport::poll() { BLE.poll(); }
void ArduinoBleTransport::restartAdvertising() { BLE.advertise(); }

} // namespace phyphox

#endif
