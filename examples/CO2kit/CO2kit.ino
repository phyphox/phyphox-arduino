#include <phyphoxBle.h>
#include "SparkFun_SCD30_Arduino_Library.h" 
#include <FS.h>
#include <SPIFFS.h>

bool FORMATFLASH = true;

SCD30 airSensor;
File fsUploadFile;                                    // a File variable to temporarily store the received file

int datasetNumber = 0;
int lineNumber = 0;

float topThreshold = 1000;
float bottomThreshold = 600;
//store data every 12s over 4h -> maxDatasets = 4*60*60/2 = 1200
int maxDatasets=1200;

const int measuredDataLength = 4;
float measuredData[measuredDataLength];  //co2,temperature,humidity,seconds since uptime, number of dataset

float averageMeasuredData[measuredDataLength];
int averageOver = 6; //6*2s rate = store data every 12s
int averageCounter=0;

void receivedConfig();

int oldDataTransmissionOffset = -1;
int oldDataTransmissionSet = -1;

int pinGreen = 26;
int pinRed = 33;
int pinBlue = 25;
bool RED = 1;
bool GREEN = 0;
bool BLUE = 1;

int readyPin = 5;

uint8_t versionID = 2;
uint8_t currentOffset;
uint8_t statusCO2calibration=0;
/*
 * The following byte array contains the complete phyphox experiment.
 * The phyphox experiment (xml-file) where the byte array is generated from can be found in the github repository https://github.com/Dorsel89/phyphox-hardware (CO2-Monitor Kit folder).
*/

