#include "phyphoxBleExperiment.h"
#include "copyToMem.h"

void PhyphoxBleExperiment::Graph::Subgraph::getBytes(char *buffArray)
{
    //strcat(buffArray, "</input>\n");
    strcat(buffArray, "\n\t\t\t<input axis=\"x\" ");
    if(COLOR){
        strcat(buffArray," color=\"");
        strcat(buffArray,COLOR);
        strcat(buffArray,"\"");
        }
    if(STYLE){
        strcat(buffArray," style=\"");
        strcat(buffArray,STYLE);
        strcat(buffArray,"\"");
        }
    if(WIDTH){
        strcat(buffArray," linewidth=\"");
        strcat(buffArray,WIDTH);
        strcat(buffArray,"\"");
        }
    strcat(buffArray, ">");
    if (!INPUTX)  {strcat(buffArray,"CH0");} else {strcat(buffArray,INPUTX);}
    strcat(buffArray, "</input>\n\t\t\t<input axis=\"y\" >");
    if (!INPUTY)  {strcat(buffArray,"CH1");} else {strcat(buffArray,INPUTY);}
    strcat(buffArray, "</input>");
}

void PhyphoxBleExperiment::Graph::Subgraph::setColor(const char *c){
	ERROR = ERROR.MESSAGE == NULL ? err_checkHex(c, "setColor") : ERROR;
	copyToMem(&COLOR, (std::string(c)).c_str());
}

void PhyphoxBleExperiment::Graph::Subgraph::setLinewidth(float w){
	ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(w, 10, "setLinewidth") : ERROR;
	char tmp[10];
	sprintf(tmp, "%.2f", w);
	copyToMem(&WIDTH, tmp);
}

void PhyphoxBleExperiment::Graph::Subgraph::setStyle(const char *s){
	ERROR = ERROR.MESSAGE == NULL ? err_checkStyle(s, "setStyle") : ERROR;
	copyToMem(&STYLE, ("" + std::string(s)).c_str());
}

void PhyphoxBleExperiment::Graph::Subgraph::setChannel(int x, int y)
{
	ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(x, numberOfChannels, "setChannel") : ERROR;
	ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(y, numberOfChannels, "setChannel") : ERROR;

	char tmpX[20];
	sprintf(tmpX, "CH%i", x);
	copyToMem(&INPUTX, tmpX);
	char tmpY[20];
	sprintf(tmpY, "CH%i", y);
	copyToMem(&INPUTY, tmpY);
}