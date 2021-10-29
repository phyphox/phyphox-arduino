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
	strcat(LABEL, " label=\"");
	strcat(LABEL, l);
	strcat(LABEL, "\"");
}

void PhyphoxBleExperiment::View::setXMLAttribute(const char *xml){
	memset(&XMLAttribute[0], 0, sizeof(XMLAttribute));
	strcat(XMLAttribute, " ");
	strcat(XMLAttribute, xml);
}

void PhyphoxBleExperiment::View::getBytes(char *buffArray, uint8_t elem)
{
	if(elem == 0) {
		strcat(buffArray, "\t<view");
		strcat(buffArray, LABEL);
		strcat(buffArray, XMLAttribute);
		strcat(buffArray,">\n");
	}

	if(ELEMENTS[elem]!=nullptr){
		if(strcmp(ELEMENTS[elem]->ERROR.MESSAGE, "") == 0) {
			ELEMENTS[elem]->getBytes(buffArray);
		}
	}

	if(elem == phyphoxBleNElements-1) {
		strcat(buffArray,"\t</view>\n");
	}



}


