#include <phyphoxBle.h> 
BleServer server("phyphox"); //Init server

/*  In this example we will send the smartphones acceleroation in z direction to our mikrocontroller.
 *  If the acceleration is larger than 0.5*9.81m/s² the led is turned off.
 *  The needed phyphox experiment can be found in the examples folder. Furthermore there is a qr code which can be used to import the 
 *  experiment into phyhox.
 */

void receivedData();


void setup()
{
   server.start();
   pinMode(LED_BUILTIN, OUTPUT);

}


void loop()
{
  //do nothing   

}

void receivedData(){
   float accelerationZ;
   server.read(accelerationZ);
   bool led;

   if(accelerationZ > 0.5*9.81){
    led = true;
   }else{
    led = false;
   }
   digitalWrite(LED_BUILTIN, led);
  }
