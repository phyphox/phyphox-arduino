#include "phyphoxBleExperiment.h"
#include "copyToMem.h"

void PhyphoxBleExperiment::Element::setLabel(const char *l){
	ERROR = ERROR.MESSAGE == NULL ? err_checkLength(l, 41, "setLabel") : ERROR;
	copyToMem(&LABEL, (std::string(l)).c_str());
}

// void PhyphoxBleExperiment::Element::setLabel(const char *l){
// 	copyToMem(&LABEL, (" label=\"" + std::string(l) + "\"").c_str());
// }
