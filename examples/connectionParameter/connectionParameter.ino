#include <phyphoxBle.h> 

void setup() {
    /*
    Minimum interval between two connection events allowed for a connection.
    It shall be less than or equal to maxConnectionInterval. This value, in units of 1.25ms, is included in the range [0x0006 : 0x0C80].  
    */
    PhyphoxBLE::minConInterval = 6; //6 = 7.5ms

    /*Maximum interval between two connection events allowed for a connection.
      It shall be greater than or equal to minConnectionInterval. This value is in unit of 1.25ms and is in the range [0x0006 : 0x0C80]. 
     */
    PhyphoxBLE::maxConInterval = 24; //10 = 12.5ms

    /*
    Number of connection events the slave can drop if it has nothing to communicate to the master.
    This value shall be in the range [0x0000 : 0x01F3].
    */    
    PhyphoxBLE::slaveLatency = 0; //
    /*
    Link supervision timeout for the connection.
    Time after which the connection is considered lost if the device didn't receive a packet from its peer.
    This value is in the range [0x000A : 0x0C80] and is in unit of 10 ms.
    */
    PhyphoxBLE::timeout = 50; //50 = 500ms

    
    PhyphoxBLE::start();                 //Start the BLE server
}

void loop() {
	float randomNumber = random(0,100); //Generate random number in the range 0 to 100
	PhyphoxBLE::write(randomNumber);     //Send value to phyphox
	delay(20);                          //Shortly pause before repeating
}