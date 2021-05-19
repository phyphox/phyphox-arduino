#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::InfoField::setInfo(const char *i)
{
	sprintf(INFO, i);
}

void PhyphoxBleExperiment::InfoField::setColor(const char *c)
{
	sprintf(COLOR, c);
} 

void PhyphoxBleExperiment::InfoField::getBytes(char *buffArray)
{

	strcat(buffArray,"\t\t<info label=\"");
	strcat(buffArray, INFO);
    strcat(buffArray,"\" color=\"");
    strcat(buffArray, COLOR);
	strcat(buffArray,"\">\n");
	strcat(buffArray,"\t\t</info>\n");
	
}
