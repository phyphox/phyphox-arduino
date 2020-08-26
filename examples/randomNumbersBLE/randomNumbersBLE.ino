#include <phyphoxBle.h> 
//BleServer server("test"); //Server anlegen

void setup()
{
   Serial.begin(115200);
  BleServer::start(); //server starten
}

void loop()
{
	  BLEDevice central = BLE.central();

	float randomNumber = random(0,100); //Erzeuge Zufallszahlen zwischen 0 - 100
	BleServer::write(randomNumber);
	delay(200);
 uint8_t readArray[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 BleServer::read(&readArray[0],20);

 for(int i=0; i<20; i++){
    Serial.print(readArray[i]);  
  }
  Serial.println(""); 
}
