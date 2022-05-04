#include "copyToMem.h"

void copyToMem(char **target, const char *data) {
  if (*target != NULL) {
    //free(*target);										//TODO: Change
  }
  *target = (char*) malloc(sizeof(char) * strlen(data));
  strcpy(*target, data);
}