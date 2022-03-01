#if defined(ARDUINO_SAMD_MKR1000)

#include "phyphoxBle.h"
#include "NINAB31serial.h"
#include "Arduino.h"
#include <stdio.h> 
/*
BLEService PhyphoxBLE::phyphoxExperimentService{phyphoxBleExperimentServiceUUID}; // create service
BLECharacteristic PhyphoxBLE::experimentCharacteristic{phyphoxBleExperimentCharacteristicUUID, BLERead | BLEWrite| BLENotify, 20, false};
BLECharacteristic PhyphoxBLE::controlCharacteristic{phyphoxBleExperimentControlCharacteristicUUID, BLERead | BLEWrite| BLENotify, 20, false};

BLEService PhyphoxBLE::phyphoxDataService{phyphoxBleDataServiceUUID}; // create service
BLECharacteristic PhyphoxBLE::dataCharacteristic{phyphoxBleDataCharacteristicUUID, BLERead | BLEWrite | BLENotify, 20, false};
BLECharacteristic PhyphoxBLE::configCharacteristic{phyphoxBleConfigCharacteristicUUID, BLERead | BLEWrite| BLENotify, 20, false};
*/
uint16_t PhyphoxBLE::minConInterval = 12;  //7.5ms
uint16_t PhyphoxBLE::maxConInterval = 48; //30ms
uint16_t PhyphoxBLE::slaveLatency = 0;
uint16_t PhyphoxBLE::timeout = 50;


int PhyphoxBLE::h_phyphoxExperimentService=0;
int PhyphoxBLE::h_experimentCharacteristic=0;
int PhyphoxBLE::h_controlCharacteristic=0;

int PhyphoxBLE::h_phyphoxDataService=0;
int PhyphoxBLE::h_dataCharacteristic=0;
int PhyphoxBLE::h_configCharacteristic=0;

bool PhyphoxBLE::exploaded=false;

uint8_t* PhyphoxBLE::data = nullptr; //this pointer points to the data the user wants to write in the characteristic
uint8_t* PhyphoxBLE::p_exp = nullptr; //this pointer will point to the byte array which holds an experiment

size_t PhyphoxBLE::expLen = 0; //try o avoid this maybe use std::array or std::vector
uint8_t PhyphoxBLE::EXPARRAY[4000] = {0};// block some storage

uint8_t PhyphoxBLE::controlCharValue[21]={0};
uint8_t PhyphoxBLE::configCharValue[21]={0};


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
    port.begin();
    port.stopAdvertise();
    
    port.checkResponse("AT+UBTAD=020A0605121800280011074A5153BA405E438B7146F7300100DFCD",1000);
    h_phyphoxExperimentService=port.parseResponse("AT+UBTGSER=CDDF000130F746718B435E40BA53514A",1000);
    h_controlCharacteristic=port.parseResponse("AT+UBTGCHA=CDDF000330F746718B435E40BA53514A,1a,1,1",1000);
    h_experimentCharacteristic=port.parseResponse("AT+UBTGCHA=CDDF000230F746718B435E40BA53514A,1a,1,1",1000);
    
    h_phyphoxDataService=port.parseResponse("AT+UBTGSER=CDDF100130F746718B435E40BA53514A",1000);
    h_dataCharacteristic=port.parseResponse("AT+UBTGCHA=CDDF100230F746718B435E40BA53514A,1a,1,1",1000);
    h_configCharacteristic=port.parseResponse("AT+UBTGCHA=CDDF100330F746718B435E40BA53514A,1a,1,1",1000);
    
    

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
  
  
  
  
  // set connection parameter
  port.setConnectionInterval(minConInterval, maxConInterval);
  port.checkResponse("AT+UBTAD=020A0605121800280011074A5153BA405E438B7146F7300100DFCD",1000);
  port.advertise();
  port.setLocalName(DEVICE_NAME);
  
}

void PhyphoxBLE::start() {
    PhyphoxBLE::start("phyphox-senseBox");
}


void PhyphoxBLE::poll(int timeout)
{
  auto starttime=millis();
  while(millis()-starttime<timeout){
    poll();
    delay(5);
  }
}

