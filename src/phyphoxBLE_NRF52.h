#ifndef PHYPHOXBLE_NRF52_H
#define PHYPHOXBLE_NRF52_H
#define NDEBUG

#include <phyphoxBle.h>
#include <mbed.h>
#include <ble/BLE.h>
#include <ble/GattServer.h>
#include <Gap.h>
#include <algorithm>
#include <UUID.h>
#include <string>
#include <AdvertisingParameters.h>
#include <AdvertisingDataBuilder.h>
#include <HardwareSerial.h>

#include "phyphoxBleExperiment.h"

#ifndef NDEBUG
using arduino::HardwareSerial;
#endif
using events::EventQueue; 
using rtos::Thread;
using mbed::callback;
using std::copy;

#ifndef DATASIZE
#define DATASIZE 20
#endif

class PhyphoxBleEventHandler : public ble::Gap::EventHandler {
    public:

    virtual void onDisconnectionComplete(const ble::DisconnectionCompleteEvent&);
    virtual void onConnectionComplete(const ble::ConnectionCompleteEvent&);
    PhyphoxBleEventHandler(BLE& ble):
        ble(ble) {
    }

    private: 
    BLE& ble;
};

class PhyphoxBLE
{
	
	private:

    static PhyphoxBleEventHandler eventHandler;


	static const UUID phyphoxExperimentServiceUUID;
	static const UUID phyphoxDataServiceUUID;

	static const UUID experimentCharacteristicUUID;
	static const UUID dataCharacteristicUUID;
	static const UUID configCharacteristicUUID;

    static char name[50];

	static uint8_t data_package[20];
	static uint8_t config_package[CONFIGSIZE];

	/*BLE stuff*/
	static BLE& ble;
	static ReadWriteArrayGattCharacteristic<uint8_t, sizeof(data_package)> dataCharacteristic; //Note: Use { } instead of () google most vexing parse
	static uint8_t readValue[DATASIZE];
	static ReadWriteArrayGattCharacteristic<uint8_t, sizeof(config_package)> configCharacteristic;
	static ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(readValue)> experimentCharacteristic;

	static Thread bleEventThread;
	static Thread transferExpThread;
	static EventQueue queue;
	/*end BLE stuff*/
	static EventQueue transferQueue;

	
	//helper function to initialize BLE server and for connection poperties
	static void bleInitComplete(BLE::InitializationCompleteCallbackContext*);
	static void when_subscription_received(GattAttribute::Handle_t);
	static void configReceived(const GattWriteCallbackParams *params);
   
	//helper functon that runs in the thread ble_server
	//static void waitForEvent();
	static void transferExp();
	static GattCharacteristic* phyphoxCharacteristics[];
	static GattService phyphoxService;

	static GattCharacteristic* phyphoxDataCharacteristics[];
	static GattService phyphoxDataService;
	
	static void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context);

	#ifndef NDEBUG
	static inline HardwareSerial* printer; //for debug purpose
	#endif
	static uint8_t* data; //this pointer points to the data the user wants to write in the characteristic
	static uint8_t* config;
	static uint8_t* p_exp; //this pointer will point to the byte array which holds an experiment


	public:

	static uint8_t EXPARRAY[4096];// block some storage
	static size_t expLen; //try o avoid this maybe use std::array or std::vector

	static inline uint16_t minConInterval = 6;	//7.5ms
	static inline uint16_t maxConInterval = 24; //30ms
	static inline uint16_t slaveLatency = 0;
	static inline uint16_t timeout = 50;

	static void (*configHandler)();

    static void poll();
    static void poll(int timeout);

    static void start(const char* DEVICE_NAME, uint8_t* p, size_t n = 0); 
    static void start(const char* DEVICE_NAME);
    static void start(uint8_t* p, size_t n = 0); 
    static void start();

	static void write(uint8_t*, unsigned int);	
	static void write(float&);
	static void write(float&, float&, float&, float&, float&);
	static void write(float&, float&, float&, float&);
	static void write(float&, float&, float&);
	static void write(float&, float&);
	static void read(uint8_t*, unsigned int);
	static void read(float&);

	static void addExperiment(PhyphoxBleExperiment&);
	#ifndef NDEBUG
	static void begin(HardwareSerial*); //for debug purpose
	static void output(const char*); //for debug purpose
	static void output(const uint32_t);
	#endif
};

#endif
