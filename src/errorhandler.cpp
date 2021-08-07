#include "phyphoxBleExperiment.h"

const char* PhyphoxBleExperiment::Errorhandler::err_checkLength(const char *input, int maxLength) {
    if(strlen(input) > 20) return "ERR_01";
    return input;
}

int PhyphoxBleExperiment::Errorhandler::err_checkUpper(int input, int upperValue) {
    if(input > upperValue) return 0;
    return input;
}

const char* PhyphoxBleExperiment::Errorhandler::err_checkHex(const char* input){
    if(strlen(input) != 6) return "ff0000";
    for(int i=0; i<=5; i++) {
		if(!((input[i] <='f' && input[i] >='a') || (input[i] <='F' && input[i] >='A') || (input[i] <='9' && input[i] >='0'))) {
			return "ff0000";
		}
	}
    return input;
}

const char* PhyphoxBleExperiment::Errorhandler::err_checkStyle(const char *input) {
    if(strcmp(input,"lines")!=0 && strcmp(input,"dots")!=0 && strcmp(input,"vbars")!=0 && strcmp(input,"hbars")!=0 && strcmp(input,"map")!=0) return "lines";
    return input;
}