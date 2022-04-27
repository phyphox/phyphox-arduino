#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::Edit::setUnit(const char *u)
{
    ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(u, 12, "setUnit") : ERROR;
    copyToMem(&UNIT, (" unit=\"" + std::string(u) + "\"").c_str());
} 

void PhyphoxBleExperiment::Edit::setSigned(bool s)
{
    if(s) copyToMem(&SIGNED, " signed=\"true\"");
    else copyToMem(&SIGNED, " signed=\"false\"");
}

void PhyphoxBleExperiment::Edit::setDecimal(bool d)
{
    if(d) copyToMem(&SIGNED, " decimal=\"true\"");
    else copyToMem(&SIGNED, " decimal=\"false\"");
}

void PhyphoxBleExperiment::Edit::setXMLAttribute(const char *xml){
    ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(xml, 98, "setXMLAttribute") : ERROR;
	copyToMem(&XMLAttribute, (" " + std::string(xml)).c_str());
}

void PhyphoxBleExperiment::Edit::setChannel(int b){
    ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkUpper(b, 1, "setChannel") : ERROR;
    char tmp[20];
	sprintf(tmp, "CB%i", b);
	copyToMem(&BUFFER, tmp);
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
