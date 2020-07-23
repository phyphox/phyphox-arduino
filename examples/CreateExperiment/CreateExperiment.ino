#include <phyphoxBle.h> 
BleServer server("phyphox"); //Init server

void setup()
{
   server.start();

   //Experiment
   Experiment PlotRandomValues;   //generate experiment on Arduino which plot random values

   PlotRandomValues.setTitle("Random Number Plotter");
   PlotRandomValues.setCategory("Arduino Experiments");
   PlotRandomValues.setDescription("Random numbers are generated on Arduino and visualized with phyphox afterwards");

   //View
   View firstView;
   firstView.setLabel("FirstView"); //Create a "view"

   //Graph
   Graph firstGraph;      //Create graph which will plot random numbers over time     
   firstGraph.setLabel("Randum number over time");
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

   firstView.addElement(firstGraph);            //attach graph to view
   PlotRandomValues.addView(firstView);         //Attach view to experiment
   server.addExperiment(PlotRandomValues);      //Attach experiment to server

}


void loop()
{
    
  float randomValue = random(0,100); //create random number between 0 - 100
  
  /*  The random number is written into Channel 1
   *  Up to 5 Channels can written at the same time with server.write(randomDistance, valueChannel2, valueChannel3.. )
   */

  server.write(randomValue);    
  delay(50);
  
}
