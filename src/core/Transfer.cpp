#include "Transfer.h"
#include "Crc32.h"
#include <string.h>

namespace phyphox {

void BufferSource::set(const uint8_t* bytes, uint32_t len) {
    bytes_ = bytes;
    len_ = len;
    crc_ = Crc32::of(bytes, len);
}

uint32_t BufferSource::read(uint32_t offset, uint8_t* out, uint32_t n) const {
    if (offset >= len_) return 0;
    uint32_t avail = len_ - offset;
    if (n > avail) n = avail;
    memcpy(out, bytes_ + offset, n);
    return n;
}

void TransferSession::begin(const ByteSource& source, uint16_t payload) {
    source_ = &source;
    payload_ = payload < 20 ? 20 : payload;
    offset_ = 0;
    lastLen_ = 0;
    packets_ = 0;
    retries_ = 0;
    retryAt_ = 0;
    pending_ = false;
    lastProgress_ = 0;
    state_ = HEADER;
}

uint16_t TransferSession::nextPacket(uint8_t* out, uint32_t nowMs) {
    if (!source_ || !(state_ == HEADER || state_ == BODY)) return 0;
    if (pending_) return 0;                                   // awaiting a verdict
    if ((int32_t)(nowMs - retryAt_) < 0) return 0;            // backing off, or the start delay
    uint16_t len = 0;
    if (state_ == HEADER) {
        // "phyphox" + u32 size + u32 crc, big endian; the rest of the packet is zero.
        memset(out, 0, payload_);
        memcpy(out, "phyphox", 7);
        uint32_t size = source_->size(), crc = source_->crc();
        out[7] = size >> 24; out[8] = size >> 16; out[9] = size >> 8; out[10] = size;
        out[11] = crc >> 24; out[12] = crc >> 16; out[13] = crc >> 8; out[14] = crc;
        len = payload_ < 15 ? 15 : payload_;
        if (len > 20) len = 20;   // the header is one 20-byte packet whatever the MTU (1.x behaviour)
    } else {
        len = (uint16_t)source_->read(offset_, out, payload_);
        if (len == 0) { state_ = DONE; return 0; }
    }
    lastLen_ = len;
    pending_ = true;
    return len;
}

void TransferSession::accepted() {
    if (!pending_) return;
    pending_ = false;
    retries_ = 0;
    retryAt_ = 0;
    packets_++;
    if (state_ == HEADER) {
        state_ = BODY;
    } else {
        offset_ += lastLen_;
        if (offset_ >= source_->size()) state_ = DONE;
    }
}

void TransferSession::refused(uint32_t nowMs) {
    if (!pending_) return;
    pending_ = false;
    if (retries_ < 0xffff) retries_++;
    uint32_t delay = PHYPHOX_BLE_TRANSFER_RETRY_DELAY_MS;
    for (uint16_t i = 1; i < retries_ && delay < PHYPHOX_BLE_TRANSFER_RETRY_MAX_MS; ++i) delay *= 2;
    if (delay > PHYPHOX_BLE_TRANSFER_RETRY_MAX_MS) delay = PHYPHOX_BLE_TRANSFER_RETRY_MAX_MS;
    retryAt_ = nowMs + delay;
}

} // namespace phyphox
