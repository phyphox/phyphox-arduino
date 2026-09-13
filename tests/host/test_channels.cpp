#include "doctest.h"
#include "helpers.h"
#include "core/ChannelStore.h"

using namespace phyphox;

static int changes = 0; static float lastValue = 0; static int presses = 0;

TEST_CASE("ChannelStore: defaults, deliveries, callbacks, changed()") {
    uint8_t storage[ChannelStore::bytesNeeded(4)];
    ChannelStore c; c.attach(4, storage);
    c.setDefault(1, 42);
    CHECK(c.value(1) == 42); CHECK(c.updates(1) == 0); CHECK(!c.changed(1));
    c.setCallbacks(1, [](float v) { changes++; lastValue = v; }, nullptr);
    c.setCallbacks(2, nullptr, []() { presses++; });
    c.deliver(1, 7.5f);
    CHECK(changes == 1); CHECK(lastValue == 7.5f); CHECK(c.value(1) == 7.5f); CHECK(c.updates(1) == 1);
    CHECK(c.changed(1)); CHECK(!c.changed(1));
    c.deliver(2, 1); c.deliver(2, 1);
    CHECK(presses == 2);
    c.deliver(9, 1);                                   // out of range: ignored
    CHECK(c.value(9) == 0);
}

TEST_CASE("Event block decoding") {
    uint8_t b[17] = {0x01, 0,0,0,0,0,0,0x03,0xE8, 0,0,0x01,0x8D,0x2C,0x4B,0x8F,0x20};
    ExperimentEvent e;
    REQUIRE(e.decode(b, 17));
    CHECK(e.type == EVENT_START);
    CHECK(e.experimentTimeMs == 1000);
    CHECK(e.systemTimeMs == 0x018D2C4B8F20LL);
    uint8_t sync[17] = {0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0,0,0,0,0,0,0,0};
    REQUIRE(e.decode(sync, 17));
    CHECK(e.type == EVENT_SYNC); CHECK(e.experimentTimeMs == -1);
    CHECK(!e.decode(b, 16));
}
