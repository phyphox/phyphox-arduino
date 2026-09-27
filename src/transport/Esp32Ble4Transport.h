// Transport for the BLE library of arduino-esp32 4.x (`#include <BLE.h>`, the global `BLE`
// object, value handles and per-event lambdas). Core 4.0 replaced the 3.x BLEDevice library
// without a compatibility layer, so 3.x keeps Esp32CoreBleTransport and Select.h picks one of
// the two by ESP_ARDUINO_VERSION_MAJOR. Written against 4.0.0-RC1 (2026-09-23).
//
// The 4.x library is stack-agnostic (Bluedroid on the classic ESP32, NimBLE elsewhere, also on
// the ESP32-P4 through esp-hosted), and keeps subscriptions per connection on both stacks —
// the 3.x CCCD workarounds are gone. What it does not do is report a refused notification:
// the broadcast notify() returns OK whatever the stack said, and onStatus() is declared but
// never called (RC1). The per-connection notify(connHandle, …) does return Fail when the stack
// refuses a packet up front, so this transport notifies each subscribed connection itself.
// Bluedroid can still drop an accepted packet inside its own task; as on 3.x, the experiment
// characteristic waits for ESP_GATTS_CONF_EVT through the custom GATTS handler (see the .cpp).
// Build with -DPHYPHOX_BLE_ESP32_TRACE to see refused confirmations on Serial. poll() is a
// no-op: the stack runs in its own tasks.
#ifndef PHYPHOX_BLE_TRANSPORT_ESP32BLE4_H
#define PHYPHOX_BLE_TRANSPORT_ESP32BLE4_H

#include "Transport.h"

namespace phyphox {

class Esp32Ble4Transport : public Transport {
public:
    bool begin(const GattLayout& layout, TransportListener& listener) override;
    bool notify(CharId id, const uint8_t* data, uint16_t len) override;
    void setValue(CharId id, const uint8_t* data, uint16_t len) override;
    bool connected() const override;
    uint16_t mtuPayload() const override;
    void requestMtu(uint16_t payload) override;
    void requestConnectionParameters(uint16_t, uint16_t, uint16_t, uint16_t) override;
    void poll() override {}
    bool drivesTransfer() const override { return true; }
    void restartAdvertising() override;
    const char* name() const override { return "ESP32"; }
    static Esp32Ble4Transport& instance();
private:
    Esp32Ble4Transport() {}
};

} // namespace phyphox

#endif
