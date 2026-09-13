// The transport interface: everything the core needs from a BLE stack, and nothing more.
//
// One implementation per BLE *library* (not per chip or stack): ArduinoBLE (covers
// STM32duinoBLE), the ESP32 core BLE library (covers Bluedroid and NimBLE variants), and the
// NINA-B31 serial AT protocol. The core never includes a BLE header; a transport never
// generates XML or interprets channel values. Keeping this boundary thin is what stops the 1.x
// backend drift from coming back.
#ifndef PHYPHOX_BLE_TRANSPORT_H
#define PHYPHOX_BLE_TRANSPORT_H

#include <stdint.h>
#include <stddef.h>
#include "../core/Config.h"

namespace phyphox {

/// The characteristics a phyphox device exposes. Fixed UUIDs (contract, ../CLAUDE.md):
/// EXPERIMENT cddf0002, CONTROL cddf0003, EVENT cddf0004 in the experiment service cddf0001.
/// Library conventions (docs/protocol.md): DATA cddf1002 in the data service cddf1001;
/// INPUT k at cddf20kk, SENSOR s at cddf30ss; LEGACY_CONFIG cddf1003 only in user-XML mode.
enum CharKind : uint8_t { CH_EXPERIMENT, CH_CONTROL, CH_EVENT, CH_DATA, CH_INPUT, CH_SENSOR, CH_LEGACY_CONFIG };

struct CharId {
    CharKind kind;
    uint8_t index;               ///< channel or sensor number for CH_INPUT / CH_SENSOR, else 0
};

/// What the core asks the transport to create at begin().
struct GattLayout {
    const char* deviceName;
    uint8_t inputChannels;       ///< highest input channel (0 in user-XML mode)
    uint32_t inputChannelMask;   ///< bit k set: create characteristic cddf20kk (sensor-fed channels have none)
    uint8_t sensors;             ///< characteristics cddf3001 … cddf30ss
    const uint8_t* sensorValueSize; ///< bytes each sensor characteristic holds (`sensors` entries)
    bool legacyConfig;           ///< also create cddf1003 (20 bytes) — user-XML mode
    uint16_t dataValueSize;      ///< bytes of the data characteristic value (MTU payload)
};

/// Callbacks from the transport into the core (implemented by Server).
class TransportListener {
public:
    virtual ~TransportListener() {}
    virtual void onConnect() = 0;
    virtual void onDisconnect() = 0;
    /// The central enabled/disabled notifications on `id` (only CH_EXPERIMENT and CH_DATA
    /// are notifiable).
    virtual void onSubscribe(CharId id, bool enabled) = 0;
    /// The central wrote `len` bytes to `id`.
    virtual void onWrite(CharId id, const uint8_t* data, uint16_t len) = 0;
    /// Asynchronous verdict on the last notify() for stacks that report it later. Transports
    /// whose notify() is synchronous never call this.
    virtual void onNotifyStatus(CharId id, bool accepted) = 0;
    /// Drive the transfer: send what is due. Called by the core from poll(), or by a transport
    /// that runs the transfer in its own task (ESP32, see drivesTransfer()). Never called from
    /// inside a stack callback.
    virtual void pump() = 0;
};

class Transport {
public:
    virtual ~Transport() {}
    /// Create services and characteristics, start advertising the experiment service.
    virtual bool begin(const GattLayout& layout, TransportListener& listener) = 0;
    /// Send a notification. Returns false if the stack refused it right now (buffers full):
    /// the core will retry. A transport that blocks until accepted (ArduinoBLE) returns true.
    /// A transport that only learns the verdict later (ESP32) returns true and reports via
    /// onNotifyStatus().
    virtual bool notify(CharId id, const uint8_t* data, uint16_t len) = 0;
    /// True if the verdict on notify() arrives later through onNotifyStatus() (ESP32);
    /// false if notify()'s return value is the verdict (ArduinoBLE, NINA-B31).
    virtual bool asyncNotifyStatus() const { return false; }
    /// Set a characteristic's value without notifying (initial values, the legacy config).
    virtual void setValue(CharId id, const uint8_t* data, uint16_t len) = 0;
    virtual bool connected() const = 0;
    /// The negotiated MTU payload (ATT_MTU − 3), or PHYPHOX_BLE_DEFAULT_MTU.
    virtual uint16_t mtuPayload() const = 0;
    /// Ask the stack for an MTU before connecting (ESP32 only today; others ignore).
    virtual void requestMtu(uint16_t payload) { (void)payload; }
    /// Ask the central for connection parameters (units of 1.25 ms / 10 ms). Transports that
    /// cannot ignore it. The core only passes Apple-compliant values (min >= 15 ms,
    /// max >= min + 15 ms).
    virtual void requestConnectionParameters(uint16_t minInterval, uint16_t maxInterval,
                                             uint16_t latency, uint16_t timeout) {
        (void)minInterval; (void)maxInterval; (void)latency; (void)timeout;
    }
    /// Give the stack CPU time. Called from PhyphoxBLE::poll(); a no-op where the stack runs
    /// on its own (ESP32).
    virtual void poll() = 0;
    /// True if the transport calls listener.pump() from a task of its own, so the core must not
    /// also pump from poll() (ESP32: the sketch's loop() may be slow, and a transfer must not
    /// run inside a stack callback).
    virtual bool drivesTransfer() const { return false; }
    virtual void restartAdvertising() = 0;
    virtual const char* name() const = 0;   ///< "ArduinoBLE", "ESP32", "NINA-B31" — for printXML/debug
};

} // namespace phyphox

#endif
