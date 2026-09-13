// The core orchestrator behind the PhyphoxBLE façade: owns the store, the serializer, the
// transfer session and the channel store; implements TransportListener; knows nothing about
// any BLE library. One instance (the radio is one).
#ifndef PHYPHOX_BLE_CORE_SERVER_H
#define PHYPHOX_BLE_CORE_SERVER_H

#include "ExperimentStore.h"
#include "Serializer.h"
#include "Transfer.h"
#include "ChannelStore.h"
#include "../transport/Transport.h"

class PhyphoxBleExperiment;

namespace phyphox {

/// Statistics a bench or a curious sketch can read (PhyphoxBLE::stats()).
struct ServerStats {
    uint32_t transfers = 0, transfersCompleted = 0, transfersAborted = 0;
    uint32_t packetsRefused = 0, dataNotifications = 0, dataRefused = 0;
    uint32_t inputWrites = 0, events = 0;
};

class Server : public TransportListener {
public:
    /// The serializer's byte view, so the transfer can read it.
    class SerializerSource : public ByteSource {
    public:
        explicit SerializerSource(const Serializer& s) : s_(s) {}
        uint32_t size() const override { return s_.size(); }
        uint32_t crc() const override { return s_.crc(); }
        uint32_t read(uint32_t o, uint8_t* out, uint32_t n) const override { return s_.read(o, out, n); }
    private:
        const Serializer& s_;
    };

    Server(Transport& transport) : transport_(transport), serializerSource_(serializer_) {}
    ~Server();
    /// Millisecond clock for the transfer's retry timing (the façade passes millis).
    void setClock(uint32_t (*nowMs)()) { clock_ = nowMs; }

    // ---- configuration (before start)
    void setDeviceName(const char* name) { deviceName_ = name; }
    void setMtu(uint16_t payload);
    void setConnectionParameters(uint16_t minInterval, uint16_t maxInterval, uint16_t latency, uint16_t timeout);
    /// Serve the description (copies it; builds the part table). May be called after start():
    /// the new document is served from the next transfer on.
    void addExperiment(const PhyphoxBleExperiment& exp);
    /// Serve user-supplied XML instead; the 1.x GATT layout (cddf1002 data, cddf1003 config
    /// with five floats, events) is created so hand-written documents keep working.
    void setCustomXml(const uint8_t* xml, uint32_t len);

    // ---- lifecycle
    /// Request the start. The transport is brought up LAZILY, at the first poll() or write():
    /// the 1.x examples call start() before addExperiment(), and the characteristic layout
    /// depends on the experiment, so the stack must not be configured before setup() is done.
    bool start();
    /// Bring the transport up now if start() was requested and it is not up yet.
    bool ensureStarted();
    void poll();                    ///< drives the transfer; calls transport_.poll()
    bool started() const { return started_; }

    // ---- data to the phone
    bool writeFloats(const float* values, uint8_t count);      ///< 1…5 floats → data characteristic
    bool writeBytes(const uint8_t* bytes, uint16_t len);       ///< raw array (≤ MTU payload)

    // ---- data from the phone
    ChannelStore& channels() { return channels_; }
    /// Attach callbacks to a channel at any time, before or after the experiment was added.
    void setChannelCallbacks(uint8_t channel, ChangeCallback onChange, PressCallback onPress);
    const ExperimentEvent& lastEvent() const { return lastEvent_; }
    void setConfigHandler(void (*h)()) { configHandler_ = h; }
    void setEventHandler(void (*h)()) { eventHandler_ = h; }
    /// User-XML mode: the raw last value of the 1.x config characteristic.
    const uint8_t* legacyConfig() const { return legacyConfig_; }
    const TransferSession& transfer() const { return transfer_; }
    bool customXml() const { return customXml_; }
    uint16_t mtu() const { return mtu_; }

    // ---- introspection
    void printXml(Sink& sink) { ensureDocument(); serializer_.writeAll(sink); }
    void ensureDocument();
    const ExperimentStore& store() const { return store_; }
    const ServerStats& stats() const { return stats_; }
    uint16_t connections() const { return transport_.connected() ? 1 : 0; }
    bool subscribed() const { return dataSubscribed_; }

    // ---- TransportListener
    void onConnect() override;
    void onDisconnect() override;
    void onSubscribe(CharId id, bool enabled) override;
    void onWrite(CharId id, const uint8_t* data, uint16_t len) override;
    void onNotifyStatus(CharId id, bool accepted) override;
    void pump() override { pumpTransfer(); }

private:
    Transport& transport_;
    ExperimentStore store_;
    Serializer serializer_;
    SerializerSource serializerSource_;
    BufferSource customSource_;
    bool customXml_ = false;
    TransferSession transfer_;
    ChannelStore channels_;
    ExperimentEvent lastEvent_;
    ServerStats stats_;
    const char* deviceName_ = nullptr;
    uint16_t mtu_ = PHYPHOX_BLE_DEFAULT_MTU;
    uint16_t connMin_ = 0, connMax_ = 0, connLatency_ = 0, connTimeout_ = 0;
    bool dataSubscribed_ = false;
    void (*configHandler_)() = nullptr;
    void (*eventHandler_)() = nullptr;
    uint8_t* packet_ = nullptr;   ///< the one transmit buffer, sized to the largest MTU payload seen
    uint16_t packetCap_ = 0;
    uint8_t* sensorSizes_ = nullptr;
    uint8_t legacyConfig_[20] = {0};
    uint8_t legacySlots_[6 * 32] = {0};   ///< ChannelStore storage in user-XML mode (5 channels)
    uint32_t (*clock_)() = nullptr;
    bool started_ = false;
    bool startRequested_ = false;
    uint32_t layoutMask_ = 0;      ///< the input channels the transport was given at begin()
    uint8_t layoutSensors_ = 0;
    /// Callbacks registered through the façade before the channel store exists; applied by rebuild().
    ChangeCallback pendingChange_[PHYPHOX_BLE_MAX_INPUT_CHANNEL + 1] = {nullptr};
    PressCallback pendingPress_[PHYPHOX_BLE_MAX_INPUT_CHANNEL + 1] = {nullptr};
    uint32_t now() const { return clock_ ? clock_() : 0; }

    void rebuild();
    bool ensurePacket(uint16_t payload);
    void startTransfer();
    void pumpTransfer();
    void deliverInput(uint8_t channel, const uint8_t* data, uint16_t len);
    void deliverSensor(uint8_t sensor, const uint8_t* data, uint16_t len);
    void deliverLegacyConfig(const uint8_t* data, uint16_t len);
};

} // namespace phyphox

#endif
