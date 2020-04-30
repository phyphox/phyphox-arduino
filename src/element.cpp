#include "element.h"

void Element::setLabel(char *l){
	memset(&LABEL[0], 0, sizeof(LABEL));
	strcat(LABEL, l);
}
