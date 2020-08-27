#ifdef ESP32
#include "phyphoxBLE_ESP32.h"
#include "Arduino.h"
#include <stdio.h>
#include "esp_system.h"

//init statics
char *BleServer::DEVICE_NAME = "phyphox";
void (*BleServer::configHandler)() = nullptr;
uint8_t storage[4000];
uint8_t *BleServer::EXPARRAY=storage;
uint8_t* BleServer::p_exp = nullptr;
size_t BleServer::expLen = 0;

BLEServer *BleServer::myServer;
BLEService *BleServer::phyphoxExperimentService;
BLEService *BleServer::phyphoxDataService;
BLEDescriptor *BleServer::myExperimentDescriptor;
BLEDescriptor *BleServer::myDataDescriptor;
BLEDescriptor *BleServer::myConfigDescriptor;
BLECharacteristic *BleServer::dataCharacteristic;
BLECharacteristic *BleServer::experimentCharacteristic;
BLECharacteristic *BleServer::configCharacteristic;
BLEAdvertising *BleServer::myAdvertising;
TaskHandle_t BleServer::TaskTransfer;
uint8_t* BleServer::data;

class MyExpCallback: public BLEDescriptorCallbacks {

    public:
      MyExpCallback(){};

    private:

    void onWrite(BLEDescriptor* pDescriptor){
      uint8_t* rxValue = pDescriptor->getValue();


    	if(pDescriptor->getLength() > 0){
    		if (rxValue[0] == 1) {
    			// start when_subscription_received() on cpu 1
    	      		BleServer::startTask();
    		}
    	}
    };
  };

class MyCharCallback: public BLECharacteristicCallbacks {
  public:
    MyCharCallback(){};
  private:
    BleServer* myServerPointer;
    void onWrite(BLECharacteristic *pCharacteristic) {
                 BleServer::configHandlerDebug();
      
    }      
};

void BleServer::configHandlerDebug(){
  
  if(configHandler!=nullptr){
    (*configHandler)();
  }
  
  
}

void BleServer::start(uint8_t* exp_pointer, size_t len){
  p_exp = exp_pointer;
  expLen = len;
  start();
}

void BleServer::start()
{
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

	BLEDevice::init(DEVICE_NAME);
	myServer = BLEDevice::createServer();

	phyphoxExperimentService = myServer->createService(phyphoxExperimentServiceUUID);

  experimentCharacteristic = phyphoxExperimentService->createCharacteristic(
          experimentCharacteristicUUID,
          BLECharacteristic::PROPERTY_READ   |
           BLECharacteristic::PROPERTY_WRITE |
           BLECharacteristic::PROPERTY_NOTIFY 
      );  

  phyphoxDataService = myServer->createService(phyphoxDataServiceUUID);

	dataCharacteristic = phyphoxDataService->createCharacteristic(
	     dataCharacteristicUUID,
	     BLECharacteristic::PROPERTY_READ |
	     BLECharacteristic::PROPERTY_WRITE |
	     BLECharacteristic::PROPERTY_NOTIFY 

	   );

    configCharacteristic = phyphoxDataService->createCharacteristic(
          configCharacteristicUUID,
          BLECharacteristic::PROPERTY_READ   |
           BLECharacteristic::PROPERTY_WRITE |
           BLECharacteristic::PROPERTY_NOTIFY 
      );

  myExperimentDescriptor = new BLE2902();
  myDataDescriptor = new BLE2902();
  myConfigDescriptor = new BLE2902();


  myExperimentDescriptor->setCallbacks(new MyExpCallback());

  dataCharacteristic->addDescriptor(myDataDescriptor);
  experimentCharacteristic->addDescriptor(myExperimentDescriptor);
  configCharacteristic->addDescriptor(myConfigDescriptor);

  configCharacteristic->setCallbacks(new MyCharCallback());

  phyphoxExperimentService->start();
  phyphoxDataService->start();
  myAdvertising = BLEDevice::getAdvertising();
  myAdvertising->addServiceUUID(phyphoxExperimentService->getUUID());
  BLEDevice::startAdvertising();

}

//thank you stackoverflow =)
void BleServer::staticStartTask(void* _this){
	BleServer::when_subscription_received();
}	

void BleServer::startTask()
{
	xTaskCreatePinnedToCore(staticStartTask, "TaskTransfer",10000, NULL,1, &TaskTransfer, 1); 
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
  dataCharacteristic->setValue(data,4);
  dataCharacteristic->notify();
}

void BleServer::write(float& f1, float& f2)
{
  float array[2] = {f1, f2};
  data = reinterpret_cast<uint8_t*>(array);
  dataCharacteristic->setValue(data,8);
  dataCharacteristic->notify();
}
void BleServer::write(float& f1, float& f2, float& f3)
{
  float array[3] = {f1, f2, f3};
  data = reinterpret_cast<uint8_t*>(array);
  dataCharacteristic->setValue(data,12);
  dataCharacteristic->notify();
}
void BleServer::write(float& f1, float& f2, float& f3, float& f4)
{
  float array[4] = {f1, f2, f3, f4};
  data = reinterpret_cast<uint8_t*>(array);
  dataCharacteristic->setValue(data,16);
  dataCharacteristic->notify();
}
void BleServer::write(float& f1, float& f2, float& f3, float& f4, float& f5)
{
  float array[5] = {f1, f2, f3, f4, f5};
  data = reinterpret_cast<uint8_t*>(array);
  dataCharacteristic->setValue(data,20);
  dataCharacteristic->notify();
}

void BleServer::read(uint8_t *arrayPointer, unsigned int arraySize)
{
  uint8_t* data = configCharacteristic->getData();
  memcpy(arrayPointer,data,arraySize);
}

void BleServer::read(float& f)
{
  uint8_t* data = configCharacteristic->getData();
  memcpy(&f,data,4);
}


void BleServer::when_subscription_received()
{

    myAdvertising->stop();

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

    copy(phyphox, phyphox+7, header);
    copy(experimentSizeArray, experimentSizeArray+ 4, header + 7);
    copy(checksumArray, checksumArray +  4, header +11); 
    experimentCharacteristic->setValue(header,sizeof(header));
    experimentCharacteristic->notify();
    for(size_t i = 0; i < exp_len/20; ++i){
        copy(exp+i*20, exp+i*20+20, header);
        experimentCharacteristic->setValue(header,sizeof(header));
        experimentCharacteristic->notify();
		delay(5);
		vTaskDelay(60);
	}
  
	if(exp_len%20 != 0){
		const size_t rest = exp_len%20;
		uint8_t slice[rest];
		copy(exp + exp_len - rest, exp + exp_len, slice);
		experimentCharacteristic->setValue(slice,sizeof(slice));
		experimentCharacteristic->notify();        
		delay(5);
	}
 

	myAdvertising->start();

	vTaskDelete( NULL );

}
void BleServer::addExperiment(Experiment& exp)
{
	char buffer[4000] =""; //this should be reworked 
	exp.getBytes(buffer);
	memcpy(&EXPARRAY[0],&buffer[0],strlen(buffer));
	p_exp = &EXPARRAY[0];
	expLen = strlen(buffer);
}

/*
void BleServer::begin(HardwareSerial* hwPrint)
{
	printer = hwPrint;
      	if(printer)
		printer->begin(115200);       
}
*/
#endif
