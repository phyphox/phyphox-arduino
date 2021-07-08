#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::Value::setColor(const char *c)
{
	memset(&COLOR[0], 0, sizeof(COLOR));
	strcat(COLOR, " color=\"");
	strcat(COLOR, c);
	strcat(COLOR, "\"");
} 

void PhyphoxBleExperiment::Value::setPrecision(int p)
{
	sprintf(PRECISION, " precision=\"%d\"", p);
}

void PhyphoxBleExperiment::Value::setUnit(const char* u)
{
	memset(&UNIT[0], 0, sizeof(UNIT));
	strcat(UNIT, " unit=\"");
	strcat(UNIT, u);
	strcat(UNIT, "\"");
}

void PhyphoxBleExperiment::Value::setChannel(int c)
{
	sprintf(INPUTVALUE, "CH%i", c);
}

void PhyphoxBleExperiment::Value::setXMLAttribute(const char *xml){
	memset(&XMLAttribute[0], 0, sizeof(XMLAttribute));
	strcat(XMLAttribute, " ");
	strcat(XMLAttribute, xml);
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
