// The text side of the u-connectXpress AT protocol the NINA-B31 transport speaks: line
// classification, the events the module raises and the responses it gives. Plain C++ with no
// Arduino dependency so the host tests can exercise every parse (tests/host/test_nina_protocol.cpp).
// Formats from the u-connectXpress AT commands manual (UBX-14044127):
//   +UUBTACLC:<conn>,<type>,<addr>      ACL connected        +UUBTACLD:<conn>       disconnected
//   +UUBTGRW:<conn>,<handle>,<hex>,<opt>  attribute written  +UUBTGRR:<conn>,<handle>  read request
//   +UBTGSER:<handle>                   service created      +UBTGCHA:<value>,<cccd>  characteristic
#ifndef PHYPHOX_BLE_TRANSPORT_NINAB31_PROTOCOL_H
#define PHYPHOX_BLE_TRANSPORT_NINAB31_PROTOCOL_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

namespace phyphox {
namespace nina {

/// Longest line kept: a 20-byte write is ~62 characters, a six-component sensor write ~70.
/// (Not LINE_MAX: that is a POSIX macro in the ESP32 toolchain's <limits.h>.)
static const uint16_t kLineMax = 128;

inline int hexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/// Decode hex digits up to the first non-hex character; returns the byte count.
inline uint16_t decodeHex(const char* s, uint8_t* out, uint16_t cap) {
    uint16_t n = 0;
    while (n < cap) {
        int hi = hexVal(s[0]); if (hi < 0) break;
        int lo = hexVal(s[1]); if (lo < 0) break;
        out[n++] = (uint8_t)((hi << 4) | lo);
        s += 2;
    }
    return n;
}

inline void encodeHex(const uint8_t* data, uint16_t len, char* out) {
    static const char* h = "0123456789ABCDEF";
    for (uint16_t i = 0; i < len; ++i) { *out++ = h[data[i] >> 4]; *out++ = h[data[i] & 15]; }
    *out = 0;
}

/// Unsolicited result codes all start with "+UU"; everything else belongs to the command in flight.
inline bool isEvent(const char* line) { return strncmp(line, "+UU", 3) == 0; }
inline bool startsWith(const char* line, const char* prefix) { return strncmp(line, prefix, strlen(prefix)) == 0; }

/// The integer after ':' (a handle), or -1 if the line has none.
inline int firstNumber(const char* line) {
    const char* colon = strchr(line, ':');
    if (!colon) return -1;
    return atoi(colon + 1);
}

/// "+UBTGCHA:<value_handle>,<cccd_handle>"; cccd is 0 when the characteristic has none.
inline bool parseCharacteristicResponse(const char* line, int& valueHandle, int& cccdHandle) {
    if (!startsWith(line, "+UBTGCHA:")) return false;
    const char* p = line + 9;
    valueHandle = atoi(p);
    const char* comma = strchr(p, ',');
    cccdHandle = comma ? atoi(comma + 1) : 0;
    return true;
}

enum EventKind : uint8_t { EV_NONE, EV_CONNECTED, EV_DISCONNECTED, EV_WRITE, EV_READ, EV_OTHER };

struct Event {
    EventKind kind = EV_NONE;
    int conn = 0;
    int handle = 0;
    uint8_t data[64];
    uint16_t len = 0;
};

/// Classify and parse one unsolicited line.
inline Event parseEvent(const char* line) {
    Event e;
    if (startsWith(line, "+UUBTACLC:")) { e.kind = EV_CONNECTED; e.conn = atoi(line + 10); return e; }
    if (startsWith(line, "+UUBTACLD:")) { e.kind = EV_DISCONNECTED; e.conn = atoi(line + 10); return e; }
    if (startsWith(line, "+UUBTGRR:")) {
        const char* p = line + 9; e.conn = atoi(p);
        const char* c1 = strchr(p, ','); if (!c1) return e;
        e.handle = atoi(c1 + 1); e.kind = EV_READ; return e;
    }
    if (startsWith(line, "+UUBTGRW:")) {
        const char* p = line + 9; e.conn = atoi(p);
        const char* c1 = strchr(p, ','); if (!c1) return e;
        e.handle = atoi(c1 + 1);
        const char* c2 = strchr(c1 + 1, ','); if (!c2) return e;
        e.len = decodeHex(c2 + 1, e.data, sizeof(e.data));
        e.kind = EV_WRITE; return e;
    }
    e.kind = EV_OTHER;
    return e;
}

/// Assembles lines from the byte stream. Terminators are CR and LF (u-connectXpress ends every
/// line with both); empty lines are skipped; an over-long line is dropped whole.
class LineReader {
public:
    /// Feed one byte; returns true when `line()` holds a complete line.
    bool feed(char c) {
        if (c == '\r' || c == '\n') {
            bool complete = len_ > 0 && !overflow_;
            buf_[len_] = 0;
            len_ = 0; overflow_ = false;
            return complete;
        }
        if (len_ < kLineMax - 1) buf_[len_++] = c; else overflow_ = true;
        return false;
    }
    const char* line() const { return buf_; }
private:
    char buf_[kLineMax] = {0};
    uint16_t len_ = 0;
    bool overflow_ = false;
};

/// Fixed ring of event lines, filled while a command waits for its response and drained from
/// poll(). No heap; the newest line is dropped when the ring is full.
template <uint8_t N>
class EventQueue {
public:
    bool push(const char* line) {
        if (count_ >= N) { dropped_++; return false; }
        strncpy(slots_[(head_ + count_) % N], line, kLineMax - 1);
        slots_[(head_ + count_) % N][kLineMax - 1] = 0;
        count_++;
        return true;
    }
    bool pop(char* out) {
        if (!count_) return false;
        strcpy(out, slots_[head_]);
        head_ = (head_ + 1) % N; count_--;
        return true;
    }
    uint8_t size() const { return count_; }
    uint32_t dropped() const { return dropped_; }
private:
    char slots_[N][kLineMax];
    uint8_t head_ = 0, count_ = 0;
    uint32_t dropped_ = 0;
};

} // namespace nina
} // namespace phyphox

#endif
