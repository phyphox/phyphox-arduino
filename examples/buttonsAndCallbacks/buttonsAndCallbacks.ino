// phyphox BLE 2.0: buttons and callbacks.
//
// The phone shows a slider, a toggle and two buttons. Every change arrives on the board as a
// callback — no polling, and no repeated writes of unchanged values. A value is streamed back
// so the effect is visible in the app.

#include <phyphoxBle.h>

float gain = 1.0f;       // set by the slider
bool enabled = true;     // set by the toggle
float offset = 0.0f;     // set by the "zero" button
float counter = 0.0f;    // reset by the other button
float last = 0.0f;

void onGain(float v) { gain = v; }
void onEnabled(float v) { enabled = v > 0.5f; }
void onZero() { offset = last; }
void onReset() { counter = 0; }

void setup() {
  Serial.begin(115200);

  PhyphoxBleExperiment exp("Buttons and callbacks", "Arduino Experiments",
                           "A slider, a toggle and two buttons control the board through callbacks.");
  PhyphoxBleExperiment::View view("Controls");

  PhyphoxBleExperiment::Graph graph("Output");
  graph.setChannel(0, 1);
  graph.setLabelX("time"); graph.setUnitX("s");
  graph.setLabelY("value");

  PhyphoxBleExperiment::Slider gainSlider("Gain", 0, 5, 0.5f, /*input channel*/ 1);
  gainSlider.setDefault(1);
  gainSlider.onChange(onGain);

  PhyphoxBleExperiment::Toggle enableToggle("Enabled", 2);
  enableToggle.setDefault(1);
  enableToggle.onChange(onEnabled);

  PhyphoxBleExperiment::Button zeroButton("Set zero here", 3);
  zeroButton.onPress(onZero);

  PhyphoxBleExperiment::Button resetButton("Reset counter", 4);
  resetButton.onPress(onReset);

  PhyphoxBleExperiment::Value counterValue("Counter", 2);
  counterValue.setPrecision(0);

  view.addElement(graph).addElement(gainSlider).addElement(enableToggle)
      .addElement(zeroButton).addElement(resetButton).addElement(counterValue);
  exp.addView(view);

  PhyphoxBLE::start("phyphox buttons", exp);
  PhyphoxBLE::printErrors(&Serial);
}

void loop() {
  last = 50.0f + 40.0f * sin(millis() / 1000.0f);
  float value = enabled ? gain * (last - offset) : 0.0f;
  counter += 1;
  PhyphoxBLE::write(value, counter);
  delay(50);
  PhyphoxBLE::poll();
}
