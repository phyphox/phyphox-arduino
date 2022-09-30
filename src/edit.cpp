#include "phyphoxBleExperiment.h"
#include "copyToMem.h"

void PhyphoxBleExperiment::Edit::setUnit(const char *u)
{
    ERROR = ERROR.MESSAGE == NULL ? err_checkLength(u, 12, "setUnit") : ERROR;
    copyToMem(&UNIT, (" unit=\"" + std::string(u) + "\"").c_str());
} 

void PhyphoxBleExperiment::Edit::setSigned(bool s)
{
    if(s) copyToMem(&SIGNED, " signed=\"true\"");
    else copyToMem(&SIGNED, " signed=\"false\"");
}

void PhyphoxBleExperiment::Edit::setDecimal(bool d)
{
    if(d) copyToMem(&DECIMAL, " decimal=\"true\"");
    else copyToMem(&DECIMAL, " decimal=\"false\"");
}

void PhyphoxBleExperiment::Edit::setXMLAttribute(const char *xml){
    ERROR = ERROR.MESSAGE == NULL ? err_checkLength(xml, 98, "setXMLAttribute") : ERROR;
	copyToMem(&XMLAttribute, (" " + std::string(xml)).c_str());
}

void PhyphoxBleExperiment::Edit::setChannel(int b){
    ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(b, 5, "setChannel") : ERROR;
    char tmp[20];
	sprintf(tmp, "CB%i", b);
	copyToMem(&BUFFER, tmp);
}

void PhyphoxBleExperiment::Edit::getBytes(char *buffArray)
{
	strcat(buffArray,"\t\t<edit");
	if (!LABEL)  {strcat(buffArray," label=\"label\"");} else {strcat(buffArray,LABEL);}
    if (SIGNED) {strcat(buffArray,SIGNED);}
    if (DECIMAL) {strcat(buffArray,DECIMAL);}
    if (UNIT) {strcat(buffArray,UNIT);}
    if (XMLAttribute) {strcat(buffArray,XMLAttribute);}
    strcat(buffArray,">\n");
    strcat(buffArray,"\t\t<output>");
    if (!BUFFER)  {strcat(buffArray,"CH5");} else {strcat(buffArray,BUFFER);}
    strcat(buffArray,"</output>\n");
	strcat(buffArray, "\t\t</edit>\n");	
}
