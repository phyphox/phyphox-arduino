#include "doctest.h"
#include "helpers.h"
#include "core/Transfer.h"

using namespace phyphox;

TEST_CASE("Header packet and body packets") {
    const char* doc = "<phyphox>0123456789abcdefghijklmnopqrstuvwxyz</phyphox>";
    BufferSource src; src.set((const uint8_t*)doc, strlen(doc));
    TransferSession t; t.begin(src, 20);
    uint8_t p[512];
    uint16_t n = t.nextPacket(p, 0);
    CHECK(n == 20);
    CHECK(std::string((char*)p, 7) == "phyphox");
    uint32_t size = (p[7] << 24) | (p[8] << 16) | (p[9] << 8) | p[10];
    CHECK(size == strlen(doc));
    CHECK(t.nextPacket(p, 0) == 0);           // nothing until the verdict
    t.accepted();
    std::string body;
    while (t.active()) {
        n = t.nextPacket(p, 0);
        if (!n) break;
        body.append((char*)p, n); t.accepted();
    }
    CHECK(body == doc);
    CHECK(t.state() == TransferSession::DONE);
    CHECK(t.packetsSent() == 1 + (strlen(doc) + 19) / 20);
}

TEST_CASE("A refused packet is re-sent with a growing pause and never given up by count") {
    const char* doc = "0123456789";
    BufferSource src; src.set((const uint8_t*)doc, 10);
    TransferSession t; t.begin(src, 20);
    uint8_t p[64];
    REQUIRE(t.nextPacket(p, 0) == 20); t.accepted();
    REQUIRE(t.nextPacket(p, 100) == 10);
    t.refused(100);
    CHECK(t.nextPacket(p, 100) == 0);                 // backing off
    CHECK(t.nextPacket(p, 100 + PHYPHOX_BLE_TRANSFER_RETRY_DELAY_MS) == 10);   // same packet again
    CHECK(std::string((char*)p, 10) == doc);
    t.refused(200);                                   // the second refusal in a row: twice the pause
    CHECK(t.nextPacket(p, 200 + PHYPHOX_BLE_TRANSFER_RETRY_DELAY_MS) == 0);
    CHECK(t.nextPacket(p, 200 + 2 * PHYPHOX_BLE_TRANSFER_RETRY_DELAY_MS) == 10);
    uint32_t now = 1000;                              // a congested stack refusing for 25 s
    for (int i = 0; i < 500; ++i) { t.refused(now); now += PHYPHOX_BLE_TRANSFER_RETRY_MAX_MS; REQUIRE(t.nextPacket(p, now) == 10); }
    CHECK(t.state() == TransferSession::BODY);        // abandoning is the Server's watchdog's job
    CHECK(t.retries() == 502);
    t.accepted();
    CHECK(t.state() == TransferSession::DONE);
}

TEST_CASE("The header is one 20-byte packet whatever the MTU") {
    BufferSource src; src.set((const uint8_t*)"abc", 3);
    TransferSession t; t.begin(src, 185);
    uint8_t p[512];
    CHECK(t.nextPacket(p, 0) == 20);
}
