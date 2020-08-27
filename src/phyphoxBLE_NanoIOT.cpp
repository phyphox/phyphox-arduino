#ifdef ARDUINO_SAMD_NANO_33_IOT

#include "phyphoxBLE_NanoIOT.h"
#include "Arduino.h"
#include <stdio.h>


void BleServer::start(uint8_t* exp_pointer, size_t len){
  p_exp = exp_pointer;
  expLen = len;
  start();
}

void BleServer::start()
{


  controlCharacteristic.setEventHandler(BLEWritten, controlCharacteristicWritten);
  configCharacteristic.setEventHandler(BLEWritten, configCharacteristicWritten);

	if(p_exp == nullptr){
  
      Experiment defaultExperiment;
      defaultExperiment.setTitle("Default Experiment");
      defaultExperiment.setCategory("Arduino Experiments");
      defaultExperiment.setDescription("This is a default experiment! It is created because there was no custom experiment added.");
      //View
      View firstView;
      firstView.setLabel("FirstView"); //Create a "view"

      //Graph
      Graph firstGraph;      //Create graph which will plot random numbers over time     
      firstGraph.setLabel("default label");
      firstGraph.setUnitX("unit x");
      firstGraph.setUnitY("unit y");
      firstGraph.setLabelX("label x");
      firstGraph.setLabelY("label y");
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

  // start advertising
  BLE.advertise();

}


void BleServer::poll()
{
  BLE.poll();
}

void BleServer::read(uint8_t *arrayPointer, unsigned int arraySize)
{
  configCharacteristic.readValue(arrayPointer, arraySize);
}

void BleServer::read(float& f)
{
  uint8_t readDATA[4];
  configCharacteristic.readValue(readDATA, 4);
  memcpy(&f,&readDATA[0],4);
}

void BleServer::addExperiment(Experiment& exp)
{
  char buffer[4000] =""; //this should be reworked 
  exp.getBytes(buffer);
  memcpy(&EXPARRAY[0],&buffer[0],strlen(buffer));
  p_exp = &EXPARRAY[0];
  expLen = strlen(buffer);
}


void BleServer::write(float& value)
{
  data = reinterpret_cast<uint8_t*>(&value);
  dataCharacteristic.writeValue(data,4);
}


void BleServer::write(float& f1, float& f2)
{
  float array[2] = {f1, f2};
  data = reinterpret_cast<uint8_t*>(array);
  dataCharacteristic.writeValue(data,8);
}

void BleServer::write(float& f1, float& f2, float& f3)
{
  float array[3] = {f1, f2, f3};
  data = reinterpret_cast<uint8_t*>(array);
  dataCharacteristic.writeValue(data,12);
}

void BleServer::write(float& f1, float& f2, float& f3 , float& f4)
{
  float array[4] = {f1, f2, f3, f4};
  data = reinterpret_cast<uint8_t*>(array);
  dataCharacteristic.writeValue(data,16);
}

void BleServer::write(float& f1, float& f2, float& f3 , float& f4, float& f5)
{
  float array[5] = {f1, f2, f3, f4, f5};
  data = reinterpret_cast<uint8_t*>(array);
  dataCharacteristic.writeValue(data,20);
}

void BleServer::controlCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
  byte value = 0;
  characteristic.readValue(value);
  if (value & 0x01) {
    //sendexperiment
    transferExperiment();
  } else {
    //experiment transfered
  }
  
}

void BleServer::transferExperiment(){

  BLE.stopAdvertise();

  uint8_t* exp = p_exp;
  size_t exp_len = expLen;

  uint8_t header[20] = {0}; //20 byte as standard package size for ble transfer
  const char phyphox[] = "phyphox";
  uint32_t table[256];
  crc32::generate_table(table);
  uint32_t checksum = crc32::update(table, 0, exp, exp_len);
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

void BleServer::configCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic){
  if(configHandler!=nullptr){
    (*configHandler)();
  }
}
#endif
