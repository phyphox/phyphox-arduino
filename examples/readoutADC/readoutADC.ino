#include <phyphoxBle.h>

// The pin to measure and the ADC's full scale. Pin 25 is an ADC input on the ESP32 only:
// on the Nano 33 IoT it is one of the pins that talk to the Bluetooth module, and reading
// it as an analogue input silences the radio (found on the bench, 2026-09-13).
#if defined(ESP32)
int ADC_GPIO = 25;
const float ADC_MAX = 4095;   // 12 bit
#else
int ADC_GPIO = A0;
const float ADC_MAX = 1023;   // 10 bit by default on the Arduino boards
#endif

void setup() {

  
  // put your setup code here, to run once:
  Serial.begin(115200);
  PhyphoxBLE::start("Voltmeter");

  PhyphoxBleExperiment Voltmeter;

  Voltmeter.setTitle("Voltmeter");
  Voltmeter.setCategory("Arduino Experiments");
  Voltmeter.setDescription("This experiment will plot the measured voltage over time.");

  //View
  PhyphoxBleExperiment::View firstView;
  firstView.setLabel("Rawdata"); //Create a "view"

  //Graph
  PhyphoxBleExperiment::Graph firstGraph;      //Create graph which will plot random numbers over time
  firstGraph.setLabel("Voltmeter");
  firstGraph.setUnitX("s");
  firstGraph.setUnitY("V");
  firstGraph.setLabelX("time");
  firstGraph.setLabelY("Voltage");

  /* Assign Channels, so which data is plotted on x or y axis
     first parameter represents x-axis, second y-axis
     Channel 0 means a timestamp is created after the BLE package arrives in phyphox
     Channel 1 to N corresponding to the N-parameter which is written in server.write()
  */

  firstGraph.setChannel(0, 1);

  firstView.addElement(firstGraph);            //attach graph to view
  Voltmeter.addView(firstView);                //Attach view to experiment
  PhyphoxBLE::addExperiment(Voltmeter);        //Attach experiment to server

  

}

void loop() {
  // put your main code here, to run repeatedly:

  float voltage = 3.3 * analogRead(ADC_GPIO) / ADC_MAX;
  delay(1);
  
  PhyphoxBLE::write(voltage);

  Serial.print("Voltage = ");
  Serial.println(voltage);

  delay(20);
  PhyphoxBLE::poll();                          //Required on the boards with ArduinoBLE; harmless elsewhere
}