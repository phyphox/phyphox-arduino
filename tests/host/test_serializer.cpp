#include "doctest.h"
#include "helpers.h"
#include "examples_config.h"
#include <fstream>
#include <sstream>
#include <cstdlib>

using namespace phyphox;

static std::string document(Server& s) { StringSink k; s.printXml(k); return k.out; }
static size_t countOf(const std::string& hay, const std::string& needle) {
    size_t n = 0, p = 0; while ((p = hay.find(needle, p)) != std::string::npos) { n++; p += needle.size(); } return n;
}

TEST_CASE("The default document has the expected shape") {
    FakeTransport t; Server s(t); s.setDeviceName("phyphox-Arduino"); s.start();
    std::string x = document(s);
    CHECK(x.find("<phyphox version=\"1.20\">") == 0);
    CHECK(x.find("<title>Arduino-Experiment</title>") != std::string::npos);
    CHECK(x.find("<bluetooth name=\"phyphox-Arduino\" id=\"phyphoxBLE\" mode=\"notification\" subscribeOnStart=\"false\">") != std::string::npos);
    CHECK(countOf(x, "cddf1002-30f7-4671-8b43-5e40ba53514a") == 6);
    CHECK(x.find("extra=\"time\">CH0</output>") != std::string::npos);
    CHECK(x.find("<analysis sleep=\"0\">\n</analysis>") != std::string::npos);
    CHECK(x.find("<graph label=\"Graph\" labelX=\"time\" labelY=\"value\" unitX=\"s\">") != std::string::npos);
    CHECK(x.find("<input axis=\"y\">CH1</input>") != std::string::npos);
    CHECK(x.find("<set name=\"mySet\">") != std::string::npos);
    CHECK(x.rfind("</phyphox>\n") == x.size() - 11);
    CHECK(x.find("mtu=") == std::string::npos);
}

TEST_CASE("Input channels produce containers, outputs and the change detector") {
    FakeTransport t; Server s(t); s.setDeviceName("d"); exampleGetDataFromSmartphone(s);
    std::string x = document(s);
    // channel 1: edit + slider share it → one characteristic, one detector
    CHECK(countOf(x, "cddf2001-30f7-4671-8b43-5e40ba53514a") == 1);
    CHECK(x.find("<container size=\"1\" init=\"100\">in1</container>") != std::string::npos);
    CHECK(x.find("<container size=\"1\" init=\"100\">last1</container>") != std::string::npos);
    CHECK(x.find("<container size=\"1\">send1</container>") != std::string::npos);
    CHECK(x.find("conversion=\"float32LittleEndian\" keep=\"false\">send1</input>") != std::string::npos);
    CHECK(x.find("<input as=\"a\" keep=\"true\">in1</input>") != std::string::npos);
    CHECK(x.find("<input as=\"in\" keep=\"true\">in1</input>") != std::string::npos);
    CHECK(x.find("<input as=\"b\" keep=\"true\">last1</input>") != std::string::npos);
    CHECK(x.find("<output as=\"result\">send1</output>") != std::string::npos);
    CHECK(x.find("<output as=\"out\">last1</output>") != std::string::npos);
    // channel 4: the button — no detector, keep=false on in4, a trigger because channel 5 submits with it
    CHECK(x.find("keep=\"false\">in4</input>") != std::string::npos);
    CHECK(x.find("last4") == std::string::npos);
    CHECK(x.find("<input type=\"value\">1</input>") != std::string::npos);
    CHECK(x.find("<trigger>btn4</trigger>") != std::string::npos);
    CHECK(x.find("keep=\"false\" triggerId=\"btn4\">send5</input>") != std::string::npos);
    // dropdown options
    CHECK(x.find("<map value=\"2\">Blink</map>") != std::string::npos);
    CHECK(x.find("<slider label=\"Interval\" default=\"100\" minValue=\"50\" maxValue=\"1000\" stepSize=\"50\" precision=\"0\">") != std::string::npos);
}

TEST_CASE("Resend mode drops the detector") {
    FakeTransport t; Server s(t); s.setDeviceName("d");
    E exp("R", "C", "D"); exp.setResendUnchanged(true); exp.setRepeating(4);
    E::View v("V"); E::Slider sl("S", 0, 10, 1, 1); v.addElement(sl); exp.addView(v); s.addExperiment(exp);
    std::string x = document(s);
    CHECK(x.find("keep=") == std::string::npos);
    CHECK(x.find("<if") == std::string::npos);
    CHECK(x.find("conversion=\"float32LittleEndian\">in1</input>") != std::string::npos);
    CHECK(countOf(x, "repeating=\"4\"") == 5);
}

TEST_CASE("Sensors: one characteristic per sensor, components at offsets") {
    FakeTransport t; Server s(t); s.setDeviceName("d"); exampleGetSensorDataFromSmartphone(s);
    std::string x = document(s);
    CHECK(x.find("<sensor type=\"accelerometer\" rate=\"80\" average=\"true\">") != std::string::npos);
    CHECK(x.find("<output component=\"z\">sen1z</output>") != std::string::npos);
    CHECK(x.find("<input char=\"cddf3001-30f7-4671-8b43-5e40ba53514a\" conversion=\"float32LittleEndian\" offset=\"8\">sen1z</input>") != std::string::npos);
    CHECK(x.find("cddf2001") == std::string::npos);   // sensor channels get no input characteristic
}

