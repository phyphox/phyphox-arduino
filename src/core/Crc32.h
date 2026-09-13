// CRC-32 (IEEE 802.3, polynomial 0xEDB88320, reflected) — the checksum the phyphox transfer
// header carries and the apps verify. Table-free, bit-serial: ~40 bytes of code, no 1 KB table
// in RAM, and a 5 KB document takes well under a millisecond on any supported board.
#ifndef PHYPHOX_BLE_CORE_CRC32_H
#define PHYPHOX_BLE_CORE_CRC32_H

#include <stdint.h>
#include <stddef.h>

namespace phyphox {

class Crc32 {
public:
    Crc32() : state_(0xFFFFFFFFu) {}
    void reset() { state_ = 0xFFFFFFFFu; }
    void update(const uint8_t* data, size_t len);
    void update(uint8_t byte);
    /// The finished value (does not alter the running state).
    uint32_t value() const { return state_ ^ 0xFFFFFFFFu; }
    /// One-shot helper.
    static uint32_t of(const uint8_t* data, size_t len) { Crc32 c; c.update(data, len); return c.value(); }
private:
    uint32_t state_;
};

} // namespace phyphox

#endif
