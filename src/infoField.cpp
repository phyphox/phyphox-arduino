#include "phyphoxBleExperiment.h"
#include "copyToMem.h"

void PhyphoxBleExperiment::InfoField::setInfo(const char *i)
{
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(i, 191, "setInfo") : ERROR;
	copyToMem(&INFO, (" label=\"" + std::string(i) + "\"").c_str());
}

void PhyphoxBleExperiment::InfoField::setColor(const char *c)
{
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkHex(c, "setColor") : ERROR;
	copyToMem(&COLOR, (" color=\"" + std::string(c) + "\"").c_str());
} 

void PhyphoxBleExperiment::InfoField::setXMLAttribute(const char *xml){
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(xml, 98, "setXMLAttribute") : ERROR;
	copyToMem(&XMLAttribute, (" " + std::string(xml)).c_str());
}

void PhyphoxBleExperiment::InfoField::getBytes(char *buffArray)
{

	strcat(buffArray,"\t\t<info");
	strcat(buffArray, INFO);
    strcat(buffArray, COLOR);
	strcat(buffArray, XMLAttribute);
	strcat(buffArray,">\n");
	strcat(buffArray,"\t\t</info>\n");
	
}