TEST_CASE("Errors are shown in an extra view") {
    FakeTransport t; Server s(t); s.setDeviceName("d");
    E exp; E::View v("v"); E::Value val("v", 1); val.setColor("nothex"); v.addElement(val); exp.addView(v); s.addExperiment(exp);
    std::string x = document(s);
    CHECK(x.find("<view label=\"ERRORS\">") != std::string::npos);
    CHECK(x.find("ERROR FOUND: ERR_03 color is not 6 hex digits, in setColor()") != std::string::npos);
}

TEST_CASE("Escaping and the mtu attribute") {
    FakeTransport t; Server s(t); s.setDeviceName("a & b"); s.setMtu(176); exampleEverything(s);
    std::string x = document(s);
    CHECK(x.find("<title>Everything &amp; &quot;more&quot;</title>") != std::string::npos);
    CHECK(x.find("<category>Arduino &lt;Experiments&gt;</category>") != std::string::npos);
    CHECK(x.find("name=\"a &amp; b\"") != std::string::npos);
    CHECK(x.find("mtu=\"179\"") != std::string::npos);
    CHECK(x.find("<map value=\"2.5\">b &amp; c</map>") != std::string::npos);
    CHECK(x.find("visibility=\"in3\"") != std::string::npos);
    CHECK(x.find("<sensor type=\"custom\" nameFilter=\"BMI160\" typeFilter=\"65600\">") != std::string::npos);
    CHECK(x.find("<input axis=\"y\" color=\"00ff00\" style=\"map\" lineWidth=\"2.5\">CH2</input>") != std::string::npos);
}

TEST_CASE("Part table and windows are consistent with the full document") {
    FakeTransport t; Server s(t); s.setDeviceName("d"); exampleEverything(s);
    std::string full = document(s);
    // the transfer reads the document through the part table in MTU-sized windows: for every
    // MTU the reassembled packets must equal the document written in one go
    for (uint16_t mtu : {20, 23, 64, 185, 512}) {
        FakeTransport t3; t3.mtu = mtu; Server s3(t3); s3.setDeviceName("d"); s3.setClock([]() -> uint32_t { return 0; }); exampleEverything(s3);
        s3.start(); t3.subscribeExperiment();
        for (int i = 0; i < 1000 && s3.transfer().active(); ++i) s3.poll();
        CHECK(t3.received() == full);
        REQUIRE(t3.experimentPackets.size() >= 2);
        const std::vector<uint8_t>& h = t3.experimentPackets[0];
        CHECK(std::string(h.begin(), h.begin() + 7) == "phyphox");
        uint32_t size = (h[7] << 24) | (h[8] << 16) | (h[9] << 8) | h[10];
        uint32_t crc = ((uint32_t)h[11] << 24) | (h[12] << 16) | (h[13] << 8) | h[14];
        CHECK(size == full.size());
        CHECK(crc == Crc32::of((const uint8_t*)full.data(), full.size()));
        for (size_t i = 1; i + 1 < t3.experimentPackets.size(); ++i) CHECK(t3.experimentPackets[i].size() == mtu);
    }
}

TEST_CASE("Golden documents") {
    // Every example document is compared with the stored copy; regenerate with gen_examples
    // and review the diff before committing a change.
    struct Row { const char* name; void (*build)(Server&); const char* device; uint16_t mtu; };
    auto gd = [](Server& s) { (void)s; };
    Row rows[] = {
        {"default", gd, "phyphox-Arduino", 20},
        {"CreateExperiment", [](Server& s) { exampleCreateExperiment(s); }, "create experiment", 20},
        {"multigraph", [](Server& s) { exampleMultigraph(s); }, "phyphox-Arduino", 20},
        {"getDataFromSmartphone", [](Server& s) { exampleGetDataFromSmartphone(s); }, "phyphox-Arduino", 20},
        {"getSensorDataFromSmartphone", [](Server& s) { exampleGetSensorDataFromSmartphone(s); }, "phyphox-Arduino", 20},
        {"rangefinder", [](Server& s) { exampleRangefinder(s); }, "Rangefinder", 20},
        {"everything", [](Server& s) { exampleEverything(s); }, "phyphox-Arduino", 176},
    };
    for (const Row& r : rows) {
        FakeTransport t; Server s(t); s.setDeviceName(r.device); s.setMtu(r.mtu); r.build(s); s.start();
        std::ifstream f(std::string(GOLDEN_DIR) + "/" + r.name + ".phyphox", std::ios::binary);
        REQUIRE_MESSAGE(f.good(), r.name);
        std::stringstream buf; buf << f.rdbuf();
        CHECK_MESSAGE(document(s) == buf.str(), r.name);
    }
}