uint8_t CO2Monitor[2133] = {80, 75, 3, 4, 20, 0, 8, 0, 8, 0, 78, 116, 126, 81, 0, 0, 0, 0, 0, 0, 0, 0, 207, 43, 0, 0, 17, 0, 32, 0, 99, 111, 50, 115, 101, 110, 115, 111, 114, 46, 112, 104, 121, 112, 104, 111, 120, 85, 84, 13, 0, 7, 228, 244, 196, 95, 16, 245, 196, 95, 228, 244, 196, 95, 117, 120, 11, 0, 1, 4, 232, 3, 0, 0, 4, 232, 3, 0, 0, 213, 90, 235, 110, 219, 54, 20, 254, 157, 62, 5, 39, 160, 88, 130, 205, 177, 101, 59, 77, 81, 216, 6, 178, 110, 93, 7, 20, 8, 176, 116, 88, 135, 162, 40, 104, 137, 138, 8, 200, 162, 38, 82, 78, 92, 244, 87, 95, 104, 123, 134, 62, 202, 158, 100, 135, 20, 117, 191, 152, 74, 157, 116, 75, 127, 84, 18, 207, 33, 191, 115, 63, 36, 189, 136, 252, 93, 228, 179, 91, 180, 37, 49, 167, 44, 92, 90, 246, 169, 61, 177, 86, 143, 16, 252, 45, 4, 21, 1, 89, 61, 191, 252, 231, 211, 167, 209, 21, 9, 57, 139, 23, 227, 244, 91, 58, 238, 96, 65, 174, 89, 188, 171, 145, 228, 159, 53, 21, 11, 88, 188, 154, 206, 46, 46, 230, 231, 48, 168, 222, 210, 17, 151, 112, 39, 166, 145, 128, 133, 87, 175, 125, 202, 17, 185, 141, 72, 76, 55, 36, 20, 104, 75, 121, 130, 3, 250, 129, 112, 36, 124, 130, 184, 154, 27, 185, 88, 96, 196, 60, 245, 41, 131, 254, 252, 114, 170, 135, 79, 23, 227, 242, 148, 233, 34, 212, 97, 33, 242, 88, 188, 193, 98, 105, 113, 17, 211, 240, 218, 74, 1, 47, 198, 114, 44, 195, 2, 51, 143, 224, 85, 96, 26, 130, 46, 210, 175, 26, 191, 254, 136, 56, 192, 89, 90, 160, 30, 135, 189, 167, 161, 148, 69, 143, 244, 83, 11, 178, 137, 134, 208, 251, 201, 102, 208, 244, 160, 176, 58, 125, 55, 131, 173, 208, 135, 236, 198, 108, 126, 91, 195, 31, 194, 32, 241, 215, 233, 247, 169, 83, 46, 50, 76, 161, 131, 56, 36, 164, 97, 75, 72, 165, 54, 56, 246, 72, 49, 72, 128, 33, 216, 7, 193, 54, 71, 44, 169, 93, 234, 121, 131, 166, 119, 88, 28, 19, 71, 12, 178, 174, 75, 121, 20, 224, 221, 48, 3, 15, 101, 146, 54, 30, 188, 144, 52, 115, 27, 83, 159, 131, 35, 26, 82, 145, 179, 127, 32, 177, 161, 225, 33, 52, 162, 168, 97, 161, 253, 228, 141, 21, 204, 192, 133, 151, 129, 91, 97, 59, 58, 234, 165, 7, 171, 18, 186, 37, 238, 143, 144, 9, 239, 176, 92, 76, 32, 183, 146, 45, 249, 33, 17, 130, 25, 102, 175, 51, 107, 181, 222, 9, 114, 17, 199, 45, 54, 131, 100, 222, 150, 147, 23, 52, 140, 18, 81, 154, 116, 29, 36, 68, 48, 38, 124, 68, 221, 165, 5, 213, 192, 66, 33, 222, 16, 253, 136, 54, 204, 133, 231, 144, 9, 234, 81, 168, 76, 80, 24, 44, 20, 67, 133, 82, 232, 121, 178, 150, 245, 98, 77, 46, 195, 43, 129, 99, 144, 196, 195, 1, 39, 86, 49, 189, 90, 130, 37, 2, 214, 68, 142, 143, 227, 165, 229, 184, 174, 103, 79, 38, 211, 209, 108, 226, 157, 143, 230, 79, 206, 237, 209, 211, 245, 124, 54, 58, 35, 243, 201, 26, 159, 205, 206, 236, 57, 182, 16, 192, 206, 107, 170, 23, 48, 44, 102, 211, 87, 84, 64, 237, 252, 41, 116, 41, 14, 243, 26, 146, 78, 125, 255, 235, 65, 213, 244, 56, 1, 1, 231, 165, 130, 244, 224, 139, 63, 45, 170, 91, 182, 246, 17, 184, 229, 125, 46, 105, 79, 75, 37, 178, 46, 240, 98, 156, 123, 79, 230, 116, 37, 247, 90, 52, 200, 155, 190, 86, 83, 158, 226, 174, 10, 50, 51, 23, 36, 15, 134, 74, 92, 212, 29, 190, 137, 185, 140, 115, 129, 67, 28, 236, 56, 116, 83, 60, 32, 36, 130, 216, 60, 133, 48, 96, 225, 111, 156, 196, 191, 200, 153, 10, 31, 47, 166, 252, 102, 52, 122, 111, 242, 55, 26, 173, 42, 76, 31, 11, 209, 175, 136, 64, 82, 207, 72, 166, 171, 178, 78, 62, 214, 153, 62, 255, 117, 168, 127, 114, 230, 202, 212, 136, 122, 40, 75, 153, 8, 84, 48, 249, 30, 221, 16, 228, 227, 45, 65, 144, 1, 208, 14, 48, 186, 68, 144, 120, 3, 9, 197, 77, 145, 66, 119, 40, 187, 201, 91, 132, 111, 41, 63, 69, 175, 136, 248, 150, 163, 132, 19, 245, 85, 206, 197, 5, 222, 68, 178, 233, 196, 225, 14, 133, 228, 38, 109, 66, 35, 70, 161, 75, 197, 161, 139, 160, 177, 116, 192, 31, 64, 122, 166, 120, 180, 51, 235, 46, 85, 206, 16, 203, 36, 148, 4, 228, 20, 85, 1, 171, 177, 214, 232, 203, 11, 69, 211, 97, 53, 83, 241, 5, 68, 14, 8, 231, 75, 75, 196, 9, 177, 16, 249, 19, 250, 102, 253, 210, 238, 156, 1, 145, 222, 169, 157, 160, 40, 48, 53, 55, 43, 113, 136, 93, 4, 233, 114, 139, 3, 57, 229, 164, 143, 178, 58, 119, 38, 68, 27, 125, 22, 242, 45, 12, 41, 156, 166, 228, 212, 251, 127, 136,157, 103, 27, 99, 185, 205, 165, 254, 242, 64, 253, 85, 215, 105, 180, 86, 133, 186, 132, 237, 193, 2, 21, 71, 17, 9, 221, 253, 58, 183, 15, 99, 157, 175, 69, 169, 77, 141, 193, 71, 225, 177, 146, 210, 139, 226, 183, 24, 103, 218, 168, 184, 246, 64, 119, 174, 55, 95, 135, 19, 17, 90, 5, 177, 179, 208, 184, 91, 182, 152, 240, 36, 16, 86, 134, 40, 133, 220, 34, 107, 206, 91, 115, 105, 83, 111, 48, 212, 114, 93, 21, 253, 170, 254, 242, 120, 186, 80, 115, 22, 133, 161, 248, 251, 47, 196, 83, 213, 77, 116, 207, 105, 156, 151, 178, 147, 130, 126, 29, 26, 47, 159, 247, 157, 230, 137, 49, 63, 123, 56, 16, 132, 172, 251, 52, 70, 144, 31, 102, 124, 41, 128, 52, 46, 134, 91, 224, 80, 43, 223, 77, 249, 135, 90, 253, 46, 122, 63, 152, 228, 119, 170, 199, 247, 155, 57, 174, 88, 156, 118, 143, 46, 9, 160, 33, 69, 110, 18, 5, 114, 123, 74, 184, 28, 125, 176, 204, 193, 1, 134, 97, 71, 99, 222, 255, 56, 189, 93, 85, 155, 143, 153, 82, 43, 167, 216, 111, 197, 146, 217, 171, 62, 188, 151, 39, 63, 137, 52, 230, 40, 157, 69, 26, 243, 20, 167, 145, 205, 218, 152, 90, 100, 120, 117, 156, 246, 41, 166, 116, 210, 215, 92, 177, 233, 218, 146, 146, 196, 36, 20, 20, 28, 210, 180, 225, 237, 182, 100, 103, 136, 117, 33, 170, 173, 95, 140, 196, 56, 188, 38, 30, 13, 68, 99, 219, 212, 5, 43, 93, 194, 172, 29, 82, 61, 4, 236, 12, 247, 52, 158, 141, 18, 57, 204, 135, 75, 14, 51, 192, 237, 135, 46, 98, 98, 145, 30, 27, 148, 169, 156, 150, 109, 73, 101, 150, 125, 206, 95, 205, 229, 93, 64, 218, 28, 161, 98, 239, 3, 54, 109, 63, 147, 144, 200, 51, 192, 244, 192, 66, 110, 252, 115, 72, 15, 150, 122, 229, 149, 84, 18, 96, 164, 255, 95, 90, 199, 111, 237, 247, 239, 70, 111, 167, 239, 190, 123, 59, 123, 119, 50, 126, 50, 49, 221, 199, 14, 115, 139, 125, 187, 222, 238, 29, 249, 30, 79, 202, 47, 6, 154, 118, 212, 50, 30, 200, 134, 47, 148, 67, 72, 197, 33, 14, 213, 211, 17, 196, 77, 13, 169, 220, 229, 33, 109, 56, 44, 31, 125, 133, 186, 216, 229, 4, 185, 173, 12, 153, 138, 212, 152, 222, 42, 244, 184, 66, 249, 186, 103, 95, 218, 48, 163, 172, 92, 236, 236, 75, 35, 221, 148, 109, 233, 4, 170, 159, 62, 45, 213, 135, 167, 91, 74, 110, 202, 151, 191, 242, 29, 5, 120, 77, 96, 51, 46, 47, 70, 234, 49, 121, 29, 227, 200, 207, 8, 212, 173, 178, 133, 2, 26, 146, 223, 169, 43, 252, 165, 53, 149, 231, 187, 1, 3, 61, 166, 247, 223, 86, 74, 250, 6, 58, 1, 192, 170, 223, 254, 200, 57, 147, 144, 138, 55, 169, 162, 213, 51, 140, 68, 209, 166, 182, 102, 201, 70, 50, 115, 45, 173, 219, 250, 61, 86, 139, 117, 26, 76, 187, 234, 197, 92, 155, 65, 199, 74, 184, 218, 71, 85, 42, 107, 2, 231, 151, 65, 17, 56, 21, 77, 143, 178, 65, 114, 238, 80, 89, 195, 61, 234, 228, 110, 148, 168, 219, 34, 41, 19, 234, 194, 151, 111, 58, 91, 33, 169, 229, 107, 31, 57, 137, 48, 100, 115, 72, 7, 62, 161, 215, 190, 80, 80, 234, 7, 22, 21, 67, 189, 6, 223, 147, 5, 32, 137, 73, 110, 32, 207, 115, 92, 27, 119, 25, 72, 148, 57, 90, 204, 244, 249, 239, 231, 247, 101, 166, 106, 160, 220, 205, 80, 21, 129, 135, 154, 171, 79, 182, 210, 14, 253, 30, 237, 245, 50, 217, 80, 151, 202, 115, 168, 44, 154, 38, 231, 179, 23, 23, 93, 198, 242, 115, 242, 22, 75, 61, 190, 47, 59, 85, 210, 212, 221, 204, 84, 200, 57, 212, 70, 221, 82, 21, 39, 24, 38, 6, 130, 15, 144, 241, 58, 18, 224, 75, 202, 193, 106, 187, 122, 14, 212, 7, 201, 154, 40, 63, 95, 102, 129, 171, 78, 196, 186, 245, 109, 112, 216, 171, 168, 117, 38, 239, 58, 217, 51, 153, 125, 100, 79, 219, 207, 15, 203, 11, 164, 181, 173, 116, 49, 121, 116, 87, 160, 229, 155, 244, 242, 124, 53, 229, 167, 154, 51, 12, 15, 9, 167, 154, 125, 147, 88, 110, 146, 16, 23, 16, 216, 252, 153, 85, 134, 92, 195, 160, 1, 171, 241, 13, 142, 16, 4, 132, 188, 180, 71, 27, 124, 155, 254, 86, 128, 229, 230, 66, 25, 231, 98, 12, 148, 117, 30, 91, 243, 216, 214, 170, 193, 240, 76, 183, 99, 8, 154, 218, 180, 69, 3, 152, 210, 241, 178, 121, 148, 8, 218, 227, 26, 202, 48, 202, 10, 52, 244, 88, 38, 254, 149, 207, 244, 145, 171, 23, 179, 13, 172, 5, 13, 97, 122, 107, 199, 229, 85, 126, 118, 1, 87, 250, 77, 153, 236, 24, 65, 210, 77, 74, 39, 47, 137, 235, 243, 87, 93, 153, 133, 193, 14, 170, 57, 248,242, 0, 63, 30, 234, 102, 173, 30, 209, 208, 78, 21, 152, 141, 124, 150, 196, 134, 128, 70, 79, 14, 2, 169, 15, 207, 84, 225, 225, 166, 128, 134, 199, 226, 96, 68, 243, 97, 136, 166, 243, 123, 64, 212, 229, 209, 61, 145, 172, 226, 7, 146, 125, 45, 152, 43, 77, 119, 103, 16, 87, 221, 181, 53, 124, 193, 23, 52, 189, 124, 90, 165, 126, 212, 78, 9, 70, 202, 72, 229, 227, 74, 219, 184, 157, 24, 244, 151, 17, 203, 199, 213, 188, 76, 124, 212, 12, 125, 245, 222, 90, 116, 210, 231, 172, 29, 135, 224, 173, 156, 80, 130, 78, 133, 254, 117, 79, 75, 72, 170, 31, 113, 234, 225, 215, 114, 67, 120, 12, 232, 78, 234, 229, 92, 18, 117, 243, 201, 95, 147, 30, 67, 131, 122, 82, 237, 143, 251, 153, 74, 237, 21, 58, 134, 118, 233, 164, 222, 181, 245, 179, 103, 101, 31, 29, 63, 62, 169, 245, 17, 85, 198, 197, 24, 228, 207, 244, 148, 233, 102, 49, 214, 191, 132, 93, 61, 250, 23, 80, 75, 7, 8, 81, 175, 45, 106, 129, 7, 0, 0, 207, 43, 0, 0, 80, 75, 1, 2, 20, 3, 20, 0, 8, 0, 8, 0, 78, 116, 126, 81, 81, 175, 45, 106, 129, 7, 0, 0, 207, 43, 0, 0, 17, 0, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 180, 129, 0, 0, 0, 0, 99, 111, 50, 115, 101, 110, 115, 111, 114, 46, 112, 104, 121, 112, 104, 111, 120, 85, 84, 13, 0, 7, 228, 244, 196, 95, 16, 245, 196, 95, 228, 244, 196, 95, 117, 120, 11, 0, 1, 4, 232, 3, 0, 0, 4, 232, 3, 0, 0, 80, 75, 5, 6, 0, 0, 0, 0, 1, 0, 1, 0, 95, 0, 0, 0, 224, 7, 0, 0, 0, 0};

