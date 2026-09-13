// The bench stimulus (tier T1): a rich experiment, data at 20 Hz, and every event echoed on
// serial so tools/bench/bench.py can assert what the board saw. Commands on serial:
//   x  print the generated document between XML-BEGIN / XML-END
//   s  print the server statistics
#include <phyphoxBle.h>

// The advertised name; the bench passes -DBENCH_NAME=bench_esp32 so that two boards on one
// desk can be told apart (a name with underscores, so it survives as a preprocessor token).
#ifndef BENCH_NAME
#define BENCH_NAME phyphox_bench
#endif
#define BENCH_STR2(x) #x
#define BENCH_STR(x) BENCH_STR2(x)

static void onCh1(float v) { Serial.print("CHANGE 1 "); Serial.println(v, 3); }
static void onCh2(float v) { Serial.print("CHANGE 2 "); Serial.println(v, 3); }
static void onCh3(float v) { Serial.print("CHANGE 3 "); Serial.println(v, 3); }
static void onPress4() { Serial.println("PRESS 4"); }
static void onCh5(float v) { Serial.print("CHANGE 5 "); Serial.println(v, 3); }
static void onCh6(float v) { Serial.print("CHANGE 6 "); Serial.println(v, 3); }
static void onCh7(float v) { Serial.print("CHANGE 7 "); Serial.println(v, 3); }
static void onCh8(float v) { Serial.print("CHANGE 8 "); Serial.println(v, 3); }
static void onEvent() {
  Serial.print("EVENT "); Serial.print(PhyphoxBLE::eventType);
  Serial.print(" "); Serial.print((long)PhyphoxBLE::experimentTime);
  Serial.print(" "); Serial.println((long)(PhyphoxBLE::systemTime / 1000));
}
static void onConfig() { Serial.println("CONFIG"); }

static void printStats() {
  const phyphox::ServerStats& s = PhyphoxBLE::stats();
  Serial.print("STATS transfers="); Serial.print(s.transfers);
  Serial.print(" completed="); Serial.print(s.transfersCompleted);
  Serial.print(" aborted="); Serial.print(s.transfersAborted);
  Serial.print(" refused="); Serial.print(s.packetsRefused);
  Serial.print(" data="); Serial.print(s.dataNotifications);
  Serial.print(" dataRefused="); Serial.print(s.dataRefused);
  Serial.print(" inputs="); Serial.print(s.inputWrites);
  Serial.print(" events="); Serial.println(s.events);
}

void setup() {
  Serial.begin(115200);
  PhyphoxBleExperiment exp("Bench", "Arduino Experiments", "The phyphox BLE 2.0 bench experiment.");
  PhyphoxBleExperiment::View view("Bench");
  PhyphoxBleExperiment::Graph graph("Counter"); graph.setChannel(0, 1); graph.setLabelX("time"); graph.setUnitX("s");
  PhyphoxBleExperiment::Value value("Counter", 1); value.setPrecision(0);
  PhyphoxBleExperiment::Edit edit("Interval", 1); edit.setDefault(100); edit.setUnit("ms"); edit.onChange(onCh1);
  PhyphoxBleExperiment::Slider slider("Interval", 50, 1000, 50, 1); slider.setDefault(100);
  PhyphoxBleExperiment::Toggle toggle("Enabled", 2); toggle.setDefault(1); toggle.onChange(onCh2);
  PhyphoxBleExperiment::Dropdown mode("Mode", 3); mode.addOption("A", 1); mode.addOption("B", 2); mode.onChange(onCh3);
  PhyphoxBleExperiment::Button button("Press me", 4); button.onPress(onPress4);
  PhyphoxBleExperiment::Edit submitted("Submitted", 5); submitted.submitWith(button); submitted.onChange(onCh5);
  PhyphoxBleExperiment::Sensor acc(SENSOR_ACCELEROMETER); acc.setRate(20);
  acc.mapChannel("x", 6); acc.mapChannel("y", 7); acc.mapChannel("z", 8);
  view.addElement(graph).addElement(value).addElement(edit).addElement(slider).addElement(toggle)
      .addElement(mode).addElement(button).addElement(submitted);
  exp.addView(view).addSensor(acc);
  PhyphoxBLE::experimentEventHandler = onEvent;
  PhyphoxBLE::configHandler = onConfig;
  PhyphoxBLE::onChange(6, onCh6); PhyphoxBLE::onChange(7, onCh7); PhyphoxBLE::onChange(8, onCh8);
  PhyphoxBLE::start(BENCH_STR(BENCH_NAME), exp);
  Serial.print("READY "); Serial.print(PhyphoxBLE::transportName()); Serial.print(" "); Serial.println(PhyphoxBLE::version());
  PhyphoxBLE::printErrors(&Serial);
}

static float counter = 0;
static unsigned long lastWrite = 0;

void loop() {
  if (Serial.available()) {
    char c = (char)Serial.read();
    if (c == 'x') { Serial.println("XML-BEGIN"); PhyphoxBLE::printXML(&Serial); Serial.println("XML-END"); }
    if (c == 's') printStats();
  }
  unsigned long now = millis();
  if (now - lastWrite >= 50) {
    lastWrite = now;
    counter += 1;
    float wave = 50.0f + 40.0f * sin(now / 1000.0f);
    PhyphoxBLE::write(counter, wave);
  }
  PhyphoxBLE::poll();
}
