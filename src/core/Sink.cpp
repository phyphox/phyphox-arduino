#include "Sink.h"
#include <string.h>

namespace phyphox {

void Sink::write(const char* text) {
    if (text) write((const uint8_t*)text, strlen(text));
}

void WindowSink::write(const uint8_t* data, size_t len) {
    // Bytes [skip_, skip_ + cap_) of the stream go to out_; everything else is dropped.
    size_t start = seen_;
    seen_ += len;
    if (filled_ >= cap_) return;
    size_t from = 0;
    if (start < skip_) {
        if (start + len <= skip_) return;
        from = skip_ - start;
    }
    size_t n = len - from;
    size_t room = cap_ - filled_;
    if (n > room) n = room;
    memcpy(out_ + filled_, data + from, n);
    filled_ += n;
}

} // namespace phyphox
