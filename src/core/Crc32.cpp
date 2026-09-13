#include "Crc32.h"

namespace phyphox {

void Crc32::update(uint8_t byte) {
    uint32_t c = state_ ^ byte;
    for (int i = 0; i < 8; ++i) c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
    state_ = c;
}

void Crc32::update(const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; ++i) update(data[i]);
}

} // namespace phyphox
