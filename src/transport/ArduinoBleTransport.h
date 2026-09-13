// Transport for the ArduinoBLE API: ArduinoBLE (Nano 33 BLE / IoT, UNO R4 WiFi, MKR WiFi 1010,
// Nano RP2040 Connect, Portenta, GIGA) and STM32duinoBLE (same API, different include).
// Notifications are flow-controlled by the library itself: BLECharacteristic::writeValue()
// blocks until the controller has a free buffer (HCI.cpp, sendAclPkt), so notify() never
// returns false here. Requires PhyphoxBLE::poll() in loop() (BLE.poll()).
#ifndef PHYPHOX_BLE_TRANSPORT_ARDUINOBLE_H
#define PHYPHOX_BLE_TRANSPORT_ARDUINOBLE_H

#include "Transport.h"

namespace phyphox {

class ArduinoBleTransport : public Transport {
public:
    bool begin(const GattLayout& layout, TransportListener& listener) override;
    bool notify(CharId id, const uint8_t* data, uint16_t len) override;
    void setValue(CharId id, const uint8_t* data, uint16_t len) override;
    bool connected() const override;
    uint16_t mtuPayload() const override;
    void requestConnectionParameters(uint16_t, uint16_t, uint16_t, uint16_t) override;
    void poll() override;
    void restartAdvertising() override;
    const char* name() const override { return "ArduinoBLE"; }
    static ArduinoBleTransport& instance();
private:
    ArduinoBleTransport() {}
    // Both transfer triggers: BLESubscribed on the experiment characteristic and a write of 1
    // to the control characteristic (docs: either is allowed).
};

} // namespace phyphox

#endif
