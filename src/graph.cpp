#include "phyphoxBleExperiment.h"

// void PhyphoxBleExperiment::Graph::setUnitX(const char *ux){

// }

// void PhyphoxBleExperiment::Graph::setUnitY(const char *uy){

// }

// void PhyphoxBleExperiment::Graph::setLabelX(const char *lx){

// }

// void PhyphoxBleExperiment::Graph::setLabelY(const char *ly){

// }

// void PhyphoxBleExperiment::Graph::setColor(const char *c){
// }

// void PhyphoxBleExperiment::Graph::setXPrecision(int px){
// }

// void PhyphoxBleExperiment::Graph::setYPrecision(int py){
// }

 void PhyphoxBleExperiment::Graph::setChannel(int x, int y)
 {

 }

// void PhyphoxBleExperiment::Graph::setStyle(const char *s){

// }

void PhyphoxBleExperiment::Graph::getBytes(char *buffArray)
{
	char tmp[] = "\n";

	for(int i=0; i<850; i++) {
			strcat(buffArray,tmp);
	}

	
}
