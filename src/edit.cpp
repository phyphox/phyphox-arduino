#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::Edit::setLabel(const char *l)
{
	sprintf(LABEL, l);
}

void PhyphoxBleExperiment::Edit::setUnit(const char *u)
{
	sprintf(UNIT, u);
} 

void PhyphoxBleExperiment::Edit::setSigned(bool s)
{
    if(s) sprintf(SIGNED, "true");
    else sprintf(SIGNED, "false");
}

void PhyphoxBleExperiment::Edit::setDecimal(bool d)
{
	if(d) sprintf(SIGNED, "true");
    else sprintf(SIGNED, "false");
}

void PhyphoxBleExperiment::Edit::setWild(const char *w){
	memset(&WILD[0], 0, sizeof(WILD));
	strcat(WILD, w);
}

void PhyphoxBleExperiment::Edit::getBytes(char *buffArray)
{
	strcat(buffArray,"\t\t<edit label=\"");
	strcat(buffArray, LABEL);
    strcat(buffArray,"\" size=\"1\" signed=\"");
    strcat(buffArray, SIGNED);
    strcat(buffArray,"\" decimal=\"");
    strcat(buffArray, DECIMAL);
    strcat(buffArray,"\" unit=\"");
    strcat(buffArray, UNIT);
    strcat(buffArray,"\" ");
	strcat(buffArray, WILD);
	strcat(buffArray," facor=\"1\" default=\"0\">\n");
    strcat(buffArray,"\t\t<output>CH3</output>\n");
	strcat(buffArray, "\t\t</edit>\n");	
}
