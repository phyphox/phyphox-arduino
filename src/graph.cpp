#include "phyphoxBleExperiment.h"
#include "copyToMem.h"
#include <sstream>

void PhyphoxBleExperiment::Graph::setUnitX(const char *ux){
	ERROR = ERROR.MESSAGE == NULL ? err_checkLength(ux, 5, "setUnitX") : ERROR;
	copyToMem(&UNITX, (" unitX=\"" + std::string(ux) + "\"").c_str());
}

void PhyphoxBleExperiment::Graph::setUnitY(const char *uy){
	ERROR = ERROR.MESSAGE == NULL ? err_checkLength(uy, 5, "setUnitY") : ERROR;
	copyToMem(&UNITY, (" unitY=\"" + std::string(uy) + "\"").c_str());
}

void PhyphoxBleExperiment::Graph::setLabelX(const char *lx){
	ERROR = ERROR.MESSAGE == NULL ? err_checkLength(lx, 20, "setLabelX") : ERROR;
	copyToMem(&LABELX, (" labelX=\"" + std::string(lx) + "\"").c_str());
}

void PhyphoxBleExperiment::Graph::setLabelY(const char *ly){
	ERROR = ERROR.MESSAGE == NULL ? err_checkLength(ly, 20, "setLabelX") : ERROR;
	copyToMem(&LABELY, (" labelY=\"" + std::string(ly) + "\"").c_str());
}

void PhyphoxBleExperiment::Graph::setColor(const char *c){
	ERROR = ERROR.MESSAGE == NULL ? err_checkHex(c, "setColor") : ERROR;
	copyToMem(&COLOR, (" color=\"" + std::string(c) + "\"").c_str());
}

void PhyphoxBleExperiment::Graph::setXPrecision(int px){
	ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(px, 9999, "setXPrecision") : ERROR;
	char tmp[20];
	sprintf(tmp, " xPrecision=\"%i\"", px);
	copyToMem(&XPRECISION, tmp);
}

void PhyphoxBleExperiment::Graph::setYPrecision(int py){
	ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(py, 9999, "setYPrecision") : ERROR;
	char tmp[20];
	sprintf(tmp, " yPrecision=\"%i\"", py);
	copyToMem(&YPRECISION, tmp);
}

void PhyphoxBleExperiment::Graph::setChannel(int x, int y)
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

void PhyphoxBleExperiment::Graph::addChannel(int x, int y, const char* col) {
	for(int i=0;i<phyphoxBleNChannel; i++) {
		if(!CHANNEL[i].isActive){
			char tmp[20];
			sprintf(tmp, "CH%i", x);
			copyToMem(&CHANNEL[i].CHANNELX, tmp);
			sprintf(tmp, "CH%i", y);
			copyToMem(&CHANNEL[i].CHANNELY, tmp);
			copyToMem(&CHANNEL[i].COLOR, col);
			CHANNEL[i].isActive = true;
			return;
		}
	}
}

void PhyphoxBleExperiment::Graph::setStyle(const char *s){
	ERROR = ERROR.MESSAGE == NULL ? err_checkStyle(s, "setStyle") : ERROR;
	copyToMem(&STYLE, (" style=\"" + std::string(s) + "\"").c_str());
}

void PhyphoxBleExperiment::Graph::setMinX(float value, const char * layout) {
	/**
     * Sets the min x value for the co-system.
     *
     * @param value The min x value.
	 * @param layout Choose between auto, extend and fixed.
     */
	std::string layoutString;
	layoutString.assign(layout);
	std::ostringstream valueStringStream;
	valueStringStream << value;
	if(strcmp(layout, LAYOUT_AUTO) == 0 || strcmp(layout, LAYOUT_EXTEND) == 0 || strcmp(layout, LAYOUT_FIXED) == 0) {
		copyToMem(&MINX, (" scaleMinX=\"" + layoutString + "\"" + " minX=\"" + valueStringStream.str() + "\"").c_str());
	} else {
		ERROR = ERROR.MESSAGE == NULL ? err_checkLayout(layout, "setMinX") : ERROR;
	}
}

