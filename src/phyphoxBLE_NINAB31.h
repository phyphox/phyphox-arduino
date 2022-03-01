#ifndef PHYPHOXBLE_NINAB31_H
#define PHYPHOXBLE_NINAB31_H

#include <phyphoxBle.h>
#include "phyphoxBleExperiment.h"
#include "NINAB31serial.h"

class PhyphoxBLE
{
    private:

        static uint8_t data_package[20];
        
        static NINAB31Serial port;
        
        static void controlCharacteristicWritten();
        static void configCharacteristicWritten();

        static int h_phyphoxExperimentService;
        static int h_experimentCharacteristic;
        static int h_controlCharacteristic;

        static int h_phyphoxDataService;
        static int h_dataCharacteristic;
        static int h_configCharacteristic;
        static bool exploaded;

        static uint8_t* data; //this pointer points to the data the user wants to write in the characteristic
        static uint8_t* p_exp; //this pointer will point to the byte array which holds an experiment

        static size_t expLen; //try o avoid this maybe use std::array or std::vector
        static uint8_t EXPARRAY[4000];// block some storage

        static uint8_t controlCharValue[21];
        static uint8_t configCharValue[21];

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
        
        

        static void poll();
        static void poll(int timeout);

        static void(*configHandler)();
        static uint16_t minConInterval;
        static uint16_t maxConInterval;
        static uint16_t slaveLatency;
        static uint16_t timeout;
        
};


#endif
