#include "phyphoxBleExperiment.h"


void PhyphoxBleExperiment::View::addElement(Element& e)
{
	for(int i=0; i<phyphoxBleNElements; i++){
		if(ELEMENTS[i]==nullptr){
			ELEMENTS[i] = &e;
			break;
		}
	}	

}

void PhyphoxBleExperiment::View::setLabel(const char *l){
	memset(&LABEL[0], 0, sizeof(LABEL));
	strcat(LABEL, l);
}

void PhyphoxBleExperiment::View::getBytes(char *buffArray, uint8_t elem)
{
	if(elem == 0) {
		strcat(buffArray, "\t<view label=\"");
		strcat(buffArray, LABEL);
		strcat(buffArray, "\">\n");
	}
	

	//loop over elements
	// for(int i=0; i<phyphoxBleNElements; i++){
	// 	if(ELEMENTS[i]!=nullptr){
	// 		ELEMENTS[i]->getBytes(buffArray);
	// 	}
	// }
	if(ELEMENTS[elem]!=nullptr){
		ELEMENTS[elem]->getBytes(buffArray);
	}

	if(elem == phyphoxBleNElements-1) {
		strcat(buffArray,"\t</view>\n");
	}



}


