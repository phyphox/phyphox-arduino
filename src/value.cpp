#include "phyphoxBleExperiment.h"
#include "copyToMem.h"

void PhyphoxBleExperiment::Value::setColor(const char *c)
{
	ERROR = ERROR.MESSAGE == NULL ? err_checkHex(c, "setColor") : ERROR;
	copyToMem(&COLOR, (" color=\"" + std::string(c) + "\"").c_str());
} 

void PhyphoxBleExperiment::Value::setPrecision(int p)
{
	ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(p, 999, "setPrecision") : ERROR;
	char tmp[20];
	sprintf(tmp, " precision=\"%i\"", p);
	copyToMem(&PRECISION, tmp);
}

void PhyphoxBleExperiment::Value::setUnit(const char* u)
{
	ERROR = ERROR.MESSAGE == NULL ? err_checkLength(u, 12, "setUnit") : ERROR;
	copyToMem(&UNIT, (" unit=\"" + std::string(u) + "\"").c_str());
}

void PhyphoxBleExperiment::Value::setChannel(int c)
{
	ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(c, numberOfChannels, "setChannel") : ERROR;
	char tmp[20];
	sprintf(tmp, "CH%i", c);
	copyToMem(&INPUTVALUE, tmp);
}

void PhyphoxBleExperiment::Value::setXMLAttribute(const char *xml){
	ERROR = ERROR.MESSAGE == NULL ? err_checkLength(xml, 98, "setXMLAttribute") : ERROR;
	copyToMem(&XMLAttribute, (" " + std::string(xml)).c_str());
}

void PhyphoxBleExperiment::Value::getBytes(char *buffArray)
{

	strcat(buffArray,"\t\t<value");
	if (LABEL)  {
		strcat(buffArray," label=\"");
		strcat(buffArray,LABEL);
		strcat(buffArray,"\"");
	}else{
		strcat(buffArray," label=\"myLabel\"");
	}		
	if (PRECISION) {strcat(buffArray,PRECISION);}
	if (UNIT) {strcat(buffArray,UNIT);}
    strcat(buffArray," facor=\"1\"");
	if (COLOR) {strcat(buffArray,COLOR);}
	if (XMLAttribute) {strcat(buffArray,XMLAttribute);}
	strcat(buffArray,">\n");

    strcat(buffArray, "\t\t\t<input>");
	if (!INPUTVALUE)  {strcat(buffArray,"CH3");} else {strcat(buffArray,INPUTVALUE);}
	strcat(buffArray, "</input>\n\t\t</value>\n");
	
}
