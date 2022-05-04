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

void PhyphoxBleExperiment::ExportData::getBytes(char *buffArray)
{
	
	strcat(buffArray,"\t\t<data");
    strcat(buffArray, LABEL);
	strcat(buffArray, XMLAttribute);
	strcat(buffArray,">");
    strcat(buffArray, BUFFER);
    strcat(buffArray, "</data>\n");
	
}

void PhyphoxBleExperiment::ExportData::setLabel(const char *l)
{
	//ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(l, 41, "setLabel") : ERROR;
	memset(&LABEL[0], 0, sizeof(LABEL));
	strcat(LABEL, " name=\"");
	strcat(LABEL, l);
	strcat(LABEL, "\"");	
}