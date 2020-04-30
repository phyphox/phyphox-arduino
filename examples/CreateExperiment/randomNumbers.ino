#include <phyphoxBle.h> 
BleServer server("phyphox"); //Server anlegen

void setup()
{
   server.start();

   //Experiment
   Experiment PlotRandomValues;		//Experiment to plot random values generated on Arduino

   Distance_ToF.setTitle("Random Number Plotter");
   Distance_ToF.setCategory("Arduino Experiments");
   Distance_ToF.setDescription("Random numbers are generated on Arduino and plotted with phyphox afterwards");

   //View
   View firstView;
   firstView.setLabel("FirstView");	//Create a View

   //Graph
   Graph firstGraph;			//Create Graph which will plot random numbers against time     
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

   firstView.addElement(firstGraph);      	//Attach Graph to View
   PlotRandomValues.addView(firstView);         //Attach View to Experiment
   server.addExperiment(PlotRandomValues);    	//Attach Experiment to Server

}


void loop()
{
    
  float randomDistance = random(0,100); //Erzeuge Zufallszahlen zwischen 0 - 100
  /*  The random number is written into Channel 1
   *  Up to 5 Channels can written at the same time with server.write(randomDistance, valueChannel2, valueChannel3.. )
   */

  server.write(randomDistance);		
  delay(50);
  
}
