#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::ExportData::setDatachannel(int d){
	sprintf(BUFFER, "CH%d", d);
}

void PhyphoxBleExperiment::ExportData::setXMLAttribute(const char *xml){
	memset(&XMLAttribute[0], 0, sizeof(XMLAttribute));
	strcat(XMLAttribute," ");
	strcat(XMLAttribute, xml);
}

void PhyphoxBleExperiment::ExportData::getBytes(char *buffArray)
{
	
	strcat(buffArray,"\t\t<data");
    strcat(buffArray, LABEL);
	strcat(buffArray, XMLAttribute);
	strcat(buffArray,">");
    strcat(buffArray, BUFFER);
    strcat(buffArray, "</data>\n");
	
}