void setup() {
  PhyphoxBLE::start("CO2 Monitor", &CO2Monitor[0], sizeof(CO2Monitor));                 //Start the BLE server
  PhyphoxBLE::configHandler=&receivedConfig;
  Serial.begin(115200);
  
  pinMode(readyPin, INPUT); 

  // initialize rgb-led
  pinMode(pinGreen, OUTPUT);
  pinMode(pinRed, OUTPUT);
  pinMode(pinBlue, OUTPUT);
  digitalWrite(pinGreen, GREEN);
  digitalWrite(pinRed, RED);
  digitalWrite(pinBlue, BLUE);
  
  Wire.begin();
  if (airSensor.begin(Wire, false) == false)
  {
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    while (1);
  }

  delay(200);
  Serial.println("init spiffs");
  initStorage();               // Start the SPIFFS and list all contents
  Serial.print("Temperature offset: ");
  Serial.println(airSensor.getTemperatureOffset());
  currentOffset = airSensor.getTemperatureOffset()*10;


}

void loop() {

  if (digitalRead(readyPin))
  {
    measuredData[0]=airSensor.getCO2();
    measuredData[1]=airSensor.getTemperature();
    measuredData[2]=airSensor.getHumidity();
    measuredData[3] = millis()/1000;

    echoDataset("Measured", measuredData);

    uint8_t Data[20]={0};
    memcpy(&Data[0],&measuredData[0],16);
    Data[16]=currentOffset;
    Data[17]=statusCO2calibration;
    Data[18]=versionID;

    PhyphoxBLE::write(&Data[0], 20);
    //PhyphoxBLE::write(measuredData[0],measuredData[1],measuredData[2],measuredData[3]);     //Send value to phyphox  
    if(averageCounter == averageOver){
      storeMeasuredData(averageMeasuredData);
      averageMeasuredData[0]=0;
      averageMeasuredData[1]=0;
      averageMeasuredData[2]=0;
      averageMeasuredData[3]=0;
      averageCounter=0;      
    }
    if(averageCounter < averageOver){
      averageMeasuredData[0]+=measuredData[0]/averageOver;
      averageMeasuredData[1]+=measuredData[1]/averageOver;
      averageMeasuredData[2]+=measuredData[2]/averageOver;
      averageMeasuredData[3]+=measuredData[3]/averageOver;
      averageCounter+=1;
    }

    
    
    updateLED(measuredData[0]);
    delay(10);
  }

  if (oldDataTransmissionOffset >= 0) {
    if (transferOldData(oldDataTransmissionSet, oldDataTransmissionOffset))
      oldDataTransmissionOffset++;
    else {
      oldDataTransmissionOffset = -1;
      Serial.println("Transfer of old date completed.");
    }
    delay(10);
  }
}

