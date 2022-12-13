#include <phyphoxBle.h> 

void setup() {
  Serial.begin(115200);
  PhyphoxBLE::start();
  PhyphoxBLE::experimentEventHandler = &newExperimentEvent; // declare which function should be called after receiving an experiment event 
  PhyphoxBLE::printXML(&Serial);
}

void loop() {
  float randomNumber = random(0,100); //Generate random number in the range 0 to 100
	PhyphoxBLE::write(randomNumber);     //Send value to phyphox
  delay(100);
}

//declare function which is called after phyphox wrote to the event characteristic
void newExperimentEvent(){
  Serial.println("New experiment event received:");
  Serial.print("Event type: ");
  Serial.print(PhyphoxBLE::eventType);
  Serial.println(" (0 = Paused, 1 = Started, 255 = SYNC)");
  Serial.print("Experiment time [ms]: ");
  Serial.println(PhyphoxBLE::experimentTime);
  Serial.print("Unix system time [ms]: ");
  Serial.println(PhyphoxBLE::systemTime);
}