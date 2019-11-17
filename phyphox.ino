#include <phyphoxBle.h>
BleServer server;


void setup()
{
	server.start();
}

void loop()
{
	//sensoren auslesen z.B. Spannungsteiler an A1 angschlossen
	int value = analogRead(A1);
	float voltage = value * (3.3 / 1023.0);
	float pressure = 1.5*voltage;
	float hum = 2 *voltage;
	
	server.write(voltage);
	delay(500);
	
}
