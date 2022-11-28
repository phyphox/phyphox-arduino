#include "phyphoxBleExperiment.h"
#include "copyToMem.h"

void PhyphoxBleExperiment::Edit::setUnit(const char *u)
{
    ERROR = ERROR.MESSAGE == NULL ? err_checkLength(u, 12, "setUnit") : ERROR;
    copyToMem(&UNIT, (std::string(u)).c_str());
} 

void PhyphoxBleExperiment::Edit::setSigned(bool s)
{
    if(s) copyToMem(&SIGNED, "true");
    else copyToMem(&SIGNED, "false");
}

void PhyphoxBleExperiment::Edit::setDecimal(bool d)
{
    if(d) copyToMem(&DECIMAL, "true");
    else copyToMem(&DECIMAL, "false");
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
    if(LABEL){
        strcat(buffArray," label=\"");
        strcat(buffArray,LABEL);
        strcat(buffArray,"\"");
    } else {
        strcat(buffArray," label=\"label\"");
    }
    if(SIGNED){
        strcat(buffArray," signed=\"");
        strcat(buffArray,SIGNED);
        strcat(buffArray,"\"");
    }

    if(DECIMAL){
        strcat(buffArray," decimal=\"");
        strcat(buffArray,DECIMAL);
        strcat(buffArray,"\"");        
    }

    if (UNIT){
        strcat(buffArray," unit=\"");
        strcat(buffArray,UNIT);
        strcat(buffArray,"\"");
    }
    if (XMLAttribute) {strcat(buffArray,XMLAttribute);}
    strcat(buffArray,">\n");
    strcat(buffArray,"\t\t<output>");
    if (!BUFFER)  {strcat(buffArray,"CH5");} else {strcat(buffArray,BUFFER);}
    strcat(buffArray,"</output>\n");
	strcat(buffArray, "\t\t</edit>\n");	
}
