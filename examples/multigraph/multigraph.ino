#include <phyphoxBle.h> 
#include <math.h>
#define PI 3.1415926535897932384626433832795
float periodTime = 2.0;//in s

void setup() {
  PhyphoxBLE::start();

  PhyphoxBleExperiment MultiGraph;

  MultiGraph.setTitle("Multi Graph Example");
  MultiGraph.setCategory("Arduino Experiments");
  MultiGraph.setDescription("ArduinoBLE Example");

  PhyphoxBleExperiment::View firstView;
  firstView.setLabel("FirstView"); //Create a "view"

  //Multiple graphs in one plot can be realised by two different methods. 
  // OPTION 1 is do create a graph as usual and add additional datastreams the following way
  PhyphoxBleExperiment::Graph myFirstGraph;
  myFirstGraph.setChannel(1,2);
  myFirstGraph.setStyle("dots");//"lines" are used if you dont set a style 
  myFirstGraph.setColor("ffffff");
  myFirstGraph.setLinewidth(2);//if you dont select a linewidth, a width of 1 is used by default
  
  PhyphoxBleExperiment::Datastream additionalData;
  additionalData.setChannel(1,3);
  additionalData.setColor("ff00ff");
  additionalData.setLinewidth(1);

  myFirstGraph.addDatastream(additionalData);

  //OPTION 2: you can also skip editing the graph object and just add datastreams
  PhyphoxBleExperiment::Graph mySecondGraph;

  PhyphoxBleExperiment::Datastream firstData;
  firstData.setChannel(1,2);
  firstData.setColor("ffffff");
  firstData.setStyle("dots");
  firstData.setLinewidth(2);
  
  mySecondGraph.addDatastream(firstData);

  PhyphoxBleExperiment::Datastream secondData;
  secondData.setChannel(1,3);
  secondData.setColor("ff00ff");
  secondData.setLinewidth(1);//if you dont select a linewidth, a width of 1 is used by default
  secondData.setStyle("lines"); //"lines" are used if you dont set a style 

  mySecondGraph.addDatastream(secondData);

  firstView.addElement(myFirstGraph);
  firstView.addElement(mySecondGraph);

  MultiGraph.addView(firstView);

  PhyphoxBLE::addExperiment(MultiGraph);
}

void loop() {

  float currentTime = millis()/1000.0;
  float sinus = generateSin(currentTime);
  float cosinus = generateSin(currentTime+0.5*periodTime);
  PhyphoxBLE::write(currentTime,sinus,cosinus);
  delay(100);
}

float generateSin(float x){
  return 1.0 * sin(x*2.0*PI/periodTime);
}