#define NDEBUG
#include <mbed.h>
#include <ble/BLE.h>
#include <ble/GattServer.h>
#include <Gap.h>
#include <CRC32.h>
#include <algorithm>
#include <UUID.h>

using std::copy, events::EventQueue, rtos::Thread, mbed::callback;
EventQueue queue(8 * EVENTS_EVENT_SIZE);
Thread ble_server, transfer;
const uint8_t customServiceUUID[UUID::LENGTH_OF_LONG_UUID]= {0xcd, 0xdf, 0x00, 0x01, 0x30, 0xf7, 0x46, 0x71, 0x8b, 0x43, 0x5e, 0x40, 0xba, 0x53, 0x51, 0x4a};                                                
const UUID phyphoxUUID = UUID("cddf0002-30f7-4671-8b43-5e40ba53514a");
const UUID dataOneUUID = UUID("59f51a40-8852-4abe-a50f-2d45e6bd51ac");

const static char DEVICE_NAME[]= "Arduino";
//static uint8_t readValue = 0;
static float readValue = 0.0;
static uint8_t data_package[20] = {0};


ReadWriteArrayGattCharacteristic<uint8_t, sizeof(data_package)> dataChar(phyphoxUUID, data_package, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
//ReadOnlyGattCharacteristic<uint8_t> readCharOne(dataOneUUID, &readValue,GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
ReadOnlyGattCharacteristic<float> readCharOne(dataOneUUID, &readValue,GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);


GattCharacteristic *characteristics[] = {&readCharOne, &dataChar};
GattService customService(customServiceUUID, characteristics, sizeof(characteristics) / sizeof(GattCharacteristic *));

volatile bool sent = false; //if we do  a class put this in the class. What if more cients subscribe? 

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *)
{
	BLE::Instance(BLE::DEFAULT_INSTANCE).gap().startAdvertising();
	sent = false;
}

void connectionCallback(const Gap::ConnectionCallbackParams_t *)
{
	Serial.println("Connection succes!");
	//TODO: start advertising again
}

void when_subscription_received(GattAttribute::Handle_t handle)
{
        #ifndef NDEBUG
        BLE::Instance(BLE::DEFAULT_INSTANCE).gap().getPreferredConnectionParams(&par2);
        Serial.println(par2.connectionSupervisionTimeout);
        Serial.println(par2.maxConnectionInterval);
        #endif
        Serial.println("Someone subscribed to characteristic updates");
        //send experiment
	if(!sent)
		queue.call(&transferExp);         
}


void bleInitComplete(BLE::InitializationCompleteCallbackContext *params)
{
	/**
	 *Setup the global connection paramters for our server 
	 * These values work, take care if you want to change.
	 */
	/*
	Gap::ConnectionParams_t par;
	par.minConnectionInterval = 6;
	par.maxConnectionInterval = 6;
	par.connectionSupervisionTimeout = 500;
	par.slaveLatency = 0;
	BLE::Instance(BLE::DEFAULT_INSTANCE).gap().setPreferredConnectionParams(&par); 
	*/
	
	BLE &ble          = params->ble;
	ble_error_t error = params->error;
    
	if (error != BLE_ERROR_NONE)
	{
		Serial.println("There was an error in initialization");
		return;
	}

	ble.gap().onConnection(connectionCallback);
	ble.gap().onDisconnection(disconnectionCallback);
	ble.gattServer().onUpdatesEnabled(when_subscription_received); //this is called when a client subscribes to a characteristic
 
	/* Setup advertising */
	ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE); // BLE only, no classic BT
	ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED); // advertising type
	ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME)); // add name
	
	uint8_t customServiceUUIDreversed[16];
	for(int i = 0; i < 16; ++i)
		customServiceUUIDreversed[16 - 1 -i] = customServiceUUID[i]; //this has to be done...little endian in advertising
	ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS, (uint8_t *)customServiceUUIDreversed, sizeof(customServiceUUID));
	ble.gap().setAdvertisingInterval(100); // 100ms or some multiple of some value.
 
	/* Add our custom service */
	ble.gattServer().addService(customService);
 
	/* Start advertising */
	ble.gap().startAdvertising();
}