void PhyphoxBleExperiment::Graph::setMaxX(float value, const char * layout) {
	/**
     * Sets the min x value for the co-system.
     *
     * @param value The max x value.
	 * @param layout Choose between auto, extend and fixed.
     */
	std::string layoutString;
	layoutString.assign(layout);
	std::ostringstream valueStringStream;
	valueStringStream << value;
	if(strcmp(layout, "auto") == 0 || strcmp(layout, "extend") == 0 || strcmp(layout, "fixed") == 0) {
		copyToMem(&MAXX, (" scaleMaxX=\"" + layoutString + "\"" + " maxX=\"" + valueStringStream.str() + "\"").c_str());
	} else {
		ERROR = ERROR.MESSAGE == NULL ? err_checkLayout(layout, "setMaxX") : ERROR;
	}
}

void PhyphoxBleExperiment::Graph::setMinY(float value, const char * layout) {
	/**
     * Sets the min x value for the co-system.
     *
     * @param value The min y value.
	 * @param layout Choose between auto, extend and fixed.
     */
	std::string layoutString;
	layoutString.assign(layout);
	std::ostringstream valueStringStream;
	valueStringStream << value;
	if(strcmp(layout, "auto") == 0 || strcmp(layout, "extend") == 0 || strcmp(layout, "fixed") == 0) {
		copyToMem(&MINY, (" scaleMinY=\"" + layoutString + "\"" + " minY=\"" + valueStringStream.str() + "\"").c_str());
	} else {
		ERROR = ERROR.MESSAGE == NULL ? err_checkLayout(layout, "setMinY") : ERROR;
	}
}
void PhyphoxBleExperiment::Graph::setMaxY(float value, const char * layout) {
	/**
     * Sets the min x value for the co-system.
     *
     * @param value The max y value.
	 * @param layout Choose between auto, extend and fixed.
     */
	std::string layoutString;
	layoutString.assign(layout);
	std::ostringstream valueStringStream;
	valueStringStream << value;
	if(strcmp(layout, "auto") == 0 || strcmp(layout, "extend") == 0 || strcmp(layout, "fixed") == 0) {
		copyToMem(&MAXY, (" scaleMaxY=\"" + layoutString + "\"" + " maxY=\"" + valueStringStream.str() + "\"").c_str());
	} else {
		ERROR = ERROR.MESSAGE == NULL ? err_checkLayout(layout, "setMaxY") : ERROR;
	}
}

void PhyphoxBleExperiment::Graph::setXMLAttribute(const char *xml){
	ERROR = ERROR.MESSAGE == NULL ? err_checkLength(xml, 98, "setXMLAttribute") : ERROR;
	copyToMem(&XMLAttribute, (" " + std::string(xml)).c_str());
}

void PhyphoxBleExperiment::Graph::getBytes(char *buffArray)
{
	strcat(buffArray,"\t\t<graph");
	if (!LABEL)  {strcat(buffArray," label=\"label\"");} else {strcat(buffArray,LABEL);}
	if (!LABELX) {strcat(buffArray, " labelX=\"label x\"");} else {strcat(buffArray,LABELX);}
	if (!LABELY) {strcat(buffArray, " labelY=\"label y\"");} else {strcat(buffArray,LABELY);}
	if (UNITX) {strcat(buffArray,UNITX);}
	if (UNITY) {strcat(buffArray,UNITY);}
	if (XPRECISION) {strcat(buffArray,XPRECISION);}
	if (YPRECISION) {strcat(buffArray,YPRECISION);}
	if (STYLE) {strcat(buffArray,STYLE);}
	if (COLOR) {strcat(buffArray,COLOR);}
	if (MINX) {strcat(buffArray,MINX);}
	if (MAXX) {strcat(buffArray,MAXX);}
	if (MINY) {strcat(buffArray,MINY);}
	if (MAXY) {strcat(buffArray,MAXY);}
	if (XMLAttribute) {strcat(buffArray,XMLAttribute);}

	strcat(buffArray,">\n");

	strcat(buffArray, "\t\t\t<input axis=\"x\">");
	if (!INPUTX) {strcat(buffArray, "CH0");} else {strcat(buffArray, INPUTX);}
	strcat(buffArray, "</input>\n\t\t\t<input axis=\"y\">");
	if (!INPUTY) {strcat(buffArray, "CH1");} else {strcat(buffArray, INPUTY);}

	for(int i=0;i<phyphoxBleNChannel; i++) {
		if(CHANNEL[i].isActive) {
			CHANNEL[i].getBytes(buffArray);
		}
	}

	strcat(buffArray, "</input>\n\t\t</graph>\n");	
	
}