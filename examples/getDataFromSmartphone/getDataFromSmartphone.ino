#include <phyphoxBle.h>

// Some boards define no LED_BUILTIN (the generic ESP32 board, the senseBox MCU-S2): blink
// GPIO 2 there, or change it to the LED of your board.
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

/*  In this example we can change the blink interval of our mikrocontroller via phyphox
 */

void receivedData();

long lastTimestamp = 0;
float blinkInterval = 100;
bool ledState = true;
bool ledToggleState = true;
int mode = 1;

void setup() {

  Serial.begin(115200);
  PhyphoxBLE::start();
  PhyphoxBLE::configHandler=&receivedData;
  pinMode(LED_BUILTIN, OUTPUT);
   
  //Experiment
  PhyphoxBleExperiment getDataFromSmartphone;
  getDataFromSmartphone.setTitle("Set Blink Interval");
  getDataFromSmartphone.setCategory("Arduino Experiments");
  getDataFromSmartphone.setDescription("User can set Blink Interval of Mikrocontroller LED");

  //View
  PhyphoxBleExperiment::View firstView;
  firstView.setLabel("FirstView"); //Create a "view"

  //Edit
  PhyphoxBleExperiment::Edit intervalEdit;
  intervalEdit.setLabel("Interval");
  intervalEdit.setDefault(100);
  intervalEdit.setUnit("ms");
  intervalEdit.setSigned(false);
  intervalEdit.setDecimal(false);
  intervalEdit.setChannel(1);

  //Slider controlling the same value as the edit element (channel 1)
  PhyphoxBleExperiment::Slider intervalSlider;
  intervalSlider.setLabel("Interval");
  intervalSlider.setDefault(100);
  intervalSlider.setMinValue(50);
  intervalSlider.setMaxValue(1000);
  intervalSlider.setStepSize(50);
  intervalSlider.setPrecision(0);
  intervalSlider.setShowValue(false);
  intervalSlider.setChannel(1);

  //Toggle
  PhyphoxBleExperiment::Toggle ledToggle;
  ledToggle.setLabel("Enable LED");
  ledToggle.setDefault(1);
  ledToggle.setChannel(2);

  //Dropdown
  PhyphoxBleExperiment::Dropdown modeDropdown;
  modeDropdown.setLabel("Mode");
  const char* const optionLabels[] = {"Always On", "Blink", "Flash"};
  const float optionValues[] = {1.0f, 2.0f, 3.0f};
  modeDropdown.setOptions(3, optionLabels, optionValues);
  modeDropdown.setChannel(3);

  firstView.addElement(intervalEdit);
  firstView.addElement(intervalSlider);
  firstView.addElement(ledToggle);
  firstView.addElement(modeDropdown);
  getDataFromSmartphone.addView(firstView);              //attach view to experiment
  PhyphoxBLE::addExperiment(getDataFromSmartphone);      //attach experiment to server
}


void loop() {
   PhyphoxBLE::poll(); //Only required for the Arduino Nano 33 IoT, but it does no harm for other boards.

   switch (mode) {
      case 1: //Always On if toggle is on
         ledState = ledToggleState;
         break;
      case 2: //Blink, i.e. toggle after each interval
         if (millis() - lastTimestamp > blinkInterval) {
            lastTimestamp = millis();
            ledState = (!ledState) && ledToggleState;
         }
         break;
      case 3: //Flash, i.e. Short flash every interval
         ledState = (millis() - lastTimestamp < 10) && ledToggleState;
         if (millis() - lastTimestamp > blinkInterval) {
            lastTimestamp = millis();
         }
         break;
   }
   digitalWrite(LED_BUILTIN, ledState);
}

void receivedData() {
   float receivedInterval;
   float receivedToggleState;
   float receivedMode;
   PhyphoxBLE::read(receivedInterval, receivedToggleState, receivedMode);
   if (blinkInterval > 0) {
     blinkInterval = receivedInterval;
   }
   ledToggleState = (receivedToggleState != 0);
   mode = receivedMode;
   Serial.print("Mode: ");
   Serial.print(mode);
   Serial.print(", LED state: ");
   Serial.print(ledToggleState);
   Serial.print(", interval: ");
   Serial.println(blinkInterval);
}
