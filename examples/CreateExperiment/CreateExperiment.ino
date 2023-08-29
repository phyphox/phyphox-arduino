#include <phyphoxBle.h>

void setup()
{
  Serial.begin(115200);
  PhyphoxBLE::start("create experiment");

  //Experiment
  PhyphoxBleExperiment plotRandomValues;   //generate experiment on Arduino which plot random values


  plotRandomValues.setTitle("Random Number Plotter");
  plotRandomValues.setCategory("Arduino Experiments");
  plotRandomValues.setDescription("Random numbers are generated on Arduino and visualized with phyphox afterwards");

  //View
  PhyphoxBleExperiment::View firstView;
  firstView.setLabel("FirstView"); //Create a "view"
  PhyphoxBleExperiment::View secondView;
  secondView.setLabel("SecondView"); //Create a "view"

  //Graph
  PhyphoxBleExperiment::Graph firstGraph;      //Create graph which will plot random numbers over time
  firstGraph.setMinY(0, LAYOUT_FIXED);
  firstGraph.setMaxY(100, LAYOUT_FIXED);
  firstGraph.setLabel("Random number over time");
  firstGraph.setUnitX("s");
  firstGraph.setUnitY("");
  firstGraph.setLabelX("time");
  firstGraph.setLabelY("random number");
  firstGraph.setXPrecision(1);                 //The amount of digits shown after the decimal point
  firstGraph.setYPrecision(1);

  /* Assign Channels, so which data is plotted on x or y axis
     first parameter represents x-axis, second y-axis
     Channel 0 means a timestamp is created after the BLE package arrives in phyphox
     Channel 1 to N corresponding to the N-parameter which is written in server.write()
  */

  firstGraph.setChannel(0, 1);
  
  //Second Graph
  
  PhyphoxBleExperiment::Graph secondGraph;      //Create graph which will plot random numbers over time
  secondGraph.setLabel("Random number squared over random number");
  secondGraph.setUnitX("");
  secondGraph.setUnitY("");
  secondGraph.setLabelX("random number");
  secondGraph.setLabelY("squared");
  secondGraph.setStyle(STYLE_DOTS);
  secondGraph.setColor("2E728E");                //Sets Color of line
  
  /* Assign Channels, so which data is plotted on x or y axis
     first parameter represents x-axis, second y-axis
     Channel 0 means a timestamp is created after the BLE package arrives in phyphox
     Channel 1 to N corresponding to the N-parameter which is written in server.write()
  */
  
  secondGraph.setChannel(1, 2);
  

  //Info
  PhyphoxBleExperiment::InfoField myInfo;      //Creates an info-box.
  myInfo.setInfo("This is an Info field!");
  myInfo.setColor("890128");                   //Sets font color. Uses a 6 digit hexadecimal value in "quotation marks".
  myInfo.setXMLAttribute("size=\"1.2\"");

  //Separator
  PhyphoxBleExperiment::Separator mySeparator;      //Creates a line to separate elements.
  mySeparator.setHeight(0.3);                       //Sets height of the separator.
  mySeparator.setColor("404040");                   //Sets color of the separator. Uses a 6 digit hexadecimal value in "quotation marks".

  //Value
  PhyphoxBleExperiment::Value myValue;         //Creates a value-box.
  myValue.setLabel("Number");                  //Sets the label
  myValue.setPrecision(2);                     //The amount of digits shown after the decimal point.
  myValue.setUnit("unit");                        //The physical unit associated with the displayed value.
  myValue.setColor("FFFFFF");                  //Sets font color. Uses a 6 digit hexadecimal value in "quotation marks".
  myValue.setChannel(0);
  myValue.setXMLAttribute("size=\"2\"");

  //Export
  PhyphoxBleExperiment::ExportSet mySet;       //Provides exporting the data to excel etc.
  mySet.setLabel("mySet");
  PhyphoxBleExperiment::ExportData myData1;
  myData1.setLabel("myData1");
  myData1.setDatachannel(1);

  PhyphoxBleExperiment::ExportData myData2;
  myData2.setLabel("myData2");
  myData2.setDatachannel(2);

  //attach to experiment

  firstView.addElement(firstGraph);            //attach graph to view
  firstView.addElement(secondGraph);            //attach second graph to view
  secondView.addElement(myInfo);                //attach info to view
  secondView.addElement(mySeparator);          //attach separator to view
  secondView.addElement(myValue);               //attach value to view

  plotRandomValues.addView(firstView);         //attach view to experiment
  plotRandomValues.addView(secondView);
  mySet.addElement(myData1);                   //attach data to exportSet
  mySet.addElement(myData2);                   //attach data to exportSet
  plotRandomValues.addExportSet(mySet);        //attach exportSet to experiment
  PhyphoxBLE::addExperiment(plotRandomValues);      //attach experiment to server
}


void loop()
{
  float randomValue = random(0, 100); //create random number between 0 - 100
  float randomValue2 = randomValue * randomValue;

  /*  The random number is written into Channel 1
      Up to 5 Channels can written at the same time with server.write(randomDistance, valueChannel2, valueChannel3.. )
  */
  PhyphoxBLE::write(randomValue, randomValue2);
  delay(50);

  PhyphoxBLE::poll(); //Only required for the Arduino Nano 33 IoT, but it does no harm for other boards.
}