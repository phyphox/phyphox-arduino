// Byte sinks the serializer writes into. The serializer never allocates and never assembles
// the document: it emits bytes into one of these.
#ifndef PHYPHOX_BLE_CORE_SINK_H
#define PHYPHOX_BLE_CORE_SINK_H

#include <stdint.h>
#include <stddef.h>
#include "Crc32.h"

namespace phyphox {

/// Receives the bytes of a serialization. Implementations: counting/CRC (start-up pass),
/// range window (transfer packets), Arduino Print (printXML), FILE (host tests).
class Sink {
public:
    virtual ~Sink() {}
    virtual void write(const uint8_t* data, size_t len) = 0;
    void write(const char* text);
    void write(char c) { uint8_t b = (uint8_t)c; write(&b, 1); }
};

/// Counts bytes and accumulates the CRC-32 — the first pass at start(), which yields the
/// transfer header (size, CRC) and the part table.
class CountingSink : public Sink {
public:
    using Sink::write;
    void write(const uint8_t* data, size_t len) override { count_ += len; crc_.update(data, len); }
    size_t count() const { return count_; }
    uint32_t crc() const { return crc_.value(); }
    void reset() { count_ = 0; crc_.reset(); }
private:
    size_t count_ = 0;
    Crc32 crc_;
};

/// Passes through only the bytes inside [skip, skip + capacity) of what is written to it, into
/// a caller-provided buffer. This is how one packet of the transfer is produced from a part of
/// the document without ever holding the part.
class WindowSink : public Sink {
public:
    WindowSink(uint8_t* out, size_t capacity, size_t skip) : out_(out), cap_(capacity), skip_(skip) {}
    using Sink::write;
    void write(const uint8_t* data, size_t len) override;
    size_t produced() const { return filled_; }
    bool full() const { return filled_ >= cap_; }
private:
    uint8_t* out_;
    size_t cap_;
    size_t skip_;
    size_t seen_ = 0;
    size_t filled_ = 0;
};

} // namespace phyphox

#endif
