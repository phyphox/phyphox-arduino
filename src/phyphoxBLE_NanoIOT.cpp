#if defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_UNOR4_WIFI)

#include "phyphoxBLE_NanoIOT.h"
#include "Arduino.h"
#include <stdio.h> 

BLEService PhyphoxBLE::phyphoxExperimentService{phyphoxBleExperimentServiceUUID}; // create service
BLECharacteristic PhyphoxBLE::experimentCharacteristic{phyphoxBleExperimentCharacteristicUUID, BLERead | BLEWrite| BLENotify, 20, false};
BLECharacteristic PhyphoxBLE::controlCharacteristic{phyphoxBleExperimentControlCharacteristicUUID, BLERead | BLEWrite| BLENotify, 20, false};

BLEService PhyphoxBLE::phyphoxDataService{phyphoxBleDataServiceUUID}; // create service
BLECharacteristic PhyphoxBLE::dataCharacteristic{phyphoxBleDataCharacteristicUUID, BLERead | BLEWrite | BLENotify, 20, false};
BLECharacteristic PhyphoxBLE::configCharacteristic{phyphoxBleConfigCharacteristicUUID, BLERead | BLEWrite| BLENotify, 20, false};

uint16_t PhyphoxBLE::minConInterval = 6;  //7.5ms
uint16_t PhyphoxBLE::maxConInterval = 24; //30ms
uint16_t PhyphoxBLE::slaveLatency = 0;
uint16_t PhyphoxBLE::timeout = 50;

uint16_t PhyphoxBLE::MTU = 20;
uint16_t PhyphoxBleExperiment::MTU = 20;

uint8_t* PhyphoxBLE::data = nullptr; //this pointer points to the data the user wants to write in the characteristic
uint8_t* PhyphoxBLE::p_exp = nullptr; //this pointer will point to the byte array which holds an experiment

size_t PhyphoxBLE::expLen = 0; //try o avoid this maybe use std::array or std::vector
uint8_t PhyphoxBLE::EXPARRAY[4000] = {0};// block some storage

void(*PhyphoxBLE::configHandler)() = nullptr;

void PhyphoxBLE::start(const char* DEVICE_NAME, uint8_t* exp_pointer, size_t len){
  p_exp = exp_pointer;
  expLen = len;
  start(DEVICE_NAME);
}

void PhyphoxBLE::start(uint8_t* exp_pointer, size_t len){
  p_exp = exp_pointer;
  expLen = len;
  start();
}

void PhyphoxBLE::start(const char* DEVICE_NAME)
{
  deviceName = DEVICE_NAME;

  controlCharacteristic.setEventHandler(BLEWritten, controlCharacteristicWritten);
  configCharacteristic.setEventHandler(BLEWritten, configCharacteristicWritten);

	if(p_exp == nullptr){
  
      PhyphoxBleExperiment defaultExperiment;

      //View
      PhyphoxBleExperiment::View firstView;

      //Graph
      PhyphoxBleExperiment::Graph firstGraph;      //Create graph which will plot random numbers over time     
      firstGraph.setChannel(0,1);    

      firstView.addElement(firstGraph);       
      defaultExperiment.addView(firstView);
      
     addExperiment(defaultExperiment);  
  }
  
  
  BLE.begin();
  BLE.setLocalName(DEVICE_NAME);
  BLE.setAdvertisedService(phyphoxExperimentService);
  //BLE.setAdvertisedService(phyphoxDataService);

  // add the characteristics to the service
  phyphoxExperimentService.addCharacteristic(experimentCharacteristic);
  phyphoxExperimentService.addCharacteristic(controlCharacteristic);
  phyphoxDataService.addCharacteristic(configCharacteristic);
  phyphoxDataService.addCharacteristic(dataCharacteristic);

  // add the service
  BLE.addService(phyphoxExperimentService);
  BLE.addService(phyphoxDataService);

  // set connection parameter
  BLE.setConnectionInterval(minConInterval, maxConInterval);

  // start advertising
  BLE.advertise();

}

void PhyphoxBLE::start() {
    PhyphoxBLE::start("phyphox-Arduino");
}

void PhyphoxBLE::poll()
{
  BLE.poll();
}

void PhyphoxBLE::poll(int timeout)
{
  BLE.poll(timeout);
}

void PhyphoxBLE::read(uint8_t *arrayPointer, unsigned int arraySize)
{
  configCharacteristic.readValue(arrayPointer, arraySize);
}

void PhyphoxBLE::read(float& f)
{
  uint8_t readDATA[4];
  configCharacteristic.readValue(readDATA, 4);
  memcpy(&f,&readDATA[0],4);
}

