#include "phyphoxBleExperiment.h"

PhyphoxBleExperiment::Error PhyphoxBleExperiment::Errorhandler::err_checkLength(const char *input, int maxLength, const char *origin) {
    Error ret;
    if(strlen(input) > maxLength) {
        strcat(ret.MESSAGE, "ERR_01, in ");
        strcat(ret.MESSAGE, origin);
        strcat(ret.MESSAGE, "(). \n");
    }
    return ret;
}

PhyphoxBleExperiment::Error PhyphoxBleExperiment::Errorhandler::err_checkUpper(int input, int upperValue, const char *origin) {
    Error ret;
    if(input > upperValue) {
        strcat(ret.MESSAGE, "ERR_02, in ");
        strcat(ret.MESSAGE, origin);
        strcat(ret.MESSAGE, "(). \n");
    }
    return ret;
}

PhyphoxBleExperiment::Error PhyphoxBleExperiment::Errorhandler::err_checkHex(const char* input, const char *origin){
    Error ret;
    if(strlen(input) != 6) {
        strcat(ret.MESSAGE, "ERR_03, in ");
        strcat(ret.MESSAGE, origin);
        strcat(ret.MESSAGE, "(). \n");
    };
    for(int i=0; i<=5; i++) {
		if(!((input[i] <='f' && input[i] >='a') || (input[i] <='F' && input[i] >='A') || (input[i] <='9' && input[i] >='0'))) {
			strcat(ret.MESSAGE, "ERR_03, in ");
            strcat(ret.MESSAGE, origin);
            strcat(ret.MESSAGE, "(). \n");
		}
	}
    return ret;
}

PhyphoxBleExperiment::Error PhyphoxBleExperiment::Errorhandler::err_checkStyle(const char *input, const char *origin) {
    Error ret;
    if(strcmp(input,"lines")!=0 && strcmp(input,"dots")!=0 && strcmp(input,"vbars")!=0 && strcmp(input,"hbars")!=0 && strcmp(input,"map")!=0) {
        strcat(ret.MESSAGE, "ERR_04, in ");
        strcat(ret.MESSAGE, origin);
        strcat(ret.MESSAGE, "(). \n");
    }
    return ret;
}