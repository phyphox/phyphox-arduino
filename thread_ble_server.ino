#define NDEBUG

#include <mbed.h>
#include <ble/BLE.h>
#include <ble/GattServer.h>
#include <Gap.h>
#include <CRC32.h>
#include <algorithm>
#include <UUID.h>

using std::copy;
using events::EventQueue;
using rtos::Thread;
using mbed::callback;


EventQueue queue;
Thread ble_server;
Thread transfer;
Gap::ConnectionParams_t par;
#ifndef NDEBUG
Gap::ConnectionParams_t par2;
#endif
//const uint8_t customServiceUUID[16] = {0xcd, 0xdf, 0x00, 0x01, 0x30, 0xf7, 0x46, 0x71, 0x8b, 0x43, 0x5e, 0x40, 0xba, 0x53,
                                                      //  0x51, 0x4a};
//next try
const uint8_t customServiceUUID[UUID::LENGTH_OF_LONG_UUID] = {0x4a, 0x51, 0x53, 0xba, 0x40, 0x5e, 0x43, 0x8b, 0x71, 0x46, 0xf7, 0x30, 0x01, 0x00, 0xdf, 0xcd};                                                  
//uint16_t customServiceUUID  = 0xA000;
const uint16_t readCharUUID       = 0xA001;
const uint16_t writeCharUUID      = 0xA002;
const uint8_t phyphoxUUID[UUID::LENGTH_OF_LONG_UUID] = {0x4a, 0x51, 0x53, 0xba, 0x40, 0x5e, 0x43, 0x8b, 0x71, 0x46, 0xf7, 0x30, 0x02, 0x00, 0xdf, 0xcd};
//const uint8_t phyphoxUUID[UUID::LENGTH_OF_LONG_UUID] = {0xcd, 0xdf, 0x00, 0x02, 0x30, 0xf7, 0x46, 0x71, 0x8b, 0x43, 0x5e, 0x40, 0xba, 0x53, 0x51, 0x4a};
const static char DEVICE_NAME[]= "BLETest";
static uint8_t readValue[1] = {0};
static uint8_t writeValue[1] = {0};
static uint8_t data_package[20] = {0};
ReadWriteArrayGattCharacteristic<uint8_t, sizeof(data_package)> dataChar(phyphoxUUID, data_package, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
//ReadWriteGattCharacteristic<uint8_t> dataChar(phyphoxUUID, writeValue, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
WriteOnlyArrayGattCharacteristic<uint8_t, sizeof(writeValue)> writeChar(writeCharUUID, writeValue);
ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(readValue)> readChar(readCharUUID, readValue, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_INDICATE /*GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY*/);
GattCharacteristic *characteristics[] = {&dataChar};
GattService customService(customServiceUUID, characteristics, sizeof(characteristics) / sizeof(GattCharacteristic *));


void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *)
{
    BLE::Instance(BLE::DEFAULT_INSTANCE).gap().startAdvertising();
}

void connectionCallback(const Gap::ConnectionCallbackParams_t *)
{
  //setPreferred Connecton parameters
  par.minConnectionInterval = 6;
  par.maxConnectionInterval = 6;
  par.connectionSupervisionTimeout = 500;
  par.slaveLatency = 0;
  BLE::Instance(BLE::DEFAULT_INSTANCE).gap().setPreferredConnectionParams(&par);   
  Serial.println("Connection succes!");
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
        //queue.event(&transferExp); 
        //queue.call(&transferExp); 
        
}

void failTransfer()
{
  //This function fails because it does not run in a thread. It does not fail if run in eventqueue context. nice.
  uint8_t test_data = 0;
  Serial.println("In transfer data");
  for(int j = 0; j < 12; ++j)
  {
    Serial.println("Sending: "); Serial.print(test_data); 
    BLE::Instance(BLE::DEFAULT_INSTANCE).gattServer().write(readChar.getValueHandle(), &test_data, sizeof(test_data));
    ++test_data;
    delay(500);
  }
}
void transferData()
{
    uint8_t test_data = 0;
    Serial.println("In transfer data");
    for(int j = 0; j < 20; ++j)
    {
      Serial.println("Sending: "); Serial.print(test_data); 
      BLE::Instance(BLE::DEFAULT_INSTANCE).gattServer().write(readChar.getValueHandle(), &test_data, sizeof(test_data));
      ++test_data;
      delay(500);
    }
} 

