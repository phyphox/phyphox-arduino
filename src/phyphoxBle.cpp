// The static façade: one Server over the selected transport, and the 1.x static fields.
#include "phyphoxBle.h"

using phyphox::Sink; using phyphox::ExperimentEvent; using phyphox::ExperimentData; using phyphox::ErrorRecord;
using phyphox::ServerStats; using phyphox::ChangeCallback; using phyphox::PressCallback; using phyphox::errorText;

void (*PhyphoxBLE::configHandler)() = nullptr;
void (*PhyphoxBLE::experimentEventHandler)() = nullptr;
uint8_t PhyphoxBLE::eventType = 0;
int64_t PhyphoxBLE::experimentTime = 0;
int64_t PhyphoxBLE::systemTime = 0;
// Apple-compliant defaults: 15–30 ms, no latency, 5 s supervision timeout (BLE units).
uint16_t PhyphoxBLE::minConInterval = 12;
uint16_t PhyphoxBLE::maxConInterval = 24;
uint16_t PhyphoxBLE::slaveLatency = 0;
uint16_t PhyphoxBLE::timeout = 500;
uint16_t PhyphoxBLE::currentConnections = 0;
bool PhyphoxBLE::isSubscribed = false;

namespace {
Print* debugOut = nullptr;
uint32_t clockMs() { return (uint32_t)millis(); }
void onConfig() { if (PhyphoxBLE::configHandler) PhyphoxBLE::configHandler(); }
void onEvent() {
    const ExperimentEvent& e = PhyphoxBLE::server().lastEvent();
    PhyphoxBLE::eventType = (uint8_t)e.type;
    PhyphoxBLE::experimentTime = e.experimentTimeMs;
    PhyphoxBLE::systemTime = e.systemTimeMs;
    if (PhyphoxBLE::experimentEventHandler) PhyphoxBLE::experimentEventHandler();
}
struct PrintSink : Sink {
    Print* out;
    explicit PrintSink(Print* p) : out(p) {}
    using Sink::write;
    void write(const uint8_t* data, size_t len) override { if (out) out->write(data, len); }
};
}

phyphox::Server& PhyphoxBLE::server() {
    static phyphox::Server instance(PHYPHOX_BLE_TRANSPORT::instance());
    return instance;
}

static void configure(const char* name) {
    phyphox::Server& s = PhyphoxBLE::server();
    s.setDeviceName(name);
    s.setClock(clockMs);
    s.setConfigHandler(onConfig);
    s.setEventHandler(onEvent);
    s.setConnectionParameters(PhyphoxBLE::minConInterval, PhyphoxBLE::maxConInterval, PhyphoxBLE::slaveLatency, PhyphoxBLE::timeout);
}

void PhyphoxBLE::start(const char* deviceName) {
    configure(deviceName);
    server().start();
}
void PhyphoxBLE::start(const char* deviceName, PhyphoxBleExperiment& experiment) {
    configure(deviceName);
    server().addExperiment(experiment);
    server().start();
}
void PhyphoxBLE::start(const char* deviceName, uint8_t* xml, size_t len) {
    configure(deviceName);
    server().setCustomXml(xml, (uint32_t)len);
    server().start();
}
void PhyphoxBLE::addExperiment(PhyphoxBleExperiment& experiment) { server().addExperiment(experiment); }

void PhyphoxBLE::poll() {
    phyphox::Server& s = server();
    s.poll();
    currentConnections = s.connections();
    isSubscribed = s.subscribed();
}
void PhyphoxBLE::poll(int) { poll(); }

void PhyphoxBLE::write(const float& v1) { float v[1] = {v1}; server().writeFloats(v, 1); }
void PhyphoxBLE::write(const float& v1, const float& v2) { float v[2] = {v1, v2}; server().writeFloats(v, 2); }
void PhyphoxBLE::write(const float& v1, const float& v2, const float& v3) { float v[3] = {v1, v2, v3}; server().writeFloats(v, 3); }
void PhyphoxBLE::write(const float& v1, const float& v2, const float& v3, const float& v4) { float v[4] = {v1, v2, v3, v4}; server().writeFloats(v, 4); }
void PhyphoxBLE::write(const float& v1, const float& v2, const float& v3, const float& v4, const float& v5) { float v[5] = {v1, v2, v3, v4, v5}; server().writeFloats(v, 5); }
void PhyphoxBLE::write(uint8_t* bytes, unsigned int len) { server().writeBytes(bytes, (uint16_t)len); }
void PhyphoxBLE::write(float* values, unsigned int count) { server().writeBytes((const uint8_t*)values, (uint16_t)(count * 4)); }

void PhyphoxBLE::read(float& v1) { v1 = server().channels().value(1); }
void PhyphoxBLE::read(float& v1, float& v2) { read(v1); v2 = server().channels().value(2); }
void PhyphoxBLE::read(float& v1, float& v2, float& v3) { read(v1, v2); v3 = server().channels().value(3); }
void PhyphoxBLE::read(float& v1, float& v2, float& v3, float& v4) { read(v1, v2, v3); v4 = server().channels().value(4); }
void PhyphoxBLE::read(float& v1, float& v2, float& v3, float& v4, float& v5) { read(v1, v2, v3, v4); v5 = server().channels().value(5); }
void PhyphoxBLE::read(uint8_t* bytes, unsigned int len) {
    if (len > 20) len = 20;
    memcpy(bytes, server().legacyConfig(), len);
}
float PhyphoxBLE::readChannel(int channel) { return server().channels().value((uint8_t)channel); }
bool PhyphoxBLE::changed(int channel) { return server().channels().changed((uint8_t)channel); }
void PhyphoxBLE::onChange(int channel, ChangeCallback cb) { server().channels().setCallbacks((uint8_t)channel, cb, nullptr); }
void PhyphoxBLE::onPress(int channel, PressCallback cb) { server().channels().setCallbacks((uint8_t)channel, nullptr, cb); }
const ExperimentEvent& PhyphoxBLE::lastEvent() { return server().lastEvent(); }

void PhyphoxBLE::setMTU(uint16_t payload) { server().setMtu(payload); }

void PhyphoxBLE::printXML(Print* out) { PrintSink sink(out); server().printXml(sink); }
void PhyphoxBLE::printErrors(Print* out) {
    if (!out) return;
    const ExperimentData& d = server().store().data();
    int n = 0;
    auto show = [&](const ErrorRecord& e) {
        if (!e.set()) return;
        out->print("ERROR FOUND: "); out->print(errorText(e.code));
        if (e.origin) { out->print(", in "); out->print(e.origin); out->print("()"); }
        out->println(); n++;
    };
    show(d.error);
    for (uint8_t v = 0; v < d.viewCount; ++v)
        for (uint8_t i = 0; i < d.views[v].elementCount; ++i) show(d.views[v].elements[i].error);
    for (uint8_t i = 0; i < d.sensorCount; ++i) show(d.sensors[i].error);
    if (!n) out->println("phyphox BLE: no configuration errors");
}
const ServerStats& PhyphoxBLE::stats() { return server().stats(); }
const char* PhyphoxBLE::transportName() { return PHYPHOX_BLE_TRANSPORT::instance().name(); }
void PhyphoxBLE::begin(Print* out) { debugOut = out; }
