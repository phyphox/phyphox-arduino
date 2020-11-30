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

/*
 * The following byte array contains the complete phyphox experiment.
 * The phyphox experiment (xml-file) where the byte array is generated from can be found in the github repository https://github.com/Dorsel89/phyphox-hardware (CO2-Monitor Kit folder).
*/

uint8_t CO2Monitor[2042] = {80, 75, 3, 4, 20, 0, 8, 0, 8, 0, 88, 90, 126, 81, 0, 0, 0, 0, 0, 0, 0, 0, 169, 41, 0, 0, 17, 0, 32, 0, 99, 111, 50, 115, 101, 110, 115, 111, 114, 46, 112, 104, 121, 112, 104, 111, 120, 85, 84, 13, 0, 7, 8, 199, 196, 95, 9, 199, 196, 95, 30, 199, 196, 95, 117, 120, 11, 0, 1, 4, 232, 3, 0, 0, 4, 232, 3, 0, 0, 213, 90, 253, 110, 219, 54, 16, 255, 59, 125, 10, 78, 64, 177, 4, 155, 227, 207, 52, 197, 96, 27, 200, 186, 118, 29, 80, 32, 192, 210, 97, 29, 138, 162, 160, 37, 42, 34, 32, 139, 26, 73, 37, 113, 209, 191, 250, 66, 219, 51, 244, 81, 246, 36, 59, 82, 212, 183, 100, 83, 169, 157, 110, 233, 31, 149, 168, 59, 242, 62, 126, 188, 59, 30, 61, 143, 131, 77, 28, 176, 59, 116, 67, 184, 160, 44, 90, 56, 227, 211, 241, 200, 89, 62, 66, 240, 55, 151, 84, 134, 100, 249, 236, 242, 159, 79, 159, 6, 87, 36, 18, 140, 207, 135, 233, 88, 250, 221, 197, 146, 92, 51, 190, 169, 145, 228, 195, 134, 138, 133, 140, 47, 39, 211, 139, 139, 217, 57, 124, 212, 111, 233, 23, 143, 8, 151, 211, 88, 194, 194, 203, 215, 1, 21, 136, 220, 197, 132, 211, 53, 137, 36, 186, 161, 34, 193, 33, 253, 64, 4, 146, 1, 65, 66, 207, 141, 60, 44, 49, 98, 190, 30, 202, 68, 127, 118, 57, 49, 159, 79, 231, 195, 242, 148, 233, 34, 212, 101, 17, 242, 25, 95, 99, 185, 112, 132, 228, 52, 186, 118, 82, 129, 231, 67, 245, 45, 147, 5, 102, 30, 192, 171, 196, 52, 2, 91, 164, 163, 70, 126, 51, 136, 4, 136, 179, 112, 192, 60, 46, 123, 79, 35, 165, 139, 249, 178, 157, 90, 146, 117, 220, 135, 62, 72, 214, 189, 166, 7, 131, 213, 233, 187, 25, 198, 90, 250, 136, 221, 218, 205, 63, 54, 226, 247, 97, 80, 242, 215, 233, 119, 153, 83, 45, 210, 207, 160, 189, 56, 148, 72, 253, 150, 80, 70, 109, 112, 236, 208, 162, 151, 2, 125, 100, 239, 37, 182, 189, 196, 138, 218, 163, 190, 223, 107, 122, 151, 113, 78, 92, 217, 203, 187, 30, 21, 113, 136, 55, 253, 28, 220, 151, 73, 249, 184, 247, 66, 202, 205, 109, 76, 219, 0, 142, 104, 68, 101, 206, 254, 129, 112, 75, 199, 195, 214, 136, 227, 134, 135, 118, 147, 55, 86, 176, 19, 46, 186, 12, 189, 123, 176, 113, 2, 49, 146, 220, 144, 31, 19, 41, 153, 101, 20, 58, 115, 150, 171, 141, 36, 23, 156, 183, 216, 30, 130, 114, 91, 108, 157, 211, 40, 78, 100, 105, 210, 85, 152, 16, 201, 152, 12, 16, 245, 22, 14, 68, 117, 7, 69, 120, 77, 204, 35, 90, 51, 15, 158, 35, 38, 169, 79, 33, 195, 64, 128, 119, 16, 135, 76, 163, 165, 23, 201, 74, 197, 253, 21, 185, 140, 174, 36, 230, 160, 137, 143, 67, 65, 156, 98, 122, 189, 4, 75, 36, 172, 137, 220, 0, 243, 133, 227, 122, 158, 63, 30, 141, 38, 131, 233, 200, 63, 31, 204, 158, 156, 143, 7, 79, 87, 179, 233, 224, 140, 204, 70, 43, 124, 54, 61, 27, 207, 176, 131, 64, 236, 60, 55, 250, 33, 195, 114, 58, 121, 69, 37, 228, 192, 231, 145, 71, 113, 148, 231, 130, 116, 234, 195, 175, 7, 217, 207, 23, 4, 20, 156, 149, 18, 203, 131, 47, 254, 180, 200, 82, 217, 218, 71, 71, 71, 7, 93, 114, 60, 41, 165, 186, 186, 194, 243, 97, 142, 158, 12, 116, 37, 120, 205, 27, 228, 77, 172, 213, 140, 167, 185, 171, 138, 76, 237, 21, 201, 55, 67, 101, 95, 212, 1, 223, 148, 185, 44, 231, 28, 71, 56, 220, 8, 168, 138, 68, 72, 72, 12, 123, 243, 20, 182, 1, 139, 126, 19, 132, 255, 162, 102, 42, 48, 94, 76, 249, 205, 96, 240, 222, 230, 111, 48, 88, 86, 152, 62, 22, 170, 95, 17, 137, 148, 157, 145, 10, 59, 101, 155, 124, 172, 51, 125, 254, 107, 95, 255, 212, 204, 149, 169, 17, 245, 81, 22, 250, 16, 152, 96, 244, 61, 186, 37, 40, 192, 55, 4, 65, 4, 64, 27, 144, 209, 35, 146, 240, 53, 4, 20, 47, 149, 20, 170, 60, 85, 21, 222, 33, 124, 71, 197, 41, 122, 69, 228, 183, 2, 37, 130, 232, 81, 53, 151, 144, 120, 29, 171, 226, 17, 71, 27, 20, 145, 219, 180, 152, 140, 25, 133, 106, 19, 71, 30, 130, 2, 209, 5, 60, 128, 246, 76, 243, 24, 48, 155, 106, 83, 205, 192, 85, 16, 74, 66, 114, 138, 170, 2, 235, 111, 173, 187, 47, 15, 248, 77, 192, 26, 166, 98, 4, 84, 14, 137, 16, 11, 71, 242, 132, 56, 136, 252, 9, 245, 175, 121, 105, 7, 103, 72, 20, 58, 13, 8, 138, 68, 81, 131, 89, 137, 67, 110, 98, 8, 151, 55, 56, 84, 83, 142, 182, 81, 86, 231, 206, 148, 104, 163, 207, 182, 124, 11, 67, 42, 78, 83, 115, 234, 255, 63, 212, 206, 163, 141, 181, 222, 246, 90, 127, 249, 70, 253,213, 228, 105, 180, 210, 137, 186, 36, 219, 131, 109, 84, 28, 199, 36, 242, 118, 219, 124, 188, 31, 239, 124, 45, 74, 227, 106, 12, 24, 133, 199, 74, 72, 47, 146, 223, 124, 152, 89, 163, 2, 237, 158, 112, 174, 23, 95, 251, 83, 17, 74, 5, 185, 113, 208, 176, 91, 55, 78, 68, 18, 74, 39, 147, 40, 21, 185, 69, 215, 156, 183, 6, 105, 91, 52, 88, 90, 185, 110, 138, 237, 166, 254, 242, 253, 116, 161, 231, 44, 18, 67, 241, 247, 95, 216, 79, 85, 152, 152, 154, 211, 58, 46, 101, 39, 254, 237, 54, 180, 94, 62, 175, 59, 237, 3, 99, 222, 67, 216, 147, 8, 89, 245, 105, 45, 65, 222, 148, 248, 82, 1, 210, 125, 209, 223, 3, 251, 90, 249, 126, 198, 223, 215, 234, 247, 177, 251, 222, 52, 191, 87, 62, 62, 108, 228, 184, 98, 60, 173, 30, 61, 18, 66, 65, 138, 188, 36, 14, 213, 241, 148, 8, 245, 245, 193, 34, 135, 0, 49, 44, 43, 26, 251, 250, 199, 221, 90, 85, 181, 97, 204, 150, 90, 131, 98, 183, 23, 75, 110, 175, 98, 120, 39, 79, 222, 81, 180, 230, 40, 245, 20, 173, 121, 138, 174, 98, 51, 55, 166, 30, 233, 159, 29, 39, 219, 12, 83, 234, 216, 53, 87, 108, 66, 91, 81, 18, 78, 34, 73, 1, 144, 182, 5, 111, 183, 39, 59, 183, 88, 151, 68, 181, 245, 139, 47, 28, 71, 215, 196, 167, 161, 108, 28, 155, 186, 196, 74, 151, 176, 43, 135, 116, 13, 1, 39, 195, 29, 133, 103, 35, 69, 246, 195, 112, 9, 48, 61, 96, 223, 119, 17, 27, 143, 108, 241, 65, 153, 202, 109, 57, 150, 84, 102, 217, 5, 254, 106, 44, 239, 18, 164, 13, 8, 21, 127, 239, 177, 104, 251, 153, 68, 68, 245, 0, 211, 134, 133, 58, 248, 231, 34, 61, 88, 232, 85, 87, 75, 73, 136, 145, 249, 127, 225, 28, 191, 29, 191, 127, 55, 120, 59, 121, 247, 221, 219, 233, 187, 147, 225, 147, 145, 237, 57, 182, 31, 44, 118, 157, 122, 187, 79, 228, 59, 144, 148, 55, 248, 155, 126, 52, 58, 238, 201, 135, 47, 52, 32, 148, 225, 144, 128, 236, 233, 74, 226, 165, 142, 212, 112, 121, 72, 31, 246, 139, 71, 95, 33, 47, 118, 129, 32, 247, 149, 37, 83, 17, 26, 211, 219, 129, 45, 80, 40, 95, 219, 236, 10, 27, 118, 148, 149, 11, 154, 93, 97, 164, 155, 178, 45, 156, 64, 246, 51, 221, 82, 211, 60, 189, 161, 228, 182, 124, 137, 171, 222, 81, 136, 87, 4, 14, 227, 63, 193, 241, 174, 190, 39, 175, 57, 142, 131, 140, 64, 223, 14, 59, 40, 164, 17, 249, 157, 122, 50, 88, 56, 19, 213, 223, 13, 25, 216, 49, 189, 199, 118, 82, 210, 55, 80, 9, 128, 172, 230, 237, 143, 156, 51, 137, 168, 124, 147, 26, 90, 63, 195, 151, 56, 94, 215, 214, 44, 249, 72, 69, 174, 133, 115, 87, 191, 143, 106, 241, 78, 131, 105, 83, 189, 96, 107, 115, 232, 80, 43, 87, 27, 212, 169, 178, 166, 112, 126, 25, 20, 3, 168, 104, 218, 202, 6, 205, 133, 75, 85, 14, 247, 169, 155, 195, 40, 209, 183, 69, 74, 39, 212, 37, 95, 126, 232, 108, 21, 73, 47, 95, 27, 20, 36, 198, 16, 205, 33, 28, 4, 132, 94, 7, 82, 139, 82, 111, 88, 84, 28, 245, 26, 176, 167, 18, 64, 194, 73, 238, 32, 223, 119, 189, 49, 238, 114, 144, 44, 115, 180, 184, 233, 243, 223, 207, 14, 229, 166, 234, 70, 185, 159, 163, 42, 10, 247, 117, 215, 54, 221, 74, 39, 244, 3, 250, 235, 101, 178, 166, 30, 85, 125, 168, 108, 55, 141, 206, 167, 47, 46, 186, 156, 21, 228, 228, 45, 158, 122, 124, 40, 63, 85, 194, 212, 253, 220, 84, 232, 217, 215, 71, 221, 90, 21, 29, 12, 27, 7, 193, 0, 68, 188, 142, 0, 248, 146, 10, 240, 218, 166, 30, 3, 77, 35, 217, 16, 229, 253, 101, 22, 122, 186, 35, 214, 109, 111, 139, 102, 175, 166, 54, 145, 188, 171, 179, 103, 51, 251, 96, 60, 105, 239, 31, 150, 23, 72, 115, 91, 107, 134, 25, 166, 74, 90, 34, 89, 221, 105, 86, 3, 101, 194, 213, 121, 198, 212, 43, 224, 205, 31, 28, 77, 117, 100, 124, 84, 201, 170, 122, 124, 141, 99, 4, 176, 85, 87, 235, 104, 141, 239, 210, 155, 121, 150, 27, 117, 62, 4, 130, 58, 233, 224, 73, 70, 172, 158, 150, 99, 20, 176, 132, 183, 83, 130, 57, 50, 82, 245, 184, 156, 104, 90, 209, 78, 60, 153, 229, 196, 234, 113, 57, 43, 19, 43, 90, 173, 176, 129, 146, 122, 181, 50, 82, 205, 101, 62, 203, 140, 117, 21, 48, 211, 75, 245, 57, 91, 163, 21, 129, 74, 47, 189, 142, 19, 234, 142, 62, 187, 89, 43, 253, 232, 75, 149, 130, 96, 156, 117, 74, 167, 110, 127, 235, 243, 87, 49, 154, 90, 198, 18, 152, 96, 203, 253, 35, 167, 42, 143, 177, 190, 173, 64, 7, 193, 114, 85, 162, 89, 63,137, 0, 22, 123, 149, 168, 28, 134, 210, 231, 172, 64, 3, 175, 87, 122, 86, 0, 47, 105, 126, 239, 209, 18, 108, 244, 207, 243, 204, 231, 215, 234, 136, 112, 12, 136, 62, 169, 7, 120, 69, 212, 205, 167, 126, 39, 120, 12, 37, 203, 73, 181, 98, 218, 206, 84, 74, 184, 232, 24, 18, 232, 73, 61, 143, 111, 103, 207, 18, 1, 58, 126, 124, 82, 203, 44, 85, 198, 249, 16, 244, 207, 236, 148, 217, 102, 62, 52, 191, 113, 92, 62, 250, 23, 80, 75, 7, 8, 184, 5, 76, 63, 38, 7, 0, 0, 169, 41, 0, 0, 80, 75, 1, 2, 20, 3, 20, 0, 8, 0, 8, 0, 88, 90, 126, 81, 184, 5, 76, 63, 38, 7, 0, 0, 169, 41, 0, 0, 17, 0, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 180, 129, 0, 0, 0, 0, 99, 111, 50, 115, 101, 110, 115, 111, 114, 46, 112, 104, 121, 112, 104, 111, 120, 85, 84, 13, 0, 7, 8, 199, 196, 95, 9, 199, 196, 95, 30, 199, 196, 95, 117, 120, 11, 0, 1, 4, 232, 3, 0, 0, 4, 232, 3, 0, 0, 80, 75, 5, 6, 0, 0, 0, 0, 1, 0, 1, 0, 95, 0, 0, 0, 133, 7, 0, 0, 0, 0};

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

}

