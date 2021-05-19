#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::ExportData::setName(const char *n){
	memset(&NAME[0], 0, sizeof(NAME));
	strcat(NAME, n);
}

void PhyphoxBleExperiment::ExportData::setDatachannel(int d){
	sprintf(BUFFER, "CH%d", d);
}

void PhyphoxBleExperiment::ExportData::getBytes(char *buffArray)
{
	
	strcat(buffArray,"\t\t<data name=\"");
    strcat(buffArray, NAME);
    strcat(buffArray, "\">");
    strcat(buffArray, BUFFER);
    strcat(buffArray, "</data>\n");
	
}