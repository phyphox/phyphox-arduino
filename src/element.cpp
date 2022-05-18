#include "phyphoxBleExperiment.h"
#include "copyToMem.h"

void PhyphoxBleExperiment::Element::setLabel(const char *l){
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(l, 41, "setLabel") : ERROR;
	memset(&LABEL[0], 0, sizeof(LABEL));
	strcat(LABEL, " label=\"");
	strcat(LABEL, l);
	strcat(LABEL, "\"");
}

// void PhyphoxBleExperiment::Element::setLabel(const char *l){
// 	copyToMem(&LABEL, (" label=\"" + std::string(l) + "\"").c_str());
// }
