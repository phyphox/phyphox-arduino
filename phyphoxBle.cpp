#include "phyphoxBle.h"


void BleServer::when_disconnection(const Gap::DisconnectionCallbackParams_t *)
{
	ble.gap().startAdvertising();
}

void BleServer::begin(HardwareSerial* hwPrint)
{
	 printer = hwPrint; //operate on the address of print
      if(printer) {
      printer->begin(9600);
      }     
}
#ifndef NDEBUG
void BleServer::output(const char* s)
{
	if(printer)
		printer->println(s);
}
#endif

void BleServer::when_subscription_received(GattAttribute::Handle_t handle)
{
	output("I received a subscirption");
	bool sub_dChar = false;
	bool sub_rChar = false;
	ble.gattServer().areUpdatesEnabled(*characteristics[1], &sub_dChar);
	ble.gattServer().areUpdatesEnabled(*characteristics[0], &sub_rChar);
	if(sub_dChar)
	{
		queue.call(transferExp, this);
		sub_dChar = false;
	}
	
}

void BleServer::transferExp(BleServer* server)
{
	BLE &ble = server ->ble;
	uint8_t* data_package = server -> data_package;
	#ifndef NDEBUG
	server -> output("In transfer exp");
	#endif
	//this is our standard phyphox experiment
	//const uint8_t EXPERIMENT[] = {80, 75, 3, 4, 20, 0, 8, 0, 8, 0, 175, 118, 109, 79, 0, 0, 0, 0, 0, 0, 0, 0, 66, 13, 0, 0, 15, 0, 32, 0, 97, 112, 112, 108, 97, 117, 115, 46, 112, 104, 121, 112, 104, 111, 120, 85, 84, 13, 0, 7, 219, 10, 204, 93, 219, 10, 204, 93, 219, 10, 204, 93, 117, 120, 11, 0, 1, 4, 232, 3, 0, 0, 4, 217, 3, 0, 0, 237, 151, 205, 142, 219, 54, 16, 128, 239, 121, 10, 130, 64, 143, 182, 254, 255, 22, 146, 129, 32, 41, 208, 0, 89, 100, 145, 110, 208, 120, 111, 180, 68, 89, 68, 36, 82, 32, 169, 68, 218, 167, 234, 51, 244, 201, 58, 146, 44, 69, 86, 133, 34, 232, 162, 187, 151, 248, 98, 206, 15, 103, 198, 156, 143, 35, 57, 174, 139, 174, 46, 68, 139, 218, 170, 228, 42, 193, 133, 214, 245, 141, 97, 92, 180, 123, 33, 207, 6, 88, 240, 104, 190, 161, 25, 211, 66, 110, 122, 141, 166, 209, 249, 43, 149, 138, 9, 158, 96, 107, 31, 96, 52, 90, 110, 22, 74, 19, 163, 82, 164, 164, 164, 9, 166, 28, 31, 94, 33, 248, 196, 154, 233, 146, 30, 94, 223, 223, 162, 215, 50, 107, 24, 23, 177, 49, 170, 70, 115, 74, 52, 61, 11, 217, 29, 110, 33, 222, 35, 227, 84, 162, 59, 73, 190, 104, 246, 165, 169, 98, 99, 182, 142, 206, 25, 85, 169, 100, 181, 134, 124, 135, 216, 88, 74, 23, 59, 209, 100, 151, 10, 174, 73, 31, 72, 141, 218, 49, 205, 164, 68, 138, 61, 66, 125, 150, 105, 66, 181, 74, 19, 205, 210, 4, 231, 164, 84, 20, 31, 62, 52, 186, 110, 52, 114, 33, 239, 228, 254, 31, 67, 88, 79, 15, 97, 63, 61, 132, 243, 143, 16, 112, 104, 91, 71, 20, 51, 14, 254, 139, 44, 167, 178, 161, 90, 8, 93, 32, 78, 42, 200, 114, 105, 29, 70, 168, 18, 25, 200, 92, 104, 150, 51, 232, 14, 156, 61, 70, 18, 186, 4, 149, 64, 25, 205, 169, 111, 201, 137, 126, 224, 191, 107, 34, 245, 92, 208, 28, 121, 136, 46, 198, 242, 210, 130, 0, 115, 94, 148, 123, 22, 113, 205, 93, 24, 122, 246, 206, 37, 39, 186, 35, 158, 153, 239, 236, 204, 245, 168, 127, 202, 192, 152, 98, 4, 21, 207, 160, 229, 165, 32, 218, 177, 223, 51, 13, 24, 253, 202, 51, 70, 160, 6, 145, 231, 138, 66, 194, 158, 65, 202, 207, 186, 72, 176, 187, 108, 199, 152, 243, 25, 11, 113, 183, 10, 177, 95, 160, 144, 112, 171, 16, 231, 255, 41, 68, 105, 201, 248, 25, 102, 67, 171, 37, 73, 176, 102, 213, 213, 181, 90, 167, 140, 141, 25, 180, 137, 207, 5, 137, 241, 210, 253, 122, 115, 76, 56, 41, 59, 197, 20, 82, 37, 165, 245, 208, 118, 36, 248, 39, 69, 229, 187, 62, 192, 53, 120, 177, 49, 185, 95, 228, 175, 140, 126, 91, 78, 135, 94, 70, 37, 252, 190, 50, 193, 31, 201, 55, 244, 22, 46, 201, 26, 218, 179, 36, 117, 49, 13, 189, 166, 97, 25, 16, 111, 125, 31, 131, 181, 80, 45, 156, 117, 96, 239, 29, 223, 50, 67, 39, 10, 93, 39, 240, 150, 230, 46, 193, 78, 16, 237, 125, 207, 14, 34, 211, 244, 35, 47, 52, 225, 202, 92, 178, 254, 214, 84, 12, 60, 59, 140, 136, 170, 105, 170, 63, 246, 87, 43, 193, 246, 222, 235, 47, 119, 215, 143, 212, 18, 46, 172, 130, 13, 240, 245, 7, 203, 250, 118, 90, 253, 225, 151, 253, 224, 206, 243, 128, 218, 54, 70, 53, 92, 58, 70, 202, 79, 117, 54, 92, 201, 241, 16, 80, 193, 20, 212, 208, 13, 27, 134, 124, 159, 161, 53, 151, 229, 113, 153, 123, 208, 60, 36, 24, 163, 134, 51, 61, 122, 245, 43, 112, 250, 101, 92, 13, 198, 82, 156, 63, 207, 193, 65, 56, 46, 133, 135, 89, 104, 239, 36, 77, 217, 8, 134, 131, 81, 119, 45, 62, 94, 139, 170, 127, 110, 220, 50, 14, 129, 73, 163, 197, 164, 32, 237, 74, 193, 248, 113, 237, 113, 92, 123, 60, 172, 61, 102, 69, 53, 36, 0, 88, 170, 33, 176, 57, 104, 142, 147, 230, 56, 105, 30, 38, 205, 184, 128, 85, 125, 57, 242, 94, 26, 62, 215, 116, 124, 31, 162, 136, 180, 12, 158, 183, 237, 18, 251, 213, 116, 221, 220, 208, 45, 135, 213, 198, 134, 216, 24, 0, 252, 1, 40, 77, 119, 3, 74, 59, 178, 252, 48, 48, 129, 88, 43, 12, 87, 84, 250, 145, 191, 247, 67, 211, 3, 110, 125, 207, 15, 156, 112, 166, 18, 90, 164, 84, 35, 233, 75, 80, 89, 111, 224, 168, 102, 28, 139, 59, 242, 19, 200, 103, 2, 210, 121, 34, 144, 142, 185, 1, 100, 0, 99, 210, 245, 76, 43, 242, 28, 203, 94, 1, 233, 57, 96, 247, 224, 197, 198, 9, 45, 215, 143, 2, 103, 6, 242, 158, 86, 53, 133, 215, 141, 23, 98, 242, 254, 95, 153, 252, 235, 207, 55, 63, 153, 124, 38, 38, 237, 31, 99, 50, 54, 250, 71, 251, 244, 22, 176, 120, 236, 199, 180, 173, 133, 156, 223, 45, 38, 41, 158, 254, 251, 28, 94, 253, 13, 80, 75, 7, 8, 15, 170, 192, 253, 79, 3, 0, 0, 66, 13, 0, 0, 80, 75, 1, 2, 20, 3, 20, 0, 8, 0, 8, 0, 175, 118, 109, 79, 15, 170, 192, 253, 79, 3, 0, 0, 66, 13, 0, 0, 15, 0, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 164, 129, 0, 0, 0, 0, 97, 112, 112, 108, 97, 117, 115, 46, 112, 104, 121, 112, 104, 111, 120, 85, 84, 13, 0, 7, 219, 10, 204, 93, 219, 10, 204, 93, 219, 10, 204, 93, 117, 120, 11, 0, 1, 4, 232, 3, 0, 0, 4, 217, 3, 0, 0, 80, 75, 5, 6, 0, 0, 0, 0, 1, 0, 1, 0, 93, 0, 0, 0, 172, 3, 0, 0, 0, 0};

	const uint8_t EXPERIMENT[] = {80, 75, 3, 4, 20, 0, 0, 0, 8, 0, 22, 111, 101, 79, 171, 229, 150, 117, 164, 2, 0, 0, 96, 6, 0, 0, 26, 0, 28, 0, 68, 101, 102, 97, 117, 108, 116, 32, 69, 120, 112, 101, 114, 105, 109, 101, 110, 116, 46, 112, 104, 121, 112, 104, 111, 120, 85, 84, 9, 0, 3, 140, 113, 193, 93, 140, 113, 193, 93, 117, 120, 11, 0, 1, 4, 232, 3, 0, 0, 4, 217, 3, 0, 0, 165, 85, 77, 143, 155, 48, 16, 189, 247, 87, 88, 190, 135, 111, 178, 100, 5, 145, 42, 237, 30, 42, 181, 221, 170, 237, 170, 75, 110, 14, 152, 96, 201, 177, 41, 54, 91, 216, 95, 223, 1, 199, 148, 208, 156, 90, 46, 153, 55, 51, 158, 25, 207, 188, 113, 210, 166, 30, 154, 90, 246, 168, 63, 115, 161, 50, 92, 107, 221, 220, 187, 238, 69, 235, 200, 246, 228, 130, 5, 27, 243, 61, 45, 153, 150, 237, 77, 47, 99, 50, 206, 175, 180, 85, 76, 138, 12, 251, 206, 29, 70, 198, 114, 191, 80, 122, 24, 113, 89, 16, 78, 51, 76, 5, 222, 191, 67, 240, 165, 154, 105, 78, 247, 15, 180, 34, 29, 215, 232, 177, 111, 104, 203, 206, 84, 232, 212, 53, 22, 227, 85, 16, 77, 79, 178, 29, 246, 66, 34, 43, 167, 238, 172, 53, 78, 37, 85, 69, 203, 26, 13, 233, 246, 169, 187, 68, 23, 59, 209, 100, 83, 72, 161, 9, 19, 80, 149, 209, 154, 240, 86, 137, 20, 123, 131, 242, 124, 207, 131, 98, 149, 38, 154, 21, 25, 174, 8, 87, 20, 239, 159, 58, 221, 116, 26, 5, 144, 215, 186, 255, 99, 8, 255, 175, 16, 80, 238, 173, 226, 82, 38, 192, 127, 145, 229, 200, 59, 170, 165, 212, 53, 18, 228, 12, 89, 222, 183, 101, 199, 132, 196, 8, 157, 101, 9, 88, 72, 205, 42, 6, 125, 129, 91, 99, 212, 66, 127, 160, 18, 40, 163, 59, 142, 205, 56, 210, 39, 241, 77, 147, 86, 207, 5, 205, 145, 167, 232, 210, 148, 87, 212, 4, 134, 29, 239, 170, 216, 39, 145, 183, 73, 146, 56, 216, 68, 228, 72, 55, 36, 246, 170, 77, 80, 70, 49, 221, 30, 75, 48, 22, 24, 65, 197, 243, 132, 43, 46, 137, 14, 131, 143, 76, 195, 224, 30, 69, 201, 136, 88, 94, 217, 68, 255, 255, 148, 180, 215, 45, 201, 176, 6, 154, 44, 135, 178, 14, 159, 186, 115, 179, 108, 143, 23, 221, 76, 151, 238, 215, 135, 83, 34, 8, 31, 20, 83, 72, 113, 74, 155, 12, 195, 28, 145, 20, 207, 138, 182, 31, 198, 0, 215, 205, 75, 93, 235, 126, 193, 175, 140, 254, 90, 114, 107, 196, 136, 195, 93, 120, 134, 63, 131, 56, 226, 117, 227, 79, 45, 105, 106, 187, 49, 93, 199, 74, 152, 90, 28, 207, 59, 212, 72, 213, 103, 120, 235, 69, 78, 226, 111, 163, 48, 78, 118, 97, 18, 198, 225, 210, 62, 100, 56, 218, 37, 206, 46, 244, 189, 208, 247, 227, 109, 148, 108, 183, 216, 166, 125, 0, 110, 33, 82, 252, 236, 152, 98, 134, 25, 68, 53, 180, 208, 95, 71, 158, 100, 56, 112, 226, 145, 169, 195, 184, 152, 28, 216, 167, 224, 32, 252, 252, 96, 165, 174, 39, 250, 20, 146, 143, 235, 95, 85, 119, 52, 8, 48, 106, 128, 65, 140, 240, 231, 166, 156, 248, 101, 186, 129, 106, 166, 160, 150, 97, 58, 48, 229, 125, 201, 240, 247, 113, 70, 6, 229, 25, 38, 78, 231, 92, 208, 33, 195, 24, 117, 130, 233, 23, 43, 228, 86, 152, 76, 92, 158, 94, 230, 200, 0, 242, 37, 56, 204, 160, 255, 210, 210, 130, 25, 246, 65, 59, 134, 107, 248, 118, 13, 213, 248, 244, 124, 98, 2, 2, 147, 78, 75, 171, 32, 253, 74, 193, 68, 190, 246, 200, 215, 30, 135, 181, 199, 172, 56, 79, 9, 128, 50, 231, 41, 176, 55, 105, 114, 171, 201, 173, 230, 96, 53, 70, 0, 169, 185, 244, 123, 68, 211, 119, 205, 145, 63, 207, 1, 34, 61, 131, 39, 187, 95, 146, 127, 245, 78, 220, 60, 48, 44, 151, 241, 198, 129, 212, 157, 104, 184, 220, 160, 145, 171, 150, 230, 11, 94, 167, 180, 111, 100, 59, 47, 143, 69, 169, 253, 103, 216, 191, 251, 13, 80, 75, 1, 2, 30, 3, 20, 0, 0, 0, 8, 0, 22, 111, 101, 79, 171, 229, 150, 117, 164, 2, 0, 0, 96, 6, 0, 0, 26, 0, 24, 0, 0, 0, 0, 0, 1, 0, 0, 0, 164, 129, 0, 0, 0, 0, 68, 101, 102, 97, 117, 108, 116, 32, 69, 120, 112, 101, 114, 105, 109, 101, 110, 116, 46, 112, 104, 121, 112, 104, 111, 120, 85, 84, 5, 0, 3, 140, 113, 193, 93, 117, 120, 11, 0, 1, 4, 232, 3, 0, 0, 4, 217, 3, 0, 0, 80, 75, 5, 6, 0, 0, 0, 0, 1, 0, 1, 0, 96, 0, 0, 0, 248, 2, 0, 0, 0, 0};
	uint8_t header[20] = {0}; //20 byte as standard package size for ble transfer
	const char phyphox[] = "phyphox";
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
	
	ble.gattServer().write(server -> dataChar.getValueHandle(), header, sizeof(header));
	wait_us(50000);
	  
	//now send experiment 
	for(size_t i = 0; i < sizeof(EXPERIMENT)/20; ++i)
	{
		copy(EXPERIMENT+i*20, EXPERIMENT+i*20 + 20, header);
		ble.gattServer().write(server -> dataChar.getValueHandle(), header, sizeof(header));
		wait_us(50000);
	}
	if(sizeof(EXPERIMENT)%20 != 0)
	{
		const size_t rest = sizeof(EXPERIMENT)%20;
		uint8_t slice[rest];
		copy(EXPERIMENT+sizeof(EXPERIMENT) -rest, EXPERIMENT + sizeof(EXPERIMENT), slice);
		ble.gattServer().write(server -> dataChar.getValueHandle(), slice, sizeof(slice));
		wait_us(50000);
	}
}
void BleServer::bleInitComplete(BLE::InitializationCompleteCallbackContext* params)
{	
	ble.gap().onDisconnection(this, &BleServer::when_disconnection);
	ble.gattServer().onUpdatesEnabled(as_cb(&BleServer::when_subscription_received)); //potential error!!
	
	ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE); // BLE only, no classic BT
	ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED); // advertising type
	//deduce length of name to set up payoad correctly
	char name[strlen(DEVICE_NAME)];
	strcpy(name, DEVICE_NAME);
	ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)name, sizeof(name)); // add name
	uint8_t customServiceUUIDreversed[16];
	for(int i = 0; i < 16; ++i)
		customServiceUUIDreversed[16 - 1 -i] = customServiceUUID[i]; //little endian in advertising
	ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS, (uint8_t *)customServiceUUIDreversed, sizeof(customServiceUUID));
	ble.gap().setAdvertisingInterval(100); // 100ms or some multiple of some value.

	/* Add our custom service */
	ble.gattServer().addService(customService);
 
	/* Start advertising */
	ble.gap().startAdvertising();
	output("in init");
}

