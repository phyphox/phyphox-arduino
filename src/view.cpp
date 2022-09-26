#include "phyphoxBleExperiment.h"
#include "copyToMem.h"

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
	copyToMem(&LABEL, (" label=\"" + std::string(l) + "\"").c_str());
}

void PhyphoxBleExperiment::View::setXMLAttribute(const char *xml){
	copyToMem(&XMLAttribute, (" " + std::string(xml)).c_str());
}

void PhyphoxBleExperiment::View::getBytes(char *buffArray, uint8_t elem)
{
	if(elem == 0) {
		strcat(buffArray, "\t<view");
		if (!LABEL)  {strcat(buffArray," label=\"label\"");} else {strcat(buffArray,LABEL);}
		if (XMLAttribute) {strcat(buffArray, XMLAttribute);}
		strcat(buffArray,">\n");
	}

	if(ELEMENTS[elem]!=nullptr){
		if(ELEMENTS[elem]->ERROR.MESSAGE == NULL) {
			ELEMENTS[elem]->getBytes(buffArray);
		}
	}

	if(elem == phyphoxBleNElements-1) {
		strcat(buffArray,"\t</view>\n");
	}



}


