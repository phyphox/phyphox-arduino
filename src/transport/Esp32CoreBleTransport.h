// Transport for the ESP32 core's BLE library (BLEDevice / BLEServer / BLECharacteristic),
// which sits on Bluedroid on the classic ESP32 and on NimBLE on the S3/C3/C6/H2/P4 variants
// of core 3.x. One transport, two `#if defined(CONFIG_BT_NIMBLE_ROLE_PERIPHERAL)` branches at
// the callback layer, because the two stacks deliver subscriptions differently (Bluedroid: a
// write to the CCCD descriptor; NimBLE: BLECharacteristicCallbacks::onSubscribe) — the
// 1.x fix in commit 42bad88. Never pins a task to a core (1.x commit ca630b9, single-core C3).
//
// notify() returns void in this library on both stacks; a refused send arrives asynchronously
// as BLECharacteristicCallbacks::onStatus(ERROR_GATT, rc), which this transport forwards as
// onNotifyStatus(false). That is the signal the transfer's retry hangs off (plan §3.4).
// poll() is a no-op: the stack runs in its own tasks.
#ifndef PHYPHOX_BLE_TRANSPORT_ESP32COREBLE_H
#define PHYPHOX_BLE_TRANSPORT_ESP32COREBLE_H

#include "Transport.h"

namespace phyphox {

class Esp32CoreBleTransport : public Transport {
public:
    bool begin(const GattLayout& layout, TransportListener& listener) override;
    bool notify(CharId id, const uint8_t* data, uint16_t len) override;
    void setValue(CharId id, const uint8_t* data, uint16_t len) override;
    bool connected() const override;
    uint16_t mtuPayload() const override;
    void requestMtu(uint16_t payload) override;
    void requestConnectionParameters(uint16_t, uint16_t, uint16_t, uint16_t) override;
    void poll() override {}
    void restartAdvertising() override;
    const char* name() const override { return "ESP32"; }
    static Esp32CoreBleTransport& instance();
private:
    Esp32CoreBleTransport() {}
    // Services are created with an explicit handle count (2 per characteristic + 1 per CCCD),
    // never the library's default of 15 — plan §3.3.
};

} // namespace phyphox

#endif
