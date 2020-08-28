#include <phyphoxBle.h>   //Phyphox BLE library

#include <Wire.h>         //Required for the VL53L0X rangefinder
#include <VL53L0X.h>      //Required for the VL53L0X rangefinder
VL53L0X sensor;           //Instance of the rangefinder sensor

void setup() {
    PhyphoxBLE::start("Rangefinder");

    //Experiment
    PhyphoxBleExperiment experiment;

    experiment.setTitle("Rangefinder");
    experiment.setCategory("Arduino Experiments");
    experiment.setDescription("Plot the distance from a time-of-flight sensor over time.");

    //View
    PhyphoxBleExperiment::View view;

    //Graph
    PhyphoxBleExperiment::Graph graph;
    graph.setLabel("Distance over time");
    graph.setUnitX("s");
    graph.setUnitY("mm");
    graph.setLabelX("time");
    graph.setLabelY("distance");

    //In contrast to other examples, we will not generate the timestamp on the phone.
    //For experiments with a high data rate, we can achieve a better temporal
    //accuracy if we generate the timestamp on the Arduino and send it in pairs with the
    //measured values.
    graph.setChannel(1,2);

    view.addElement(graph);                 //Attach graph to view
    experiment.addView(view);               //Attach view to experiment
    PhyphoxBLE::addExperiment(experiment);  //Attach experiment to server

    //Start the rangefinder
    Wire.begin();
    while (!sensor.init())
      delay(100);
    sensor.setTimeout(500);
    sensor.startContinuous();
}


void loop() {
    float t = 0.001 * (float)millis();                        //Time in seconds
    float distance = sensor.readRangeContinuousMillimeters(); //Distance in millimeters

    if (distance == 8190)           //This is an error state if no distance could be determined. Zero looks better in this case.
        distance = 0;

    PhyphoxBLE::write(t, distance); //Send data to phyphox
  
    PhyphoxBLE::poll(); //Only required for the Arduino Nano 33 IoT, but it does no harm for other boards.
}
