#include "phyphoxBleExperiment.h"
#include "copyToMem.h"

void PhyphoxBleExperiment::Channel::getBytes(char *buffArray)
{
    strcat(buffArray, "</input>\n");
    strcat(buffArray, "\t\t\t<input axis=\"x\" color=\"");
    strcat(buffArray, COLOR);
    strcat(buffArray, "\">");
    if (!CHANNELX)  {strcat(buffArray,"CH0");} else {strcat(buffArray,CHANNELX);}
    strcat(buffArray, "</input>\n\t\t\t<input axis=\"y\">");
    if (!CHANNELY)  {strcat(buffArray,"CH1");} else {strcat(buffArray,CHANNELY);}
}
