#ifndef NINAB31SERIAL_H
#define NINAB31SERIAL_H

#include <Arduino.h>


class NINAB31Serial
{
	
	private:

    static String m_input;
    static bool checkUnsolicited();
    static bool connected;

	public:
    static bool begin();
    static bool setLocalName(String name);
    static bool writeValue(int characteristic, String value);
    static bool writeValue(int characteristic, uint8_t* value, int len);
    static bool setConnectionInterval(int minInterval, int maxInterval);

    static int parseResponse(String msg, uint32_t timeout);
    static bool checkResponse(String msg, uint32_t timeout);
    static bool poll();
    static String checkCharWritten(int handle);
    static void flushInput();
    static bool advertise();
    static bool stopAdvertise();
    
};

#endif
