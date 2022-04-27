#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::Element::setLabel(const char *l){
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(l, 41, "setLabel") : ERROR;
	memset(&LABEL[0], 0, sizeof(LABEL));
	strcat(LABEL, " label=\"");
	strcat(LABEL, l);
	strcat(LABEL, "\"");
}

void PhyphoxBleExperiment::Element::copyToMem(char **target, const char *data) {
  if (*target != NULL) {
    //free(*target);										//TODO: Change
  }
  *target = (char*) malloc(sizeof(char) * strlen(data));
  strcpy(*target, data);
}