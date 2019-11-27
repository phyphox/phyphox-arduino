#include "phyphoxBle.h"

void BleServer::onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event)
{
	#ifndef NDEBUG
	if(printer)
		printer -> println("Disconnection");
	#endif
	ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
}

void BleServer::onConnectionComplete(const ble::ConnectionCompleteEvent &event)
{
	#ifndef NDEBUG
	if(printer)
		printer -> println("Connection with device");
	#endif	
}

#ifndef NDEBUG
void BleServer::begin(HardwareSerial* hwPrint)
{
	 printer = hwPrint;
      if(printer)
		printer->begin(9600);       
}

void BleServer::output(const char* s)
{
	if(printer)
		printer->println(s);
}

void BleServer::output(const uint32_t num)
{
	if(printer)
		printer -> println(num);
}

#endif



void BleServer::when_subscription_received(GattAttribute::Handle_t handle)
{
	#ifndef NDEBUG
	output("Received a subscription");
	#endif
	bool sub_dChar = false;
	ble.gattServer().areUpdatesEnabled(*characteristics[1], &sub_dChar);
	if(sub_dChar)
	{
		queue.call(transferExp, this);
		sub_dChar = false;
	}
	//after experiment has been transfered start advertising again
	ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);

}

void BleServer::transferExp(BleServer* server)
{
	BLE &ble = server ->ble;
	uint8_t* exp = server -> exp;
	size_t exp_len = server -> exp_len;
	#ifndef NDEBUG
	server -> output("In transfer exp");
	#endif
	//this is our standard phyphox experiment
	uint8_t EXPERIMENT[] = {80, 75, 3, 4, 20, 0, 0, 0, 8, 0, 22, 111, 101, 79, 171, 229, 150, 117, 164, 2, 0, 0, 96, 6, 0, 0, 26, 0, 28, 0, 68, 101, 102, 97, 117, 108, 116, 32, 69, 120, 112, 101, 114, 105, 109, 101, 110, 116, 46, 112, 104, 121, 112, 104, 111, 120, 85, 84, 9, 0, 3, 140, 113, 193, 93, 140, 113, 193, 93, 117, 120, 11, 0, 1, 4, 232, 3, 0, 0, 4, 217, 3, 0, 0, 165, 85, 77, 143, 155, 48, 16, 189, 247, 87, 88, 190, 135, 111, 178, 100, 5, 145, 42, 237, 30, 42, 181, 221, 170, 237, 170, 75, 110, 14, 152, 96, 201, 177, 41, 54, 91, 216, 95, 223, 1, 199, 148, 208, 156, 90, 46, 153, 55, 51, 158, 25, 207, 188, 113, 210, 166, 30, 154, 90, 246, 168, 63, 115, 161, 50, 92, 107, 221, 220, 187, 238, 69, 235, 200, 246, 228, 130, 5, 27, 243, 61, 45, 153, 150, 237, 77, 47, 99, 50, 206, 175, 180, 85, 76, 138, 12, 251, 206, 29, 70, 198, 114, 191, 80, 122, 24, 113, 89, 16, 78, 51, 76, 5, 222, 191, 67, 240, 165, 154, 105, 78, 247, 15, 180, 34, 29, 215, 232, 177, 111, 104, 203, 206, 84, 232, 212, 53, 22, 227, 85, 16, 77, 79, 178, 29, 246, 66, 34, 43, 167, 238, 172, 53, 78, 37, 85, 69, 203, 26, 13, 233, 246, 169, 187, 68, 23, 59, 209, 100, 83, 72, 161, 9, 19, 80, 149, 209, 154, 240, 86, 137, 20, 123, 131, 242, 124, 207, 131, 98, 149, 38, 154, 21, 25, 174, 8, 87, 20, 239, 159, 58, 221, 116, 26, 5, 144, 215, 186, 255, 99, 8, 255, 175, 16, 80, 238, 173, 226, 82, 38, 192, 127, 145, 229, 200, 59, 170, 165, 212, 53, 18, 228, 12, 89, 222, 183, 101, 199, 132, 196, 8, 157, 101, 9, 88, 72, 205, 42, 6, 125, 129, 91, 99, 212, 66, 127, 160, 18, 40, 163, 59, 142, 205, 56, 210, 39, 241, 77, 147, 86, 207, 5, 205, 145, 167, 232, 210, 148, 87, 212, 4, 134, 29, 239, 170, 216, 39, 145, 183, 73, 146, 56, 216, 68, 228, 72, 55, 36, 246, 170, 77, 80, 70, 49, 221, 30, 75, 48, 22, 24, 65, 197, 243, 132, 43, 46, 137, 14, 131, 143, 76, 195, 224, 30, 69, 201, 136, 88, 94, 217, 68, 255, 255, 148, 180, 215, 45, 201, 176, 6, 154, 44, 135, 178, 14, 159, 186, 115, 179, 108, 143, 23, 221, 76, 151, 238, 215, 135, 83, 34, 8, 31, 20, 83, 72, 113, 74, 155, 12, 195, 28, 145, 20, 207, 138, 182, 31, 198, 0, 215, 205, 75, 93, 235, 126, 193, 175, 140, 254, 90, 114, 107, 196, 136, 195, 93, 120, 134, 63, 131, 56, 226, 117, 227, 79, 45, 105, 106, 187, 49, 93, 199, 74, 152, 90, 28, 207, 59, 212, 72, 213, 103, 120, 235, 69, 78, 226, 111, 163, 48, 78, 118, 97, 18, 198, 225, 210, 62, 100, 56, 218, 37, 206, 46, 244, 189, 208, 247, 227, 109, 148, 108, 183, 216, 166, 125, 0, 110, 33, 82, 252, 236, 152, 98, 134, 25, 68, 53, 180, 208, 95, 71, 158, 100, 56, 112, 226, 145, 169, 195, 184, 152, 28, 216, 167, 224, 32, 252, 252, 96, 165, 174, 39, 250, 20, 146, 143, 235, 95, 85, 119, 52, 8, 48, 106, 128, 65, 140, 240, 231, 166, 156, 248, 101, 186, 129, 106, 166, 160, 150, 97, 58, 48, 229, 125, 201, 240, 247, 113, 70, 6, 229, 25, 38, 78, 231, 92, 208, 33, 195, 24, 117, 130, 233, 23, 43, 228, 86, 152, 76, 92, 158, 94, 230, 200, 0, 242, 37, 56, 204, 160, 255, 210, 210, 130, 25, 246, 65, 59, 134, 107, 248, 118, 13, 213, 248, 244, 124, 98, 2, 2, 147, 78, 75, 171, 32, 253, 74, 193, 68, 190, 246, 200, 215, 30, 135, 181, 199, 172, 56, 79, 9, 128, 50, 231, 41, 176, 55, 105, 114, 171, 201, 173, 230, 96, 53, 70, 0, 169, 185, 244, 123, 68, 211, 119, 205, 145, 63, 207, 1, 34, 61, 131, 39, 187, 95, 146, 127, 245, 78, 220, 60, 48, 44, 151, 241, 198, 129, 212, 157, 104, 184, 220, 160, 145, 171, 150, 230, 11, 94, 167, 180, 111, 100, 59, 47, 143, 69, 169, 253, 103, 216, 191, 251, 13, 80, 75, 1, 2, 30, 3, 20, 0, 0, 0, 8, 0, 22, 111, 101, 79, 171, 229, 150, 117, 164, 2, 0, 0, 96, 6, 0, 0, 26, 0, 24, 0, 0, 0, 0, 0, 1, 0, 0, 0, 164, 129, 0, 0, 0, 0, 68, 101, 102, 97, 117, 108, 116, 32, 69, 120, 112, 101, 114, 105, 109, 101, 110, 116, 46, 112, 104, 121, 112, 104, 111, 120, 85, 84, 5, 0, 3, 140, 113, 193, 93, 117, 120, 11, 0, 1, 4, 232, 3, 0, 0, 4, 217, 3, 0, 0, 80, 75, 5, 6, 0, 0, 0, 0, 1, 0, 1, 0, 96, 0, 0, 0, 248, 2, 0, 0, 0, 0};
	if(exp == nullptr)
	{
		exp = EXPERIMENT;
		exp_len = sizeof(EXPERIMENT);
	}
	
	uint8_t header[20] = {0}; //20 byte as standard package size for ble transfer
	const char phyphox[] = "phyphox";
	uint32_t table[256];
	crc32::generate_table(table);
	uint32_t checksum = crc32::update(table, 0, exp, exp_len);
	#ifndef NDEBUG
	server -> output("checksum: ");
	server -> output(checksum);
	#endif
	size_t arrayLength = exp_len;
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
	wait_us(60000);
	
	for(size_t i = 0; i < exp_len/20; ++i)
	{
		copy(exp+i*20, exp+i*20+20, header);
		ble.gattServer().write(server -> dataChar.getValueHandle(), header, sizeof(header));
		wait_us(60000);
	}
	
	if(exp_len%20 != 0)
	{
		const size_t rest = exp_len%20;
		uint8_t slice[rest];
		copy(exp + exp_len - rest, exp + exp_len, slice);
		ble.gattServer().write(server -> dataChar.getValueHandle(), slice, sizeof(slice));
		wait_us(60000);
	}
}

