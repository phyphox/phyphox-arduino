#include <phyphoxBle.h> 
BleServer server; //Server anlegen

void setup()
{
  server.start(); //server starten...wer hätte es gedacht
}

void loop()
{
	int val = analogRead(A1);
	float voltage = 3.3*val/1023.;
	server.write(voltage);
	delay(500);
}
