#ifdef ESP32
#ifndef PHYPHOXBLE_ESP32_H
#define PHYPHOXBLE_ESP32_H

#include <phyphoxBle.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <esp_system.h>
#include <BLE2902.h>
#include "phyphoxBleExperiment.h"	

using std::copy;


class PhyphoxBLE
{
    private:

        static uint8_t data_package[20];

        static BLEServer *myServer;
        static BLEService *phyphoxDataService;
        static BLEService *phyphoxExperimentService;
        static BLEDescriptor *myExperimentDescriptor;
        static BLEDescriptor *myDataDescriptor;
        static BLEDescriptor *myEventDescriptor;
        static BLEDescriptor *myConfigDescriptor;
        static BLECharacteristic *dataCharacteristic;
        static BLECharacteristic *experimentCharacteristic;
        static BLECharacteristic *eventCharacteristic;
        static BLECharacteristic *configCharacteristic;
        static BLEAdvertising *myAdvertising;

        //     void when_connected();
        //     virtual void onDisconnectionComplete();
        
        static uint8_t* data; //this pointer points to the data the user wants to write in the characteristic
        static uint8_t* p_exp; //this pointer will point to the byte array which holds an experiment
        static TaskHandle_t TaskTransfer;

        static uint8_t *EXPARRAY;

        static size_t expLen; //try o avoid this maybe use std::array or std::vector

    public:

        static void disconnected();
        static void start(const char* DEVICE_NAME, uint8_t* p, size_t n = 0); 
        static void start(const char* DEVICE_NAME);
        static void start(uint8_t* p, size_t n = 0); 
        static void start();

        static void when_subscription_received();
        static void addExperiment(PhyphoxBleExperiment&);

        static void poll();
        static void poll(int timeout);

        static void write(float&);
        static void write(float&, float&);
        static void write(float&, float&, float&);
        static void write(float&, float&, float&, float&);
        static void write(float&, float&, float&, float&, float&);
		static void write(uint8_t *, unsigned int );
        static void write(float *,unsigned int);


        static void read(uint8_t*, unsigned int);
        static void read(float&);
        static void read(float&, float&);
        static void read(float&, float&, float&);
        static void read(float&, float&, float&, float&);
        static void read(float&, float&, float&, float&, float&);
        static void configHandlerDebug();
        static void (*configHandler)();

        static void eventCharacteristicHandler();
        static void (*experimentEventHandler)();

        static void setMTU(uint16_t);
        static uint16_t MTU;

        static void startTask();
        static void staticStartTask(void*);

        static HardwareSerial* printer; //for debug purpose
        static void begin(HardwareSerial*); //for debug purpose

        static uint16_t minConInterval;
        static uint16_t maxConInterval;
        static uint16_t slaveLatency;
        static uint16_t timeout;
        static uint16_t currentConnections;
        static bool     isSubscribed;
        
        static uint8_t eventData[17];
        static int64_t experimentTime;
        static int64_t systemTime;
        static uint8_t eventType;
};


#endif
#endif
