#include <phyphoxBle.h> 

void setup() {
    PhyphoxBLE::begin(&Serial);
    delay(3000);
    PhyphoxBLE::start();                 //Start the BLE server

}

void loop() {
	float randomNumber = random(0,100); //Generate random number in the range 0 to 100
	PhyphoxBLE::write(randomNumber);     //Send value to phyphox
	delay(50);                          //Shortly pause before repeating
}
