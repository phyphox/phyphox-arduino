#include <phyphoxBle.h> 

void setup() {
  Serial.begin(115200);
  
  PhyphoxBLE::start();
  PhyphoxBLE::configHandler=&receivedData;

  PhyphoxBleExperiment getDataFromSmartphonesensor;
  getDataFromSmartphonesensor.setTitle("Get Accelerometer Data");
  getDataFromSmartphonesensor.setCategory("Arduino Experiments");
  getDataFromSmartphonesensor.setDescription("Send smartphone accelerometer data to an arduino/esp32");

  PhyphoxBleExperiment::View firstView;
  firstView.setLabel("FirstView"); //Create a "view"

  PhyphoxBleExperiment::InfoField infoText;
  infoText.setInfo("Acc data is sent to smartphone");
  firstView.addElement(infoText);

  PhyphoxBleExperiment::Sensor smartphoneAcc; // add new sensor

  // Set type of sensor:
  // SENSOR_ACCELEROMETER
  // SENSOR_ACCELEROMETER_WITHOUT_G
  // SENSOR_GYROSCOPE
  // SENSOR_MAGNETOMETER
  // SENSOR_PRESSURE
  smartphoneAcc.setType(SENSOR_ACCELEROMETER);
  smartphoneAcc.setAverage(true);
  smartphoneAcc.setRate(80);

  // map sensor channel to incoming data channels
  smartphoneAcc.mapChannel("x",1);
  smartphoneAcc.mapChannel("y",2);
  smartphoneAcc.mapChannel("z",3);
  
  getDataFromSmartphonesensor.addView(firstView);
  getDataFromSmartphonesensor.addSensor(smartphoneAcc);
  
  PhyphoxBLE::addExperiment(getDataFromSmartphonesensor);
  PhyphoxBLE::printXML(&Serial); //print the generated xml file into the serial monitor
}

void loop() {
  delay(100);
}

void receivedData(){
  Serial.println("data:");
  float x,y,z;
  PhyphoxBLE::read(x,y,z);
  Serial.print("x: ");
  Serial.print(x);

  Serial.print(" y: ");
  Serial.print(y);

  Serial.print(" z: ");
  Serial.println(z);  
}