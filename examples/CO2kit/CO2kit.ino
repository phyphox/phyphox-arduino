/*
   Dies ist eine aktualisierte Version des Arduino Codes für das phyphox CO2 Monitor Kit

   Um Ihren CO2-Monitor zu aktualisieren, löschen Sie in der phyphox App die Experimente "CO2-Sensor" und "CO₂-Sensor Maintenance"
   und folgen Sie dem Software-Teil der Aufbauanleitung.
   https://phyphox.org/wp-content/uploads/2021/03/CO2-Monitor-Rev_1_1.pdf


   This is an updated version of the Arduino Code for the phyphox CO2 Monitor Kit

   To update your CO2 Monitor delete in the phyphox app the experiements "CO2-Sensor" and "CO₂-Sensor Maintenance"
   and follow the software part of the assembly instructions
   https://phyphox.org/wp-content/uploads/2021/03/CO2-Monitor-Rev_1_1.pdf
*/


#include <phyphoxBle.h>
#include "SparkFun_SCD30_Arduino_Library.h"
#include <FS.h>
#include <SPIFFS.h>

//LED Update Thresholds
//Can be set as desired
float topThreshold = 1000;
float bottomThreshold = 600;


bool FORMATFLASH = true;

SCD30 airSensor;
File fsUploadFile; // a File variable to temporarily store the received file

int maxDatasets = 7200; //store data every 12s over 24h -> maxDatasets = 24*60*60/12 = 7200
int transferedDatasets = 11; //mtu size = 176 -> 4*4*11 = 176

const int measuredDataLength = 4;
float measuredData[measuredDataLength];  //co2,temperature,humidity,seconds since uptime, number of dataset

float averageMeasuredData[measuredDataLength];
int averageOver = 6; //6*2s rate = store data every 12s
int averageCounter = 0;
int oldDataTransmissionOffset = -1;
int lineNumber = 0;

void receivedConfig();
long timeStampBlue = -3; //used to timestamp the moment of connection
int currentConnections = 0;
//Pinout
int pinGreen = 26;
int pinRed = 33;
int pinBlue = 25;
bool RED = 1;
bool GREEN = 0;
bool BLUE = 1;
int readyPin = 5;


