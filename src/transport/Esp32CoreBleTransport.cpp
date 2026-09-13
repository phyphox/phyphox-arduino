#include "Select.h"
#if defined(PHYPHOX_BLE_USE_ESP32)

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "Esp32CoreBleTransport.h"
#include <string.h>
#include <stdlib.h>

#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)
  #define PHYPHOX_BLE_NIMBLE 1
#endif

namespace phyphox {
namespace {

const char* const kSuffix = "-30f7-4671-8b43-5e40ba53514a";

TransportListener* listener = nullptr;
BLEServer* server = nullptr;
BLECharacteristic* chars[8] = {nullptr};          // EXPERIMENT, CONTROL, EVENT, DATA, LEGACY
BLECharacteristic** inputChars = nullptr;
BLECharacteristic** sensorChars = nullptr;
uint8_t inputCount = 0, sensorCount = 0;
volatile bool lastStatusOk = true;
volatile uint16_t negotiatedMtu = 23;
uint32_t connections = 0;
uint16_t requestedMtu = 0;
uint16_t connMin = 0, connMax = 0, connLatency = 0, connTimeout = 0;

void hex2(char* out, uint8_t v) { const char* h = "0123456789abcdef"; out[0] = h[v >> 4]; out[1] = h[v & 15]; }
void makeUuid(char* out, uint8_t group, uint8_t index) {
    memcpy(out, "cddf", 4); hex2(out + 4, group); hex2(out + 6, index); strcpy(out + 8, kSuffix);
}

/// Per-characteristic callbacks: writes and (NimBLE) subscriptions go to the core, the
/// notify status is captured for notify() to return synchronously.
class CharCb : public BLECharacteristicCallbacks {
public:
    explicit CharCb(CharId i) : id(i) {}
    void onWrite(BLECharacteristic* c) override {
        if (listener) listener->onWrite(id, c->getData(), (uint16_t)c->getLength());
    }
    void onStatus(BLECharacteristic*, Status s, uint32_t) override {
        // Only a send the stack refused counts as "not accepted" (the transfer retries it).
        // "Nobody subscribed" is not a refusal: data written while no phone listens is simply
        // dropped, as on every transport.
        lastStatusOk = (s != ERROR_GATT);
    }
#if defined(PHYPHOX_BLE_NIMBLE)
    void onSubscribe(BLECharacteristic*, ble_gap_conn_desc*, uint16_t subValue) override {
        if (listener) listener->onSubscribe(id, subValue != 0);
    }
#endif
private:
    CharId id;
};

#if !defined(PHYPHOX_BLE_NIMBLE)
/// Bluedroid: a subscription is a write to the CCCD descriptor.
class DescCb : public BLEDescriptorCallbacks {
public:
    explicit DescCb(CharId i) : id(i) {}
    void onWrite(BLEDescriptor* d) override {
        uint8_t* v = d->getValue();
        if (listener) listener->onSubscribe(id, d->getLength() > 0 && (v[0] & 1));
    }
private:
    CharId id;
};
#endif

class ServerCb : public BLEServerCallbacks {
public:
#if defined(PHYPHOX_BLE_NIMBLE)
    void onConnect(BLEServer* s, ble_gap_conn_desc* desc) override {
        if (connMin || connMax) s->requestConnParams(desc->conn_handle, connMin, connMax, connLatency, connTimeout);
        connections++;
        if (listener) listener->onConnect();
    }
    void onMtuChanged(BLEServer*, ble_gap_conn_desc*, uint16_t mtu) override { negotiatedMtu = mtu; }
#else
    void onConnect(BLEServer* s, esp_ble_gatts_cb_param_t* p) override {
        if (connMin || connMax) s->requestConnParams(p->connect.remote_bda, connMin, connMax, connLatency, connTimeout);
        connections++;
        if (listener) listener->onConnect();
    }
    void onMtuChanged(BLEServer*, esp_ble_gatts_cb_param_t* p) override { negotiatedMtu = p->mtu.mtu; }
#endif
    void onDisconnect(BLEServer*) override {
        if (connections) connections--;
        negotiatedMtu = 23;
        if (listener) listener->onDisconnect();
    }
};

BLECharacteristic* make(BLEService* svc, uint8_t group, uint8_t index, CharId id, uint32_t props, bool notifies) {
    char uuid[37]; makeUuid(uuid, group, index);
    BLECharacteristic* c = svc->createCharacteristic(uuid, props);
    c->setCallbacks(new CharCb(id));
    if (notifies) {
#if !defined(PHYPHOX_BLE_NIMBLE)
        BLE2902* d = new BLE2902();
        d->setCallbacks(new DescCb(id));
        c->addDescriptor(d);
#endif
    }
    return c;
}

/// The transfer runs here, never inside a stack callback and independent of how often the
/// sketch's loop() runs. Not pinned to a core (single-core chips exist).
void pumpTask(void*) {
    for (;;) {
        if (listener) listener->pump();
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

} // namespace

Esp32CoreBleTransport& Esp32CoreBleTransport::instance() { static Esp32CoreBleTransport t; return t; }

bool Esp32CoreBleTransport::begin(const GattLayout& layout, TransportListener& li) {
    listener = &li;
    BLEDevice::init(layout.deviceName);
    if (requestedMtu) BLEDevice::setMTU(requestedMtu + 3);
    server = BLEDevice::createServer();
    server->setCallbacks(new ServerCb());

    const uint32_t WRITE = BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR;
    const uint32_t NOTIFY = BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY;
    // handle counts: 1 per service, 2 per characteristic, 1 per CCCD — never the default 15
    BLEService* exp = server->createService(BLEUUID("cddf0001-30f7-4671-8b43-5e40ba53514a"), 1 + 3 * 2 + 1 + 2);
    chars[CH_EXPERIMENT] = make(exp, 0x00, 0x02, CharId{CH_EXPERIMENT, 0}, NOTIFY, true);
    chars[CH_CONTROL] = make(exp, 0x00, 0x03, CharId{CH_CONTROL, 0}, BLECharacteristic::PROPERTY_READ | WRITE, false);
    chars[CH_EVENT] = make(exp, 0x00, 0x04, CharId{CH_EVENT, 0}, BLECharacteristic::PROPERTY_READ | WRITE, false);
    exp->start();

    inputCount = layout.inputChannels; sensorCount = layout.sensors;
    uint8_t inputCharCount = 0;
    for (uint8_t k = 1; k <= inputCount; ++k) if (layout.inputChannelMask & (1u << k)) inputCharCount++;
    uint32_t handles = 1 + 3 + 2 * (inputCharCount + sensorCount) + (layout.legacyConfig ? 2 : 0) + 2;
    BLEService* data = server->createService(BLEUUID("cddf1001-30f7-4671-8b43-5e40ba53514a"), handles);
    chars[CH_DATA] = make(data, 0x10, 0x02, CharId{CH_DATA, 0}, NOTIFY, true);
    inputChars = (BLECharacteristic**)calloc(inputCount + 1, sizeof(BLECharacteristic*));
    for (uint8_t k = 1; k <= inputCount; ++k)
        if (layout.inputChannelMask & (1u << k))
            inputChars[k] = make(data, 0x20, k, CharId{CH_INPUT, k}, BLECharacteristic::PROPERTY_READ | WRITE, false);
    sensorChars = (BLECharacteristic**)calloc(sensorCount + 1, sizeof(BLECharacteristic*));
    for (uint8_t s = 1; s <= sensorCount; ++s)
        sensorChars[s] = make(data, 0x30, s, CharId{CH_SENSOR, s}, BLECharacteristic::PROPERTY_READ | WRITE, false);
    if (layout.legacyConfig)
        chars[CH_LEGACY_CONFIG] = make(data, 0x10, 0x03, CharId{CH_LEGACY_CONFIG, 0}, BLECharacteristic::PROPERTY_READ | WRITE, false);
    data->start();

    BLEAdvertising* adv = BLEDevice::getAdvertising();
    adv->addServiceUUID(exp->getUUID());
    adv->setScanResponse(true);
    BLEDevice::startAdvertising();

    xTaskCreate(pumpTask, "phyphoxBLE", 4096, nullptr, 1, nullptr);
    return true;
}

static BLECharacteristic* lookup(CharId id) {
    switch (id.kind) {
        case CH_INPUT: return (id.index && id.index <= inputCount) ? inputChars[id.index] : nullptr;
        case CH_SENSOR: return (id.index && id.index <= sensorCount) ? sensorChars[id.index] : nullptr;
        default: return chars[id.kind];
    }
}

bool Esp32CoreBleTransport::notify(CharId id, const uint8_t* data, uint16_t len) {
    BLECharacteristic* c = lookup(id);
    if (!c) return false;
    lastStatusOk = true;                       // onStatus() runs inside notify() on both stacks
    c->setValue(data, len);
    c->notify();
    return lastStatusOk;
}

void Esp32CoreBleTransport::setValue(CharId id, const uint8_t* data, uint16_t len) {
    BLECharacteristic* c = lookup(id);
    if (c) c->setValue(data, len);
}

bool Esp32CoreBleTransport::connected() const { return connections > 0; }
uint16_t Esp32CoreBleTransport::mtuPayload() const {
    uint16_t m = negotiatedMtu;
    return m > 23 ? (uint16_t)(m - 3) : PHYPHOX_BLE_DEFAULT_MTU;
}
void Esp32CoreBleTransport::requestMtu(uint16_t payload) { requestedMtu = payload; }
void Esp32CoreBleTransport::requestConnectionParameters(uint16_t mn, uint16_t mx, uint16_t lat, uint16_t to) {
    connMin = mn; connMax = mx; connLatency = lat; connTimeout = to;   // applied in onConnect
}
void Esp32CoreBleTransport::restartAdvertising() { BLEDevice::startAdvertising(); }

} // namespace phyphox

#endif
