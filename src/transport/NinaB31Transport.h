// Transport for a u-blox NINA-B31 module driven over UART with u-connectXpress AT commands:
// the senseBox MCU / MCU-S2 with the Bluetooth-Bee, and the MKR1000 with a NINA-B31.
// Tier 2: compiled in CI, tested by the senseBox colleagues on their hardware (plan §1.8).
// The module keeps the GATT table; the number of characteristics it accepts is a module
// limit (u-connectXpress raised NINA-B1's to 29) — PHYPHOX_BLE_INPUT_CHANNELS keeps the total
// well below that. Requires PhyphoxBLE::poll() in loop() to read the serial port.
#ifndef PHYPHOX_BLE_TRANSPORT_NINAB31_H
#define PHYPHOX_BLE_TRANSPORT_NINAB31_H

#include "Transport.h"

namespace phyphox {

class NinaB31Transport : public Transport {
public:
    bool begin(const GattLayout& layout, TransportListener& listener) override;
    bool notify(CharId id, const uint8_t* data, uint16_t len) override;
    void setValue(CharId id, const uint8_t* data, uint16_t len) override;
    bool connected() const override;
    uint16_t mtuPayload() const override;
    void poll() override;
    void restartAdvertising() override;
    const char* name() const override { return "NINA-B31"; }
    static NinaB31Transport& instance();
private:
    NinaB31Transport() {}
};

} // namespace phyphox

#endif