void transferExp()
{
  uint8_t EXPERIMENT[870] = {80, 75, 3, 4, 20, 0, 0, 0, 8, 0, 161, 99, 195, 78, 150, 29, 14, 210, 176, 2, 0, 0, 248, 9, 0, 0, 24, 0, 0, 0, 114, 97, 110, 100, 111, 109, 78, 117, 109, 98, 101, 114, 115, 66, 76, 69, 46, 112, 104, 121, 112, 104, 111, 120, 221, 86, 77, 107, 220, 48, 16, 61, 103, 127, 133, 208, 169, 61, 184, 254, 90, 239, 58, 32, 47, 20, 210, 67, 161, 13, 165, 16, 154, 61, 202, 150, 188, 22, 216, 146, 177, 228, 116, 221, 95, 223, 177, 252, 177, 206, 54, 109, 210, 16, 104, 201, 94, 172, 121, 154, 55, 35, 207, 155, 29, 139, 212, 69, 87, 23, 234, 136, 238, 120, 163, 133, 146, 9, 246, 223, 69, 24, 149, 42, 163, 37, 79, 48, 227, 120, 183, 66, 240, 35, 70, 152, 146, 239, 26, 42, 153, 170, 174, 219, 42, 5, 119, 226, 14, 224, 224, 144, 81, 195, 15, 170, 233, 118, 30, 186, 226, 105, 123, 32, 238, 140, 12, 14, 140, 235, 172, 17, 181, 129, 44, 187, 170, 187, 58, 89, 196, 93, 110, 141, 206, 212, 80, 39, 83, 210, 80, 33, 33, 213, 128, 218, 157, 25, 68, 90, 252, 128, 35, 122, 24, 105, 67, 141, 200, 18, 156, 211, 82, 195, 129, 141, 79, 220, 217, 235, 175, 152, 153, 106, 165, 121, 46, 185, 126, 46, 177, 248, 133, 72, 220, 7, 223, 159, 8, 89, 183, 102, 17, 59, 45, 91, 110, 148, 50, 5, 146, 180, 130, 216, 239, 27, 214, 10, 169, 48, 170, 20, 3, 83, 42, 35, 114, 145, 209, 190, 174, 160, 227, 204, 179, 92, 213, 26, 8, 134, 178, 130, 54, 9, 142, 46, 243, 200, 167, 107, 207, 137, 227, 40, 112, 214, 52, 229, 14, 141, 188, 220, 9, 216, 58, 226, 155, 148, 193, 102, 134, 17, 63, 154, 134, 38, 216, 136, 106, 172, 242, 16, 227, 249, 145, 231, 200, 240, 166, 115, 3, 230, 165, 162, 38, 12, 62, 9, 3, 205, 245, 65, 50, 65, 229, 172, 204, 148, 240, 247, 249, 252, 48, 140, 121, 230, 165, 78, 204, 185, 231, 172, 47, 121, 232, 164, 124, 227, 59, 81, 184, 101, 153, 183, 9, 131, 212, 15, 159, 144, 175, 120, 210, 203, 209, 232, 146, 81, 207, 223, 58, 241, 198, 131, 100, 49, 131, 21, 223, 110, 156, 205, 118, 27, 179, 140, 165, 204, 247, 227, 39, 36, 171, 23, 201, 78, 233, 136, 59, 203, 59, 30, 130, 184, 139, 6, 32, 203, 90, 220, 175, 12, 161, 146, 150, 157, 22, 26, 233, 146, 243, 218, 246, 28, 82, 242, 70, 243, 230, 99, 31, 96, 238, 188, 145, 59, 185, 143, 246, 157, 224, 223, 199, 245, 108, 163, 18, 148, 43, 19, 124, 13, 203, 222, 198, 103, 26, 28, 26, 90, 23, 147, 147, 125, 248, 24, 81, 93, 243, 204, 124, 237, 251, 47, 193, 65, 63, 87, 180, 233, 250, 177, 82, 66, 83, 107, 140, 250, 199, 55, 193, 76, 1, 178, 245, 101, 42, 21, 212, 52, 207, 183, 60, 8, 48, 170, 105, 99, 4, 45, 111, 106, 6, 99, 100, 58, 49, 42, 132, 54, 48, 82, 44, 193, 166, 185, 29, 250, 17, 189, 209, 111, 71, 100, 15, 241, 149, 170, 145, 109, 153, 62, 139, 58, 220, 206, 124, 48, 246, 147, 129, 142, 95, 26, 158, 137, 65, 24, 104, 138, 238, 190, 169, 251, 17, 248, 89, 72, 32, 211, 214, 168, 9, 160, 199, 51, 64, 200, 253, 185, 199, 12, 84, 150, 14, 229, 175, 44, 205, 179, 200, 126, 66, 236, 226, 126, 33, 109, 49, 173, 202, 136, 30, 133, 78, 112, 55, 247, 190, 69, 31, 241, 62, 14, 255, 203, 7, 92, 137, 107, 37, 218, 173, 30, 211, 45, 248, 135, 186, 13, 159, 23, 228, 57, 190, 247, 10, 116, 43, 254, 168, 217, 75, 106, 22, 254, 31, 154, 189, 6, 209, 234, 23, 19, 237, 4, 244, 3, 115, 183, 154, 102, 248, 98, 188, 18, 126, 172, 85, 3, 1, 46, 46, 136, 230, 102, 252, 138, 247, 51, 118, 216, 64, 0, 158, 79, 218, 254, 102, 48, 57, 210, 234, 116, 105, 233, 241, 101, 82, 160, 78, 243, 125, 202, 66, 220, 241, 182, 183, 91, 253, 4, 80, 75, 1, 2, 31, 0, 20, 0, 0, 0, 8, 0, 161, 99, 195, 78, 150, 29, 14, 210, 176, 2, 0, 0, 248, 9, 0, 0, 24, 0, 36, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 114, 97, 110, 100, 111, 109, 78, 117, 109, 98, 101, 114, 115, 66, 76, 69, 46, 112, 104, 121, 112, 104, 111, 120, 10, 0, 32, 0, 0, 0, 0, 0, 1, 0, 24, 0, 224, 161, 158, 41, 247, 25, 213, 1, 17, 171, 213, 95, 124, 20, 213, 1, 64, 50, 206, 95, 124, 20, 213, 1, 80, 75, 5, 6, 0, 0, 0, 0, 1, 0, 1, 0, 106, 0, 0, 0, 230, 2, 0, 0, 0, 0};
  uint8_t header[20] = {0}; //20 byte as standard package size for ble transfer
  char phyphox[] = "phyphox";
  uint32_t checksum = CRC32::calculate(EXPERIMENT, sizeof(EXPERIMENT));
  int arrayLength = sizeof(EXPERIMENT);
  uint8_t experimentSizeArray[4] = {0};
  experimentSizeArray[0]=  (arrayLength >> 24);
  experimentSizeArray[1]=  (arrayLength >> 16);
  experimentSizeArray[2]=  (arrayLength >> 8);
  experimentSizeArray[3]=  arrayLength; 

  uint8_t checksumArray[4] = {0};
  checksumArray[0]= (checksum >> 24) & 0xFF; //((checksum & 0xff000000) >> 24);
  checksumArray[1]= (checksum >> 16) & 0xFF;  //((checksum & 0x00ff0000) >> 16);
  checksumArray[2]= (checksum >> 8) & 0xFF;  //((checksum & 0x0000ff00) >> 8);
  checksumArray[3]= checksum & 0xFF;  //checksum & 0xff;//

  //build up header data
  memcpy(header,phyphox, 7);
  memcpy(header+7,experimentSizeArray, 4);
  memcpy(header+11,checksumArray, 4);
  Serial.println("Trying to send...");

  //now send header 
  BLE::Instance(BLE::DEFAULT_INSTANCE).gattServer().write(dataChar.getValueHandle(), header, sizeof(header));
  delay(100); //necessary? check out later...
  
  //now send experiment 
  for(int i = 0; i < sizeof(EXPERIMENT)/20; ++i)
  {
    memcpy(header, EXPERIMENT + i*20, 20); //use header to save memory
    BLE::Instance(BLE::DEFAULT_INSTANCE).gattServer().write(dataChar.getValueHandle(), header, sizeof(header));
    delay(100);
  }
  if(sizeof(EXPERIMENT)%20 != 0)
  {
    const int rest = sizeof(EXPERIMENT)%20;
    uint8_t slice[rest];
    memcpy(slice, EXPERIMENT + sizeof(EXPERIMENT)-rest, rest);
    BLE::Instance(BLE::DEFAULT_INSTANCE).gattServer().write(dataChar.getValueHandle(), slice, sizeof(slice));
    delay(100);
  }
}

