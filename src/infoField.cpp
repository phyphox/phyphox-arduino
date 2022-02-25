#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::InfoField::setInfo(const char *i)
{
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(i, 191, "setInfo") : ERROR;
	memset(&INFO[0], 0, sizeof(INFO));
	strcat(INFO, " label=\"");
	strcat(INFO, i);
	strcat(INFO, "\"");
}

void PhyphoxBleExperiment::InfoField::setColor(const char *c)
{
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkHex(c, "setColor") : ERROR;
	memset(&COLOR[0], 0, sizeof(COLOR));
	strcat(COLOR, " color=\"");
	strcat(COLOR, c);
	strcat(COLOR, "\"");
} 

void PhyphoxBleExperiment::InfoField::setXMLAttribute(const char *w){
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(w, 98, "setXMLAttribute") : ERROR;
	memset(&XMLAttribute[0], 0, sizeof(XMLAttribute));
	strcat(XMLAttribute, " ");
	strcat(XMLAttribute, w);
}

void PhyphoxBleExperiment::InfoField::getBytes(char *buffArray)
{

	strcat(buffArray,"\t\t<info");
	strcat(buffArray, INFO);
    strcat(buffArray, COLOR);
	strcat(buffArray, XMLAttribute);
	strcat(buffArray,">\n");
	strcat(buffArray,"\t\t</info>\n");
	
}
