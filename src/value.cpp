#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::Value::setLabel(const char *l)
{
	sprintf(LABEL, l);
}

void PhyphoxBleExperiment::Value::setColor(const char *c)
{
	sprintf(COLOR, c);
} 

void PhyphoxBleExperiment::Value::setPrecision(int p)
{
	sprintf(PRECISION, "%d", p);
}

void PhyphoxBleExperiment::Value::setUnit(const char* u)
{
	sprintf(UNIT, u);
}

void PhyphoxBleExperiment::Value::setChannel(int c)
{
	printf(INPUTVALUE, "CH%i", c);
}

void PhyphoxBleExperiment::Value::setWild(const char *w){
	memset(&WILD[0], 0, sizeof(WILD));
	strcat(WILD, w);
}

void PhyphoxBleExperiment::Value::getBytes(char *buffArray)
{

	strcat(buffArray,"\t\t<value label=\"");
	strcat(buffArray, LABEL);
    strcat(buffArray,"\" size=\"1\" precision=\"");
    strcat(buffArray, PRECISION);
    strcat(buffArray,"\" scientific=\"false\" unit=\"");
    strcat(buffArray, UNIT);
    strcat(buffArray,"\" facor=\"1\" color=\"");
    strcat(buffArray, COLOR);
	strcat(buffArray,"\" ");
	strcat(buffArray, WILD);
	strcat(buffArray,">\n");

    strcat(buffArray, "\t\t\t<input>");
	strcat(buffArray, INPUTVALUE);
	strcat(buffArray, "</input>\n\t\t</value>\n");
	
}