void loop() {

  if (digitalRead(readyPin))
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
    1       calibrate: 1=CO2 2=temperature
     */

    uint8_t readArray[6] = {0};
    PhyphoxBLE::read(&readArray[0],6);
    if(readArray[0]==1){
        // just send current data
        Serial.println("Resending of old data requested.");
        oldDataTransmissionOffset = 0;
        oldDataTransmissionSet = datasetNumber;
    }
    if(readArray[1]==1){
      //CO2 CALIBRATION
      Serial.print("Calibration with fresh air ");
      airSensor.setAutoSelfCalibration(false);
      Serial.println(airSensor.setForcedRecalibrationFactor(400));
      }
    if(readArray[1]==2){
      //TEMPERATURE CALIBRATION
      Serial.print("Temperature is set to: ");
      Serial.print(readArray[5]);
      Serial.println(" °C");
      delay(50);
      
      Serial.print("Current Offset: ");
      Serial.print(airSensor.getTemperatureOffset());
      Serial.println(" °C");
      delay(50);
      
      float offset = airSensor.getTemperature()-readArray[5]+airSensor.getTemperatureOffset();
      airSensor.setTemperatureOffset(offset);
      delay(50);
      Serial.print("New offset: ");
      Serial.print(offset);
      Serial.println(" °C");
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