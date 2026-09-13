#include "doctest.h"
#include "helpers.h"
#include "examples_config.h"

using namespace phyphox;

static uint32_t fakeNow = 0;
static uint32_t clockFn() { return fakeNow; }
static int handlerCalls = 0; static void handler() { handlerCalls++; }
static float seen = 0; static void onCh1(float v) { seen = v; }
static int pressed = 0; static void onBtn() { pressed++; }

TEST_CASE("Both triggers start the transfer, a second one is ignored, disconnect aborts") {
    FakeTransport t; Server s(t); s.setDeviceName("d"); s.setClock(clockFn); s.start();
    CHECK(t.began); CHECK(t.layout.inputChannels == 0); CHECK(!t.layout.legacyConfig);
    t.subscribeExperiment();
    CHECK(s.stats().transfers == 1);
    t.writeControl(1);                                   // during the transfer: ignored
    CHECK(s.stats().transfers == 1);
    for (int i = 0; i < 100 && s.transfer().active(); ++i) s.poll();
    CHECK(s.stats().transfersCompleted == 1);
    CHECK(t.advertisingRestarts == 1);
    std::string doc = t.received();
    CHECK(doc.find("<phyphox version=") == 0);

    t.experimentPackets.clear();
    t.writeControl(1);                                   // the control write also starts one
    CHECK(s.stats().transfers == 2);
    t.listener->onDisconnect();                          // mid-transfer disconnect
    CHECK(s.stats().transfersAborted == 1);
    CHECK(!s.transfer().active());
}

TEST_CASE("Flow control: refused packets are retried, nothing is lost") {
    FakeTransport t; Server s(t); s.setDeviceName("d"); s.setClock(clockFn); exampleCreateExperiment(s); s.start();
    StringSink full; s.printXml(full);
    t.refuseNext = 7;
    t.subscribeExperiment();
    for (int i = 0; i < 2000 && s.transfer().active(); ++i) { fakeNow += 1; s.poll(); }
    CHECK(t.refusedCount == 7);
    CHECK(s.stats().packetsRefused == 7);
    CHECK(t.received() == full.out);
    CHECK(s.stats().transfersCompleted == 1);
}

TEST_CASE("Asynchronous verdicts (the ESP32 path)") {
    FakeTransport t; t.async = true; Server s(t); s.setDeviceName("d"); s.setClock(clockFn); s.start();
    StringSink full; s.printXml(full);
    t.subscribeExperiment();
    CHECK(t.experimentPackets.size() == 0);              // nothing is sent inside the trigger callback
    s.poll();                                            // one packet is out; the core waits for the status callback
    CHECK(t.experimentPackets.size() == 1);
    s.poll(); CHECK(t.experimentPackets.size() == 1);
    int guard = 0;
    while (s.transfer().active() && guard++ < 5000) {
        // report the last packet as refused every 5th time, accepted otherwise
        bool ok = (guard % 5) != 0;
        if (!ok) t.experimentPackets.pop_back();          // a refused notification never left the board
        t.listener->onNotifyStatus(CharId{CH_EXPERIMENT, 0}, ok);
        fakeNow += 10; s.poll();
    }
    CHECK(t.received() == full.out);
    CHECK(s.stats().transfersCompleted == 1);
}

TEST_CASE("Input writes reach the channel store and the callbacks") {
    FakeTransport t; Server s(t); s.setDeviceName("d"); s.setClock(clockFn);
    E exp; E::View v("v");
    E::Slider sl("s", 0, 10, 1, 1); sl.setDefault(4); sl.onChange(onCh1);
    E::Button b("b", 2); b.onPress(onBtn);
    v.addElement(sl).addElement(b); exp.addView(v);
    s.addExperiment(exp); s.setConfigHandler(handler); s.start();
    CHECK(t.layout.inputChannels == 2);
    CHECK(s.channels().value(1) == 4);                    // the default before any write
    t.writeInput(1, 7);
    CHECK(seen == 7); CHECK(s.channels().value(1) == 7); CHECK(handlerCalls == 1);
    t.writeInput(2, 1); t.writeInput(2, 1);
    CHECK(pressed == 2); CHECK(handlerCalls == 3);
    CHECK(s.stats().inputWrites == 3);
}

TEST_CASE("Sensor writes fan out to the mapped channels") {
    FakeTransport t; Server s(t); s.setDeviceName("d"); exampleGetSensorDataFromSmartphone(s); s.start();
    CHECK(t.layout.sensors == 1);
    CHECK(t.layout.sensorValueSize[0] == 12);
    t.writeSensor(1, {1.5f, -2.5f, 9.81f});
    CHECK(s.channels().value(1) == 1.5f); CHECK(s.channels().value(2) == -2.5f); CHECK(s.channels().value(3) == 9.81f);
}

TEST_CASE("User XML: served verbatim with the 1.x config layout") {
    const char* xml = "<phyphox version=\"1.15\"><title>Mine</title></phyphox>";
    FakeTransport t; Server s(t); s.setDeviceName("d"); s.setClock(clockFn);
    s.setCustomXml((const uint8_t*)xml, strlen(xml)); s.start();
    CHECK(t.layout.legacyConfig); CHECK(t.layout.inputChannels == 0);
    t.subscribeExperiment();
    for (int i = 0; i < 100 && s.transfer().active(); ++i) s.poll();
    CHECK(t.received() == xml);
    uint8_t cfg[20]; float f[5] = {1, 2, 3, 4, 5}; memcpy(cfg, f, 20);
    t.listener->onWrite(CharId{CH_LEGACY_CONFIG, 0}, cfg, 20);
    CHECK(s.channels().value(3) == 3);
    CHECK(memcmp(s.legacyConfig(), cfg, 20) == 0);
}

TEST_CASE("Data notifications") {
    FakeTransport t; Server s(t); s.setDeviceName("d"); s.start();
    float v[3] = {1, 2, 3};
    CHECK(s.writeFloats(v, 3));
    REQUIRE(t.dataPackets.size() == 1);
    CHECK(t.dataPackets[0].size() == 20);
    float back[5]; memcpy(back, t.dataPackets[0].data(), 20);
    CHECK(back[0] == 1); CHECK(back[2] == 3); CHECK(back[4] == 0);
    uint8_t raw[8] = {1,2,3,4,5,6,7,8};
    CHECK(s.writeBytes(raw, 8));
    CHECK(t.dataPackets[1].size() == 8);
}

TEST_CASE("Events") {
    FakeTransport t; Server s(t); s.setDeviceName("d"); s.setEventHandler(handler); s.start();
    int before = handlerCalls;
    uint8_t b[17] = {0x02, 0,0,0,0,0,0,0,5, 0,0,0,0,0,0,0,9};
    t.listener->onWrite(CharId{CH_EVENT, 0}, b, 17);
    CHECK(handlerCalls == before + 1);
    CHECK(s.lastEvent().type == EVENT_CLEAR); CHECK(s.lastEvent().experimentTimeMs == 5); CHECK(s.lastEvent().systemTimeMs == 9);
}
