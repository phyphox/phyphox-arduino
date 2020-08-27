#ifndef PHYPHOXBLE_NANOIOT_H
#define PHYPHOXBLE_NANOIOT_H

#include <phyphoxBle.h>

#include "element.h"	
#include "graph.h"
#include "view.h"
#include "experiment.h"

class BleServer
{
    private:

        static uint8_t data_package[20];// = {0};
        static inline char DEVICE_NAME[20] = "phyphox"; 


        static void controlCharacteristicWritten(BLEDevice, BLECharacteristic);
        static void configCharacteristicWritten(BLEDevice, BLECharacteristic);

		static inline BLEService phyphoxExperimentService{"cddf0001-30f7-4671-8b43-5e40ba53514a"}; // create service
        static inline BLECharacteristic experimentCharacteristic{"cddf0002-30f7-4671-8b43-5e40ba53514a", BLERead | BLEWrite| BLENotify, 20, false};
        static inline BLECharacteristic controlCharacteristic{"cddf0003-30f7-4671-8b43-5e40ba53514a", BLERead | BLEWrite| BLENotify, 20, false};

        static inline BLEService phyphoxDataService{"cddf1001-30f7-4671-8b43-5e40ba53514a"}; // create service
		static inline BLECharacteristic dataCharacteristic{"cddf1002-30f7-4671-8b43-5e40ba53514a", BLERead | BLEWrite | BLENotify, 20, false};
		static inline BLECharacteristic configCharacteristic{"cddf1003-30f7-4671-8b43-5e40ba53514a", BLERead | BLEWrite| BLENotify, 20, false};
		

        static inline uint8_t* data = nullptr; //this pointer points to the data the user wants to write in the characteristic
        static inline uint8_t* p_exp = nullptr; //this pointer will point to the byte array which holds an experiment

        static inline size_t expLen = 0; //try o avoid this maybe use std::array or std::vector
        static inline uint8_t EXPARRAY[4000] = {0};// block some storage

    public:

        BleServer() {};
        BleServer(const char* s) {strcpy(DEVICE_NAME, s);};
        BleServer(const BleServer&) = delete; //there is no need to copy a BleServer once established
        BleServer &operator=(const BleServer&) = delete; //there is no need to assign a BleServer to a BleServer
        ~BleServer() = default; //no dynamic memory allocation 
        
        static void start(uint8_t* p, size_t n = 0); 
        static void start(); 

        static void addExperiment(Experiment&);
        static void transferExperiment();
        static void write(float&);
        static void write(float&, float&);
        static void write(float&, float&, float&);
        static void write(float&, float&, float&, float&);
        static void write(float&, float&, float&, float&, float&);

        static void read(uint8_t*, unsigned int);
        static void read(float&);
        
        

        static void poll();

        static inline void(*configHandler)() = nullptr;
        
};


#endif