#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::Separator::setHeight(float h)
{
	sprintf(HEIGHT, " height=\"%.2f\"", h);
}

void PhyphoxBleExperiment::Separator::setColor(const char *c)
{
	memset(&COLOR[0], 0, sizeof(COLOR));
	strcat(COLOR, " color=\"");
	strcat(COLOR, c);
	strcat(COLOR, "\"");
} 

void PhyphoxBleExperiment::Separator::setXMLAttribute(const char * xml) {
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
