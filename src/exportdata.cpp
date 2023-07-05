#include "phyphoxBleExperiment.h"
#include "copyToMem.h"

void PhyphoxBleExperiment::ExportData::setDatachannel(int d){
	char tmp[20];
	sprintf(tmp, "CH%d", d);
	copyToMem(&BUFFER, tmp);
}

void PhyphoxBleExperiment::ExportData::setXMLAttribute(const char *xml){
		copyToMem(&XMLAttribute, (" " + std::string(xml)).c_str());
}

void PhyphoxBleExperiment::ExportData::setLabel(const char *l)
{
	copyToMem(&LABEL, (std::string(l)).c_str());
}

void PhyphoxBleExperiment::ExportData::getBytes(char *buffArray)
{
	strcat(buffArray,"\t\t<data name=\"");
    if (!LABEL)  {strcat(buffArray,"label");} else {strcat(buffArray,LABEL);}
	if (XMLAttribute) {strcat(buffArray,XMLAttribute);}
	strcat(buffArray,"\">");
    if (!BUFFER)  {strcat(buffArray,"CH1");} else {strcat(buffArray,BUFFER);}
    strcat(buffArray, "</data>\n");
	
}