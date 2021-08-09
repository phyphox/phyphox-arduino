#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::Edit::setUnit(const char *u)
{
    ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(u, 12, "setUnit") : ERROR;
    memset(&UNIT[0], 0, sizeof(UNIT));
	strcat(UNIT, " unit=\"");
	strcat(UNIT, u);
	strcat(UNIT, "\"");
} 

void PhyphoxBleExperiment::Edit::setSigned(bool s)
{
    if(s) sprintf(SIGNED, " signed=\"true\"");
    else sprintf(SIGNED, " signed=\"false\"");
}

void PhyphoxBleExperiment::Edit::setDecimal(bool d)
{
	if(d) sprintf(SIGNED, " decimal=\"true\"");
    else sprintf(SIGNED, " decimal=\"false\"");
}

void PhyphoxBleExperiment::Edit::setXMLAttribute(const char *xml){
    ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(xml, 98, "setXMLAttribute") : ERROR;
	memset(&XMLAttribute[0], 0, sizeof(XMLAttribute));
    strcat(XMLAttribute, " ");
	strcat(XMLAttribute, xml);
}

void PhyphoxBleExperiment::Edit::setChannel(int b){
    ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkUpper(b, 1, "setChannel") : ERROR;
	sprintf(BUFFER, "CB%i", b);
}

void PhyphoxBleExperiment::Edit::getBytes(char *buffArray)
{
	strcat(buffArray,"\t\t<edit");
	strcat(buffArray, LABEL);
    strcat(buffArray, SIGNED);
    strcat(buffArray, DECIMAL);
    strcat(buffArray, UNIT);
	strcat(buffArray, XMLAttribute);
    strcat(buffArray,">\n");
    strcat(buffArray,"\t\t<output>");
    strcat(buffArray, BUFFER);
    strcat(buffArray,"</output>\n");
	strcat(buffArray, "\t\t</edit>\n");	
}