void transferExp()
{
	if(!sent) //already checked before fix later
	{
		//this is the uint8 version
		//uint8_t EXPERIMENT[] = {80, 75, 3, 4, 20, 0, 0, 0, 8, 0, 115, 97, 101, 79, 228, 40, 71, 18, 158, 2, 0, 0, 83, 6, 0, 0, 26, 0, 28, 0, 68, 101, 102, 97, 117, 108, 116, 32, 69, 120, 112, 101, 114, 105, 109, 101, 110, 116, 46, 112, 104, 121, 112, 104, 111, 120, 85, 84, 9, 0, 3, 233, 88, 193, 93, 136, 90, 193, 93, 117, 120, 11, 0, 1, 4, 232, 3, 0, 0, 4, 217, 3, 0, 0, 165, 85, 77, 111, 219, 48, 12, 189, 247, 87, 8, 186, 199, 31, 73, 156, 58, 133, 29, 96, 64, 119, 232, 97, 235, 176, 173, 91, 147, 155, 98, 203, 177, 0, 71, 18, 36, 186, 179, 251, 235, 71, 91, 177, 231, 120, 57, 109, 57, 233, 145, 79, 36, 77, 62, 42, 137, 46, 91, 93, 170, 134, 52, 231, 74, 218, 148, 150, 0, 250, 193, 247, 47, 86, 79, 153, 147, 143, 30, 234, 220, 15, 60, 23, 160, 204, 77, 150, 115, 57, 242, 27, 55, 86, 40, 153, 210, 208, 187, 167, 196, 121, 30, 38, 198, 128, 146, 74, 101, 172, 226, 41, 229, 146, 238, 238, 8, 254, 18, 16, 80, 241, 221, 35, 47, 88, 93, 1, 249, 216, 104, 110, 196, 153, 75, 72, 124, 231, 113, 172, 140, 1, 63, 41, 211, 238, 164, 34, 195, 57, 241, 71, 171, 35, 229, 220, 102, 70, 104, 192, 116, 187, 196, 159, 162, 139, 159, 1, 91, 100, 74, 2, 19, 18, 171, 114, 86, 23, 126, 48, 18, 43, 222, 177, 188, 48, 8, 176, 88, 11, 12, 68, 150, 210, 130, 85, 150, 211, 221, 115, 13, 186, 6, 178, 196, 188, 3, 253, 31, 67, 132, 127, 133, 192, 114, 111, 21, 151, 8, 137, 252, 73, 150, 99, 85, 115, 80, 10, 74, 34, 217, 25, 179, 124, 48, 121, 45, 164, 162, 132, 156, 85, 142, 88, 42, 16, 133, 192, 190, 224, 87, 83, 98, 176, 63, 88, 9, 150, 81, 31, 187, 102, 28, 249, 179, 252, 6, 204, 192, 88, 208, 24, 185, 143, 174, 92, 121, 89, 201, 112, 216, 209, 182, 136, 66, 182, 14, 22, 113, 28, 45, 23, 107, 118, 228, 11, 22, 5, 197, 98, 153, 175, 35, 190, 57, 230, 232, 204, 40, 193, 138, 199, 9, 215, 79, 18, 226, 233, 71, 186, 120, 255, 159, 132, 55, 96, 88, 74, 1, 133, 49, 29, 195, 60, 124, 226, 143, 237, 25, 186, 58, 233, 95, 50, 165, 95, 95, 78, 152, 100, 85, 107, 133, 37, 182, 226, 92, 167, 20, 39, 71, 148, 124, 177, 220, 60, 117, 1, 174, 219, 149, 248, 3, 253, 130, 223, 4, 255, 53, 85, 83, 135, 73, 133, 223, 82, 165, 244, 51, 30, 59, 60, 111, 245, 201, 48, 93, 14, 59, 82, 215, 34, 199, 57, 69, 209, 184, 53, 90, 217, 38, 165, 155, 96, 237, 197, 225, 102, 189, 138, 226, 237, 42, 94, 69, 171, 169, 191, 77, 233, 122, 27, 123, 219, 85, 24, 172, 194, 48, 218, 172, 227, 205, 134, 14, 105, 31, 81, 77, 184, 40, 26, 106, 195, 41, 97, 86, 243, 12, 190, 118, 170, 72, 233, 210, 139, 58, 93, 182, 221, 26, 230, 10, 44, 222, 65, 197, 253, 20, 57, 148, 189, 86, 50, 85, 117, 187, 94, 20, 247, 124, 185, 164, 68, 163, 92, 4, 171, 94, 116, 222, 139, 201, 53, 130, 148, 194, 98, 25, 109, 127, 161, 79, 249, 154, 210, 239, 221, 120, 28, 218, 167, 244, 7, 195, 89, 92, 224, 33, 165, 148, 212, 82, 0, 178, 172, 59, 33, 131, 121, 181, 231, 64, 239, 175, 212, 233, 117, 140, 143, 96, 63, 5, 135, 17, 52, 95, 12, 207, 132, 19, 28, 246, 163, 189, 134, 239, 215, 208, 118, 175, 205, 39, 33, 49, 48, 171, 65, 13, 6, 214, 204, 12, 66, 238, 231, 140, 253, 156, 113, 152, 51, 70, 195, 185, 79, 128, 154, 57, 247, 129, 131, 222, 178, 31, 44, 251, 193, 114, 24, 44, 238, 128, 39, 125, 233, 122, 135, 250, 223, 181, 72, 254, 188, 0, 132, 53, 2, 95, 233, 102, 170, 254, 217, 211, 112, 243, 66, 59, 221, 198, 27, 23, 18, 191, 215, 225, 116, 133, 58, 177, 14, 58, 159, 8, 59, 225, 141, 86, 102, 220, 158, 1, 37, 195, 159, 193, 238, 238, 55, 80, 75, 1, 2, 30, 3, 20, 0, 0, 0, 8, 0, 115, 97, 101, 79, 228, 40, 71, 18, 158, 2, 0, 0, 83, 6, 0, 0, 26, 0, 24, 0, 0, 0, 0, 0, 1, 0, 0, 0, 164, 129, 0, 0, 0, 0, 68, 101, 102, 97, 117, 108, 116, 32, 69, 120, 112, 101, 114, 105, 109, 101, 110, 116, 46, 112, 104, 121, 112, 104, 111, 120, 85, 84, 5, 0, 3, 233, 88, 193, 93, 117, 120, 11, 0, 1, 4, 232, 3, 0, 0, 4, 217, 3, 0, 0, 80, 75, 5, 6, 0, 0, 0, 0, 1, 0, 1, 0, 96, 0, 0, 0, 242, 2, 0, 0, 0, 0};
		//this is tzhe float32 version
		//uint8_t EXPERIMENT[] = {80, 75, 3, 4, 20, 0, 0, 0, 8, 0, 79, 100, 101, 79, 235, 181, 200, 201, 138, 2, 0, 0, 69, 6, 0, 0, 23, 0, 28, 0, 68, 101, 102, 97, 117, 108, 116, 32, 65, 114, 100, 117, 105, 110, 111, 46, 112, 104, 121, 112, 104, 111, 120, 85, 84, 9, 0, 3, 70, 94, 193, 93, 70, 94, 193, 93, 117, 120, 11, 0, 1, 4, 232, 3, 0, 0, 4, 217, 3, 0, 0, 165, 84, 75, 143, 155, 48, 16, 190, 247, 87, 88, 190, 39, 188, 179, 100, 5, 145, 42, 181, 135, 74, 109, 183, 106, 181, 234, 146, 155, 3, 38, 88, 114, 108, 132, 205, 22, 246, 215, 119, 140, 99, 74, 104, 78, 45, 23, 102, 190, 25, 207, 203, 223, 56, 107, 155, 177, 109, 228, 128, 134, 11, 23, 42, 199, 141, 214, 237, 163, 231, 93, 209, 173, 236, 206, 30, 88, 176, 53, 63, 210, 138, 105, 217, 221, 245, 178, 38, 235, 252, 74, 59, 197, 164, 200, 113, 176, 125, 192, 200, 90, 30, 23, 160, 143, 17, 151, 37, 225, 52, 199, 84, 224, 195, 59, 4, 95, 166, 153, 230, 244, 240, 129, 214, 164, 231, 26, 189, 239, 170, 158, 9, 153, 121, 22, 182, 46, 37, 209, 244, 44, 187, 241, 32, 36, 114, 114, 230, 205, 168, 117, 170, 168, 42, 59, 214, 106, 200, 117, 200, 188, 165, 118, 181, 19, 77, 54, 165, 20, 154, 48, 1, 37, 89, 212, 134, 119, 32, 82, 236, 13, 106, 11, 124, 31, 42, 85, 154, 104, 86, 230, 184, 38, 92, 81, 124, 120, 234, 117, 219, 107, 20, 66, 94, 231, 254, 143, 33, 130, 191, 66, 64, 185, 247, 138, 203, 152, 0, 255, 69, 150, 19, 239, 169, 150, 82, 55, 72, 144, 11, 100, 185, 14, 11, 35, 116, 145, 21, 232, 66, 106, 86, 51, 152, 11, 116, 141, 81, 7, 243, 129, 74, 160, 140, 254, 100, 134, 113, 162, 79, 226, 135, 38, 157, 158, 11, 154, 35, 79, 209, 165, 45, 175, 108, 8, 220, 116, 178, 175, 147, 128, 196, 254, 38, 77, 147, 112, 19, 147, 19, 221, 144, 196, 175, 55, 97, 21, 39, 116, 119, 170, 192, 88, 98, 4, 21, 207, 215, 91, 115, 73, 116, 20, 126, 102, 26, 46, 238, 163, 168, 24, 17, 203, 150, 109, 244, 255, 79, 73, 7, 221, 145, 28, 107, 118, 185, 185, 148, 117, 248, 204, 155, 135, 229, 102, 188, 152, 102, 182, 116, 191, 61, 156, 17, 65, 248, 168, 152, 66, 138, 83, 218, 230, 24, 238, 17, 73, 241, 172, 104, 247, 201, 4, 184, 29, 94, 230, 57, 247, 171, 254, 202, 232, 175, 37, 183, 140, 142, 56, 244, 194, 115, 252, 21, 68, 163, 175, 7, 127, 238, 72, 219, 184, 117, 233, 123, 86, 25, 254, 196, 243, 2, 181, 82, 13, 57, 222, 249, 241, 54, 13, 118, 113, 148, 164, 251, 40, 141, 146, 104, 105, 31, 115, 28, 239, 211, 237, 62, 10, 252, 40, 8, 146, 93, 156, 238, 118, 216, 165, 197, 136, 168, 150, 150, 250, 187, 225, 69, 142, 195, 109, 98, 152, 57, 154, 45, 228, 192, 54, 5, 142, 240, 251, 201, 42, 221, 76, 116, 41, 37, 55, 187, 94, 215, 15, 52, 12, 49, 106, 129, 49, 140, 240, 231, 182, 154, 248, 100, 187, 71, 13, 83, 144, 123, 156, 14, 76, 121, 94, 76, 162, 73, 42, 102, 233, 104, 164, 94, 48, 253, 226, 132, 194, 9, 147, 137, 203, 243, 203, 28, 17, 148, 98, 169, 28, 103, 101, 248, 214, 209, 146, 89, 150, 65, 219, 227, 173, 250, 118, 171, 42, 243, 190, 124, 97, 2, 2, 147, 94, 75, 7, 144, 97, 5, 48, 81, 172, 61, 138, 181, 199, 113, 237, 49, 3, 151, 41, 1, 80, 227, 50, 5, 246, 39, 164, 112, 72, 225, 144, 163, 67, 172, 0, 82, 123, 157, 179, 209, 166, 239, 150, 11, 127, 214, 30, 145, 129, 193, 187, 60, 44, 73, 190, 122, 15, 238, 30, 24, 151, 75, 119, 231, 64, 230, 77, 116, 91, 110, 138, 225, 164, 163, 243, 130, 191, 25, 29, 90, 217, 205, 75, 226, 180, 204, 61, 255, 135, 119, 191, 1, 80, 75, 1, 2, 30, 3, 20, 0, 0, 0, 8, 0, 79, 100, 101, 79, 235, 181, 200, 201, 138, 2, 0, 0, 69, 6, 0, 0, 23, 0, 24, 0, 0, 0, 0, 0, 1, 0, 0, 0, 164, 129, 0, 0, 0, 0, 68, 101, 102, 97, 117, 108, 116, 32, 65, 114, 100, 117, 105, 110, 111, 46, 112, 104, 121, 112, 104, 111, 120, 85, 84, 5, 0, 3, 70, 94, 193, 93, 117, 120, 11, 0, 1, 4, 232, 3, 0, 0, 4, 217, 3, 0, 0, 80, 75, 5, 6, 0, 0, 0, 0, 1, 0, 1, 0, 93, 0, 0, 0, 219, 2, 0, 0, 0, 0};
		uint8_t EXPERIMENT[] = {80, 75, 3, 4, 20, 0, 0, 0, 8, 0, 22, 111, 101, 79, 171, 229, 150, 117, 164, 2, 0, 0, 96, 6, 0, 0, 26, 0, 28, 0, 68, 101, 102, 97, 117, 108, 116, 32, 69, 120, 112, 101, 114, 105, 109, 101, 110, 116, 46, 112, 104, 121, 112, 104, 111, 120, 85, 84, 9, 0, 3, 140, 113, 193, 93, 140, 113, 193, 93, 117, 120, 11, 0, 1, 4, 232, 3, 0, 0, 4, 217, 3, 0, 0, 165, 85, 77, 143, 155, 48, 16, 189, 247, 87, 88, 190, 135, 111, 178, 100, 5, 145, 42, 237, 30, 42, 181, 221, 170, 237, 170, 75, 110, 14, 152, 96, 201, 177, 41, 54, 91, 216, 95, 223, 1, 199, 148, 208, 156, 90, 46, 153, 55, 51, 158, 25, 207, 188, 113, 210, 166, 30, 154, 90, 246, 168, 63, 115, 161, 50, 92, 107, 221, 220, 187, 238, 69, 235, 200, 246, 228, 130, 5, 27, 243, 61, 45, 153, 150, 237, 77, 47, 99, 50, 206, 175, 180, 85, 76, 138, 12, 251, 206, 29, 70, 198, 114, 191, 80, 122, 24, 113, 89, 16, 78, 51, 76, 5, 222, 191, 67, 240, 165, 154, 105, 78, 247, 15, 180, 34, 29, 215, 232, 177, 111, 104, 203, 206, 84, 232, 212, 53, 22, 227, 85, 16, 77, 79, 178, 29, 246, 66, 34, 43, 167, 238, 172, 53, 78, 37, 85, 69, 203, 26, 13, 233, 246, 169, 187, 68, 23, 59, 209, 100, 83, 72, 161, 9, 19, 80, 149, 209, 154, 240, 86, 137, 20, 123, 131, 242, 124, 207, 131, 98, 149, 38, 154, 21, 25, 174, 8, 87, 20, 239, 159, 58, 221, 116, 26, 5, 144, 215, 186, 255, 99, 8, 255, 175, 16, 80, 238, 173, 226, 82, 38, 192, 127, 145, 229, 200, 59, 170, 165, 212, 53, 18, 228, 12, 89, 222, 183, 101, 199, 132, 196, 8, 157, 101, 9, 88, 72, 205, 42, 6, 125, 129, 91, 99, 212, 66, 127, 160, 18, 40, 163, 59, 142, 205, 56, 210, 39, 241, 77, 147, 86, 207, 5, 205, 145, 167, 232, 210, 148, 87, 212, 4, 134, 29, 239, 170, 216, 39, 145, 183, 73, 146, 56, 216, 68, 228, 72, 55, 36, 246, 170, 77, 80, 70, 49, 221, 30, 75, 48, 22, 24, 65, 197, 243, 132, 43, 46, 137, 14, 131, 143, 76, 195, 224, 30, 69, 201, 136, 88, 94, 217, 68, 255, 255, 148, 180, 215, 45, 201, 176, 6, 154, 44, 135, 178, 14, 159, 186, 115, 179, 108, 143, 23, 221, 76, 151, 238, 215, 135, 83, 34, 8, 31, 20, 83, 72, 113, 74, 155, 12, 195, 28, 145, 20, 207, 138, 182, 31, 198, 0, 215, 205, 75, 93, 235, 126, 193, 175, 140, 254, 90, 114, 107, 196, 136, 195, 93, 120, 134, 63, 131, 56, 226, 117, 227, 79, 45, 105, 106, 187, 49, 93, 199, 74, 152, 90, 28, 207, 59, 212, 72, 213, 103, 120, 235, 69, 78, 226, 111, 163, 48, 78, 118, 97, 18, 198, 225, 210, 62, 100, 56, 218, 37, 206, 46, 244, 189, 208, 247, 227, 109, 148, 108, 183, 216, 166, 125, 0, 110, 33, 82, 252, 236, 152, 98, 134, 25, 68, 53, 180, 208, 95, 71, 158, 100, 56, 112, 226, 145, 169, 195, 184, 152, 28, 216, 167, 224, 32, 252, 252, 96, 165, 174, 39, 250, 20, 146, 143, 235, 95, 85, 119, 52, 8, 48, 106, 128, 65, 140, 240, 231, 166, 156, 248, 101, 186, 129, 106, 166, 160, 150, 97, 58, 48, 229, 125, 201, 240, 247, 113, 70, 6, 229, 25, 38, 78, 231, 92, 208, 33, 195, 24, 117, 130, 233, 23, 43, 228, 86, 152, 76, 92, 158, 94, 230, 200, 0, 242, 37, 56, 204, 160, 255, 210, 210, 130, 25, 246, 65, 59, 134, 107, 248, 118, 13, 213, 248, 244, 124, 98, 2, 2, 147, 78, 75, 171, 32, 253, 74, 193, 68, 190, 246, 200, 215, 30, 135, 181, 199, 172, 56, 79, 9, 128, 50, 231, 41, 176, 55, 105, 114, 171, 201, 173, 230, 96, 53, 70, 0, 169, 185, 244, 123, 68, 211, 119, 205, 145, 63, 207, 1, 34, 61, 131, 39, 187, 95, 146, 127, 245, 78, 220, 60, 48, 44, 151, 241, 198, 129, 212, 157, 104, 184, 220, 160, 145, 171, 150, 230, 11, 94, 167, 180, 111, 100, 59, 47, 143, 69, 169, 253, 103, 216, 191, 251, 13, 80, 75, 1, 2, 30, 3, 20, 0, 0, 0, 8, 0, 22, 111, 101, 79, 171, 229, 150, 117, 164, 2, 0, 0, 96, 6, 0, 0, 26, 0, 24, 0, 0, 0, 0, 0, 1, 0, 0, 0, 164, 129, 0, 0, 0, 0, 68, 101, 102, 97, 117, 108, 116, 32, 69, 120, 112, 101, 114, 105, 109, 101, 110, 116, 46, 112, 104, 121, 112, 104, 111, 120, 85, 84, 5, 0, 3, 140, 113, 193, 93, 117, 120, 11, 0, 1, 4, 232, 3, 0, 0, 4, 217, 3, 0, 0, 80, 75, 5, 6, 0, 0, 0, 0, 1, 0, 1, 0, 96, 0, 0, 0, 248, 2, 0, 0, 0, 0};
		uint8_t header[20] = {0}; //20 byte as standard package size for ble transfer
		char phyphox[] = "phyphox";
		uint32_t checksum = CRC32::calculate(EXPERIMENT, sizeof(EXPERIMENT));
		size_t arrayLength = sizeof(EXPERIMENT);
		uint8_t experimentSizeArray[4] = {0};
		experimentSizeArray[0]=  (arrayLength >> 24);
		experimentSizeArray[1]=  (arrayLength >> 16);
		experimentSizeArray[2]=  (arrayLength >> 8);
		experimentSizeArray[3]=  arrayLength; 

		uint8_t checksumArray[4] = {0};
		checksumArray[0]= (checksum >> 24) & 0xFF;
		checksumArray[1]= (checksum >> 16) & 0xFF;  
		checksumArray[2]= (checksum >> 8) & 0xFF;
		checksumArray[3]= checksum & 0xFF; 

		copy(phyphox, phyphox+7, header);
		copy(experimentSizeArray, experimentSizeArray+ 4, header + 7);
		copy(checksumArray, checksumArray +  4, header +11); 
	  
		Serial.println("Trying to send...");

		//now send header 
		BLE::Instance(BLE::DEFAULT_INSTANCE).gattServer().write(dataChar.getValueHandle(), header, sizeof(header));
		delay(50); 
	  
		//now send experiment 
		for(int i = 0; i < sizeof(EXPERIMENT)/20; ++i)
		{
			copy(EXPERIMENT+i*20, EXPERIMENT+i*20 + 20, header);
			BLE::Instance(BLE::DEFAULT_INSTANCE).gattServer().write(dataChar.getValueHandle(), header, sizeof(header));
			delay(50);
		}
		if(sizeof(EXPERIMENT)%20 != 0)
		{
			const size_t rest = sizeof(EXPERIMENT)%20;
			uint8_t slice[rest];
			copy(EXPERIMENT+sizeof(EXPERIMENT) -rest, EXPERIMENT + sizeof(EXPERIMENT), slice);
			BLE::Instance(BLE::DEFAULT_INSTANCE).gattServer().write(dataChar.getValueHandle(), slice, sizeof(slice));
			delay(50);
		}
		sent = true;
	  }
}

