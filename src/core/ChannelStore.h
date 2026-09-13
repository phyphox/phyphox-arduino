// The board-side view of everything the phone can send: one slot per input channel with the
// last value received, plus the callbacks to fire. Written by the Server when a characteristic
// is written, read by PhyphoxBLE::read() and by the sketch's callbacks.
#ifndef PHYPHOX_BLE_CORE_CHANNELSTORE_H
#define PHYPHOX_BLE_CORE_CHANNELSTORE_H

#include <stdint.h>
#include "Config.h"
#include "ElementData.h"

namespace phyphox {

class ChannelStore {
public:
    void clear();
    /// Pre-fill a channel with the element's default so read() returns it before any change.
    void setDefault(uint8_t channel, float value);
    void setCallbacks(uint8_t channel, ChangeCallback onChange, PressCallback onPress);
    /// A value arrived from the phone. Stores it, bumps the counter, fires the callbacks.
    void deliver(uint8_t channel, float value);
    float value(uint8_t channel) const;
    /// How many deliveries the channel has seen since start (lets a sketch poll for changes).
    uint32_t updates(uint8_t channel) const;
    /// True once per delivery: returns whether a value arrived since the last call.
    bool changed(uint8_t channel);

private:
    struct Slot {
        float value = 0;
        uint32_t updates = 0;
        uint32_t seen = 0;
        ChangeCallback onChange = nullptr;
        PressCallback onPress = nullptr;
    };
    Slot slots_[PHYPHOX_BLE_INPUT_CHANNELS + 1];   ///< 1-based
};

/// The 17-byte block phyphox writes to the event characteristic (bluetooth-low-energy.md,
/// "Phyphox event characteristic").
enum EventType : uint8_t { EVENT_PAUSE = 0x00, EVENT_START = 0x01, EVENT_CLEAR = 0x02, EVENT_SYNC = 0xff };
struct ExperimentEvent {
    EventType type = EVENT_PAUSE;
    int64_t experimentTimeMs = 0;   ///< −1 for SYNC
    int64_t systemTimeMs = 0;       ///< Unix time in ms
    /// Decode 17 big-endian bytes; returns false if len < 17.
    bool decode(const uint8_t* bytes, uint16_t len);
};

} // namespace phyphox

#endif
