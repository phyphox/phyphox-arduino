#include "Select.h"
#if defined(PHYPHOX_BLE_USE_NINAB31)

// u-blox NINA-B31 over UART with u-connectXpress AT commands: the senseBox MCU / MCU-S2 with
// the Bluetooth-Bee, and the MKR1000 with a NINA-B31. Tier 2 (compiled in CI, tested on
// senseBox hardware). The command set is the one the 1.x backend used since 2021; what is new
// is how the serial line is read:
//
//  - The module raises unsolicited events (+UU…: connect, disconnect, attribute written, read
//    request) at any time, also while a command waits for its OK. 1.x flushed the input before
//    every command and read the response until "OK", so an event that arrived during any command
//    was lost — and with 2.0 sending a data notification per write() from the moment the phone
//    connects, the control write that starts the transfer was lost more often than not. Now one
//    line reader classifies every line: events go into a queue and are dispatched from poll(),
//    responses complete the command in flight. Nothing is ever flushed.
//  - The CCCD handle the module returns for a notify characteristic lets the transport see the
//    phone subscribe: the transfer starts on subscription as the phyphox docs describe (the
//    control write remains the second trigger), and data notifications are not sent to a phone
//    that has not subscribed to them — each would be a UART round trip for nothing.
//  - A read request on a characteristic without a stored value is answered (with the last value
//    or a zero byte) instead of leaving the phone's GATT queue hanging for the ATT timeout.
//  - Characteristics get the properties they need (notify only where the phone subscribes),
//    which is what the module manual recommends against its attribute-table limit.
//
// Requires PhyphoxBLE::poll() in loop() to read the serial port. -DPHYPHOX_BLE_NINA_TRACE
// mirrors the whole AT dialogue on Serial.
#include <Arduino.h>
#include "NinaB31Transport.h"
#include "NinaB31Protocol.h"
#include <string.h>
#include <stdlib.h>

// The UART the module hangs on: Serial3 on the senseBox MCU's core (which identifies as an
// MKR1000), Serial1 on the MCU-S2. Override with -DPHYPHOX_BLE_NINA_SERIAL=SerialX.
#if defined(PHYPHOX_BLE_NINA_SERIAL)
  #define SerialBLE PHYPHOX_BLE_NINA_SERIAL
#elif defined(ARDUINO_SAMD_MKR1000)
  #define SerialBLE Serial3
#elif defined(ARDUINO_SENSEBOX_MCU_ESP32S2)
  #define SerialBLE Serial1
#endif