void waitForEvent(BLE* ble)
{
	while(1)
		ble -> waitForEvent(); //waitForEvent() puts the processor to sleep until a hardware interrupt occurs. 
}



void setup()
{
	/**
	 *Here we set up a BLE Server and initialize it by calling our function bleInitComplete
	 *The function also sets connection paramters for our connection to clients.
	 *We run two Threads: One to monitor connections to our server the other one to 
	 *run an event queue over and over to react to things from the client. Note that this has
	 * to be done in the context of event queues here.
	 */
	Serial.begin(9600); 
	BLE& ble = BLE::Instance(BLE::DEFAULT_INSTANCE);
	ble.init(bleInitComplete);
	ble_server.start(mbed::callback(waitForEvent, &ble));
	queue.event(&transferExp); 
	transfer.start(mbed::callback(&queue, &EventQueue::dispatch_forever)); 
}

void loop()
{
	uint8_t* data = nullptr; //pointer that points to the buffer that will contain the float
	while(sent)
	{
		int value = analogRead(A1);
		float voltage = value * (3.3 / 1023.0);
		data = reinterpret_cast<uint8_t*>(&voltage); //Note: This is necessary as gattServer.write expects uint8_t* as second argument
		BLE::Instance(BLE::DEFAULT_INSTANCE).gattServer().write(readCharOne.getValueHandle(), data, 4);
		delay(500);
	}
	
}
