#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::Separator::setHeight(float h)
{
	char tmp[10];
	sprintf(tmp, "%f", h);;
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(tmp, 10, "setHeight") : ERROR;
	sprintf(HEIGHT, " height=\"%.2f\"", h);
}

void PhyphoxBleExperiment::Separator::setColor(const char *c)
{
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkHex(c, "setColor") : ERROR;
	memset(&COLOR[0], 0, sizeof(COLOR));
	strcat(COLOR, " color=\"");
	strcat(COLOR, c);
	strcat(COLOR, "\"");
} 

void PhyphoxBleExperiment::Separator::setXMLAttribute(const char * xml) {
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(xml, 98, "setXMLAttribute") : ERROR;
	memset(&XMLAttribute[0], 0, sizeof(XMLAttribute));
	strcat(XMLAttribute, " ");
	strcat(XMLAttribute, xml);
}

void PhyphoxBleExperiment::Separator::getBytes(char *buffArray)
{

	strcat(buffArray,"\t\t<separator");
	strcat(buffArray, HEIGHT);
	strcat(buffArray, COLOR);
	strcat(buffArray,XMLAttribute);
	strcat(buffArray,">\n");
	strcat(buffArray, "\t\t</separator>\n");

}
