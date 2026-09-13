// The experiment transfer, as the phyphox docs specify it (bluetooth-low-energy.md, "Phyphox
// service"): a header packet `phyphox` + u32 size + u32 CRC-32 (big endian), then the document
// in MTU-sized notifications on cddf0002. Started by the app subscribing to cddf0002 OR by the
// app writing 1 to cddf0003, whichever comes first; a second trigger while a transfer runs is
// ignored; a disconnect aborts.
//
// Flow control (plan §3.4): the transport reports whether the stack accepted a packet. A
// refused packet is re-sent after PHYPHOX_BLE_TRANSFER_RETRY_DELAY_MS, up to
// PHYPHOX_BLE_TRANSFER_RETRIES times; no fixed pacing delay anywhere.
//
// This class is transport-independent: it only hands out the next packet and takes the
// verdict back. The Server drives it from poll() / the transport's callbacks.
#ifndef PHYPHOX_BLE_CORE_TRANSFER_H
#define PHYPHOX_BLE_CORE_TRANSFER_H

#include <stdint.h>
#include "Config.h"

namespace phyphox {

/// Anything that can serve document bytes by offset: the Serializer for generated experiments,
/// a plain buffer for user-supplied XML.
class ByteSource {
public:
    virtual ~ByteSource() {}
    virtual uint32_t size() const = 0;
    virtual uint32_t crc() const = 0;
    virtual uint32_t read(uint32_t offset, uint8_t* out, uint32_t n) const = 0;
};

class BufferSource : public ByteSource {
public:
    void set(const uint8_t* bytes, uint32_t len);
    uint32_t size() const override { return len_; }
    uint32_t crc() const override { return crc_; }
    uint32_t read(uint32_t offset, uint8_t* out, uint32_t n) const override;
private:
    const uint8_t* bytes_ = nullptr;
    uint32_t len_ = 0;
    uint32_t crc_ = 0;
};

class TransferSession {
public:
    enum State : uint8_t { IDLE, HEADER, BODY, DONE, ABORTED };

    /// Start a transfer of `source` with `payload` bytes per packet (MTU − 3).
    void begin(const ByteSource& source, uint16_t payload);
    void abort() { state_ = ABORTED; }
    bool active() const { return state_ == HEADER || state_ == BODY; }
    State state() const { return state_; }

    /// Fill `out` with the packet to send now (header first, then body). Returns its length,
    /// 0 when nothing is due (done, aborted, or waiting out a retry delay — see `retryAt`).
    uint16_t nextPacket(uint8_t* out, uint32_t nowMs);
    /// The transport's verdict on the packet last returned by nextPacket().
    void accepted();
    void refused(uint32_t nowMs);

    uint32_t bytesSent() const { return offset_; }
    uint16_t packetsSent() const { return packets_; }
    uint16_t retries() const { return retries_; }

private:
    const ByteSource* source_ = nullptr;
    State state_ = IDLE;
    uint16_t payload_ = PHYPHOX_BLE_DEFAULT_MTU;
    uint32_t offset_ = 0;        ///< next body byte to send
    uint16_t lastLen_ = 0;       ///< length of the packet awaiting a verdict
    uint16_t packets_ = 0;
    uint16_t retries_ = 0;       ///< consecutive refusals of the current packet
    uint32_t retryAt_ = 0;       ///< millis() before which nextPacket() stays quiet
    bool pending_ = false;       ///< a packet is out, verdict not yet in
};

} // namespace phyphox

#endif
