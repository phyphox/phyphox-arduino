#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::ExportSet::addElement(Element& e)
{
	for(int i=0; i<phyphoxBleNElements; i++){
		if(ELEMENTS[i]==nullptr){
			ELEMENTS[i] = &e;
			break;
		}
	}	

}

void PhyphoxBleExperiment::ExportSet::setName(const char *n){
	memset(&NAME[0], 0, sizeof(NAME));
	strcat(NAME, n);
}

void PhyphoxBleExperiment::ExportSet::setWild(const char *w){
	memset(&WILD[0], 0, sizeof(WILD));
	strcat(WILD, w);
}

void PhyphoxBleExperiment::ExportSet::getBytes(char *buffArray)
{
	
	strcat(buffArray, "\t<set name=\"");
	strcat(buffArray, NAME);
	strcat(buffArray,"\" ");
	strcat(buffArray, WILD);
	strcat(buffArray,">\n");

	//loop over elements
	for(int i=0; i<phyphoxBleNElements; i++){
		if(ELEMENTS[i]!=nullptr){
			ELEMENTS[i]->getBytes(buffArray);
		}
	}
	strcat(buffArray,"\t</set>\n");



}


