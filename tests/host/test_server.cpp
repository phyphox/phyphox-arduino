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
    FakeTransport t; Server s(t); s.setDeviceName("d"); s.setClock(clockFn); s.start(); s.poll();
    CHECK(t.began); CHECK(t.layout.inputChannels == 0); CHECK(!t.layout.legacyConfig);
    t.subscribeExperiment();
    CHECK(s.stats().transfers == 1);
    t.writeControl(1);                                   // during the transfer: ignored
    CHECK(s.stats().transfers == 1);
    s.poll(); CHECK(t.experimentPackets.size() == 0);   // the start delay: nothing yet
    fakeNow += PHYPHOX_BLE_TRANSFER_START_DELAY_MS + 1;
    for (int i = 0; i < 100 && s.transfer().active(); ++i) { fakeNow += 1; s.poll(); }
    CHECK(s.stats().transfersCompleted == 1);
    CHECK(t.advertisingRestarts == 1);
    std::string doc = t.received();
    CHECK(doc.find("<phyphox version=") == 0);

    t.experimentPackets.clear();
    t.writeControl(1);                                   // the control write also starts one
    CHECK(s.stats().transfers == 2);
    fakeNow += PHYPHOX_BLE_TRANSFER_START_DELAY_MS + 1; s.poll();
    t.listener->onDisconnect();                          // mid-transfer disconnect
    CHECK(s.stats().transfersAborted == 1);
    CHECK(!s.transfer().active());
}

TEST_CASE("The radio comes up at addExperiment() when start() came first, and at start() otherwise") {
    { FakeTransport t; Server s(t); s.setDeviceName("d"); s.setClock(clockFn);
      s.start(); CHECK(!t.began);                          // default experiment: waits for poll()/write()
      PhyphoxBleExperiment e("T"); PhyphoxBleExperiment::View v("v"); PhyphoxBleExperiment::Slider sl("s", 0, 1, 0.1f); sl.setChannel(2);
      v.addElement(sl); e.addView(v);
      s.addExperiment(e); CHECK(t.began);                  // known now: up before loop() ever runs
      CHECK(t.layout.inputChannels == 2); }                // with the experiment's channels
    { FakeTransport t; Server s(t); s.setDeviceName("d"); s.setClock(clockFn);
      PhyphoxBleExperiment e("T"); s.addExperiment(e); CHECK(!t.began);
      s.start(); CHECK(t.began); }
}

TEST_CASE("A sketch that only writes still serves the transfer (write services the stack)") {
    FakeTransport t; Server s(t); s.setDeviceName("d"); s.setClock(clockFn); s.start();
    float v = 1;
    s.writeFloats(&v, 1);                                // the lazy start happens here
    t.subscribeExperiment();
    fakeNow += PHYPHOX_BLE_TRANSFER_START_DELAY_MS + 1;
    for (int i = 0; i < 200 && s.transfer().active(); ++i) { fakeNow += 1; s.writeFloats(&v, 1); }   // never poll()
    CHECK(s.stats().transfersCompleted == 1);
    StringSink full; s.printXml(full);
    CHECK(t.received() == full.out);
}

TEST_CASE("A stalled transfer is abandoned and the next trigger starts fresh") {
    FakeTransport t; Server s(t); s.setDeviceName("d"); s.setClock(clockFn); s.start(); s.poll();
    t.refuseNext = 1000000;                              // the stack refuses everything
    t.subscribeExperiment();
    for (int i = 0; i < 20; ++i) { fakeNow += 100; s.poll(); }
    CHECK(s.transfer().active());                       // still retrying (the stall clock counts from the last accepted packet)
    fakeNow += PHYPHOX_BLE_TRANSFER_STALL_MS + 100; s.poll();
    CHECK(!s.transfer().active());
    CHECK(s.stats().transfersAborted == 1);
    CHECK(s.stats().transfersStalled == 1);
    t.refuseNext = 0; t.experimentPackets.clear();
    t.subscribeExperiment();                             // a new trigger is not ignored
    for (int i = 0; i < 400 && s.transfer().active(); ++i) { fakeNow += 1; s.poll(); }
    CHECK(s.stats().transfersCompleted == 1);
    StringSink full; s.printXml(full);
    CHECK(t.received() == full.out);
}

TEST_CASE("Flow control: refused packets are retried, nothing is lost") {
    FakeTransport t; Server s(t); s.setDeviceName("d"); s.setClock(clockFn); exampleCreateExperiment(s); s.start(); s.poll();
    StringSink full; s.printXml(full);
    t.refuseNext = 7;
    t.subscribeExperiment();
    for (int i = 0; i < 3000 && s.transfer().active(); ++i) { fakeNow += 1; s.poll(); }
    CHECK(t.refusedCount == 7);
    CHECK(s.stats().packetsRefused == 7);
    CHECK(t.received() == full.out);
    CHECK(s.stats().transfersCompleted == 1);
}

