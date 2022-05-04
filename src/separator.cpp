#include "phyphoxBleExperiment.h"
#include "copyToMem.h"

void PhyphoxBleExperiment::Separator::setHeight(float h)
{
	char tmp[20];
	sprintf(tmp, "%f", h);;
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(tmp, 10, "setHeight") : ERROR;
	sprintf(tmp, " height=\"%.2f\"", h);
	copyToMem(&HEIGHT, tmp);
}

void PhyphoxBleExperiment::Separator::setColor(const char *c)
{
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkHex(c, "setColor") : ERROR;
	copyToMem(&COLOR, (" color=\"" + std::string(c) + "\"").c_str());
} 

void PhyphoxBleExperiment::Separator::setXMLAttribute(const char * xml) {
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(xml, 98, "setXMLAttribute") : ERROR;
	copyToMem(&XMLAttribute, (" " + std::string(xml)).c_str());
}

void PhyphoxBleExperiment::Separator::getBytes(char *buffArray)
{

	strcat(buffArray,"\t\t<separator");
	strcat(buffArray, HEIGHT);
	strcat(buffArray, COLOR);
	strcat(buffArray,XMLAttribute);
	strcat(buffArray,">\n");
	strcat(buffArray, "\t\t</separator>\n");

}
