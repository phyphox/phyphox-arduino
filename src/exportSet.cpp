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

void PhyphoxBleExperiment::ExportSet::setLabel(const char *l){
	memset(&LABEL[0], 0, sizeof(LABEL));
	strcat(LABEL, " label=\"");
	strcat(LABEL, l);
	strcat(LABEL, "\"");
}

void PhyphoxBleExperiment::ExportSet::setXMLAttribute(const char *xml){
	memset(&XMLAttribute[0], 0, sizeof(XMLAttribute));
	strcat(XMLAttribute, " ");
	strcat(XMLAttribute, xml);
}

void PhyphoxBleExperiment::ExportSet::getBytes(char *buffArray)
{
	
	strcat(buffArray, "\t<set");
	strcat(buffArray, LABEL);
	strcat(buffArray, XMLAttribute);
	strcat(buffArray,">\n");

	//loop over elements
	for(int i=0; i<phyphoxBleNElements; i++){
		if(ELEMENTS[i]!=nullptr){
			ELEMENTS[i]->getBytes(buffArray);
		}
	}
	strcat(buffArray,"\t</set>\n");



}


