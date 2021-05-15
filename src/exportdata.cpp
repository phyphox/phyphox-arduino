#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::Exportdata::setName(const char *n){
	memset(&NAME[0], 0, sizeof(NAME));
	strcat(NAME, n);
}

void PhyphoxBleExperiment::Exportdata::setBuffer(const char *b){
	memset(&BUFFER[0], 0, sizeof(BUFFER));
	strcat(BUFFER, b);
}

void PhyphoxBleExperiment::Exportdata::getBytes(char *buffArray)
{
	
	strcat(buffArray,"\t\t<data name=\"");
    strcat(buffArray, NAME);
    strcat(buffArray, "\">");
    strcat(buffArray, BUFFER);
    strcat(buffArray, "</data>\n");
	
}