uint8_t versionID = 3;
uint8_t currentOffset;
uint8_t statusCO2calibration = 0;
/*
   The following byte array contains the complete phyphox experiment.
   The phyphox experiment (xml-file) where the byte array is generated from can be found in the github repository https://github.com/Dorsel89/phyphox-hardware (CO2-Monitor Kit folder).
*/
uint8_t CO2Monitor[2877] = {80, 75, 3, 4, 20, 0, 0, 0, 8, 0, 129, 91, 97, 84, 18, 124, 19, 223, 161, 10, 0, 0, 113, 58, 0, 0, 11, 0, 0, 0, 99, 111, 50, 46, 112, 104, 121, 112, 104, 111, 120, 213, 91, 221, 146, 219, 182, 21, 190, 222, 206, 244, 29, 80, 206, 120, 98, 79, 35, 81, 210, 202, 187, 30, 143, 86, 157, 141, 29, 39, 158, 38, 217, 142, 215, 205, 79, 61, 30, 15, 68, 130, 75, 100, 33, 128, 5, 193, 213, 202, 147, 43, 191, 72, 111, 114, 223, 62, 64, 175, 226, 55, 201, 147, 244, 0, 32, 41, 146, 162, 36, 80, 145, 215, 169, 60, 182, 68, 242, 156, 131, 239, 252, 226, 0, 132, 39, 73, 188, 76, 98, 113, 139, 110, 136, 76, 169, 224, 103, 222, 176, 63, 28, 120, 211, 63, 254, 1, 193, 103, 162, 168, 98, 100, 250, 228, 226, 215, 119, 239, 122, 95, 11, 78, 149, 144, 19, 223, 222, 204, 41, 2, 172, 200, 149, 144, 203, 156, 232, 146, 240, 84, 211, 148, 183, 11, 50, 193, 132, 156, 142, 142, 207, 207, 199, 167, 19, 223, 94, 229, 143, 66, 146, 6, 146, 38, 10, 70, 159, 190, 140, 105, 138, 200, 109, 66, 36, 157, 19, 174, 208, 13, 77, 51, 204, 232, 91, 146, 34, 21, 19, 148, 26, 233, 40, 196, 10, 35, 17, 153, 91, 5, 126, 51, 124, 78, 208, 71, 207, 128, 106, 46, 36, 65, 148, 71, 66, 206, 177, 22, 254, 169, 150, 70, 149, 225, 202, 169, 169, 34, 90, 78, 46, 163, 47, 228, 85, 31, 189, 140, 137, 36, 134, 8, 39, 9, 163, 129, 225, 69, 32, 5, 100, 133, 2, 190, 48, 213, 178, 141, 45, 40, 191, 66, 22, 48, 195, 148, 147, 176, 143, 126, 16, 25, 10, 48, 71, 152, 165, 2, 69, 192, 97, 97, 84, 84, 162, 33, 193, 41, 90, 80, 21, 3, 158, 144, 8, 163, 153, 36, 253, 137, 95, 53, 68, 110, 27, 26, 216, 177, 65, 131, 51, 47, 85, 122, 64, 207, 90, 122, 226, 211, 160, 164, 179, 190, 146, 152, 167, 204, 192, 77, 55, 220, 71, 76, 4, 152, 145, 51, 143, 112, 175, 66, 210, 116, 53, 154, 183, 185, 186, 248, 56, 186, 188, 248, 124, 8, 255, 182, 217, 202, 140, 229, 87, 148, 221, 101, 131, 144, 108, 179, 65, 239, 235, 15, 101, 131, 167, 148, 164, 160, 239, 231, 43, 43, 164, 138, 48, 166, 80, 72, 9, 178, 130, 192, 0, 132, 35, 96, 106, 168, 127, 89, 216, 7, 66, 252, 59, 2, 209, 11, 161, 245, 124, 21, 225, 132, 155, 144, 131, 175, 75, 16, 133, 179, 168, 100, 211, 129, 126, 35, 120, 25, 233, 232, 169, 144, 10, 130, 80, 134, 102, 212, 115, 190, 32, 60, 204, 32, 154, 223, 102, 18, 189, 192, 217, 156, 101, 145, 122, 255, 223, 25, 145, 11, 28, 196, 250, 1, 145, 215, 236, 253, 207, 82, 245, 209, 121, 246, 254, 95, 68, 134, 100, 94, 29, 44, 52, 242, 114, 72, 21, 213, 158, 135, 4, 40, 230, 144, 118, 223, 154, 96, 223, 232, 58, 99, 38, 27, 225, 8, 18, 235, 138, 114, 204, 206, 188, 23, 224, 163, 207, 8, 35, 89, 16, 43, 64, 145, 166, 10, 48, 92, 147, 137, 111, 41, 29, 248, 225, 14, 185, 129, 44, 103, 161, 9, 43, 111, 250, 5, 73, 19, 66, 3, 200, 57, 176, 201, 83, 99, 104, 172, 77, 24, 18, 201, 157, 229, 190, 36, 115, 80, 17, 171, 76, 2, 192, 213, 133, 51, 255, 151, 217, 156, 134, 84, 45, 189, 233, 87, 96, 232, 200, 232, 71, 175, 174, 193, 126, 206, 34, 212, 111, 132, 16, 31, 0, 2, 248, 216, 155, 254, 163, 11, 203, 83, 227, 3, 99, 118, 103, 158, 111, 41, 89, 120, 211, 115, 158, 130, 215, 220, 71, 122, 146, 73, 105, 147, 11, 204, 146, 62, 6, 1, 215, 42, 131, 60, 35, 18, 93, 218, 91, 200, 89, 22, 23, 101, 0, 33, 73, 2, 66, 111, 72, 232, 77, 191, 17, 65, 140, 192, 96, 188, 136, 35, 112, 66, 132, 249, 85, 7, 213, 214, 164, 62, 134, 18, 199, 72, 160, 16, 134, 236, 98, 144, 77, 104, 70, 152, 88, 180, 134, 109, 57, 220, 99, 180, 120, 255, 115, 204, 8, 202, 184, 185, 79, 57, 84, 5, 224, 189, 193, 140, 65, 25, 72, 157, 225, 92, 198, 98, 97, 241, 68, 82, 204, 97, 232, 72, 228, 115, 33, 88, 81, 170, 162, 38, 87, 10, 184, 158, 23, 193, 58, 115, 75, 135, 185, 13, 135, 8, 234, 148, 198, 14, 57, 165, 27, 139, 43, 131, 146, 112, 139, 251, 177, 51, 156, 33, 138, 69, 38, 189, 233, 16, 60, 150, 65, 169, 113, 102, 28, 25, 198, 212, 155, 142, 114, 78, 119, 143, 140, 11, 214, 113, 103, 214, 81, 201, 59, 234, 206, 92, 134, 171, 113, 63, 212, 198, 74, 196, 66, 201, 77, 23, 56, 102, 238, 150, 19, 156, 45, 17, 131, 128, 202, 139, 222, 55, 153, 180, 151, 221, 50, 239, 5, 209, 115, 38, 176, 173, 138, 69, 121, 107, 239, 170, 33, 9, 235, 107, 129, 125, 35, 172, 111, 5, 245, 221, 13, 133, 229, 12, 102, 178, 144, 138, 91, 170, 167, 240, 191, 138, 152, 233, 112, 19, 81, 100, 239, 57, 75, 130, 153, 80, 137, 116, 201, 33, 164, 211, 90, 91, 226, 77, 255, 86, 121, 84, 157, 207, 90, 100, 183, 245, 28, 19, 191, 165, 25, 155, 48, 202, 175, 17, 195, 51, 98, 198, 46, 91, 78, 232, 134, 71, 30, 138, 233, 85, 204, 224, 47, 244, 121, 17, 102, 41, 40, 22, 43, 149, 164, 143, 125, 127, 177, 88, 244, 27, 228, 254, 196, 215, 194, 218, 5, 111, 82, 106, 199, 16, 75, 145, 169, 108, 70, 250, 129, 152, 251, 11, 172, 130, 248, 47, 55, 103, 151, 217, 143, 163, 243, 31, 111, 191, 188, 24, 95, 215, 70, 204, 199, 213, 177, 213, 11, 4, 87, 186, 249, 149, 181, 166, 179, 188, 11, 77, 246, 91, 104, 182, 96, 73, 17, 136, 55, 148, 79, 252, 242, 201, 14, 114, 61, 191, 117, 98, 128, 144, 234, 54, 0, 152, 101, 141, 97, 11, 203, 208, 168, 192, 197, 194, 113, 136, 97, 174, 67, 39, 14, 173, 196, 26, 195, 78, 187, 234, 113, 58, 90, 182, 27, 139, 134, 213, 141, 195, 88, 119, 157, 101, 151, 42, 85, 234, 237, 180, 157, 21, 232, 134, 189, 78, 189, 155, 62, 164, 81, 212, 109, 132, 64, 64, 197, 15, 84, 71, 87, 135, 52, 133, 197, 230, 178, 155, 157, 186, 115, 105, 135, 119, 102, 50, 62, 111, 229, 218, 26, 244, 176, 176, 166, 170, 20, 240, 150, 72, 225, 158, 47, 73, 162, 121, 186, 210, 175, 143, 225, 8, 144, 95, 176, 176, 206, 120, 116, 180, 149, 161, 232, 235, 116, 223, 187, 215, 136, 50, 95, 193, 124, 150, 41, 37, 92, 171, 219, 67, 111, 58, 91, 42, 114, 46, 101, 155, 251, 38, 126, 123, 225, 158, 80, 158, 100, 170, 42, 119, 198, 50, 162, 132, 80, 49, 162, 225, 153, 247, 228, 2, 230, 41, 142, 231, 36, 255, 9, 93, 95, 72, 116, 115, 172, 104, 148, 239, 147, 120, 8, 22, 33, 86, 131, 52, 155, 233, 117, 222, 140, 92, 240, 75, 221, 60, 22, 147, 14, 154, 171, 12, 8, 78, 79, 214, 86, 223, 48, 1, 193, 248, 40, 136, 177, 60, 243, 130, 48, 140, 134, 131, 193, 168, 119, 60, 136, 78, 123, 227, 147, 211, 97, 239, 209, 108, 124, 220, 123, 72, 198, 131, 25, 126, 120, 252, 112, 56, 198, 30, 2, 29, 202, 141, 171, 136, 9, 172, 142, 71, 95, 81, 5, 235, 245, 207, 121, 72, 161, 27, 133, 142, 53, 74, 9, 140, 125, 12, 200, 72, 66, 176, 210, 45, 0, 12, 127, 82, 206, 71, 118, 212, 187, 196, 114, 186, 142, 197, 36, 233, 199, 65, 51, 28, 174, 195, 209, 217, 95, 67, 115, 116, 116, 244, 97, 65, 60, 92, 7, 81, 206, 207, 85, 20, 213, 190, 171, 140, 206, 50, 174, 171, 1, 60, 41, 217, 182, 132, 115, 211, 208, 148, 175, 169, 120, 236, 174, 98, 153, 115, 181, 244, 171, 130, 218, 4, 188, 134, 117, 130, 161, 61, 93, 234, 238, 45, 101, 132, 36, 80, 6, 250, 144, 109, 130, 255, 61, 37, 242, 185, 22, 86, 164, 82, 189, 138, 252, 169, 215, 123, 227, 242, 233, 245, 166, 117, 174, 159, 86, 6, 184, 36, 10, 105, 187, 35, 93, 33, 171, 150, 249, 105, 141, 235, 151, 127, 31, 234, 143, 17, 93, 23, 142, 104, 132, 138, 66, 173, 119, 88, 7, 159, 162, 5, 172, 66, 48, 172, 59, 160, 220, 160, 37, 192, 12, 9, 172, 48, 231, 122, 215, 213, 130, 21, 220, 172, 77, 111, 17, 190, 165, 41, 172, 42, 136, 250, 36, 69, 89, 106, 151, 175, 90, 22, 44, 97, 231, 137, 94, 194, 98, 190, 68, 156, 228, 11, 221, 68, 80, 88, 117, 97, 189, 81, 171, 155, 94, 164, 55, 136, 133, 225, 177, 62, 41, 214, 188, 90, 130, 222, 247, 13, 51, 70, 250, 168, 9, 217, 60, 109, 207, 218, 114, 138, 90, 143, 71, 191, 96, 171, 220, 3, 197, 25, 73, 211, 51, 79, 201, 12, 138, 37, 249, 103, 134, 89, 126, 177, 33, 88, 25, 193, 178, 140, 136, 194, 104, 235, 97, 87, 97, 81, 203, 4, 106, 244, 13, 102, 90, 232, 96, 43, 105, 93, 122, 161, 74, 43, 131, 213, 175, 149, 195, 34, 106, 49, 0, 141, 254, 143, 180, 47, 11, 146, 187, 250, 157, 148, 63, 64, 2, 151, 251, 157, 51, 211, 46, 124, 164, 4, 198, 73, 66, 120, 232, 96, 253, 225, 161, 28, 245, 17, 73, 115, 191, 99, 8, 90, 248, 89, 171, 252, 149, 137, 107, 226, 151, 70, 169, 135, 123, 215, 16, 111, 246, 131, 135, 212, 20, 154, 16, 181, 244, 144, 191, 69, 69, 73, 210, 140, 41, 175, 0, 101, 97, 183, 169, 188, 57, 204, 75, 67, 28, 202, 226, 77, 147, 236, 52, 251, 1, 18, 237, 220, 136, 93, 77, 35, 191, 191, 68, 171, 135, 77, 32, 58, 86, 174, 98, 171, 99, 167, 49, 221, 49, 148, 77, 174, 59, 138, 213, 6, 202, 1, 113, 20, 221, 173, 59, 12, 205, 113, 48, 20, 54, 103, 246, 112, 200, 33, 135, 223, 211, 23, 135, 132, 176, 151, 27, 14, 9, 96, 207, 249, 252, 195, 23, 152, 75, 33, 109, 75, 26, 18, 6, 93, 46, 10, 51, 123, 16, 129, 164, 232, 142, 11, 76, 42, 164, 115, 107, 212, 161, 147, 10, 68, 7, 98, 27, 119, 206, 228, 38, 72, 28, 60, 90, 137, 129, 70, 100, 239, 100, 10, 68, 103, 22, 77, 223, 153, 105, 181, 235, 218, 50, 173, 230, 174, 217, 103, 98, 29, 109, 53, 80, 101, 59, 179, 101, 216, 182, 112, 215, 196, 68, 191, 189, 162, 16, 163, 174, 225, 242, 198, 106, 214, 45, 247, 54, 194, 106, 66, 168, 60, 147, 250, 237, 163, 125, 153, 234, 138, 77, 75, 115, 110, 173, 76, 35, 2, 11, 209, 93, 237, 236, 218, 252, 218, 49, 176, 43, 33, 212, 37, 25, 186, 178, 184, 249, 102, 155, 55, 170, 100, 129, 216, 65, 176, 59, 39, 26, 101, 127, 19, 152, 214, 176, 168, 251, 254, 112, 21, 26, 62, 95, 16, 174, 79, 92, 216, 157, 1, 179, 233, 96, 110, 223, 117, 133, 214, 135, 127, 50, 134, 81, 254, 125, 230, 221, 127, 53, 124, 243, 186, 247, 106, 244, 250, 207, 175, 142, 95, 63, 112, 94, 54, 111, 240, 246, 222, 171, 236, 205, 219, 0, 187, 162, 170, 124, 47, 210, 226, 207, 92, 203, 195, 249, 242, 153, 137, 13, 115, 140, 193, 190, 121, 39, 161, 117, 168, 137, 156, 59, 246, 101, 61, 90, 127, 167, 179, 104, 27, 117, 205, 109, 174, 92, 171, 186, 105, 223, 170, 108, 139, 139, 234, 139, 175, 157, 229, 196, 145, 180, 246, 138, 107, 103, 121, 217, 66, 218, 94, 102, 38, 126, 177, 141, 155, 147, 78, 110, 40, 89, 20, 23, 229, 141, 226, 189, 189, 61, 24, 213, 24, 255, 74, 226, 36, 46, 40, 26, 7, 31, 16, 163, 156, 124, 71, 67, 21, 159, 121, 35, 189, 15, 205, 4, 152, 86, 159, 176, 29, 159, 122, 150, 231, 251, 252, 136, 150, 189, 250, 65, 111, 121, 255, 250, 238, 157, 135, 50, 78, 21, 60, 75, 73, 96, 127, 195, 147, 36, 153, 123, 38, 240, 47, 248, 247, 197, 238, 91, 186, 76, 193, 156, 47, 233, 156, 180, 239, 79, 160, 149, 99, 117, 249, 59, 243, 110, 155, 175, 0, 205, 195, 93, 92, 203, 250, 123, 205, 214, 48, 240, 141, 37, 154, 119, 205, 252, 187, 201, 60, 229, 171, 180, 4, 194, 146, 218, 13, 122, 176, 83, 26, 80, 221, 37, 68, 52, 40, 227, 48, 51, 239, 218, 140, 5, 54, 98, 45, 215, 195, 237, 240, 12, 146, 230, 221, 148, 36, 24, 102, 8, 40, 45, 49, 177, 199, 46, 134, 45, 155, 43, 53, 31, 87, 207, 21, 22, 46, 141, 162, 32, 28, 226, 77, 46, 173, 30, 3, 108, 115, 236, 47, 255, 121, 242, 17, 29, 171, 209, 253, 118, 215, 214, 172, 210, 213, 175, 218, 0, 27, 145, 86, 54, 24, 62, 172, 99, 215, 79, 82, 149, 25, 59, 56, 61, 126, 118, 190, 201, 189, 171, 195, 82, 109, 206, 189, 247, 17, 93, 91, 43, 159, 251, 122, 182, 197, 44, 93, 253, 123, 111, 179, 138, 171, 125, 27, 119, 231, 54, 104, 242, 125, 244, 18, 238, 218, 113, 226, 58, 125, 199, 141, 110, 67, 158, 207, 39, 27, 119, 49, 93, 228, 247, 30, 157, 140, 7, 131, 221, 131, 216, 153, 182, 178, 127, 113, 116, 180, 55, 220, 234, 193, 134, 246, 233, 211, 183, 214, 107, 145, 82, 157, 60, 245, 44, 88, 33, 105, 251, 85, 155, 41, 237, 113, 96, 215, 148, 212, 26, 214, 66, 110, 237, 92, 112, 213, 12, 13, 181, 236, 189, 156, 96, 142, 19, 4, 237, 138, 62, 149, 129, 230, 248, 214, 156, 206, 104, 59, 24, 60, 241, 129, 114, 141, 105, 152, 51, 13, 189, 169, 235, 161, 223, 138, 160, 85, 172, 234, 11, 183, 250, 179, 162, 202, 43, 141, 48, 109, 138, 87, 50, 13, 250, 219, 217, 26, 194, 27, 116, 250, 127, 214, 20, 54, 61, 192, 185, 97, 132, 25, 189, 2, 59, 73, 61, 166, 231, 79, 183, 230, 97, 243, 120, 171, 91, 146, 116, 207, 143, 214, 104, 174, 157, 66, 64, 235, 224, 138, 83, 203, 142, 153, 123, 124, 50, 56, 16, 176, 173, 168, 202, 35, 209, 142, 176, 78, 71, 119, 2, 107, 220, 17, 214, 112, 60, 190, 27, 115, 117, 5, 182, 95, 1, 118, 1, 182, 37, 33, 55, 23, 183, 202, 41, 242, 90, 125, 179, 227, 239, 172, 107, 245, 4, 107, 175, 104, 38, 114, 115, 22, 251, 123, 106, 67, 127, 3, 249, 233, 104, 69, 110, 126, 79, 243, 152, 220, 64, 111, 92, 93, 48, 216, 139, 233, 184, 202, 113, 100, 50, 17, 254, 49, 102, 90, 49, 62, 58, 169, 48, 218, 139, 233, 104, 92, 229, 172, 213, 212, 82, 204, 134, 73, 201, 94, 148, 171, 56, 40, 98, 118, 231, 181, 226, 32, 149, 31, 139, 107, 171, 70, 230, 148, 116, 254, 92, 247, 101, 232, 62, 160, 124, 208, 236, 186, 52, 209, 22, 198, 39, 23, 35, 116, 31, 86, 40, 15, 234, 107, 165, 29, 92, 149, 190, 25, 221, 135, 62, 248, 65, 179, 33, 223, 193, 191, 214, 157, 161, 251, 247, 30, 52, 58, 191, 134, 136, 137, 15, 214, 40, 13, 87, 218, 106, 226, 231, 39, 216, 167, 255, 3, 80, 75, 1, 2, 31, 0, 20, 0, 0, 0, 8, 0, 129, 91, 97, 84, 18, 124, 19, 223, 161, 10, 0, 0, 113, 58, 0, 0, 11, 0, 36, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 99, 111, 50, 46, 112, 104, 121, 112, 104, 111, 120, 10, 0, 32, 0, 0, 0, 0, 0, 1, 0, 24, 0, 218, 42, 214, 7, 87, 45, 216, 1, 151, 133, 123, 7, 88, 45, 216, 1, 3, 16, 118, 7, 88, 45, 216, 1, 80, 75, 5, 6, 0, 0, 0, 0, 1, 0, 1, 0, 93, 0, 0, 0, 202, 10, 0, 0, 0, 0};

