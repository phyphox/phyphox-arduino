#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::InfoField::setInfo(const char *i)
{
	sprintf(INFO, i);
}

void PhyphoxBleExperiment::InfoField::getBytes(char *buffArray)
{

	strcat(buffArray,"\t\t<info label=\"");
	strcat(buffArray, INFO);
	strcat(buffArray,"\">\n");
	strcat(buffArray, "\t\t</info>\n");
	
}
