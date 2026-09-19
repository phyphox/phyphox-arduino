#include "doctest.h"
#include "transport/NinaB31Protocol.h"
#include <string>
#include <vector>

using namespace phyphox::nina;

TEST_CASE("Lines are assembled from CR/LF terminated input, empty lines skipped") {
    LineReader r;
    std::string in = "\r\n+UUBTACLC:0,0,112233445566p\r\nOK\r\n";
    std::vector<std::string> lines;
    for (char c : in) if (r.feed(c)) lines.push_back(r.line());
    REQUIRE(lines.size() == 2);
    CHECK(lines[0] == "+UUBTACLC:0,0,112233445566p");
    CHECK(lines[1] == "OK");
}

TEST_CASE("An over-long line is dropped whole, the next one survives") {
    LineReader r;
    std::string big(LINE_MAX + 40, 'x');
    for (char c : big) CHECK(!r.feed(c));
    CHECK(!r.feed('\r'));
    for (char c : std::string("OK")) r.feed(c);
    CHECK(r.feed('\r'));
    CHECK(std::string(r.line()) == "OK");
}

TEST_CASE("Events and responses are told apart") {
    CHECK(isEvent("+UUBTGRW:0,25,01,1"));
    CHECK(!isEvent("+UBTGCHA:25,26"));
    CHECK(!isEvent("OK"));
    int v = 0, c = 0;
    CHECK(parseCharacteristicResponse("+UBTGCHA:25,26", v, c));
    CHECK(v == 25); CHECK(c == 26);
    CHECK(parseCharacteristicResponse("+UBTGCHA:30,0", v, c));
    CHECK(c == 0);
    CHECK(!parseCharacteristicResponse("+UBTGSER:24", v, c));
    CHECK(firstNumber("+UBTGSER:24") == 24);
    CHECK(firstNumber("OK") == -1);
}

TEST_CASE("Attribute written: the control write, a CCCD write, a sensor write") {
    Event e = parseEvent("+UUBTGRW:0,25,01,1");
    CHECK(e.kind == EV_WRITE); CHECK(e.conn == 0); CHECK(e.handle == 25); CHECK(e.len == 1); CHECK(e.data[0] == 1);
    e = parseEvent("+UUBTGRW:0,26,0100,1");
    CHECK(e.kind == EV_WRITE); CHECK(e.handle == 26); CHECK(e.len == 2); CHECK(e.data[0] == 1); CHECK(e.data[1] == 0);
    e = parseEvent("+UUBTGRW:1,40,0000803F00000040000040400000804000000000000000000000000000000000,0");
    CHECK(e.kind == EV_WRITE); CHECK(e.conn == 1); CHECK(e.len == 32);
    float f; memcpy(&f, e.data + 4, 4); CHECK(f == 2.0f);
    e = parseEvent("+UUBTGRW:0,25,abCD");          // no options field, mixed case
    CHECK(e.kind == EV_WRITE); CHECK(e.len == 2); CHECK(e.data[0] == 0xab); CHECK(e.data[1] == 0xcd);
    e = parseEvent("+UUBTGRW:0");                    // truncated
    CHECK(e.kind == EV_NONE);
}

TEST_CASE("Connection, disconnection and read request events") {
    Event e = parseEvent("+UUBTACLC:2,0,AABBCCDDEEFFp");
    CHECK(e.kind == EV_CONNECTED); CHECK(e.conn == 2);
    e = parseEvent("+UUBTACLD:2");
    CHECK(e.kind == EV_DISCONNECTED); CHECK(e.conn == 2);
    e = parseEvent("+UUBTGRR:0,25");
    CHECK(e.kind == EV_READ); CHECK(e.handle == 25);
    e = parseEvent("+UUDPC:1,1,1,04D3B0285423p,1011");
    CHECK(e.kind == EV_OTHER);
}

TEST_CASE("Hex encoding round-trips and stops at the first non-hex character") {
    uint8_t bytes[4] = {0x00, 0x7f, 0x80, 0xff};
    char hex[9]; encodeHex(bytes, 4, hex);
    CHECK(std::string(hex) == "007F80FF");
    uint8_t back[4]; CHECK(decodeHex(hex, back, 4) == 4);
    CHECK(memcmp(back, bytes, 4) == 0);
    CHECK(decodeHex("0102,1", back, 4) == 2);
    CHECK(decodeHex("0", back, 4) == 0);
}

TEST_CASE("The event queue keeps order, bounds itself and counts drops") {
    EventQueue<3> q;
    CHECK(q.push("a")); CHECK(q.push("b")); CHECK(q.push("c"));
    CHECK(!q.push("d"));
    CHECK(q.dropped() == 1);
    char out[LINE_MAX];
    CHECK(q.pop(out)); CHECK(std::string(out) == "a");
    CHECK(q.push("e"));
    CHECK(q.pop(out)); CHECK(std::string(out) == "b");
    CHECK(q.pop(out)); CHECK(std::string(out) == "c");
    CHECK(q.pop(out)); CHECK(std::string(out) == "e");
    CHECK(!q.pop(out));
}
