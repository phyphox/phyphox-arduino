// Shared test helpers: a string sink, a fake transport, and the example configurations.
#ifndef PHYPHOX_TEST_HELPERS_H
#define PHYPHOX_TEST_HELPERS_H

#include <string>
#include <vector>
#include <cstring>
#include "core/Sink.h"
#include "core/Server.h"
#include "transport/Transport.h"

struct StringSink : phyphox::Sink {
    using phyphox::Sink::write;
    std::string out;
    void write(const uint8_t* data, size_t len) override { out.append((const char*)data, len); }
};

/// A transport that records what the core asks of it and lets a test play the phone.
struct FakeTransport : phyphox::Transport {
    phyphox::GattLayout layout{};
    phyphox::TransportListener* listener = nullptr;
    std::vector<std::vector<uint8_t>> experimentPackets, dataPackets;
    bool isConnected = false;
    uint16_t mtu = 20;
    bool async = false;
    int refuseNext = 0;            ///< refuse this many notifications, then accept
    int refusedCount = 0;
    int advertisingRestarts = 0;
    bool began = false;
    uint16_t reqMin = 0, reqMax = 0, reqLatency = 0, reqTimeout = 0;

    bool begin(const phyphox::GattLayout& l, phyphox::TransportListener& li) override { layout = l; listener = &li; began = true; return true; }
    bool notify(phyphox::CharId id, const uint8_t* data, uint16_t len) override {
        if (refuseNext > 0) { refuseNext--; refusedCount++; if (!async) return false; }
        std::vector<uint8_t> p(data, data + len);
        if (id.kind == phyphox::CH_EXPERIMENT) experimentPackets.push_back(p); else dataPackets.push_back(p);
        return true;
    }
    bool asyncNotifyStatus() const override { return async; }
    void setValue(phyphox::CharId, const uint8_t*, uint16_t) override {}
    bool connected() const override { return isConnected; }
    uint16_t mtuPayload() const override { return mtu; }
    void requestConnectionParameters(uint16_t a, uint16_t b, uint16_t c, uint16_t d) override { reqMin = a; reqMax = b; reqLatency = c; reqTimeout = d; }
    void poll() override {}
    void restartAdvertising() override { advertisingRestarts++; }
    const char* name() const override { return "fake"; }

    // the phone's side
    void subscribeExperiment() { listener->onSubscribe(phyphox::CharId{phyphox::CH_EXPERIMENT, 0}, true); }
    void writeControl(uint8_t v) { listener->onWrite(phyphox::CharId{phyphox::CH_CONTROL, 0}, &v, 1); }
    void writeInput(uint8_t channel, float v) { uint8_t b[4]; memcpy(b, &v, 4); listener->onWrite(phyphox::CharId{phyphox::CH_INPUT, channel}, b, 4); }
    void writeSensor(uint8_t sensor, const std::vector<float>& v) {
        std::vector<uint8_t> b(v.size() * 4); memcpy(b.data(), v.data(), b.size());
        listener->onWrite(phyphox::CharId{phyphox::CH_SENSOR, sensor}, b.data(), (uint16_t)b.size());
    }
    std::string received() const {   // the document as the phone would reassemble it (after the header)
        std::string s;
        for (size_t i = 1; i < experimentPackets.size(); ++i) s.append(experimentPackets[i].begin(), experimentPackets[i].end());
        return s;
    }
};

#endif
