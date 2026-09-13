#include "Server.h"
#include "../phyphoxBleExperiment.h"
#include <stdlib.h>
#include <string.h>

namespace phyphox {

Server::~Server() {
    if (packet_) free(packet_);
    if (sensorSizes_) free(sensorSizes_);
}

void Server::setMtu(uint16_t payload) {
    if (payload < PHYPHOX_BLE_DEFAULT_MTU) payload = PHYPHOX_BLE_DEFAULT_MTU;
    if (payload > PHYPHOX_BLE_MAX_MTU) payload = PHYPHOX_BLE_MAX_MTU;
    mtu_ = payload;
}

void Server::setConnectionParameters(uint16_t minInterval, uint16_t maxInterval, uint16_t latency, uint16_t timeout) {
    connMin_ = minInterval; connMax_ = maxInterval; connLatency_ = latency; connTimeout_ = timeout;
}

void Server::addExperiment(const PhyphoxBleExperiment& exp) {
    customXml_ = false;
    store_.copyFrom(exp);
    rebuild();
    // After the transport is up the characteristic layout cannot change any more: the new
    // document is served, but input channels or sensors it adds have no characteristic.
    if (started_) {
        const ExperimentData& d = store_.data();
        bool fits = d.sensorCount <= layoutSensors_;
        for (uint8_t k = 1; k <= store_.inputChannelsUsed() && fits; ++k)
            if (store_.input(k).used && !store_.input(k).fromSensor && !(layoutMask_ & (1u << k))) fits = false;
        if (!fits) store_.data().error.record(ERR_02_ABOVE_LIMIT, "addExperiment after the first poll()/write()");
    }
}

void Server::printXml(Sink& sink) {
    if (customXml_) {
        uint8_t buf[64];
        for (uint32_t off = 0; off < customSource_.size(); ) {
            uint32_t n = customSource_.read(off, buf, sizeof(buf));
            if (!n) break;
            sink.write(buf, n); off += n;
        }
        return;
    }
    ensureDocument();
    serializer_.writeAll(sink);
}

void Server::ensureDocument() {
    if (!customXml_ && store_.data().viewCount == 0) { store_.buildDefault(); rebuild(); }
}

void Server::setCustomXml(const uint8_t* xml, uint32_t len) {
    customXml_ = true;
    customSource_.set(xml, len);
    channels_.attach(PHYPHOX_BLE_DATA_CHANNELS, legacySlots_);
}

/// After the store changed: part table, channel store, defaults and callbacks.
void Server::rebuild() {
    serializer_.build(store_, deviceName_, mtu_);
    channels_.attach(store_.inputChannelsUsed(), store_.channelStorage());
    const ExperimentData& d = store_.data();
    for (uint8_t k = 1; k <= store_.inputChannelsUsed(); ++k)
        if (store_.input(k).hasDefault) channels_.setDefault(k, store_.input(k).defaultValue);
    for (uint8_t v = 0; v < d.viewCount; ++v)
        for (uint8_t i = 0; i < d.views[v].elementCount; ++i) {
            const ElementData& e = d.views[v].elements[i];
            const InputCommon* in = e.input();
            if (!in || !in->channel) continue;
            channels_.setCallbacks(in->channel, in->onChange, e.type == EL_BUTTON ? e.button.onPress : nullptr);
        }
    for (uint8_t k = 1; k <= PHYPHOX_BLE_MAX_INPUT_CHANNEL; ++k)
        if (pendingChange_[k] || pendingPress_[k]) channels_.setCallbacks(k, pendingChange_[k], pendingPress_[k]);
}

void Server::setChannelCallbacks(uint8_t channel, ChangeCallback onChange, PressCallback onPress) {
    if (channel == 0 || channel > PHYPHOX_BLE_MAX_INPUT_CHANNEL) return;
    if (onChange) pendingChange_[channel] = onChange;
    if (onPress) pendingPress_[channel] = onPress;
    channels_.setCallbacks(channel, onChange, onPress);   // a no-op until the store exists
}

bool Server::start() {
    startRequested_ = true;
    return true;
}

bool Server::ensureStarted() {
    if (started_ || !startRequested_) return started_;
    ensureDocument();
    if (!ensurePacket(mtu_)) return false;

    GattLayout layout;
    layout.deviceName = deviceName_ ? deviceName_ : "phyphox-Arduino";
    layout.inputChannels = customXml_ ? 0 : store_.inputChannelsUsed();
    layout.inputChannelMask = 0;
    for (uint8_t k = 1; k <= layout.inputChannels; ++k)
        if (store_.input(k).used && !store_.input(k).fromSensor) layout.inputChannelMask |= (1u << k);
    const ExperimentData& d = store_.data();
    layout.sensors = customXml_ ? 0 : d.sensorCount;
    if (sensorSizes_) free(sensorSizes_);
    sensorSizes_ = nullptr;
    if (layout.sensors) {
        sensorSizes_ = (uint8_t*)malloc(layout.sensors);
        for (uint8_t i = 0; i < layout.sensors; ++i) sensorSizes_[i] = (uint8_t)(d.sensors[i].componentCount * 4);
    }
    layout.sensorValueSize = sensorSizes_;
    layout.legacyConfig = customXml_;
    layout.dataValueSize = mtu_;
    layoutMask_ = layout.inputChannelMask; layoutSensors_ = layout.sensors;

    if (mtu_ > PHYPHOX_BLE_DEFAULT_MTU) transport_.requestMtu(mtu_);
    if (!transport_.begin(layout, *this)) return false;
    if (connMin_ || connMax_) transport_.requestConnectionParameters(connMin_, connMax_, connLatency_, connTimeout_);
    started_ = true;
    return true;
}

void Server::poll() {
    if (!ensureStarted()) return;
    transport_.poll();
    if (!transport_.drivesTransfer()) pumpTransfer();
}

// ---------------------------------------------------------------- data to the phone

/// A write also services the stack and the transfer (what poll() does), so a sketch that
/// only writes — the 1.x ESP32 examples had no poll() — still answers the phone on the
/// transports that need polling. Callbacks may therefore run from inside write() as well.
void Server::serviceFromWrite() {
    transport_.poll();
    if (!transport_.drivesTransfer()) pumpTransfer();
}

bool Server::writeFloats(const float* values, uint8_t count) {
    if (!ensureStarted()) return false;
    serviceFromWrite();
    uint8_t buf[PHYPHOX_BLE_DATA_CHANNELS * 4] = {0};
    if (count > PHYPHOX_BLE_DATA_CHANNELS) count = PHYPHOX_BLE_DATA_CHANNELS;
    memcpy(buf, values, count * 4);      // float32 little-endian on every supported board
    stats_.dataNotifications++;
    bool ok = transport_.notify(CharId{CH_DATA, 0}, buf, sizeof(buf));
    if (!ok) stats_.dataRefused++;
    return ok;
}

bool Server::writeBytes(const uint8_t* bytes, uint16_t len) {
    if (!ensureStarted()) return false;
    serviceFromWrite();
    if (len > mtu_) len = mtu_;
    stats_.dataNotifications++;
    bool ok = transport_.notify(CharId{CH_DATA, 0}, bytes, len);
    if (!ok) stats_.dataRefused++;
    return ok;
}

// ---------------------------------------------------------------- the transfer

bool Server::ensurePacket(uint16_t payload) {
    if (payload < PHYPHOX_BLE_DEFAULT_MTU) payload = PHYPHOX_BLE_DEFAULT_MTU;
    if (payload > PHYPHOX_BLE_MAX_MTU) payload = PHYPHOX_BLE_MAX_MTU;
    if (packet_ && packetCap_ >= payload) return true;
    uint8_t* p = (uint8_t*)realloc(packet_, payload);
    if (!p) return false;
    packet_ = p; packetCap_ = payload;
    return true;
}

void Server::startTransfer() {
    if (transfer_.active()) return;          // a second trigger during a transfer is ignored
    uint16_t payload = transport_.mtuPayload();
    if (!ensurePacket(payload)) return;
    if (payload > packetCap_) payload = packetCap_;
    transfer_.begin(customXml_ ? (const ByteSource&)customSource_ : (const ByteSource&)serializerSource_, payload);
    transfer_.noteProgress(now());
    if (clock_) transfer_.holdUntil(now() + PHYPHOX_BLE_TRANSFER_START_DELAY_MS);
    stats_.transfers++;
    // no packet is sent here: the trigger arrives inside a stack callback; pump() sends
}

void Server::pumpTransfer() {
    if (!packet_) return;
    // watchdog: a session that has not moved for PHYPHOX_BLE_TRANSFER_STALL_MS is abandoned,
    // so whatever wedged it cannot also swallow every later trigger
    if (transfer_.active() && clock_ && (int32_t)(now() - transfer_.lastProgressMs()) > (int32_t)PHYPHOX_BLE_TRANSFER_STALL_MS) {
        transfer_.abort();
    }
    for (int i = 0; i < PHYPHOX_BLE_TRANSFER_BURST && transfer_.active(); ++i) {
        uint16_t len = transfer_.nextPacket(packet_, now());
        if (!len) break;
        bool ok = transport_.notify(CharId{CH_EXPERIMENT, 0}, packet_, len);
        if (transport_.asyncNotifyStatus()) {
            if (!ok) { stats_.packetsRefused++; transfer_.refused(now()); }
            break;                           // the verdict arrives through onNotifyStatus()
        }
        if (ok) { transfer_.accepted(); transfer_.noteProgress(now()); }
        else { stats_.packetsRefused++; transfer_.refused(now()); }
    }
    // bookkeeping once per finished transfer (the session then goes idle)
    if (transfer_.state() == TransferSession::DONE) { stats_.transfersCompleted++; transport_.restartAdvertising(); transfer_.reset(); }
    else if (transfer_.state() == TransferSession::ABORTED) { stats_.transfersAborted++; stats_.transfersStalled++; transfer_.reset(); }
}

// ---------------------------------------------------------------- transport events

void Server::onConnect() {}

void Server::onDisconnect() {
    if (transfer_.active()) { transfer_.reset(); stats_.transfersAborted++; }
    dataSubscribed_ = false;
    transport_.restartAdvertising();
}

void Server::onSubscribe(CharId id, bool enabled) {
    if (id.kind == CH_EXPERIMENT && enabled) startTransfer();
    if (id.kind == CH_DATA) dataSubscribed_ = enabled;
}

void Server::onNotifyStatus(CharId id, bool accepted) {
    if (id.kind != CH_EXPERIMENT) return;
    if (accepted) { transfer_.accepted(); transfer_.noteProgress(now()); }
    else { stats_.packetsRefused++; transfer_.refused(now()); }
    pumpTransfer();
}

static float leFloat(const uint8_t* p) { float f; memcpy(&f, p, 4); return f; }

void Server::onWrite(CharId id, const uint8_t* data, uint16_t len) {
    switch (id.kind) {
        case CH_CONTROL:
            if (len >= 1 && data[0] == 1) startTransfer();
            else if (len >= 1 && data[0] == 0 && transfer_.active()) { transfer_.reset(); stats_.transfersAborted++; }
            break;
        case CH_EVENT:
            if (lastEvent_.decode(data, len)) { stats_.events++; if (eventHandler_) eventHandler_(); }
            break;
        case CH_INPUT: deliverInput(id.index, data, len); break;
        case CH_SENSOR: deliverSensor(id.index, data, len); break;
        case CH_LEGACY_CONFIG: deliverLegacyConfig(data, len); break;
        default: break;
    }
}

void Server::deliverInput(uint8_t channel, const uint8_t* data, uint16_t len) {
    if (len < 4) return;
    stats_.inputWrites++;
    channels_.deliver(channel, leFloat(data));
    if (configHandler_) configHandler_();
}

void Server::deliverSensor(uint8_t sensor, const uint8_t* data, uint16_t len) {
    const ExperimentData& d = store_.data();
    if (sensor == 0 || sensor > d.sensorCount) return;
    const SensorData& sd = d.sensors[sensor - 1];
    stats_.inputWrites++;
    for (uint8_t i = 0; i < sd.componentCount && (uint16_t)(i * 4 + 4) <= len; ++i)
        channels_.deliver(sd.channels[i], leFloat(data + i * 4));
    if (configHandler_) configHandler_();
}

void Server::deliverLegacyConfig(const uint8_t* data, uint16_t len) {
    if (len > 20) len = 20;
    memset(legacyConfig_, 0, sizeof(legacyConfig_));
    memcpy(legacyConfig_, data, len);
    stats_.inputWrites++;
    for (uint8_t k = 1; k <= PHYPHOX_BLE_DATA_CHANNELS && (uint16_t)(k * 4) <= len; ++k)
        channels_.deliver(k, leFloat(data + (k - 1) * 4));
    if (configHandler_) configHandler_();
}

} // namespace phyphox
