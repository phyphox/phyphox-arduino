#ifdef ESP32
#ifndef PHYPHOXBLE_ESP32_H
#define PHYPHOXBLE_ESP32_H

#include <phyphoxBle.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <esp_system.h>
#include <BLE2902.h>
#include "element.h"	
#include "graph.h"
#include "view.h"
#include "experiment.h"

using std::copy;


class BleServer
{
    private:

        const char *customServiceUUID = "cddf0001-30f7-4671-8b43-5e40ba53514a";
        const char *phyphoxUUID = "cddf0002-30f7-4671-8b43-5e40ba53514a";        
        uint8_t data_package[20] = {0};
        char DEVICE_NAME[20] = "phyphox";    
        const char *dataOneUUID = "59f51a40-8852-4abe-a50f-2d45e6bd51ac";
        const char *configUUID = "59f51a40-8852-4abe-a50f-2d45e6bd51ad";

        BLEServer *myServer;
        BLEService *myService;
        BLEDescriptor *myExperimentDescriptor;
        BLEDescriptor *myDataDescriptor;
        BLEDescriptor *myConfigDescriptor;
        BLECharacteristic *dataCharacteristic;
        BLECharacteristic *experimentCharacteristic;
        BLECharacteristic *configCharacteristic;
        BLEAdvertising *myAdvertising;

        //     void when_connected();
        //     virtual void onDisconnectionComplete();
        //     virtual void onConnectionComplete();
        uint8_t* data = nullptr; //this pointer points to the data the user wants to write in the characteristic
        uint8_t* p_exp = nullptr; //this pointer will point to the byte array which holds an experiment
        TaskHandle_t TaskTransfer;


        size_t expLen = 0; //try o avoid this maybe use std::array or std::vector
        uint8_t EXPARRAY[4000] = {0};// block some storage

    public:
        BleServer() {};
        BleServer(const char* s) {strcpy(DEVICE_NAME, s);};
        BleServer(const BleServer&) = delete; //there is no need to copy a BleServer once established
        BleServer &operator=(const BleServer&) = delete; //there is no need to assign a BleServer to a BleServer
        ~BleServer() = default; //no dynamic memory allocation 
        void start(uint8_t* p = nullptr, size_t n = 0); 

        void when_subscription_received();
        void addExperiment(Experiment&);

        void write(float&);
        void write(float&, float&);
        void write(float&, float&, float&);
        void write(float&, float&, float&, float&);
        void write(float&, float&, float&, float&, float&);

        void read(uint8_t*, unsigned int);
        void read(float&);
        void configHandlerDebug();
        void (*configHandler)() = nullptr;

        void startTask();
        static void staticStartTask(void*);

};


#endif
#endif
