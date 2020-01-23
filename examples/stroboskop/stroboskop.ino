#include <phyphoxBle.h> 
BleServer server("Strobo"); //Server anlegen
float frequenz = 0;

void setup()
{
  Serial.begin(115200);
  server.start(); //server starten
}

void loop()
{
  frequenz = server.read();
  Serial.println(frequenz);
	delay(500);
}