void BleServer::bleInitComplete(BLE::InitializationCompleteCallbackContext* params)
{	
	/*
	ble.gap().onDisconnection(this, &BleServer::when_disconnection);
	ble.gap().onConnection(this, &BleServer::when_connected);
	*/
	ble.gattServer().onUpdatesEnabled(as_cb(&BleServer::when_subscription_received)); //this is not deprecated
	ble.gap().setEventHandler(this);
	char name[strlen(DEVICE_NAME)];
	strcpy(name, DEVICE_NAME);
	uint8_t _adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];
	ble::AdvertisingDataBuilder adv_data_builder(_adv_buffer);
	ble::AdvertisingParameters adv_parameters(ble::advertising_type_t::CONNECTABLE_UNDIRECTED, ble::adv_interval_t(ble::millisecond_t(100)));
	adv_data_builder.setFlags();
    adv_data_builder.setLocalServiceList(mbed::make_Span(&customServiceUUID, 1));
    adv_data_builder.setName(name);
    ble.gap().setAdvertisingParameters(ble::LEGACY_ADVERTISING_HANDLE, adv_parameters);
    ble.gap().setAdvertisingPayload(ble::LEGACY_ADVERTISING_HANDLE,adv_data_builder.getAdvertisingData());
    ble.gattServer().addService(customService);
 
	ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
	#ifndef NDEBUG
	output("in init");
	#endif
}

