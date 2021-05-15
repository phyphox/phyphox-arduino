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

   //Export
   PhyphoxBleExperiment::Export firstExport;
   firstExport.setName("FirstExport");

   PhyphoxBleExperiment::Exportdata firstData;
   firstData.setName("firstData");
   firstData.setBuffer("1");

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

   firstView.addElement(firstGraph);            //attach graph to view
   firstView.addElement(secondGraph);            //attach second graph to view
   plotRandomValues.addView(firstView);         //Attach view to experiment
   firstExport.addElement(firstData);
   plotRandomValues.addExport(firstExport);
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
