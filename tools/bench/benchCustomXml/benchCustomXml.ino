// Bench stimulus for the user-XML mode (tier T1): the sketch serves a hand-written phyphox
// document (the 1.x way: a document converted to a header) and the library creates the 1.x
// characteristic layout for it — data on cddf1002, the five-float config on cddf1003. Serial:
//   x  print the served document between XML-BEGIN / XML-END
//   s  print the server statistics
// Every config write is echoed as CONFIG a b c d e.
#include <phyphoxBle.h>

#ifndef BENCH_NAME
#define BENCH_NAME phyphox_customxml
#endif
#define BENCH_STR2(x) #x
#define BENCH_STR(x) BENCH_STR2(x)

static const char xml[] =
"<phyphox version=\"1.15\">\n"
"<title>Custom XML</title>\n<category>Arduino Experiments</category>\n<description>Served as written.</description>\n"
"<data-containers>\n\t<container size=\"0\">CH0</container>\n\t<container size=\"0\">CH1</container>\n"
"\t<container size=\"1\">CB1</container>\n\t<container size=\"1\">CB2</container>\n</data-containers>\n"
"<input>\n\t<bluetooth name=\"" BENCH_STR(BENCH_NAME) "\" id=\"phyphoxBLE\" mode=\"notification\" subscribeOnStart=\"false\">\n"
"\t\t<output char=\"cddf1002-30f7-4671-8b43-5e40ba53514a\" conversion=\"float32LittleEndian\" offset=\"0\">CH1</output>\n"
"\t\t<output char=\"cddf1002-30f7-4671-8b43-5e40ba53514a\" extra=\"time\">CH0</output>\n\t</bluetooth>\n</input>\n"
"<output>\n\t<bluetooth id=\"phyphoxBLE\" name=\"" BENCH_STR(BENCH_NAME) "\">\n"
"\t\t<input char=\"cddf1003-30f7-4671-8b43-5e40ba53514a\" conversion=\"float32LittleEndian\">CB1</input>\n"
"\t\t<input char=\"cddf1003-30f7-4671-8b43-5e40ba53514a\" conversion=\"float32LittleEndian\" offset=\"4\">CB2</input>\n"
"\t</bluetooth>\n</output>\n"
"<analysis sleep=\"0\"></analysis>\n"
"<views>\n\t<view label=\"Custom\">\n\t\t<graph label=\"Value\" labelX=\"time\" unitX=\"s\" labelY=\"value\">\n"
"\t\t\t<input axis=\"x\">CH0</input>\n\t\t\t<input axis=\"y\">CH1</input>\n\t\t</graph>\n"
"\t\t<edit label=\"A\" default=\"1\">\n\t\t\t<output>CB1</output>\n\t\t</edit>\n"
"\t\t<edit label=\"B\" default=\"2\">\n\t\t\t<output>CB2</output>\n\t\t</edit>\n\t</view>\n</views>\n"
"<export>\n\t<set name=\"Data\">\n\t\t<data name=\"t\">CH0</data>\n\t\t<data name=\"v\">CH1</data>\n\t</set>\n</export>\n</phyphox>\n";

static void onConfig() {
  float a, b, c, d, e;
  PhyphoxBLE::read(a, b, c, d, e);
  Serial.print("CONFIG "); Serial.print(a, 3); Serial.print(" "); Serial.print(b, 3); Serial.print(" ");
  Serial.print(c, 3); Serial.print(" "); Serial.print(d, 3); Serial.print(" "); Serial.println(e, 3);
}

void setup() {
  Serial.begin(115200);
  PhyphoxBLE::configHandler = onConfig;
  PhyphoxBLE::start(BENCH_STR(BENCH_NAME), (uint8_t*)xml, sizeof(xml) - 1);
  Serial.print("READY "); Serial.print(PhyphoxBLE::transportName()); Serial.print(" "); Serial.println(PhyphoxBLE::version());
}

static float counter = 0;
static unsigned long lastWrite = 0;
void loop() {
  if (Serial.available()) {
    char c = (char)Serial.read();
    if (c == 'x') { Serial.println("XML-BEGIN"); PhyphoxBLE::printXML(&Serial); Serial.println("XML-END"); }
    if (c == 's') {
      const phyphox::ServerStats& s = PhyphoxBLE::stats();
      Serial.print("STATS transfers="); Serial.print(s.transfers); Serial.print(" completed="); Serial.print(s.transfersCompleted);
      Serial.print(" aborted="); Serial.print(s.transfersAborted); Serial.print(" refused="); Serial.print(s.packetsRefused);
      Serial.print(" data="); Serial.print(s.dataNotifications); Serial.print(" dataRefused="); Serial.print(s.dataRefused);
      Serial.print(" inputs="); Serial.print(s.inputWrites); Serial.print(" events="); Serial.println(s.events);
    }
  }
  unsigned long now = millis();
  if (now - lastWrite >= 50) { lastWrite = now; counter += 1; PhyphoxBLE::write(counter); }
  PhyphoxBLE::poll();
}
