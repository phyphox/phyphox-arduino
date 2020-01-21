#include <phyphoxBle.h> 
BleServer server("BLE-Name"); //Server anlegen

void setup()
{
  server.start(); //server starten
}

void loop()
{
	
	float randomNumber = random(0,100); //Erzeuge Zufallszahlen zwischen 0 - 100
	server.write(randomNumber);
	delay(50);
}
