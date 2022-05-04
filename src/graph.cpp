#include "phyphoxBleExperiment.h"
#include "copyToMem.h"
#include <sstream>

void PhyphoxBleExperiment::Graph::setUnitX(const char *ux){
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(ux, 5, "setUnitX") : ERROR;
	copyToMem(&UNITX, (" unitX=\"" + std::string(ux) + "\"").c_str());
}

void PhyphoxBleExperiment::Graph::setUnitY(const char *uy){
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(uy, 5, "setUnitY") : ERROR;
	copyToMem(&UNITY, (" unitY=\"" + std::string(uy) + "\"").c_str());
}

void PhyphoxBleExperiment::Graph::setLabelX(const char *lx){
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(lx, 20, "setLabelX") : ERROR;
	copyToMem(&LABELX, (" labelX=\"" + std::string(lx) + "\"").c_str());
}

void PhyphoxBleExperiment::Graph::setLabelY(const char *ly){
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(ly, 20, "setLabelX") : ERROR;
	copyToMem(&LABELY, (" labelY=\"" + std::string(ly) + "\"").c_str());
}

void PhyphoxBleExperiment::Graph::setColor(const char *c){
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkHex(c, "setColor") : ERROR;
	copyToMem(&COLOR, (" color=\"" + std::string(c) + "\"").c_str());
}

void PhyphoxBleExperiment::Graph::setXPrecision(int px){
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkUpper(px, 9999, "setXPrecision") : ERROR;
	char tmp[20];
	sprintf(tmp, " xPrecision=\"%i\"", px);
	copyToMem(&XPRECISION, tmp);
	//sprintf(XPRECISION, " xPrecision=\"%i\"", px);
}

void PhyphoxBleExperiment::Graph::setYPrecision(int py){
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkUpper(py, 9999, "setYPrecision") : ERROR;
	char tmp[20];
	sprintf(tmp, " yPrecision=\"%i\"", py);
	copyToMem(&YPRECISION, tmp);
}

void PhyphoxBleExperiment::Graph::setChannel(int x, int y)
{
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkUpper(x, numberOfChannels, "setChannel") : ERROR;
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkUpper(y, numberOfChannels, "setChannel") : ERROR;
	char tmpX[20];
	sprintf(tmpX, "CH%i", x);
	copyToMem(&INPUTX, tmpX);
	char tmpY[20];
	sprintf(tmpY, "CH%i", y);
	copyToMem(&INPUTY, tmpY);
}

void PhyphoxBleExperiment::Graph::setStyle(const char *s){
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkStyle(s, "setStyle") : ERROR;
	copyToMem(&STYLE, (" style=\"" + std::string(s) + "\"").c_str());
}

void PhyphoxBleExperiment::Graph::setXMLAttribute(const char *xml){
	ERROR = (strcmp(ERROR.MESSAGE, "")==0) ? err_checkLength(xml, 98, "setXMLAttribute") : ERROR;
	copyToMem(&XMLAttribute, (" " + std::string(xml)).c_str());
}

void PhyphoxBleExperiment::Graph::getBytes(char *buffArray)
{
	strcat(buffArray,"\t\t<graph");
	strcat(buffArray,LABEL);
	strcat(buffArray,LABELX);
	strcat(buffArray,LABELY);
	//strcat(buffArray," labelZ=\"\"");
	strcat(buffArray,UNITX);
	strcat(buffArray,UNITY);
	strcat(buffArray,XPRECISION);
	strcat(buffArray,YPRECISION);
	strcat(buffArray,STYLE);
	strcat(buffArray,COLOR);
	strcat(buffArray,XMLAttribute);
	strcat(buffArray,">\n");

	strcat(buffArray, "\t\t\t<input axis=\"x\">");
	strcat(buffArray, INPUTX);
	strcat(buffArray, "</input>\n\t\t\t<input axis=\"y\">");
	strcat(buffArray, INPUTY);
	strcat(buffArray, "</input>\n\t\t</graph>\n");	
}