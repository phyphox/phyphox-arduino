// Writes every example configuration (and the default experiment) as a .phyphox document into
// the directory given as argv[1]. The conformance runner validates them against the spec.
#include <cstdio>
#include <string>
#include "helpers.h"
#include "examples_config.h"

static void dump(phyphox::Server& s, const std::string& dir, const char* name) {
    StringSink sink; s.printXml(sink);
    std::string path = dir + "/" + name + ".phyphox";
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) { perror(path.c_str()); exit(1); }
    fwrite(sink.out.data(), 1, sink.out.size(), f); fclose(f);
    printf("%s (%zu bytes, block %zu bytes)\n", path.c_str(), sink.out.size(), s.store().blockSize());
}

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "usage: gen_examples <dir>\n"); return 2; }
    std::string dir = argv[1];
    { FakeTransport t; phyphox::Server s(t); s.setDeviceName("phyphox-Arduino"); s.start(); dump(s, dir, "default"); }
    { FakeTransport t; phyphox::Server s(t); s.setDeviceName("create experiment"); exampleCreateExperiment(s); dump(s, dir, "CreateExperiment"); }
    { FakeTransport t; phyphox::Server s(t); s.setDeviceName("phyphox-Arduino"); exampleMultigraph(s); dump(s, dir, "multigraph"); }
    { FakeTransport t; phyphox::Server s(t); s.setDeviceName("phyphox-Arduino"); exampleGetDataFromSmartphone(s); dump(s, dir, "getDataFromSmartphone"); }
    { FakeTransport t; phyphox::Server s(t); s.setDeviceName("phyphox-Arduino"); exampleGetSensorDataFromSmartphone(s); dump(s, dir, "getSensorDataFromSmartphone"); }
    { FakeTransport t; phyphox::Server s(t); s.setDeviceName("Rangefinder"); exampleRangefinder(s); dump(s, dir, "rangefinder"); }
    { FakeTransport t; phyphox::Server s(t); s.setDeviceName("phyphox-Arduino"); s.setMtu(176); exampleEverything(s); dump(s, dir, "everything"); }
    { FakeTransport t; phyphox::Server s(t); s.setDeviceName("phyphox-Arduino");
      E exp("Resend", "Arduino Experiments", "1.x behaviour"); exp.setResendUnchanged(true); exp.setRepeating(4);
      E::View v("V"); E::Slider sl("S", 0, 10, 1, 1); v.addElement(sl); exp.addView(v); s.addExperiment(exp); dump(s, dir, "resendUnchanged"); }
    return 0;
}