TEST_CASE("Asynchronous verdicts (the ESP32 path)") {
    FakeTransport t; t.async = true; Server s(t); s.setDeviceName("d"); s.setClock(clockFn); s.start(); s.poll();
    StringSink full; s.printXml(full);
    t.subscribeExperiment();
    CHECK(t.experimentPackets.size() == 0);              // nothing is sent inside the trigger callback
    fakeNow += PHYPHOX_BLE_TRANSFER_START_DELAY_MS + 1;
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
    s.addExperiment(exp); s.setConfigHandler(handler); s.start(); s.poll();
    CHECK(t.layout.inputChannels == 2);
    CHECK(s.channels().value(1) == 4);                    // the default before any write
    t.writeInput(1, 7);
    CHECK(seen == 7); CHECK(s.channels().value(1) == 7); CHECK(handlerCalls == 1);
    t.writeInput(2, 1); t.writeInput(2, 1);
    CHECK(pressed == 2); CHECK(handlerCalls == 3);
    CHECK(s.stats().inputWrites == 3);
}

static float sensorSeen = 0; static void onCh3(float v) { sensorSeen = v; }

TEST_CASE("Sensor writes fan out to the mapped channels, and callbacks may be registered before start") {
    FakeTransport t; Server s(t); s.setDeviceName("d");
    s.setChannelCallbacks(3, onCh3, nullptr);           // before the experiment exists
    exampleGetSensorDataFromSmartphone(s); s.start(); s.poll();
    CHECK(t.layout.sensors == 1);
    CHECK(t.layout.inputChannels == 3);
    CHECK(t.layout.inputChannelMask == 0);              // all three channels are sensor-fed: no input characteristics
    CHECK(t.layout.sensorValueSize[0] == 12);
    t.writeSensor(1, {1.5f, -2.5f, 9.81f});
    CHECK(s.channels().value(1) == 1.5f); CHECK(s.channels().value(2) == -2.5f); CHECK(s.channels().value(3) == 9.81f);
    CHECK(sensorSeen == 9.81f);
}

TEST_CASE("The layout mask names only channels with an element") {
    FakeTransport t; Server s(t); s.setDeviceName("d"); exampleGetDataFromSmartphone(s); s.start(); s.poll();
    CHECK(t.layout.inputChannels == 5);
    CHECK(t.layout.inputChannelMask == ((1u << 1) | (1u << 2) | (1u << 3) | (1u << 4) | (1u << 5)));
}

TEST_CASE("User XML: served verbatim with the 1.x config layout") {
    const char* xml = "<phyphox version=\"1.15\"><title>Mine</title></phyphox>";
    FakeTransport t; Server s(t); s.setDeviceName("d"); s.setClock(clockFn);
    s.setCustomXml((const uint8_t*)xml, strlen(xml)); s.start(); s.poll();
    CHECK(t.layout.legacyConfig); CHECK(t.layout.inputChannels == 0);
    t.subscribeExperiment();
    fakeNow += PHYPHOX_BLE_TRANSFER_START_DELAY_MS + 1;
    for (int i = 0; i < 100 && s.transfer().active(); ++i) { fakeNow += 1; s.poll(); }
    CHECK(t.received() == xml);
    StringSink dump; s.printXml(dump);
    CHECK(dump.out == xml);                              // printXML() shows the user's bytes, not a generated document
    uint8_t cfg[20]; float f[5] = {1, 2, 3, 4, 5}; memcpy(cfg, f, 20);
    t.listener->onWrite(CharId{CH_LEGACY_CONFIG, 0}, cfg, 20);
    CHECK(s.channels().value(3) == 3);
    CHECK(memcmp(s.legacyConfig(), cfg, 20) == 0);
}

TEST_CASE("start() before addExperiment(): the 1.x order still gets every characteristic") {
    FakeTransport t; Server s(t); s.setDeviceName("d"); s.setClock(clockFn);
    s.start();                                           // the 1.x examples start first …
    CHECK(!t.began);                                     // … so nothing happens yet
    exampleGetDataFromSmartphone(s);                     // … then describe the experiment
    float v = 1; s.writeFloats(&v, 1);                   // the first write from loop() brings the stack up
    CHECK(t.began);
    CHECK(t.layout.inputChannels == 5);                  // with the experiment's channels
    CHECK(!s.store().hasErrors());
    // and a description added after the stack is up is served, but flagged if it needs more
    E exp2; E::View v2("v"); E::Slider sl("s", 0, 1, 1, 9); v2.addElement(sl); exp2.addView(v2);
    s.addExperiment(exp2);
    CHECK(s.store().hasErrors());
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
    FakeTransport t; Server s(t); s.setDeviceName("d"); s.setEventHandler(handler); s.start(); s.poll();
    int before = handlerCalls;
    uint8_t b[17] = {0x02, 0,0,0,0,0,0,0,5, 0,0,0,0,0,0,0,9};
    t.listener->onWrite(CharId{CH_EVENT, 0}, b, 17);
    CHECK(handlerCalls == before + 1);
    CHECK(s.lastEvent().type == EVENT_CLEAR); CHECK(s.lastEvent().experimentTimeMs == 5); CHECK(s.lastEvent().systemTimeMs == 9);
}
