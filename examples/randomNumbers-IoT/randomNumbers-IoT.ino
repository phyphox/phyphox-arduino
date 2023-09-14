#include <phyphoxBle.h> 

void setup() {
    PhyphoxBLE::start();                 //Start the BLE server
}

void loop() {
    float randomNumber = random(0,100); //Generate random number in the range 0 to 100
    PhyphoxBLE::write(randomNumber);     //Send value to phyphox
    delay(50);                          //Shortly pause before repeating

    PhyphoxBLE::poll();                  //IMPORTANT: In contrast to other devices, poll() needs to be called periodically on the 33 IoT
}
