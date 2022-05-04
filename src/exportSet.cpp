#include "phyphoxBleExperiment.h"
#include "copyToMem.h"

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
	copyToMem(&LABEL, (" name=\"" + std::string(l) + "\"").c_str());
}

void PhyphoxBleExperiment::ExportSet::setXMLAttribute(const char *xml){
	copyToMem(&XMLAttribute, (" " + std::string(xml)).c_str());
}

void PhyphoxBleExperiment::ExportSet::getBytes(char *buffArray)
{
	
	strcat(buffArray, "\t<set");
	strcat(buffArray, LABEL);
	strcat(buffArray, XMLAttribute);
	strcat(buffArray,">\n");

	//loop over elements
	for(int i=0; i<phyphoxBleNExportSets; i++){
		if(ELEMENTS[i]!=nullptr){
			ELEMENTS[i]->getBytes(buffArray);
		}
	}
	strcat(buffArray,"\t</set>\n");



}


