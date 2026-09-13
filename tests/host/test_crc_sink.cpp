#include "doctest.h"
#include "helpers.h"
#include "core/Crc32.h"
#include <string>

using namespace phyphox;

TEST_CASE("CRC-32 matches the IEEE reference") {
    const char* s = "123456789";
    CHECK(Crc32::of((const uint8_t*)s, 9) == 0xCBF43926u);
    CHECK(Crc32::of(nullptr, 0) == 0u);
    Crc32 c; c.update((const uint8_t*)"1234", 4); c.update((const uint8_t*)"56789", 5);
    CHECK(c.value() == 0xCBF43926u);          // incremental == one-shot
}

TEST_CASE("CountingSink counts and hashes") {
    CountingSink cs;
    cs.write("123456789");
    CHECK(cs.count() == 9);
    CHECK(cs.crc() == 0xCBF43926u);
}

TEST_CASE("WindowSink passes exactly the requested range") {
    uint8_t out[8];
    for (size_t skip = 0; skip < 12; ++skip) {
        for (size_t cap = 1; cap <= 8; ++cap) {
            WindowSink w(out, cap, skip);
            w.write("abc"); w.write("defgh"); w.write("ij");   // stream "abcdefghij"
            std::string expect = std::string("abcdefghij").substr(skip < 10 ? skip : 10, cap);
            CHECK(std::string((const char*)out, w.produced()) == expect);
        }
    }
}