void echoDataset(String note, float * data) {
  Serial.print(note);
  Serial.print(" => CO2: ");
  Serial.print(data[0]);
  Serial.print(", Temperature: ");
  Serial.print(data[1]);
  Serial.print(", Humidity: ");
  Serial.print(data[2]);
  Serial.print(", Timestamp: ");
  Serial.println(data[3]);
}

void initStorage() { // Start the SPIFFS and list all contents
  if (!SPIFFS.begin(true)) {
      Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  if(FORMATFLASH){
   bool formatted = SPIFFS.format();
   if ( formatted ) {
    Serial.println("SPIFFS formatted successfully");
   } else {
    Serial.println("Error formatting");
   }
  }

  int totalBytes = SPIFFS.totalBytes();
  int usedBytes = SPIFFS.usedBytes();
  Serial.print("total Bytes ");
  Serial.println(totalBytes);
  Serial.print("total Bytes ");
  Serial.println(usedBytes); 

  File file = SPIFFS.open("/set"+String(datasetNumber)+".txt", FILE_WRITE);
  file.close();

}

void printSetData(float _setNumber){
  File file = SPIFFS.open("/set"+String(_setNumber)+".txt", "r");
  char buffer[4*measuredDataLength];
  while (file.available()) {
    int l = file.readBytes(buffer, 4*measuredDataLength);
    float bufferArray[measuredDataLength];
    memcpy(&bufferArray[0],&buffer[0],4*measuredDataLength);
    Serial.print(bufferArray[0]);
    Serial.print(" ");
    Serial.print(bufferArray[1]);
    Serial.print(" ");      
    Serial.print(bufferArray[2]);
    Serial.print(" ");      
    Serial.println(bufferArray[3]);
    }
  }
void receivedConfig(){

    /*
    byte    information
    0       bool transfer Data
    1       calibrate: 1=CO2 2=temperature
     */

    uint8_t readArray[6] = {0};
    PhyphoxBLE::read(&readArray[0],6);
    if(readArray[0]==1){
         Serial.println("Resending of old data requested.");
        oldDataTransmissionOffset = 0;
        oldDataTransmissionSet = datasetNumber;
    }
    if(readArray[1]==1){
      //CO2 CALIBRATION
      Serial.print("Calibration with fresh air ");
      airSensor.setAutoSelfCalibration(false);
      Serial.println(airSensor.setForcedRecalibrationFactor(400));
      statusCO2calibration=1;
      }
    if(readArray[1]==2){  
      //TEMPERATURE CALIBRATION
      Serial.print("Current Offset: ");
      Serial.print(airSensor.getTemperatureOffset());
      Serial.println(" °C");
      delay(50);
      currentOffset = readArray[5]/10.0;
      airSensor.setTemperatureOffset(currentOffset);
      Serial.print("New Offset: ");    
      Serial.print(currentOffset);
      Serial.println(" °C");
      delay(50);
      Serial.print("Restarting ESP32..");
      delay(1000);
      ESP.restart();
      }
      
}

bool transferOldData(int setNumber, int offset){
    File file = SPIFFS.open("/set"+String(setNumber)+".txt", "r");
    file.seek(offset*4*measuredDataLength, SeekSet);
    char buffer[4*measuredDataLength];
    if (!file.available()) {
      file.close();
      return false;
    }
      
    int l = file.readBytes(buffer, 4*measuredDataLength);
    file.close();
    
    float bufferArray[measuredDataLength];
    memcpy(&bufferArray[0],&buffer[0],4*measuredDataLength);

    echoDataset("Old", bufferArray);
    
    PhyphoxBLE::write(bufferArray[0],bufferArray[1],bufferArray[2],bufferArray[3]);     //Send value to phyphox
    
    return true;
}

void storeMeasuredData(float dataArray[4]){
  byte byteArray[4*measuredDataLength];
  memcpy(&byteArray[0],&dataArray[0],4*measuredDataLength);
  File file = SPIFFS.open("/set"+String(datasetNumber)+".txt", "r+");  
  file.seek(lineNumber*4*measuredDataLength, SeekSet);    
  file.write(byteArray, 4*measuredDataLength);
  file.close();
  
  if(lineNumber<(maxDatasets-1)){
      lineNumber+=1;
    }else{
      lineNumber=0;
  }
}

void updateLED(float co2value){
  if(co2value>topThreshold && GREEN == false){
    GREEN=true;
    RED=false;
    digitalWrite(pinGreen, GREEN);
    digitalWrite(pinRed, RED);
  }
  if(co2value<bottomThreshold && RED == false){
    GREEN=false;
    RED=true;
    digitalWrite(pinGreen, GREEN);
    digitalWrite(pinRed, RED);
  }  
    
}