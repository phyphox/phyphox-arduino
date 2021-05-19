#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::Separator::setHeight(float h)
{
	sprintf(HEIGHT, "%.2f", h);
}

void PhyphoxBleExperiment::Separator::setColor(const char *c)
{
	sprintf(COLOR, c);
} 

void PhyphoxBleExperiment::Separator::getBytes(char *buffArray)
{

	strcat(buffArray,"\t\t<separator height=\"");
	strcat(buffArray, HEIGHT);
    strcat(buffArray,"\" color=\"");
	strcat(buffArray, COLOR);
	strcat(buffArray,"\">\n");
	strcat(buffArray, "\t\t</separator>\n");

}
