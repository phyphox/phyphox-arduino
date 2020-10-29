#include <phyphoxBle.h>
#include "SparkFun_SCD30_Arduino_Library.h" 
#include <FS.h>
#include <SPIFFS.h>

bool FORMATFLASH = false;

SCD30 airSensor;
File fsUploadFile;                                    // a File variable to temporarily store the received file

int datasetNumber = 0;
int lineNumber = 0;

float topThreshold = 1000;
float bottomThreshold = 600;
//store data every 12s over 24h -> maxDatasets = 24*60*60/2 = 7200
int maxDatasets=7200;

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

uint8_t CO2Monitor[1879] = {80, 75, 3, 4, 20, 0, 0, 0, 8, 0, 118, 100, 93, 81, 154, 70, 250, 22, 175, 6, 0, 0, 130, 40, 0, 0, 17, 0, 0, 0, 99, 111, 50, 115, 101, 110, 115, 111, 114, 46, 112, 104, 121, 112, 104, 111, 120, 213, 26, 219, 110, 219, 54, 244, 185, 253, 10, 78, 192, 176, 4, 155, 35, 203, 118, 154, 62, 200, 1, 178, 110, 93, 7, 20, 8, 176, 116, 88, 135, 162, 8, 104, 137, 138, 8, 200, 164, 70, 82, 73, 92, 244, 169, 63, 180, 125, 67, 63, 101, 95, 50, 94, 36, 89, 146, 117, 99, 106, 183, 157, 243, 16, 137, 60, 247, 115, 120, 206, 33, 41, 63, 141, 55, 105, 76, 239, 193, 45, 98, 28, 83, 178, 116, 188, 19, 111, 234, 156, 63, 6, 242, 231, 11, 44, 18, 116, 254, 236, 242, 223, 15, 31, 38, 87, 136, 112, 202, 124, 215, 140, 153, 249, 0, 10, 116, 67, 217, 166, 1, 82, 14, 231, 80, 52, 161, 236, 124, 54, 191, 184, 88, 156, 249, 174, 121, 51, 51, 33, 226, 1, 195, 169, 144, 140, 207, 95, 197, 152, 3, 116, 159, 34, 134, 215, 136, 8, 112, 139, 121, 6, 19, 252, 14, 113, 32, 98, 4, 184, 166, 13, 66, 40, 32, 160, 145, 30, 42, 68, 127, 118, 57, 203, 167, 79, 124, 183, 74, 210, 48, 193, 1, 37, 32, 162, 108, 13, 197, 210, 225, 130, 97, 114, 227, 24, 129, 125, 87, 205, 229, 96, 138, 242, 68, 190, 10, 136, 137, 180, 133, 25, 213, 51, 229, 32, 224, 82, 156, 165, 35, 205, 19, 208, 107, 76, 124, 183, 156, 233, 135, 22, 104, 157, 218, 192, 199, 217, 218, 138, 188, 52, 88, 19, 190, 27, 193, 211, 210, 19, 122, 55, 142, 190, 151, 139, 111, 131, 160, 228, 111, 194, 15, 153, 83, 49, 177, 51, 168, 21, 134, 18, 201, 10, 65, 27, 117, 7, 99, 64, 139, 241, 212, 45, 101, 183, 18, 187, 6, 60, 8, 29, 226, 40, 178, 34, 31, 80, 198, 80, 32, 172, 188, 27, 98, 158, 38, 112, 99, 101, 31, 107, 36, 229, 99, 91, 28, 237, 230, 54, 164, 190, 0, 7, 152, 96, 81, 162, 191, 67, 140, 142, 94, 26, 105, 170, 80, 44, 193, 119, 56, 140, 19, 142, 92, 38, 225, 3, 208, 24, 146, 57, 18, 221, 162, 31, 51, 33, 232, 200, 44, 116, 234, 156, 175, 54, 2, 93, 48, 214, 98, 123, 223, 109, 205, 173, 62, 38, 105, 38, 42, 68, 87, 73, 134, 4, 165, 34, 6, 56, 92, 58, 50, 171, 59, 128, 192, 53, 202, 31, 193, 154, 134, 242, 153, 80, 129, 35, 28, 64, 149, 224, 29, 192, 160, 48, 210, 243, 108, 165, 242, 254, 10, 93, 146, 43, 1, 153, 212, 36, 130, 9, 71, 206, 150, 188, 102, 65, 51, 33, 121, 130, 32, 134, 108, 233, 4, 97, 24, 121, 211, 233, 108, 50, 159, 70, 103, 147, 197, 147, 51, 111, 242, 116, 181, 152, 79, 78, 209, 98, 186, 130, 167, 243, 83, 111, 1, 29, 32, 197, 46, 107, 99, 148, 80, 40, 230, 179, 151, 88, 200, 26, 248, 51, 9, 49, 36, 101, 45, 48, 164, 15, 207, 79, 86, 191, 136, 35, 169, 224, 34, 95, 38, 95, 132, 249, 83, 179, 220, 170, 188, 31, 61, 122, 116, 80, 150, 222, 204, 172, 183, 86, 133, 125, 183, 140, 158, 34, 232, 42, 225, 229, 239, 128, 239, 198, 90, 195, 120, 152, 236, 40, 50, 31, 175, 72, 185, 24, 106, 235, 2, 147, 33, 153, 171, 114, 250, 144, 192, 100, 195, 101, 87, 196, 19, 132, 82, 185, 54, 79, 228, 50, 160, 228, 119, 142, 216, 175, 138, 210, 54, 198, 183, 36, 191, 153, 76, 174, 199, 252, 38, 147, 243, 26, 210, 251, 173, 234, 87, 72, 0, 101, 103, 160, 210, 78, 213, 38, 239, 155, 72, 31, 255, 222, 215, 159, 162, 92, 35, 13, 112, 4, 138, 212, 7, 164, 9, 166, 63, 128, 59, 4, 98, 120, 139, 128, 204, 0, 96, 35, 101, 12, 145, 64, 108, 45, 19, 74, 104, 36, 165, 68, 119, 133, 247, 0, 222, 99, 126, 2, 94, 34, 241, 29, 7, 25, 71, 122, 84, 209, 226, 2, 174, 83, 213, 60, 66, 178, 1, 4, 221, 153, 102, 50, 165, 88, 118, 155, 144, 132, 64, 54, 136, 129, 140, 7, 169, 61, 213, 56, 198, 25, 69, 183, 169, 40, 48, 149, 132, 178, 4, 157, 128, 186, 192, 122, 174, 117, 245, 229, 25, 188, 45, 96, 115, 164, 237, 136, 84, 57, 65, 156, 47, 29, 193, 50, 228, 0, 244, 151, 236, 127, 243, 151, 246, 224, 76, 16, 100, 101, 16, 20, 214, 218, 9, 179, 10, 134, 216, 164, 50, 93, 222, 194, 68, 145, 156, 246, 65, 214, 105, 23, 74, 180, 193, 27, 197, 90, 17, 140, 56, 187, 154, 227, 232, 255, 161, 118, 153, 109, 70, 235, 61, 94, 235, 79, 95, 168, 191, 229, 117, 26, 172, 116, 161, 254, 18, 11, 21, 166, 41, 34, 225, 176, 205, 189, 253, 120, 231, 75, 65, 230, 174, 134, 50, 70, 229, 99, 45, 165, 111, 139, 159, 239, 22, 214, 168, 133, 182, 101, 56,55, 155, 175, 253, 169, 40, 91, 5, 177, 113, 128, 219, 173, 27, 67, 60, 75, 132, 83, 72, 100, 68, 110, 209, 181, 43, 164, 11, 253, 247, 100, 229, 166, 41, 250, 77, 253, 233, 235, 233, 66, 211, 220, 22, 134, 175, 107, 61, 213, 195, 36, 160, 118, 121, 169, 216, 241, 247, 219, 112, 52, 251, 178, 239, 28, 45, 192, 246, 12, 97, 79, 34, 20, 221, 231, 104, 9, 20, 194, 94, 4, 48, 235, 194, 222, 3, 251, 226, 252, 48, 227, 239, 139, 123, 195, 238, 195, 204, 245, 233, 197, 126, 120, 63, 172, 30, 31, 54, 115, 92, 81, 102, 186, 199, 16, 37, 178, 33, 5, 97, 150, 38, 106, 123, 138, 56, 248, 156, 153, 131, 83, 54, 182, 163, 25, 223, 255, 4, 116, 60, 172, 137, 177, 177, 208, 58, 40, 134, 189, 88, 113, 123, 61, 134, 7, 113, 2, 106, 139, 161, 192, 109, 113, 182, 167, 138, 187, 181, 209, 120, 196, 190, 58, 206, 250, 12, 83, 57, 177, 219, 229, 184, 27, 218, 10, 18, 49, 68, 4, 150, 1, 57, 50, 60, 174, 141, 66, 86, 75, 172, 75, 162, 6, 255, 237, 12, 131, 228, 6, 69, 56, 17, 136, 141, 20, 75, 145, 26, 219, 14, 233, 30, 66, 238, 12, 7, 26, 207, 157, 18, 105, 23, 195, 149, 128, 177, 8, 123, 75, 140, 81, 30, 233, 241, 65, 21, 42, 160, 253, 243, 131, 193, 95, 207, 229, 93, 130, 180, 5, 66, 205, 223, 251, 74, 189, 242, 247, 11, 34, 72, 157, 1, 234, 221, 185, 222, 248, 235, 225, 207, 154, 122, 213, 213, 82, 150, 64, 144, 255, 95, 58, 71, 111, 188, 235, 183, 147, 55, 179, 183, 223, 191, 153, 191, 61, 118, 159, 76, 199, 238, 99, 219, 125, 252, 208, 93, 111, 247, 142, 124, 32, 146, 202, 3, 254, 93, 63, 230, 58, 238, 201, 135, 207, 117, 64, 40, 195, 1, 46, 171, 103, 32, 80, 104, 28, 169, 195, 229, 115, 250, 176, 22, 159, 95, 99, 93, 108, 3, 174, 249, 106, 36, 210, 54, 53, 154, 219, 129, 158, 80, 168, 94, 219, 12, 165, 141, 78, 200, 26, 100, 237, 130, 102, 40, 141, 116, 67, 182, 165, 19, 223, 45, 78, 75, 13, 156, 127, 139, 209, 93, 254, 92, 190, 131, 4, 174, 144, 220, 140, 255, 36, 183, 119, 205, 53, 121, 195, 96, 26, 23, 0, 250, 118, 216, 1, 9, 38, 232, 15, 28, 138, 120, 233, 204, 212, 249, 110, 66, 165, 29, 205, 61, 182, 99, 64, 95, 47, 29, 37, 107, 254, 246, 103, 137, 153, 17, 44, 94, 27, 67, 235, 103, 57, 147, 166, 235, 6, 79, 176, 245, 145, 202, 92, 75, 231, 190, 121, 31, 165, 39, 7, 144, 54, 245, 11, 182, 54, 135, 186, 90, 185, 198, 160, 46, 149, 13, 133, 203, 203, 160, 84, 6, 21, 54, 71, 217, 82, 115, 30, 96, 85, 195, 35, 28, 148, 97, 148, 233, 219, 34, 165, 19, 232, 146, 175, 220, 116, 182, 138, 164, 217, 55, 6, 57, 74, 161, 204, 230, 50, 29, 196, 8, 223, 196, 66, 139, 226, 246, 57, 234, 149, 140, 61, 85, 0, 50, 134, 74, 7, 69, 81, 16, 122, 176, 203, 65, 162, 138, 209, 226, 166, 143, 255, 60, 59, 148, 155, 20, 235, 79, 117, 84, 77, 97, 91, 119, 245, 233, 102, 164, 59, 180, 191, 94, 100, 107, 28, 98, 177, 217, 174, 166, 233, 217, 252, 249, 69, 151, 179, 226, 18, 188, 197, 83, 223, 30, 202, 79, 181, 52, 245, 48, 55, 109, 245, 180, 245, 81, 183, 86, 90, 174, 209, 14, 242, 93, 149, 241, 58, 18, 224, 11, 204, 165, 215, 54, 205, 28, 152, 31, 36, 231, 64, 229, 249, 50, 77, 66, 125, 34, 214, 0, 183, 59, 236, 213, 208, 121, 38, 239, 58, 217, 27, 67, 125, 226, 205, 166, 131, 12, 76, 109, 107, 173, 48, 174, 81, 242, 33, 145, 140, 73, 68, 11, 227, 92, 197, 52, 63, 38, 140, 24, 93, 131, 21, 146, 77, 140, 185, 105, 226, 234, 250, 185, 184, 52, 170, 124, 207, 164, 186, 28, 66, 193, 218, 192, 169, 139, 77, 183, 215, 252, 30, 136, 105, 198, 70, 218, 124, 242, 228, 0, 70, 169, 203, 51, 179, 146, 199, 155, 29, 92, 160, 133, 149, 64, 179, 197, 126, 5, 170, 174, 47, 243, 92, 116, 30, 210, 231, 122, 235, 95, 9, 46, 145, 127, 200, 208, 178, 138, 244, 119, 103, 249, 244, 43, 213, 251, 30, 201, 28, 119, 220, 204, 92, 10, 168, 27, 79, 125, 0, 119, 36, 107, 241, 113, 189, 21, 232, 71, 170, 84, 18, 112, 36, 43, 195, 113, 179, 64, 245, 163, 23, 25, 14, 28, 125, 123, 220, 72, 153, 117, 68, 223, 149, 250, 23, 118, 42, 108, 227, 187, 249, 199, 123, 231, 143, 255, 3, 80, 75, 1, 2, 31, 0, 20, 0, 0, 0, 8, 0, 118, 100, 93, 81, 154, 70, 250, 22, 175, 6, 0, 0, 130, 40, 0, 0, 17, 0, 36, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 99, 111, 50, 115, 101, 110, 115, 111, 114, 46, 112,104, 121, 112, 104, 111, 120, 10, 0, 32, 0, 0, 0, 0, 0, 1, 0, 24, 0, 184, 238, 174, 163, 231, 173, 214, 1, 87, 124, 129, 163, 231, 173, 214, 1, 87, 124, 129, 163, 231, 173, 214, 1, 80, 75, 5, 6, 0, 0, 0, 0, 1, 0, 1, 0, 99, 0, 0, 0, 222, 6, 0, 0, 0, 0};