namespace phyphox {
namespace {

using namespace nina;

Print* trace = nullptr;          ///< PhyphoxBLE::begin(&Serial); or the compile-time define
#if defined(PHYPHOX_BLE_NINA_TRACE)
  #define NINA_TRACE(prefix, s) do { Print* o = trace ? trace : &Serial; o->print(prefix); o->println(s); } while (0)
#else
  #define NINA_TRACE(prefix, s) do { if (trace) { trace->print(prefix); trace->println(s); } } while (0)
#endif
#define NINA_TRACE_OUT(s) NINA_TRACE("> ", s)
#define NINA_TRACE_LINE(s) NINA_TRACE("< ", s)

struct Entry {
    int handle = -1;             ///< characteristic value handle
    int cccd = 0;                ///< its CCCD handle, 0 when it has none
    CharId id;
    bool subscribed = false;     ///< the phone enabled notifications (CCCD written 01 00)
    uint8_t value[20];           ///< last value set or notified, for read requests
    uint8_t valueLen = 0;
};

Entry* entries = nullptr;
uint8_t entryCount = 0;
TransportListener* listener = nullptr;
bool isConnected = false;
int connHandle = 0;
bool cccdReported = false;       ///< the module reports CCCD writes: subscription tracking works
bool dispatching = false;

LineReader reader;
EventQueue<PHYPHOX_BLE_NINA_EVENT_QUEUE> events;
char response[LINE_MAX];         ///< the payload line of the command in flight (+UBTGCHA:…)
bool responseDone = false, responseOk = false;

const char* const kSuffixHex = "30F746718B435E40BA53514A";
// advertising payload with the experiment service UUID (cddf0001…), as 1.x sent it
const char* const kAdvertisement = "020A0605121800280011074A5153BA405E438B7146F7300100DFCD";

void uuidHex(uint8_t group, uint8_t index, char* out) {
    snprintf(out, 40, "CDDF%02X%02X%s", group, index, kSuffixHex);
}

/// Every byte from the module passes here: events are queued, the rest completes the command.
void feed(char c) {
    if (!reader.feed(c)) return;
    const char* line = reader.line();
    NINA_TRACE_LINE(line);
    if (isEvent(line)) { events.push(line); return; }
    if (strcmp(line, "OK") == 0) { responseOk = true; responseDone = true; return; }
    if (strcmp(line, "ERROR") == 0) { responseOk = false; responseDone = true; return; }
    strncpy(response, line, LINE_MAX - 1); response[LINE_MAX - 1] = 0;
}

void drain() { while (SerialBLE.available()) feed((char)SerialBLE.read()); }

/// Send a command and wait for OK/ERROR; events arriving meanwhile are queued, not lost.
bool command(const char* cmd, uint32_t timeout) {
    drain();
    responseDone = false; responseOk = false; response[0] = 0;
    NINA_TRACE_OUT(cmd);
    SerialBLE.print(cmd); SerialBLE.write('\r'); SerialBLE.flush();
    uint32_t start = millis();
    while (!responseDone && millis() - start < timeout) {
        if (SerialBLE.available()) feed((char)SerialBLE.read());
    }
    return responseDone && responseOk;
}

bool configModule() {
    // blind: the module may still have flow control on and not answer; store and restart
    SerialBLE.print("AT+UMRS=115200,2,8,1,1\r");   // 115200 8N1, no flow control
    SerialBLE.print("AT&W0\r");
    SerialBLE.print("AT+CPWROFF\r");
    SerialBLE.flush();
    delay(2000);
    return command("ATE0", 500);
}

int addService(uint8_t group, uint8_t index) {
    char cmd[64], uuid[40]; uuidHex(group, index, uuid);
    snprintf(cmd, sizeof(cmd), "AT+UBTGSER=%s", uuid);
    return command(cmd, 1000) ? firstNumber(response) : -1;
}

/// properties: a bit field (02 read, 04 write without response, 08 write, 10 notify).
int addCharacteristic(uint8_t group, uint8_t index, uint8_t properties, CharId id) {
    char cmd[64], uuid[40]; uuidHex(group, index, uuid);
    snprintf(cmd, sizeof(cmd), "AT+UBTGCHA=%s,%02x,1,1", uuid, properties);
    Entry& e = entries[entryCount++];
    e.id = id; e.handle = -1; e.cccd = 0; e.subscribed = false; e.valueLen = 0;
    if (command(cmd, 1000)) parseCharacteristicResponse(response, e.handle, e.cccd);
    return e.handle;
}

Entry* find(CharId id) {
    for (uint8_t i = 0; i < entryCount; ++i) if (entries[i].id.kind == id.kind && entries[i].id.index == id.index) return &entries[i];
    return nullptr;
}
Entry* findByHandle(int handle) {
    for (uint8_t i = 0; i < entryCount; ++i) if (entries[i].handle == handle) return &entries[i];
    return nullptr;
}
Entry* findByCccd(int handle) {
    if (handle <= 0) return nullptr;
    for (uint8_t i = 0; i < entryCount; ++i) if (entries[i].cccd == handle) return &entries[i];
    return nullptr;
}

void answerRead(const Event& ev) {
    Entry* e = findByHandle(ev.handle);
    char cmd[80];
    char hex[44];
    if (e && e->valueLen) encodeHex(e->value, e->valueLen, hex); else strcpy(hex, "00");
    snprintf(cmd, sizeof(cmd), "AT+UBTGRR=%d,%s", ev.conn, hex);
    command(cmd, 1000);
}

void dispatch(const char* line) {
    Event ev = parseEvent(line);
    switch (ev.kind) {
        case EV_CONNECTED:
            isConnected = true; connHandle = ev.conn;
            for (uint8_t i = 0; i < entryCount; ++i) entries[i].subscribed = false;
            if (listener) listener->onConnect();
            break;
        case EV_DISCONNECTED:
            isConnected = false;
            for (uint8_t i = 0; i < entryCount; ++i) entries[i].subscribed = false;
            if (listener) listener->onDisconnect();
            break;
        case EV_WRITE: {
            if (Entry* e = findByHandle(ev.handle)) {
                if (ev.len <= sizeof(e->value)) { memcpy(e->value, ev.data, ev.len); e->valueLen = (uint8_t)ev.len; }
                if (listener) listener->onWrite(e->id, ev.data, ev.len);
            } else if (Entry* c = findByCccd(ev.handle)) {
                cccdReported = true;
                c->subscribed = ev.len >= 1 && (ev.data[0] & 1);
                if (listener) listener->onSubscribe(c->id, c->subscribed);
            }
            break;
        }
        case EV_READ: answerRead(ev); break;
        default: break;
    }
}

} // namespace

NinaB31Transport& NinaB31Transport::instance() { static NinaB31Transport t; return t; }
void NinaB31Transport::setTrace(Print* out) { trace = out; }

bool NinaB31Transport::begin(const GattLayout& layout, TransportListener& li) {
    listener = &li;
    SerialBLE.begin(115200);
    delay(500);
    command("AT", 500);                      // a throwaway in case the port was already open
    bool up = false;
    for (int i = 0; i < 3 && !up; ++i) up = command("ATE0", 500) || configModule();
    if (!up) return false;
    command("AT+UBTDM=1", 1000);             // stop advertising while configuring
    char cmd[80];
    snprintf(cmd, sizeof(cmd), "AT+UBTAD=%s", kAdvertisement); command(cmd, 1000);

    uint8_t total = 4 + layout.inputChannels + layout.sensors + (layout.legacyConfig ? 1 : 0);
    if (entries) free(entries);
    entries = (Entry*)calloc(total, sizeof(Entry));
    if (!entries) return false;
    entryCount = 0;
    // properties: 02 read, 04 write without response, 08 write, 10 notify
    if (addService(0x00, 0x01) < 0) return false;
    addCharacteristic(0x00, 0x03, 0x0a, CharId{CH_CONTROL, 0});      // read | write
    if (addCharacteristic(0x00, 0x02, 0x12, CharId{CH_EXPERIMENT, 0}) < 0) return false;   // read | notify
    addCharacteristic(0x00, 0x04, 0x0c, CharId{CH_EVENT, 0});        // write, write without response
    if (addService(0x10, 0x01) < 0) return false;
    addCharacteristic(0x10, 0x02, 0x12, CharId{CH_DATA, 0});         // read | notify
    for (uint8_t k = 1; k <= layout.inputChannels; ++k)
        if (layout.inputChannelMask & (1u << k)) addCharacteristic(0x20, k, 0x0c, CharId{CH_INPUT, k});
    for (uint8_t s = 1; s <= layout.sensors; ++s) addCharacteristic(0x30, s, 0x0c, CharId{CH_SENSOR, s});
    if (layout.legacyConfig) addCharacteristic(0x10, 0x03, 0x0e, CharId{CH_LEGACY_CONFIG, 0});

    snprintf(cmd, sizeof(cmd), "AT+UBTLN=\"%.29s\"", layout.deviceName); command(cmd, 1000);
    snprintf(cmd, sizeof(cmd), "AT+UBTAD=%s", kAdvertisement); command(cmd, 1000);
    return command("AT+UBTDM=3", 1000);
}

bool NinaB31Transport::notify(CharId id, const uint8_t* data, uint16_t len) {
    Entry* e = find(id);
    if (!e || e->handle < 0 || !isConnected) return false;
    if (len > sizeof(e->value)) len = sizeof(e->value);   // the module sends 20 bytes per notification
    memcpy(e->value, data, len); e->valueLen = (uint8_t)len;
    // Once the module has shown it reports CCCD writes, a notification nobody subscribed to is
    // not sent — as a stack would drop it — instead of costing a UART round trip per write().
    if (id.kind == CH_DATA && cccdReported && !e->subscribed) return true;
    char cmd[80], hex[44];
    encodeHex(data, len, hex);
    snprintf(cmd, sizeof(cmd), "AT+UBTGSN=%d,%d,%s", connHandle, e->handle, hex);
    return command(cmd, 1000);
}

void NinaB31Transport::setValue(CharId id, const uint8_t* data, uint16_t len) {
    Entry* e = find(id);
    if (!e) return;
    if (len > sizeof(e->value)) len = sizeof(e->value);
    memcpy(e->value, data, len); e->valueLen = (uint8_t)len;
}

bool NinaB31Transport::connected() const { return isConnected; }
uint16_t NinaB31Transport::mtuPayload() const { return PHYPHOX_BLE_DEFAULT_MTU; }

void NinaB31Transport::poll() {
    drain();
    if (dispatching) return;                 // a callback called write() or poll(): the queue waits
    dispatching = true;
    char line[LINE_MAX];
    while (events.pop(line)) { dispatch(line); drain(); }
    dispatching = false;
}

void NinaB31Transport::restartAdvertising() { command("AT+UBTDM=3", 1000); }

} // namespace phyphox

#endif
