#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::Element::setLabel(const char *l){
	memset(&LABEL[0], 0, sizeof(LABEL));
	strcat(LABEL, l);
}

// void PhyphoxBleExperiment::Element::setWild(const char *w){
// 	memset(&WILD[0], 0, sizeof(WILD));
// 	strcat(WILD, w);
// }