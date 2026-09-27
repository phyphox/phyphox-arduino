#include "Select.h"
#if defined(PHYPHOX_BLE_USE_ESP32_BLE4)

#include <Arduino.h>
#include <BLE.h>
#include "Esp32Ble4Transport.h"
#include <string.h>

#if BLE_BLUEDROID
  #include <esp_gatts_api.h>
#endif

namespace phyphox {
namespace {

const char* const kSuffix = "-30f7-4671-8b43-5e40ba53514a";
const uint16_t kNoConn = 0xFFFF;
const uint8_t kMaxConns = 8;                      // above both stacks' configured connection limits

TransportListener* listener = nullptr;
BLEServer server;
BLECharacteristic chars[8];                       // EXPERIMENT, CONTROL, EVENT, DATA, LEGACY
BLECharacteristic* inputChars = nullptr;
BLECharacteristic* sensorChars = nullptr;
uint8_t inputCount = 0, sensorCount = 0;
uint16_t requestedMtu = 0;
uint16_t connMin = 0, connMax = 0, connLatency = 0, connTimeout = 0;

/// The connections this transport knows, with what they subscribed to. The library keeps the
/// same per connection, but only hands it out as a freshly allocated vector; this table is
/// read on every notify(). Written from the stack's task, read from the transfer task and
/// loop(), so every access goes through `mux`.
struct Conn {
    uint16_t handle;
    uint16_t mtu;
    bool experiment;                              // subscribed to cddf0002
    bool data;                                    // subscribed to cddf1002
};
Conn conns[kMaxConns];
uint8_t connCount = 0;
/// The connection that asked for the experiment (subscribed to it or wrote 1 to the control
/// characteristic); the transfer goes to that phone only.
uint16_t transferConn = kNoConn;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

Conn* findConn(uint16_t h) {
    for (uint8_t i = 0; i < connCount; ++i) if (conns[i].handle == h) return &conns[i];
    return nullptr;
}

void hex2(char* out, uint8_t v) { const char* h = "0123456789abcdef"; out[0] = h[v >> 4]; out[1] = h[v & 15]; }
void makeUuid(char* out, uint8_t group, uint8_t index) {
    memcpy(out, "cddf", 4); hex2(out + 4, group); hex2(out + 6, index); strcpy(out + 8, kSuffix);
}

#if BLE_BLUEDROID
/// Bluedroid reports the fate of a notification in two places. esp_ble_gatts_send_indicate()
/// returns ESP_FAIL when the ATT channel is already congested (notify(connHandle, …) returns
/// Fail, notify() below returns false). But a packet that passes that check is only queued to
/// the stack's own task, and if the channel congests before the task gets to it, the packet is
/// dropped there — reported solely through ESP_GATTS_CONF_EVT, which the 4.x library only logs.
/// A transfer packet therefore counts as sent only when its confirmation says so (ESP_GATT_OK,
/// or ESP_GATT_CONGESTED = "queued, now stop"), and while the channel is congested
/// (ESP_GATTS_CONGEST_EVT, not handled by the library either) nothing is sent at all. Measured
/// on 3.x with a Pixel 9 Pro (2026-09-13, Esp32CoreBleTransport); the stack underneath is the
/// same.
volatile bool congested = false;
volatile bool confPending = false;
volatile uint16_t confStatus = 0;
volatile uint16_t confConn = kNoConn;
uint16_t experimentHandle = 0;
const int kConfWaitMs = 20;                       // a confirmation normally arrives within a tick

int gattsHook(void* ev, void* arg) {
    esp_gatts_cb_event_t event = *(esp_gatts_cb_event_t*)ev;
    esp_ble_gatts_cb_param_t* param = (esp_ble_gatts_cb_param_t*)arg;
    if (event == ESP_GATTS_CONGEST_EVT) {
        congested = param->congest.congested;
    } else if (event == ESP_GATTS_CONF_EVT) {
        if (confPending && param->conf.handle == experimentHandle && param->conf.conn_id == confConn) {
            confStatus = param->conf.status;
            confPending = false;
        }
#if defined(PHYPHOX_BLE_ESP32_TRACE)
        if (param->conf.status != ESP_GATT_OK)
            Serial.printf("[phyphoxBLE] conf handle=%u status=0x%02x\n", param->conf.handle, param->conf.status);
#endif
    } else if (event == ESP_GATTS_DISCONNECT_EVT) {
        congested = false;
        confStatus = ESP_GATT_ERROR;
        confPending = false;
    }
    return 0;
}
#endif

void onSubscribed(CharId id, uint16_t h, bool enabled) {
    portENTER_CRITICAL(&mux);
    Conn* c = findConn(h);
    if (c) {
        if (id.kind == CH_EXPERIMENT) c->experiment = enabled;
        else if (id.kind == CH_DATA) c->data = enabled;
    }
    if (id.kind == CH_EXPERIMENT && enabled) transferConn = h;
    portEXIT_CRITICAL(&mux);
    if (listener) listener->onSubscribe(id, enabled);
}

BLECharacteristic make(BLEService& svc, uint8_t group, uint8_t index, CharId id, bool notifies) {
    char uuid[37]; makeUuid(uuid, group, index);
    BLECharacteristic c;
    if (notifies) {
        // the CCCD is created by the library; permissions fail closed, so reading the value
        // has to be allowed by name
        c = svc.createCharacteristic(BLEUUID(uuid), BLEProperty::Read | BLEProperty::Notify, BLEPermission::ReadOpen);
        c.onSubscribe([id](const BLECharacteristic&, const BLEConnInfo& conn, uint16_t subValue) {
            onSubscribed(id, conn.getHandle(), (subValue & 1) != 0);
        });
    } else {
        c = svc.createCharacteristic(BLEUUID(uuid), BLEProperty::Read | BLEProperty::Write | BLEProperty::WriteNR,
                                     BLEPermission::ReadWriteOpen);
    }
    c.onWrite([id](const BLECharacteristic& chr, const BLEConnInfo& conn) {
        size_t len = 0;
        const uint8_t* v = chr.getValue(&len);
        if (id.kind == CH_CONTROL && len >= 1 && v[0] == 1) {
            portENTER_CRITICAL(&mux);
            transferConn = conn.getHandle();
            portEXIT_CRITICAL(&mux);
        }
        if (listener) listener->onWrite(id, v, (uint16_t)len);
    });
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

Esp32Ble4Transport& Esp32Ble4Transport::instance() { static Esp32Ble4Transport t; return t; }

bool Esp32Ble4Transport::begin(const GattLayout& layout, TransportListener& li) {
    listener = &li;
    if (!BLE.begin(layout.deviceName)) return false;
#if BLE_BLUEDROID
    BLE.setCustomGattsHandler(gattsHook);
#endif
    if (requestedMtu) BLE.setMTU(requestedMtu + 3);
    server = BLE.createServer();
    if (!server) return false;
    server.advertiseOnDisconnect(false);          // the core restarts advertising itself

    server.onConnect([](const BLEServer&, const BLEConnInfo& conn) {
        uint16_t h = conn.getHandle();
        portENTER_CRITICAL(&mux);
        if (!findConn(h) && connCount < kMaxConns) {
            uint16_t m = conn.getMTU();
            conns[connCount++] = Conn{h, (uint16_t)(m > 23 ? m : 23), false, false};
        }
        portEXIT_CRITICAL(&mux);
        if (connMin || connMax) {
            BLEConnParams p;
            p.minInterval = connMin; p.maxInterval = connMax; p.latency = connLatency; p.timeout = connTimeout;
            server.updateConnParams(h, p);
        }
        if (listener) listener->onConnect();
    });
    server.onMtuChanged([](const BLEServer&, const BLEConnInfo& conn, uint16_t mtu) {
        portENTER_CRITICAL(&mux);
        Conn* c = findConn(conn.getHandle());
        if (c) c->mtu = mtu;
        portEXIT_CRITICAL(&mux);
    });
    server.onDisconnect([](const BLEServer&, const BLEConnInfo& conn, uint8_t) {
        uint16_t h = conn.getHandle();
        portENTER_CRITICAL(&mux);
        for (uint8_t i = 0; i < connCount; ++i) {
            if (conns[i].handle == h) { conns[i] = conns[--connCount]; break; }
        }
        if (transferConn == h) transferConn = kNoConn;
        portEXIT_CRITICAL(&mux);
        if (listener) listener->onDisconnect();
    });

    BLEUUID expUuid("cddf0001-30f7-4671-8b43-5e40ba53514a");
    BLEService exp = server.createService(expUuid);
    chars[CH_EXPERIMENT] = make(exp, 0x00, 0x02, CharId{CH_EXPERIMENT, 0}, true);
    chars[CH_CONTROL] = make(exp, 0x00, 0x03, CharId{CH_CONTROL, 0}, false);
    chars[CH_EVENT] = make(exp, 0x00, 0x04, CharId{CH_EVENT, 0}, false);

    inputCount = layout.inputChannels; sensorCount = layout.sensors;
    BLEService data = server.createService(BLEUUID("cddf1001-30f7-4671-8b43-5e40ba53514a"));
    chars[CH_DATA] = make(data, 0x10, 0x02, CharId{CH_DATA, 0}, true);
    inputChars = new BLECharacteristic[inputCount + 1];
    for (uint8_t k = 1; k <= inputCount; ++k)
        if (layout.inputChannelMask & (1u << k))
            inputChars[k] = make(data, 0x20, k, CharId{CH_INPUT, k}, false);
    sensorChars = new BLECharacteristic[sensorCount + 1];
    for (uint8_t s = 1; s <= sensorCount; ++s)
        sensorChars[s] = make(data, 0x30, s, CharId{CH_SENSOR, s}, false);
    if (layout.legacyConfig)
        chars[CH_LEGACY_CONFIG] = make(data, 0x10, 0x03, CharId{CH_LEGACY_CONFIG, 0}, false);

    // 4.x commits the whole GATT database here (handle counts are computed by the library)
    if (!server.start()) return false;
#if BLE_BLUEDROID
    experimentHandle = chars[CH_EXPERIMENT].getHandle();   // valid once the server has started
#endif

    BLEAdvertising adv = BLE.getAdvertising();
    adv.addServiceUUID(expUuid);
    adv.setScanResponse(true);                    // the name goes into the scan response
    BLE.startAdvertising();

    xTaskCreate(pumpTask, "phyphoxBLE", 4096, nullptr, 1, nullptr);
    return true;
}

static BLECharacteristic* lookup(CharId id) {
    switch (id.kind) {
        case CH_INPUT: return (id.index && id.index <= inputCount && inputChars[id.index]) ? &inputChars[id.index] : nullptr;
        case CH_SENSOR: return (id.index && id.index <= sensorCount && sensorChars[id.index]) ? &sensorChars[id.index] : nullptr;
        default: return chars[id.kind] ? &chars[id.kind] : nullptr;
    }
}

bool Esp32Ble4Transport::notify(CharId id, const uint8_t* data, uint16_t len) {
    BLECharacteristic* c = lookup(id);
    if (!c) return false;
    if (len > 512) len = 512;                  // the attribute's maximum on both stacks
    const bool experiment = (id.kind == CH_EXPERIMENT);

    // Who gets it: the experiment only the phone that asked for it (or, if that one has not
    // subscribed yet, nobody — the core retries); data every subscribed phone.
    uint16_t targets[kMaxConns];
    uint8_t n = 0;
    portENTER_CRITICAL(&mux);
    for (uint8_t i = 0; i < connCount; ++i) {
        if (experiment ? (conns[i].experiment && conns[i].handle == transferConn) : conns[i].data)
            targets[n++] = conns[i].handle;
    }
    portEXIT_CRITICAL(&mux);
    // "Nobody subscribed" is not a refusal for data: written while no phone listens, it is
    // simply dropped, as on every transport. A transfer packet with no receiver is refused, so
    // the core holds it until the phone that triggered the transfer has subscribed.
    if (!n) return !experiment;

#if BLE_BLUEDROID
    if (experiment) {
        if (congested) return false;           // the stack would drop it; the core retries later
        confStatus = 0;
        confConn = targets[0];
        confPending = true;
    }
#endif
    bool ok = true;
    for (uint8_t i = 0; i < n; ++i)
        if (c->notify(targets[i], data, len) != BTStatus::OK) ok = false;
#if BLE_BLUEDROID
    if (experiment) {
        if (!ok) { confPending = false; return false; }
        for (int i = 0; i < kConfWaitMs && confPending; ++i) vTaskDelay(1);
        if (confPending) {                     // no confirmation at all: count it as sent, never stall
            confPending = false;
#if defined(PHYPHOX_BLE_ESP32_TRACE)
            Serial.println("[phyphoxBLE] no conf within the wait");
#endif
            return true;
        }
        return confStatus == ESP_GATT_OK || confStatus == ESP_GATT_CONGESTED;
    }
#endif
    return ok;
}

void Esp32Ble4Transport::setValue(CharId id, const uint8_t* data, uint16_t len) {
    BLECharacteristic* c = lookup(id);
    if (c) c->setValue(data, len);
}

bool Esp32Ble4Transport::connected() const { return connCount > 0; }
uint16_t Esp32Ble4Transport::mtuPayload() const {
    // the MTU of the phone the transfer goes to; with no such phone, the latest connection's
    uint16_t m = 23;
    portENTER_CRITICAL(&mux);
    Conn* c = findConn(transferConn);
    if (c) m = c->mtu;
    else if (connCount) m = conns[connCount - 1].mtu;
    portEXIT_CRITICAL(&mux);
    return m > 23 ? (uint16_t)(m - 3) : PHYPHOX_BLE_DEFAULT_MTU;
}
void Esp32Ble4Transport::requestMtu(uint16_t payload) { requestedMtu = payload; }
void Esp32Ble4Transport::requestConnectionParameters(uint16_t mn, uint16_t mx, uint16_t lat, uint16_t to) {
    connMin = mn; connMax = mx; connLatency = lat; connTimeout = to;   // applied in onConnect
}
void Esp32Ble4Transport::restartAdvertising() { BLE.startAdvertising(); }

} // namespace phyphox

#endif
