#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::Export::addElement(Element& e)
{
	for(int i=0; i<phyphoxBleNElements; i++){
		if(ELEMENTS[i]==nullptr){
			ELEMENTS[i] = &e;
			break;
		}
	}	

}

void PhyphoxBleExperiment::Export::setName(const char *n){
	memset(&NAME[0], 0, sizeof(NAME));
	strcat(NAME, n);
}

void PhyphoxBleExperiment::Export::getBytes(char *buffArray)
{
	
	strcat(buffArray, "\t<set name=\"");
	strcat(buffArray, NAME);
	strcat(buffArray, "\">\n");

	//loop over elements
	for(int i=0; i<phyphoxBleNElements; i++){
		if(ELEMENTS[i]!=nullptr){
			ELEMENTS[i]->getBytes(buffArray);
		}
	}
	strcat(buffArray,"\t</set>\n");



}


