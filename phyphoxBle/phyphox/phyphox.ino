#include <phyphoxBle.h> 
BleServer server; //Server anlegen

void setup()
{
  HardwareSerial* ser = &Serial;
  server.begin(ser);
  server.start(); //server starten...wer hätte es gedacht
}

void loop()
{
	//Spannungsteiler mit Poti auslesen 
	int value = analogRead(A1);
	float voltage = value * (3.3 / 1023.0);
	//hier kann man prinzipipiell natürlich was anderes auslesen (Temp,...)

 
	//Daten an Server senden und in phyphox sichtbar machen
	server.write(voltage);
	delay(100); //50ms warten
	
}
