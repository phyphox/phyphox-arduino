#include "view.h"


void View::addElement(Graph& g)
{
	for(int i=0; i<nElements; i++){
		if(ELEMENTS[i]==nullptr){
			ELEMENTS[i] = &g;
			break;
		}
	}	

}

void View::setLabel(char *l){
	memset(&LABEL[0], 0, sizeof(LABEL));
	strcat(LABEL, l);
}

void View::getBytes(char *buffArray)
{
	
	strcat(buffArray, "\t<view label=\"");
	strcat(buffArray, LABEL);
	strcat(buffArray, "\">\n");

	//loop over elements
	for(int i=0; i<nElements; i++){
		if(ELEMENTS[i]!=nullptr){
			ELEMENTS[i]->getBytes(buffArray);
		}
	}
	strcat(buffArray,"\t</view>\n");



}