void setup() {

  //Start the SPIFFS and list all contents
  Serial.println("init spiffs");
  initStorage();
  delay(50);

  //Start the BLE server
  PhyphoxBLE::start("CO2 Monitor", &CO2Monitor[0], sizeof(CO2Monitor));
  PhyphoxBLE::configHandler = &receivedConfig;
  PhyphoxBLE::setMTU(176);

  Serial.begin(115200);

  pinMode(readyPin, INPUT);
  //initialize rgb-led
  pinMode(pinGreen, OUTPUT);
  pinMode(pinRed, OUTPUT);
  pinMode(pinBlue, OUTPUT);
  digitalWrite(pinGreen, GREEN);
  digitalWrite(pinRed, RED);
  digitalWrite(pinBlue, BLUE);

  //initialize the air Sensor
  Wire.begin();
  if (airSensor.begin(Wire, false) == false)
  {
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    while (1);
  }

  delay(200);

  Serial.print("Temperature offset: ");
  Serial.println(airSensor.getTemperatureOffset());
  currentOffset = airSensor.getTemperatureOffset() * 10;
}

void loop() {
  if (digitalRead(readyPin))
  {
    //read data
    measuredData[0] = airSensor.getCO2();
    measuredData[1] = airSensor.getTemperature();
    measuredData[2] = airSensor.getHumidity();
    measuredData[3] = millis() / 1000;

    //print data
    echoDataset("Measured", measuredData);

    uint8_t Data[20] = {0};
    Data[0] = currentOffset;
    Data[1] = statusCO2calibration;
    Data[2] = versionID;
    memcpy(&Data[3], &measuredData[0], 16);

    //Send data to phyphox
    PhyphoxBLE::write(&Data[0], 20);

    //calculate average Data over [averageOver] periods
    if (averageCounter == averageOver) {
      storeMeasuredData(averageMeasuredData);
      averageMeasuredData[0] = 0;
      averageMeasuredData[1] = 0;
      averageMeasuredData[2] = 0;
      averageMeasuredData[3] = 0;
      averageCounter = 0;
    }
    if (averageCounter < averageOver) {
      averageMeasuredData[0] += measuredData[0] / averageOver;
      averageMeasuredData[1] += measuredData[1] / averageOver;
      averageMeasuredData[2] += measuredData[2] / averageOver;
      averageMeasuredData[3] += measuredData[3] / averageOver;
      averageCounter += 1;
    }

    updateLED(measuredData[0]);
    delay(10);
  }

  //Data transfer loop
  if (oldDataTransmissionOffset >= 0) {
    if (transferOldData(oldDataTransmissionOffset))
      oldDataTransmissionOffset++;
    else {
      oldDataTransmissionOffset = -1;
      Serial.println("Transfer of old date completed.");
    }
    delay(10);
  }
}

