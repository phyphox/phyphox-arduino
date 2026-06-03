#include "phyphoxBleExperiment.h"
#include "copyToMem.h"

void PhyphoxBleExperiment::Dropdown::setDefault(float v) {
    char tmp[20];
	sprintf(tmp, "%f", v);
	copyToMem(&DEFAULT, tmp);
}

void PhyphoxBleExperiment::Dropdown::setOptions(int n, const char * const labels[], const float * values) {
    N_OPTIONS = n;
    LABELS = (char**) malloc(sizeof(char*) * n);
    VALUES = (char**) malloc(sizeof(char*) * n);
    char tmp[20];
    for (int i = 0; i < n; i++) {
        ERROR = ERROR.MESSAGE == NULL ? err_checkLength(labels[i], 41, "setOptions") : ERROR;
	    copyToMem(&LABELS[i], (std::string(labels[i])).c_str());
	    sprintf(tmp, "%f", values[i]);
	    copyToMem(&VALUES[i], tmp);
    }
}

void PhyphoxBleExperiment::Dropdown::setXMLAttribute(const char *xml) {
    ERROR = ERROR.MESSAGE == NULL ? err_checkLength(xml, 98, "setXMLAttribute") : ERROR;
	copyToMem(&XMLAttribute, (" " + std::string(xml)).c_str());
}

void PhyphoxBleExperiment::Dropdown::setChannel(int b) {
    ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(b, 5, "setChannel") : ERROR;
    char tmp[20];
	sprintf(tmp, "CB%i", b);
	copyToMem(&BUFFER, tmp);
}

void PhyphoxBleExperiment::Dropdown::getBytes(char *buffArray) {
	strcat(buffArray, "\t\t<dropdown");    
    if (LABEL) {
        strcat(buffArray, " label=\"");
        strcat(buffArray, LABEL);
        strcat(buffArray, "\"");
    } else {
        strcat(buffArray, " label=\"label\"");
    }
    if (DEFAULT) {
        strcat(buffArray, " default=\"");
        strcat(buffArray, DEFAULT);
        strcat(buffArray, "\"");
    }
    if (XMLAttribute) {
        strcat(buffArray, XMLAttribute);
    }
    strcat(buffArray, ">\n");
    strcat(buffArray, "\t\t<output>");
    if (!BUFFER) {
        strcat(buffArray, "CH5");
    } else {
        strcat(buffArray, BUFFER);
    }
    strcat(buffArray, "</output>\n");
    for (int i = 0; i < N_OPTIONS; i++) {
        strcat(buffArray, "\t\t<map value=\"");
        strcat(buffArray, VALUES[i]);
        strcat(buffArray, "\">");
        strcat(buffArray, LABELS[i]);
        strcat(buffArray, "</map>\n");
    }
	strcat(buffArray, "\t\t</dropdown>\n");	
}