void BleServer::waitForEvent(BleServer* server)
{
	while(1)
		server -> ble.waitForEvent(); //waitForEvent() puts the processor to sleep until a hardware interrupt occurs. 
}


void BleServer::start()
{
	output("In start");
	ble.init(this, &BleServer::bleInitComplete);
	ble_server.start(mbed::callback(BleServer::waitForEvent, this));
	queue.event(mbed::callback(BleServer::transferExp, this)); //potential error!!
	transfer.start(mbed::callback(&queue, &EventQueue::dispatch_forever));
}

	
void BleServer::write(float& value)
{
	data = reinterpret_cast<uint8_t*>(&value);
	ble.gattServer().write(readCharOne.getValueHandle(), data, 4);
}

void BleServer::write(float* input, uint8_t num)
{
	data = reinterpret_cast<uint8_t*>(input);
	ble.gattServer().write(readCharOne.getValueHandle(), data, num*4);
	
}

void BleServer::write(float& f1, float& f2)
{
	float array[3] = {f1, f2};
	data = reinterpret_cast<uint8_t*>(array);
	ble.gattServer().write(readCharOne.getValueHandle(), data, 8);
}

void BleServer::write(float& f1, float& f2, float& f3)
{
	float array[3] = {f1, f2, f3};
	data = reinterpret_cast<uint8_t*>(array);
	ble.gattServer().write(readCharOne.getValueHandle(), data, 12);
}
