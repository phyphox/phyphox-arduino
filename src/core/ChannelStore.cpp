#include "ChannelStore.h"
#include <string.h>

namespace phyphox {

size_t ChannelStore::bytesNeeded(uint8_t channels) { return sizeof(Slot) * (channels + 1); }

void ChannelStore::attach(uint8_t channels, void* storage) {
    channels_ = channels;
    slots_ = (Slot*)storage;
    clear();
}

void ChannelStore::clear() {
    if (!slots_) return;
    for (uint8_t i = 0; i <= channels_; ++i) slots_[i] = Slot();
}

void ChannelStore::setDefault(uint8_t channel, float value) {
    if (!slots_ || channel == 0 || channel > channels_) return;
    slots_[channel].value = value;
}

void ChannelStore::setCallbacks(uint8_t channel, ChangeCallback onChange, PressCallback onPress) {
    if (!slots_ || channel == 0 || channel > channels_) return;
    if (onChange) slots_[channel].onChange = onChange;
    if (onPress) slots_[channel].onPress = onPress;
}

void ChannelStore::deliver(uint8_t channel, float value) {
    if (!slots_ || channel == 0 || channel > channels_) return;
    Slot& s = slots_[channel];
    s.value = value;
    s.updates++;
    if (s.onChange) s.onChange(value);
    if (s.onPress) s.onPress();
}

float ChannelStore::value(uint8_t channel) const {
    if (!slots_ || channel == 0 || channel > channels_) return 0;
    return slots_[channel].value;
}

uint32_t ChannelStore::updates(uint8_t channel) const {
    if (!slots_ || channel == 0 || channel > channels_) return 0;
    return slots_[channel].updates;
}

bool ChannelStore::changed(uint8_t channel) {
    if (!slots_ || channel == 0 || channel > channels_) return false;
    Slot& s = slots_[channel];
    bool c = s.updates != s.seen;
    s.seen = s.updates;
    return c;
}

static int64_t bigEndian64(const uint8_t* p) {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v = (v << 8) | p[i];
    return (int64_t)v;
}

bool ExperimentEvent::decode(const uint8_t* bytes, uint16_t len) {
    if (!bytes || len < 17) return false;
    type = (EventType)bytes[0];
    experimentTimeMs = bigEndian64(bytes + 1);
    systemTimeMs = bigEndian64(bytes + 9);
    return true;
}

} // namespace phyphox