void setup() {
  PhyphoxBLE::start("CO2 Monitor", &CO2Monitor[0], sizeof(CO2Monitor));                 //Start the BLE server
  PhyphoxBLE::configHandler=&receivedConfig;
  Serial.begin(115200);

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

}

void loop() {

  if (airSensor.dataAvailable())
  {
    measuredData[0]=airSensor.getCO2();
    measuredData[1]=airSensor.getTemperature();
    measuredData[2]=airSensor.getHumidity();
    measuredData[3] = millis()/1000;

    echoDataset("Measured", measuredData);

    PhyphoxBLE::write(measuredData[0],measuredData[1],measuredData[2],measuredData[3]);     //Send value to phyphox  

    if(averageCounter < averageOver){
      averageMeasuredData[0]+=measuredData[0]/averageOver;
      averageMeasuredData[1]+=measuredData[1]/averageOver;
      averageMeasuredData[2]+=measuredData[2]/averageOver;
      averageMeasuredData[3]+=measuredData[3]/averageOver;
      averageCounter+=1;
    }else{
      storeMeasuredData(averageMeasuredData);
      averageMeasuredData[0]=0;
      averageMeasuredData[1]=0;
      averageMeasuredData[2]=0;
      averageMeasuredData[3]=0;
      averageCounter=0;      
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
    1       bool calibrate
     */

    uint8_t readArray[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    PhyphoxBLE::read(&readArray[0],20);
    if(readArray[0]==1){
        // just send current data
        Serial.println("Resending of old data requested.");
        oldDataTransmissionOffset = 0;
        oldDataTransmissionSet = datasetNumber;
    }
    if(readArray[1]==1){
      //CALIBRATION
      Serial.print("Calibration with fresh air ");
      airSensor.setAutoSelfCalibration(false);
      Serial.println(airSensor.setForcedRecalibrationFactor(400));
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
  File file = SPIFFS.open("/set"+String(datasetNumber)+".txt", "a");  
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
  