#include "phyphoxBleExperiment.h"
#include "copyToMem.h"

void PhyphoxBleExperiment::Toggle::setDefault(float v) {
    char tmp[20];
	sprintf(tmp, "%f", v);
	copyToMem(&DEFAULT, tmp);
}

void PhyphoxBleExperiment::Toggle::setXMLAttribute(const char *xml) {
    ERROR = ERROR.MESSAGE == NULL ? err_checkLength(xml, 98, "setXMLAttribute") : ERROR;
	copyToMem(&XMLAttribute, (" " + std::string(xml)).c_str());
}

void PhyphoxBleExperiment::Toggle::setChannel(int b) {
    ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(b, 5, "setChannel") : ERROR;
    char tmp[20];
	sprintf(tmp, "CB%i", b);
	copyToMem(&BUFFER, tmp);
}

void PhyphoxBleExperiment::Toggle::getBytes(char *buffArray) {
	strcat(buffArray,"\t\t<toggle");    
    if (LABEL) {
        strcat(buffArray," label=\"");
        strcat(buffArray,LABEL);
        strcat(buffArray,"\"");
    } else {
        strcat(buffArray," label=\"label\"");
    }
    if (DEFAULT) {
        strcat(buffArray," default=\"");
        strcat(buffArray,DEFAULT);
        strcat(buffArray,"\"");
    }
    if (XMLAttribute) {
        strcat(buffArray,XMLAttribute);
    }
    strcat(buffArray,">\n");
    strcat(buffArray,"\t\t<output>");
    if (!BUFFER) {
        strcat(buffArray,"CH5");
    } else {
        strcat(buffArray,BUFFER);
    }
    strcat(buffArray,"</output>\n");
	strcat(buffArray, "\t\t</toggle>\n");	
}
