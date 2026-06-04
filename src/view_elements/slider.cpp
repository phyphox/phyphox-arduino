#include "phyphoxBleExperiment.h"
#include "copyToMem.h"

void PhyphoxBleExperiment::Slider::setDefault(float v) {
    char tmp[20];
	sprintf(tmp, "%f", v);
	copyToMem(&DEFAULT_VALUE, tmp);
}

void PhyphoxBleExperiment::Slider::setMinValue(float v) {
    char tmp[20];
	sprintf(tmp, "%f", v);
	copyToMem(&MINV, tmp);
}

void PhyphoxBleExperiment::Slider::setMaxValue(float v) {
    char tmp[20];
	sprintf(tmp, "%f", v);
	copyToMem(&MAXV, tmp);
}

void PhyphoxBleExperiment::Slider::setStepSize(float v) {
    char tmp[20];
	sprintf(tmp, "%f", v);
	copyToMem(&STEPSIZE, tmp);
}

void PhyphoxBleExperiment::Slider::setPrecision(int n) {
    char tmp[20];
	sprintf(tmp, "%i", n);
	copyToMem(&PRECISION, tmp);
}

void PhyphoxBleExperiment::Slider::setShowValue(bool b) {
    if (b)
        copyToMem(&SHOWVALUE, "true");
    else
        copyToMem(&SHOWVALUE, "false");
}

void PhyphoxBleExperiment::Slider::setXMLAttribute(const char *xml) {
    ERROR = ERROR.MESSAGE == NULL ? err_checkLength(xml, 98, "setXMLAttribute") : ERROR;
	copyToMem(&XMLAttribute, (" " + std::string(xml)).c_str());
}

void PhyphoxBleExperiment::Slider::setChannel(int b) {
    ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(b, 5, "setChannel") : ERROR;
    char tmp[20];
	sprintf(tmp, "CB%i", b);
	copyToMem(&BUFFER, tmp);
}

void PhyphoxBleExperiment::Slider::getBytes(char *buffArray) {
	strcat(buffArray, "\t\t<slider");    
    if (LABEL) {
        strcat(buffArray, " label=\"");
        strcat(buffArray, LABEL);
        strcat(buffArray, "\"");
    } else {
        strcat(buffArray, " label=\"label\"");
    }
    if (DEFAULT_VALUE) {
        strcat(buffArray, " default=\"");
        strcat(buffArray, DEFAULT_VALUE);
        strcat(buffArray, "\"");
    }
    if (MINV) {
        strcat(buffArray, " minValue=\"");
        strcat(buffArray, MINV);
        strcat(buffArray, "\"");
    }
    if (MAXV) {
        strcat(buffArray, " maxValue=\"");
        strcat(buffArray, MAXV);
        strcat(buffArray, "\"");
    }
    if (STEPSIZE) {
        strcat(buffArray, " stepSize=\"");
        strcat(buffArray, STEPSIZE);
        strcat(buffArray, "\"");
    }
    if (PRECISION) {
        strcat(buffArray, " precision=\"");
        strcat(buffArray, PRECISION);
        strcat(buffArray, "\"");
    }
    if (SHOWVALUE) {
        strcat(buffArray, " showValue=\"");
        strcat(buffArray, SHOWVALUE);
        strcat(buffArray, "\"");
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
	strcat(buffArray, "\t\t</slider>\n");	
}