//Print one dataset
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

// Start the SPIFFS and list all contents
void initStorage() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  if (FORMATFLASH) {
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

  File file = SPIFFS.open("/set.txt", FILE_WRITE);
  file.close();
  delay(1);
}

//Interpretation of received data
void receivedConfig() {
  /*
    phyphox experiment may send an Array with 5 Bytes
    byte    information
    0       bool transfer Data
    1       calibrate: 1=CO2 2=temperature
    2       bool setColorBlue
  */
  //read data
  uint8_t readArray[6] = {0};
  PhyphoxBLE::read(&readArray[0], 6);
  
  //DATA TRANSFER
  if (readArray[0] == 1) {
    Serial.println("Resending of old data requested.");
    oldDataTransmissionOffset = 0;
  }
  
  //CO2 CALIBRATION
  if (readArray[1] == 1) {
    Serial.print("Calibration with fresh air ");
    airSensor.setAutoSelfCalibration(false);
    Serial.println(airSensor.setForcedRecalibrationFactor(400));
    statusCO2calibration = 1;
  }

  //TEMPERATURE CALIBRATION
  if (readArray[1] == 2) {
    Serial.print("Current Offset: ");
    Serial.print(airSensor.getTemperatureOffset());
    Serial.println(" °C");
    delay(50);
    currentOffset = readArray[5] / 10.0;
    if (currentOffset >= 0) {
      airSensor.setTemperatureOffset(currentOffset);
    }
    Serial.print("New Offset: ");
    Serial.print(currentOffset);
    Serial.println(" °C");
    delay(50);
    Serial.print("Restarting ESP32..");
    delay(1000);
    ESP.restart();
  }

}