void writeCharCallback(const GattWriteCallbackParams *params)
{  
    /* Check to see what characteristic was written, by handle */
    if(params->handle == writeChar.getValueHandle()) {
        /* toggle LED if only 1 byte is written */
        if(params->len == 1)
        {
          switch(params -> data[0])
          {
            case 0: queue.event(&transferExp); queue.call(&transferExp); break;
            case 1: queue.event(&failTransfer); queue.call(&failTransfer); break;
            case 2: queue.event(&transferData); queue.call(&transferData); break;
            default: break;
          }
        }
    }
}

void bleInitComplete(BLE::InitializationCompleteCallbackContext *params)
{
    BLE &ble          = params->ble;
    ble_error_t error = params->error;
    
    if (error != BLE_ERROR_NONE) {
        return;
    }
    ble.gap().onConnection(connectionCallback);
    ble.gap().onDisconnection(disconnectionCallback);
    ble.gattServer().onDataWritten(writeCharCallback);
    ble.gattServer().onUpdatesEnabled(when_subscription_received);
 
    /* Setup advertising */
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE); // BLE only, no classic BT
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED); // advertising type
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME)); // add name
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS, (uint8_t *)customServiceUUID, sizeof(customServiceUUID));
    ble.gap().setAdvertisingInterval(100); // 100ms.
 
    /* Add our custom service */
    ble.gattServer().addService();
 
    /* Start advertising */
    ble.gap().startAdvertising(); //deprecated use of startAdvertising -> to be fixed
}

void waitForEvent(BLE* ble)
{
  while(1)
    ble -> waitForEvent();
}


void setup() 
{
  Serial.begin(9600); 
}

void loop() 
{
  BLE& ble = BLE::Instance(BLE::DEFAULT_INSTANCE);
  ble.init(bleInitComplete);
  ble_server.start(mbed::callback(waitForEvent, &ble));
  transfer.start(mbed::callback(&queue, &EventQueue::dispatch_forever)); //it doesn't work with a simple functioin call. figure out later why seems strange
  while(1)
  {
  }   
}
