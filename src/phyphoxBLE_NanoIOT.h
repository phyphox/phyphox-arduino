#ifndef PHYPHOXBLE_NANOIOT_H
#define PHYPHOXBLE_NANOIOT_H

#include <phyphoxBle.h>
#include "phyphoxBleExperiment.h"

class PhyphoxBLE
{
    private:

        static uint8_t data_package[20];

        static void controlCharacteristicWritten(BLEDevice, BLECharacteristic);
        static void eventCharacteristicWritten(BLEDevice, BLECharacteristic);
        static void configCharacteristicWritten(BLEDevice, BLECharacteristic);

		static BLEService phyphoxExperimentService;
        static BLECharacteristic experimentCharacteristic;
        static BLECharacteristic controlCharacteristic;
        static BLECharacteristic eventCharacteristic;

        static BLEService phyphoxDataService;
		static BLECharacteristic dataCharacteristic;
		static BLECharacteristic configCharacteristic;
		

        static uint8_t* data; //this pointer points to the data the user wants to write in the characteristic
        static uint8_t* p_exp; //this pointer will point to the byte array which holds an experiment

        static size_t expLen; //try o avoid this maybe use std::array or std::vector
        static char *EXPARRAY;

    public:
        
        static void start(const char* DEVICE_NAME, uint8_t* p, size_t n = 0); 
        static void start(const char* DEVICE_NAME);
        static void start(uint8_t* p, size_t n = 0); 
        static void start();

        static void addExperiment(PhyphoxBleExperiment&);
        static void transferExperiment();
        static void write(float&);
        static void write(float&, float&);
        static void write(float&, float&, float&);
        static void write(float&, float&, float&, float&);
        static void write(float&, float&, float&, float&, float&);

        static void read(uint8_t*, unsigned int);
        static void read(float&);
        static void read(float&, float&);
        static void read(float&, float&, float&);
        static void read(float&, float&, float&, float&);
        static void read(float&, float&, float&, float&, float&);
        

        static void poll();
        static void poll(int timeout);

        static void(*configHandler)();
        static void(*experimentEventHandler)();

        static uint16_t minConInterval;
        static uint16_t maxConInterval;
        static uint16_t slaveLatency;
        static uint16_t timeout;
        static uint16_t MTU;

        static int64_t experimentTime;
        static int64_t systemTime;
        static uint8_t eventType;

        static uint8_t eventData[17];

        static void printXML(HardwareSerial*);

};


#endif
