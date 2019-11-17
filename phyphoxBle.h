#ifndef PHYPHOXBLE_H
#define PHYPHOXBLE_H

#include <mbed.h>
#include <ble/BLE.h>
#include <ble/GattServer.h>
#include <Gap.h>
#include <algorithm>
#include <UUID.h>
#include <string>
#include <HardwareSerial.h>
#include <CRC32.h> //todo: own crc32 checksum

#ifndef NDEBUG
using arduino::HardwareSerial;
#endif
using events::EventQueue; 
using rtos::Thread;
using mbed::callback;
using std::copy;

class BleServer
{
	private:
	/* Members every server need to have
	 * phyphox service uuid
	 * phyphox uuid for experiment transfer
	 * a buffer that hlds the data_packages which contans 20bytes
	 * an underlying BLE object 
	 * a characteristic to that phyphox subscribes named data char
	 */
    const uint8_t customServiceUUID[UUID::LENGTH_OF_LONG_UUID]= {0xcd, 0xdf, 0x00, 0x01, 0x30, 0xf7, 0x46, 0x71, 0x8b, 0x43, 0x5e, 0x40, 0xba, 0x53, 0x51, 0x4a};                                                
	const UUID phyphoxUUID = UUID("cddf0002-30f7-4671-8b43-5e40ba53514a");
	uint8_t data_package[20] = {0};
	const char DEVICE_NAME[20] = "Arduino";
	const UUID dataOneUUID = UUID("59f51a40-8852-4abe-a50f-2d45e6bd51ac");
	
	/*BLE stuff*/
	BLE& ble = BLE::Instance(BLE::DEFAULT_INSTANCE);
    ReadWriteArrayGattCharacteristic<uint8_t, sizeof(data_package)> dataChar{phyphoxUUID, data_package, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY}; //Note: Use { } instead of () google most vexing parse
	/*
	float readValue = 0.0;
	ReadOnlyGattCharacteristic<float> readCharOne{dataOneUUID, &readValue, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY};
	*/
	uint8_t readValue[12] = {0};
	ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(readValue)> readCharOne{dataOneUUID, readValue, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY};

	Thread ble_server, transfer;
	EventQueue queue{8 * EVENTS_EVENT_SIZE};
	/*end BLE stuff*/
	
	//helper function to initialize BLE server and for connection poperties
	void bleInitComplete(BLE::InitializationCompleteCallbackContext*);
	void when_disconnection(const Gap::DisconnectionCallbackParams_t *);
	void when_subscription_received(GattAttribute::Handle_t);
	
	//helper functon that runs in the thread ble_server
	static void waitForEvent(BleServer*);
	static void transferExp(BleServer*);
	GattCharacteristic* characteristics[2] = {&readCharOne, &dataChar};
	GattService customService{customServiceUUID, characteristics, sizeof(characteristics) / sizeof(GattCharacteristic *)};
	
	//helper function to construct event handler from class function
	template<typename ContextType> FunctionPointerWithContext<ContextType> as_cb(void (BleServer::*member)(ContextType context)) 
	{
        return makeFunctionPointer(this, member);
	}
	
	#ifndef NDEBUG
	HardwareSerial* printer; //for debug purpose
	#endif
	uint8_t* data = nullptr;
	
	public:
	BleServer() {};
	BleServer(const BleServer&) = delete; //there is no need to copy a BleServer once established
	BleServer &operator=(const BleServer&) = delete; //there is no need to assign a BleServer to a BleServer
	~BleServer() = default; //no dynamic memory allocation 
	
	void write(float&); //write a single float to characteristic
	void write(float*, uint8_t);
	/*other possibilities*/
	void write(float&, float&, float&);
	void write(float&, float&);
	void start();
	#ifndef NDEBUG
	void begin(HardwareSerial*); //for debug purpose
	void output(const char*); //for debug purpose
	#endif
};

#endif