void PhyphoxBLE::read(float& f1, float& f2)
{
  uint8_t readDATA[8];
  configCharacteristic.readValue(readDATA, 8);
  memcpy(&f1,data,4);
  memcpy(&f2,data+4,4);
}
void PhyphoxBLE::read(float& f1, float& f2, float& f3)
{
  uint8_t readDATA[12];
  configCharacteristic.readValue(readDATA, 12);
  memcpy(&f1,data,4);
  memcpy(&f2,data+4,4);
  memcpy(&f3,data+8,4);
}
void PhyphoxBLE::read(float& f1, float& f2, float& f3, float& f4)
{
  uint8_t readDATA[16];
  configCharacteristic.readValue(readDATA, 16);
  memcpy(&f1,data,4);
  memcpy(&f2,data+4,4);
  memcpy(&f3,data+8,4);
  memcpy(&f4,data+12,4);
}
void PhyphoxBLE::read(float& f1, float& f2, float& f3, float& f4, float& f5)
{
  uint8_t readDATA[20];
  configCharacteristic.readValue(readDATA, 20);
  memcpy(&f1,data,4);
  memcpy(&f2,data+4,4);
  memcpy(&f3,data+8,4);
  memcpy(&f4,data+12,4);
  memcpy(&f5,data+16,4);
}

void PhyphoxBLE::addExperiment(PhyphoxBleExperiment& exp)
{
  char buffer[2000] ="";
  uint16_t length = 0;

	exp.getFirstBytes(buffer, deviceName);
	memcpy(&EXPARRAY[length],&buffer[0],strlen(buffer));
  length += strlen(buffer);
  memset(&(buffer[0]), NULL, strlen(buffer));

  for(uint8_t i=0;i<phyphoxBleNViews; i++){
    for(int j=0; j<phyphoxBleNElements; j++){
      exp.getViewBytes(buffer,0,j);
	    memcpy(&EXPARRAY[length],&buffer[0],strlen(buffer));
      length += strlen(buffer);
      memset(&(buffer[0]), NULL, strlen(buffer));
    }
  }

  exp.getLastBytes(buffer);
	memcpy(&EXPARRAY[length],&buffer[0],strlen(buffer));
  length += strlen(buffer);
	p_exp = &EXPARRAY[0];
	expLen = length;
}



void PhyphoxBLE::write(float& value)
{
  data = reinterpret_cast<uint8_t*>(&value);
  dataCharacteristic.writeValue(data,4);
}


void PhyphoxBLE::write(float& f1, float& f2)
{
  float array[2] = {f1, f2};
  data = reinterpret_cast<uint8_t*>(array);
  dataCharacteristic.writeValue(data,8);
}

void PhyphoxBLE::write(float& f1, float& f2, float& f3)
{
  float array[3] = {f1, f2, f3};
  data = reinterpret_cast<uint8_t*>(array);
  dataCharacteristic.writeValue(data,12);
}

void PhyphoxBLE::write(float& f1, float& f2, float& f3 , float& f4)
{
  float array[4] = {f1, f2, f3, f4};
  data = reinterpret_cast<uint8_t*>(array);
  dataCharacteristic.writeValue(data,16);
}

void PhyphoxBLE::write(float& f1, float& f2, float& f3 , float& f4, float& f5)
{
  float array[5] = {f1, f2, f3, f4, f5};
  data = reinterpret_cast<uint8_t*>(array);
  dataCharacteristic.writeValue(data,20);
}

void PhyphoxBLE::controlCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
  byte value = 0;
  characteristic.readValue(value);
  if (value & 0x01) {
    //sendexperiment
    transferExperiment();
  } else {
    //experiment transfered
  }
  
}

void PhyphoxBLE::transferExperiment(){

  BLE.stopAdvertise();

  uint8_t* exp = p_exp;
  size_t exp_len = expLen;

  uint8_t header[20] = {0}; //20 byte as standard package size for ble transfer
  const char phyphox[] = "phyphox";
  uint32_t table[256];
  phyphoxBleCrc32::generate_table(table);
  uint32_t checksum = phyphoxBleCrc32::update(table, 0, exp, exp_len);
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

  memcpy(&header[0],&phyphox[0],7);
  memcpy(&header[0]+7,&experimentSizeArray[0],4);
  memcpy(&header[0]+7+4,&checksumArray[0],4);
  experimentCharacteristic.writeValue(header,sizeof(header));
 
  for(size_t i = 0; i < exp_len/20; ++i){
    memcpy(&header[0],&exp[0]+i*20,20);
    experimentCharacteristic.writeValue(header,sizeof(header));
    delay(5);
  }

if(exp_len%20 != 0){
  const size_t rest = exp_len%20;
  uint8_t slice[rest];
  memcpy(&slice[0],&exp[0]+exp_len-rest,rest);
  experimentCharacteristic.writeValue(slice,sizeof(slice));
      
  delay(5);
}

  BLE.advertise();
}

void PhyphoxBLE::configCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic){
  if(configHandler!=nullptr){
    (*configHandler)();
  }
}
#endif