void BleServer::waitForEvent(BleServer* server)
{
	while(1)
		server -> ble.waitForEvent(); //deprecated.
	

	
}


void BleServer::start(uint8_t* exp_pointer, size_t len)
{
	/**
	 * \brief start the server and begin advertising
	 * This methods starts the server by. By specifying
	 * exp_pointer and len you can send your own phyphox
	 * experiment to your smartphone. Otherwise a default
	 * experiment will be sent. 
	 * \param exp_pointer a pointer to an experiment 
	 * \param len the len of the experiment 
	 */
	#ifndef NDEBUG
	output("In start");
	#endif
	exp = exp_pointer;
	exp_len = len;
	ble.init(this, &BleServer::bleInitComplete);
	ble_server.start(mbed::callback(BleServer::waitForEvent, this));
	queue.event(mbed::callback(BleServer::transferExp, this));
	transfer.start(mbed::callback(&queue, &EventQueue::dispatch_forever));
}

	
void BleServer::write(float& value)
{
	/**
	 * \brief Write a single float into characteristic
     * The float is parsed to uint8_t* 
     * because the gattServer write method 
     * expects a pointer to uint8_t
     * \param f1 represent a float most likeley sensor data
    */
	data = reinterpret_cast<uint8_t*>(&value);
	ble.gattServer().write(readCharOne.getValueHandle(), data, 4);
}


///todo: public getter for gatt server 


void BleServer::write(float& f1, float& f2)
{
	 /**
   * \brief Write 2 floats into characteristic
   * The floats are parsed to uint8_t* 
   * because the gattServer write method e
   * expects a pointer to uint8_t
   * \param f1 and f2 represent two floats most likeley sensor data
   */
	float array[2] = {f1, f2};
	data = reinterpret_cast<uint8_t*>(array);
	ble.gattServer().write(readCharOne.getValueHandle(), data, 8);
}

void BleServer::write(float& f1, float& f2, float& f3)
{
	 /**
   * \brief Write 3 floats into characteristic
   * The floats are parsed to uint8_t* 
   * because the gattServer write method 
   * expects a pointer to uint8_t
   * \param param f1 and f2 and f3 represents three floats most likeley sensor data
   */
	float array[3] = {f1, f2, f3};
	data = reinterpret_cast<uint8_t*>(array);
	ble.gattServer().write(readCharOne.getValueHandle(), data, 12);
}
