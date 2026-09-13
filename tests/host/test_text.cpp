#include "doctest.h"
#include "helpers.h"
#include "core/Serializer.h"

using namespace phyphox;

static std::string esc(const char* s) { StringSink k; writeEscaped(k, s); return k.out; }
static std::string flt(float v) { StringSink k; writeFloat(k, v); return k.out; }
static std::string integer(int32_t v) { StringSink k; writeInt(k, v); return k.out; }

TEST_CASE("XML escaping") {
    CHECK(esc("a & b < c > d \"e\"") == "a &amp; b &lt; c &gt; d &quot;e&quot;");
    CHECK(esc("plain") == "plain");
    CHECK(esc(nullptr) == "");
    CHECK(esc("µT °C") == "µT °C");   // UTF-8 passes through
}

TEST_CASE("Integers") {
    CHECK(integer(0) == "0"); CHECK(integer(7) == "7"); CHECK(integer(-42) == "-42");
    CHECK(integer(2147483647) == "2147483647"); CHECK(integer(-2147483647 - 1) == "-2147483648");
}

TEST_CASE("Floats: fixed notation in the usual range, trimmed") {
    CHECK(flt(0) == "0"); CHECK(flt(1) == "1"); CHECK(flt(-1) == "-1");
    CHECK(flt(0.5f) == "0.5"); CHECK(flt(100) == "100"); CHECK(flt(-3.25f) == "-3.25");
    CHECK(flt(0.1f) == "0.1"); CHECK(flt(1000000) == "1000000"); CHECK(flt(0.001f) == "0.001");
    CHECK(flt(2.5f) == "2.5"); CHECK(flt(1e-4f) == "0.0001");
}

TEST_CASE("Floats: scientific outside it, and the odd values") {
    CHECK(flt(1e12f) == "1e12"); CHECK(flt(1.5e-5f) == "1.5e-5"); CHECK(flt(2e9f) == "2e9");
    CHECK(flt(0.0f / 0.0f) == "0");
    CHECK(flt(1e38f * 10) == "1e38");
}

TEST_CASE("Characteristic UUIDs") {
    StringSink a; writeInputCharUuid(a, 1);  CHECK(a.out == "cddf2001-30f7-4671-8b43-5e40ba53514a");
    StringSink b; writeInputCharUuid(b, 16); CHECK(b.out == "cddf2010-30f7-4671-8b43-5e40ba53514a");
    StringSink c; writeSensorCharUuid(c, 2); CHECK(c.out == "cddf3002-30f7-4671-8b43-5e40ba53514a");
}
