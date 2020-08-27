#include <phyphoxBle.h> 

/*  In this example we will send the smartphones acceleroation in z direction to our mikrocontroller.
 *  If the acceleration is larger than 0.5*9.81m/s² the led is turned off.
 *  The needed phyphox experiment can be found in the examples folder. Furthermore there is a qr code which can be used to import the 
 *  experiment into phyhox.
 */

void receivedData();


void setup()
{
   PhyphoxBLE::start();
   PhyphoxBLE::configHandler=&receivedData;
   pinMode(LED_BUILTIN, OUTPUT);
}


void loop()
{
    PhyphoxBLE::poll(); //Only required for the Arduino Nano 33 IoT, but it does no harm for other boards.
}

void receivedData(){
   float accelerationZ;
   PhyphoxBLE::read(accelerationZ);
   bool led;

   if(accelerationZ > 0.5*9.81){
    led = true;
   }else{
    led = false;
   }
   digitalWrite(LED_BUILTIN, led);
}
