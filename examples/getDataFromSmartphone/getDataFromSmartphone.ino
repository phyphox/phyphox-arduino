#include <phyphoxBle.h> 

/*  In this example we can change the blink interval of our mikrocontroller via phyphox
 */

void receivedData();

long lastTimestamp = 0;
float blinkInterval = 100;
bool led = true;

void setup()
{
   PhyphoxBLE::begin(&Serial);
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
  PhyphoxBleExperiment::Edit Interval;
  Interval.setLabel("Interval");
  Interval.setUnit("ms");
  Interval.setSigned(false);
  Interval.setDecimal(false);
  Interval.setChannel(1);

  firstView.addElement(Interval);
  getDataFromSmartphone.addView(firstView);         //attach view to experiment
  PhyphoxBLE::addExperiment(getDataFromSmartphone);      //attach experiment to server
}


void loop()
{
    PhyphoxBLE::poll(); //Only required for the Arduino Nano 33 IoT, but it does no harm for other boards.

    if(millis()-lastTimestamp>blinkInterval){
      lastTimestamp = millis();
      led=!led;
      digitalWrite(LED_BUILTIN, led);
      }
    
}

void receivedData(){
   float receivedInterval;
   PhyphoxBLE::read(receivedInterval);
   if(receivedInterval>0){
    blinkInterval =receivedInterval;
    }
   Serial.println(blinkInterval);
}