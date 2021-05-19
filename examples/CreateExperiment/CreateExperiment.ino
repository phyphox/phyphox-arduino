#include <phyphoxBle.h> 

void setup()
{
   PhyphoxBLE::start("My Device");

   //Experiment
   PhyphoxBleExperiment plotRandomValues;   //generate experiment on Arduino which plot random values

   plotRandomValues.setTitle("Random Number Plotter");
   plotRandomValues.setCategory("Arduino Experiments");
   plotRandomValues.setDescription("Random numbers are generated on Arduino and visualized with phyphox afterwards");

   //View
   PhyphoxBleExperiment::View firstView;
   firstView.setLabel("FirstView"); //Create a "view"

   //Graph
   PhyphoxBleExperiment::Graph firstGraph;      //Create graph which will plot random numbers over time     
   firstGraph.setLabel("Random number over time");
   firstGraph.setUnitX("s");
   firstGraph.setUnitY("");
   firstGraph.setLabelX("time");
   firstGraph.setLabelY("random number");

   /* Assign Channels, so which data is plotted on x or y axis 
   *  first parameter represents x-axis, second y-axis
   *  Channel 0 means a timestamp is created after the BLE package arrives in phyphox
   *  Channel 1 to N corresponding to the N-parameter which is written in server.write()
   */

   firstGraph.setChannel(0,1);

   //Second Graph
   PhyphoxBleExperiment::Graph secondGraph;      //Create graph which will plot random numbers over time     
   secondGraph.setLabel("Random number squared over random number");
   secondGraph.setUnitX("");
   secondGraph.setUnitY("");
   secondGraph.setLabelX("random number");
   secondGraph.setLabelY("squared");
   secondGraph.setStyle("dots");

   /* Assign Channels, so which data is plotted on x or y axis 
   *  first parameter represents x-axis, second y-axis
   *  Channel 0 means a timestamp is created after the BLE package arrives in phyphox
   *  Channel 1 to N corresponding to the N-parameter which is written in server.write()
   */

   secondGraph.setChannel(1,2);

   //Separator
   PhyphoxBleExperiment::Separator mySeparator;      //Creates a line to separate elements.
   mySeparator.setHeight(0.3);                       //Sets height of the separator.
   mySeparator.setColor("404040");                   //Sets color of the separator. Uses a 6 digit hexadecimal value in "quotation marks".

   //Info
   PhyphoxBleExperiment::InfoField myInfo;      //Creates an info-box.
   myInfo.setInfo("Random Info Text");
   myInfo.setColor("404040");                   //Sets font color. Uses a 6 digit hexadecimal value in "quotation marks".

   //Export
   PhyphoxBleExperiment::ExportSet mySet;       //Provides exporting the data to excel etc.
   mySet.setName("mySet");

   PhyphoxBleExperiment::ExportData myData1;
   myData1.setName("myData1");
   myData1.setDatachannel(1);

   PhyphoxBleExperiment::ExportData myData2;
   myData2.setName("myData2");
   myData2.setDatachannel(2);

   //attach to experiment

   firstView.addElement(firstGraph);            //attach graph to view
   firstView.addElement(secondGraph);            //attach second graph to view
   firstView.addElement(mySeparator);
   firstView.addElement(myInfo);                //attach info to view
   plotRandomValues.addView(firstView);         //Attach view to experiment
   mySet.addElement(myData1);                   //attach data to exportSet
   mySet.addElement(myData2);                   //attach data to exportSet
   plotRandomValues.addExportSet(mySet);        //attach exportSet to experiment
   PhyphoxBLE::addExperiment(plotRandomValues);      //Attach experiment to server

}


void loop()
{
    
  float randomValue = random(0,100); //create random number between 0 - 100
  float randomValue2 = randomValue*randomValue;
  
  /*  The random number is written into Channel 1
   *  Up to 5 Channels can written at the same time with server.write(randomDistance, valueChannel2, valueChannel3.. )
   */

  PhyphoxBLE::write(randomValue, randomValue2);
  delay(50);

  PhyphoxBLE::poll(); //Only required for the Arduino Nano 33 IoT, but it does no harm for other boards.
}
