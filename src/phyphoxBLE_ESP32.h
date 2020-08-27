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

        static uint8_t data_package[20];

        static constexpr char *phyphoxExperimentServiceUUID = "cddf0001-30f7-4671-8b43-5e40ba53514a";
        static constexpr  char *experimentCharacteristicUUID = "cddf0002-30f7-4671-8b43-5e40ba53514a";        
        
        static constexpr char *phyphoxDataServiceUUID = "cddf1001-30f7-4671-8b43-5e40ba53514a";
        static constexpr  char *dataCharacteristicUUID = "cddf1002-30f7-4671-8b43-5e40ba53514a";
        static constexpr  char *configCharacteristicUUID = "cddf1003-30f7-4671-8b43-5e40ba53514a";

        static BLEServer *myServer;
        static BLEService *phyphoxDataService;
        static BLEService *phyphoxExperimentService;
        static BLEDescriptor *myExperimentDescriptor;
        static BLEDescriptor *myDataDescriptor;
        static BLEDescriptor *myConfigDescriptor;
        static BLECharacteristic *dataCharacteristic;
        static BLECharacteristic *experimentCharacteristic;
        static BLECharacteristic *configCharacteristic;
        static BLEAdvertising *myAdvertising;

        //     void when_connected();
        //     virtual void onDisconnectionComplete();
        //     virtual void onConnectionComplete();
        static uint8_t* data; //this pointer points to the data the user wants to write in the characteristic
        static uint8_t* p_exp; //this pointer will point to the byte array which holds an experiment
        static TaskHandle_t TaskTransfer;

        static char *DEVICE_NAME; //[20] = {"phyphox"};
        static uint8_t *EXPARRAY;

        static size_t expLen; //try o avoid this maybe use std::array or std::vector

    public:

        BleServer() {};
        BleServer(const char* s) {strcpy(DEVICE_NAME, s);};
        BleServer(const BleServer&) = delete; //there is no need to copy a BleServer once established
        BleServer &operator=(const BleServer&) = delete; //there is no need to assign a BleServer to a BleServer
        ~BleServer() = default; //no dynamic memory allocation 

        static void start(uint8_t* p, size_t n = 0); 
        static void start(); 
        static void when_subscription_received();
        static void addExperiment(Experiment&);


        static void write(float&);
        static void write(float&, float&);
        static void write(float&, float&, float&);
        static void write(float&, float&, float&, float&);
        static void write(float&, float&, float&, float&, float&);

        static void read(uint8_t*, unsigned int);
        static void read(float&);
        static void configHandlerDebug();
        static void (*configHandler)();

        static void startTask();
        static void staticStartTask(void*);

};


#endif
#endif