void PhyphoxBLE::read(uint8_t *arrayPointer, unsigned int arraySize)
{
  memcpy(arrayPointer, configCharValue, arraySize);
  //configCharacteristic.readValue(arrayPointer, arraySize);
}

void PhyphoxBLE::read(float& f)
{
  //uint8_t readDATA[4];
  //configCharacteristic.readValue(readDATA, 4);
  memcpy(&f,configCharValue,4);
}

void PhyphoxBLE::addExperiment(PhyphoxBleExperiment& exp)
{
  char buffer[4000] =""; //this should be reworked 
  exp.getBytes(buffer);
  memcpy(&EXPARRAY[0],&buffer[0],strlen(buffer));
  p_exp = &EXPARRAY[0];
  expLen = strlen(buffer);
}


void PhyphoxBLE::write(float& value)
{
  if(!exploaded){
      return;
  }
  data = reinterpret_cast<uint8_t*>(&value);
  port.writeValue(h_dataCharacteristic,data,4);
}


void PhyphoxBLE::write(float& f1, float& f2)
{
  if(!exploaded){
      return;
  }
  float array[2] = {f1, f2};
  data = reinterpret_cast<uint8_t*>(array);
  port.writeValue(h_dataCharacteristic,data,8);
}

void PhyphoxBLE::write(float& f1, float& f2, float& f3)
{
  if(!exploaded){
      return;
  }
  float array[3] = {f1, f2, f3};
  data = reinterpret_cast<uint8_t*>(array);
  port.writeValue(h_dataCharacteristic,data,12);
}

void PhyphoxBLE::write(float& f1, float& f2, float& f3 , float& f4)
{
  if(!exploaded){
      return;
  }
  float array[4] = {f1, f2, f3, f4};
  data = reinterpret_cast<uint8_t*>(array);
  port.writeValue(h_dataCharacteristic,data,16);
}

void PhyphoxBLE::write(float& f1, float& f2, float& f3 , float& f4, float& f5)
{
  if(!exploaded){
      return;
  }
  float array[5] = {f1, f2, f3, f4, f5};
  data = reinterpret_cast<uint8_t*>(array);
  port.writeValue(h_dataCharacteristic,data,20);
}


bool parseValue(uint8_t* target, String s){
    String hextable="0123456789ABCDEF";
    if(s.length() && s.length()<41){
        s.toUpperCase();
        for(int i=0;i<s.length();i+=2){
            int highbits=hextable.indexOf(s[i]);
            int lowbits=hextable.indexOf(s[i+1]);
            if(highbits==-1 || lowbits==-1){
                return false;
            }
            target[i]=(((highbits<<4)|lowbits)&0xff);
        }
        return true;
    }
    return false;
}


void PhyphoxBLE::poll(){
    if(port.poll()){
        if(parseValue(controlCharValue,port.checkCharWritten(h_controlCharacteristic))){
            controlCharacteristicWritten();
        }
        if(parseValue(configCharValue,port.checkCharWritten(h_configCharacteristic))){
            configCharacteristicWritten();
        }
        port.flushInput();
    }
    
}

void PhyphoxBLE::controlCharacteristicWritten() {
  if (controlCharValue[0] & 0x01) {
    //sendexperiment
    exploaded=false;
    //Serial.println("Starting experiment xfer");
    transferExperiment();
  } else {
    //experiment transfered
    exploaded=true;
    
  }
  
}

void PhyphoxBLE::transferExperiment(){

  port.stopAdvertise();

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
  port.writeValue(h_experimentCharacteristic,header,sizeof(header));
  
  for(size_t i = 0; i < exp_len/20; ++i){
    memcpy(&header[0],&exp[0]+i*20,20);
    port.writeValue(h_experimentCharacteristic,header,sizeof(header));
    delay(5);
  }

if(exp_len%20 != 0){
  const size_t rest = exp_len%20;
  uint8_t slice[rest];
  memcpy(&slice[0],&exp[0]+exp_len-rest,rest);
  port.writeValue(h_experimentCharacteristic,slice,sizeof(slice));
}
  exploaded=true;
    
  port.advertise();
}

void PhyphoxBLE::configCharacteristicWritten(){
  if(configHandler!=nullptr){
    (*configHandler)();
  }
}
#endif
