#ifndef PHYPHOXBLE_NANOIOT_H
#define PHYPHOXBLE_NANOIOT_H

#include <phyphoxBle.h>

//#include <string>

#include "element.h"	
#include "graph.h"
#include "view.h"
#include "experiment.h"

//using std::copy;



class BleServer
{
    private:

        static uint8_t data_package[20];// = {0};
        static inline char DEVICE_NAME[20] = "phyphox"; 


        static void controlCharacteristicWritten(BLEDevice, BLECharacteristic);

		static inline BLEService phyphoxService{"CDDF0001-30F7-4671-8B43-5E40BA53514A"}; // create service
		static inline BLECharacteristic dataCharacteristic{"59f51a40-8852-4abe-a50f-2d45e6bd51ac", BLERead | BLEWrite | BLENotify, 20, false};
		static inline BLECharacteristic experimentCharacteristic{"cddf0002-30f7-4671-8b43-5e40ba53514a", BLERead | BLEWrite| BLENotify, 20, false};
		static inline BLECharacteristic configCharacteristic{"59f51a40-8852-4abe-a50f-2d45e6bd51ad", BLERead | BLEWrite| BLENotify, 20, false};
		static inline BLECharacteristic controlCharacteristic{"cddf0003-30f7-4671-8b43-5e40ba53514a", BLERead | BLEWrite| BLENotify, 20, false};

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
        
        //static void start(uint8_t* p = nullptr, size_t n = 0); 
        static void start(); 

        static void addExperiment(Experiment&);
        static void transferExperiment();
        static void write(float&);
        static void write(float&, float&);
        static void write(float&, float&, float&);
        static void write(float&, float&, float&, float&);
        static void write(float&, float&, float&, float&, float&);

        static void read(uint8_t*, unsigned int);
        //static void read(float&);
        //void configHandlerDebug();
        //void (*configHandler)() = nullptr;

        static void poll();


};


#endif