//Transfer [transferedDatasets] datasets at position [offset]
bool transferOldData(int offset) {
  int i_buffer = transferedDatasets;
  char buffer[4 * measuredDataLength * transferedDatasets];

  File file = SPIFFS.open("/set.txt", "r");

  //Check if enough datasets are available
  for (int i = 0; i < transferedDatasets; i++) {
    file.seek(offset * 4 * measuredDataLength * transferedDatasets + i * 4 * measuredDataLength, SeekSet);
    if (!file.available() && i == 0) {
      //No datasets available
      file.close();
      return false;
    }
    else if (!file.available()) {
      //Update number of datsets for transfer
      i_buffer = i;
      break;
    }
  }

  //Read and send the available amount of datasets
  file.seek(offset * 4 * measuredDataLength * transferedDatasets, SeekSet);
  int l = file.readBytes(buffer, 4 * measuredDataLength * i_buffer);
  file.close();
  uint8_t bufferArray[4 * measuredDataLength * i_buffer + 3];
  memcpy(&bufferArray[3], &buffer[0], 4 * measuredDataLength * i_buffer);
  PhyphoxBLE::write(bufferArray, 4 * measuredDataLength * i_buffer); //Send value to phyphox

  //All datasets are read -> return false
  if (i_buffer != transferedDatasets) {
    return false;
  } else {
    return true;
  }
}

