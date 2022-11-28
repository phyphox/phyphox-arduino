#include "copyToMem.h"

void copyToMem(char **target, const char *data) {
  // if (*target != NULL) {
  //    free(*target);										// Causes crashes
  // }
  *target = (char*) malloc(sizeof(char) * (strlen(data)+1));
  strcpy(*target, data);
}