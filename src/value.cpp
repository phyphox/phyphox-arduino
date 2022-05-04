#include "phyphoxBleExperiment.h"
#include "copyToMem.h"

void PhyphoxBleExperiment::Value::setColor(const char *c)
{
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkHex(c, "setColor") : ERROR;
	copyToMem(&COLOR, (" color=\"" + std::string(c) + "\"").c_str());
} 

void PhyphoxBleExperiment::Value::setPrecision(int p)
{
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkUpper(p, 999, "setPrecision") : ERROR;
	char tmp[20];
	sprintf(tmp, " precision=\"%i\"", p);
	copyToMem(&PRECISION, tmp);
}

void PhyphoxBleExperiment::Value::setUnit(const char* u)
{
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(u, 12, "setUnit") : ERROR;
	copyToMem(&UNIT, (" unit=\"" + std::string(u) + "\"").c_str());
}

void PhyphoxBleExperiment::Value::setChannel(int c)
{
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkUpper(c, numberOfChannels, "setChannel") : ERROR;
	char tmp[20];
	sprintf(tmp, "CH%i", c);
	copyToMem(&INPUTVALUE, tmp);
}

void PhyphoxBleExperiment::Value::setXMLAttribute(const char *xml){
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(xml, 98, "setXMLAttribute") : ERROR;
	copyToMem(&XMLAttribute, (" " + std::string(xml)).c_str());
}

void PhyphoxBleExperiment::Value::getBytes(char *buffArray)
{

	strcat(buffArray,"\t\t<value");
	strcat(buffArray, LABEL);
    strcat(buffArray, PRECISION);
    strcat(buffArray, UNIT);
    strcat(buffArray," facor=\"1\"");
    strcat(buffArray, COLOR);
	strcat(buffArray, XMLAttribute);
	strcat(buffArray,">\n");

    strcat(buffArray, "\t\t\t<input>");
	strcat(buffArray, INPUTVALUE);
	strcat(buffArray, "</input>\n\t\t</value>\n");
	
}