//store one dataset in the ESP32 file system
void storeMeasuredData(float dataArray[4]) {
  byte byteArray[4 * measuredDataLength];
  memcpy(&byteArray[0], &dataArray[0], 4 * measuredDataLength);
  File file = SPIFFS.open("/set.txt", "r+");
  file.seek(lineNumber * 4 * measuredDataLength, SeekSet);
  file.write(byteArray, 4 * measuredDataLength);
  file.close();

  //circular data storage
  if (lineNumber < (maxDatasets - 1)) {
    lineNumber += 1;
  } else {
    lineNumber = 0;
  }
}


//Update LED according to thresholds and current CO2 Value or changed connection count
void updateLED(float co2value) {
  //update LED to blue if a device (dis)connects
  if (PhyphoxBLE::currentConnections != currentConnections) {
    BLUE = false;
    digitalWrite(pinBlue, BLUE);
    timeStampBlue = millis()/1000;
    currentConnections = PhyphoxBLE::currentConnections;
    Serial.print("Connection Count: ");
    Serial.print(currentConnections);
    Serial.print(" @ ");
    Serial.println(timeStampBlue);    
  }
  if(millis()/1000 > timeStampBlue+0.3){
    BLUE = true;
    digitalWrite(pinBlue, BLUE);
  }

  if (co2value > topThreshold && RED == true) {
    Serial.println("Update to red");
    GREEN = true;
    RED = false;
    BLUE = true;
    digitalWrite(pinGreen, GREEN);
    digitalWrite(pinRed, RED);
    digitalWrite(pinBlue, BLUE);
  }
  if (co2value < bottomThreshold && GREEN == true) {
    Serial.println("Update to green");
    GREEN = false;
    RED = true;
    BLUE = true;
    digitalWrite(pinGreen, GREEN);
    digitalWrite(pinRed, RED);
    digitalWrite(pinBlue, BLUE);

  }

}
