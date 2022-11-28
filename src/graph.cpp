#include "phyphoxBleExperiment.h"
#include "copyToMem.h"
#include <sstream>

void PhyphoxBleExperiment::Graph::setUnitX(const char *ux){
	ERROR = ERROR.MESSAGE == NULL ? err_checkLength(ux, 5, "setUnitX") : ERROR;
	copyToMem(&UNITX, (std::string(ux)).c_str());
}

void PhyphoxBleExperiment::Graph::setUnitY(const char *uy){
	ERROR = ERROR.MESSAGE == NULL ? err_checkLength(uy, 5, "setUnitY") : ERROR;
	copyToMem(&UNITY, (std::string(uy)).c_str());
}

void PhyphoxBleExperiment::Graph::setLabelX(const char *lx){
	ERROR = ERROR.MESSAGE == NULL ? err_checkLength(lx, 20, "setLabelX") : ERROR;
	copyToMem(&LABELX, (std::string(lx)).c_str());
}

void PhyphoxBleExperiment::Graph::setLabelY(const char *ly){
	ERROR = ERROR.MESSAGE == NULL ? err_checkLength(ly, 20, "setLabelY") : ERROR;
	copyToMem(&LABELY, (std::string(ly)).c_str());
}


void PhyphoxBleExperiment::Graph::setColor(const char *c){
	FIRSTSUBGRAPH.setColor(c);
}

void PhyphoxBleExperiment::Graph::setLinewidth(float w){
	FIRSTSUBGRAPH.setLinewidth(w);	
}

void PhyphoxBleExperiment::Graph::setXPrecision(int px){
	ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(px, 9999, "setXPrecision") : ERROR;
	char tmp[20];
	sprintf(tmp, "%i", px);
	copyToMem(&XPRECISION, tmp);
}

void PhyphoxBleExperiment::Graph::setYPrecision(int py){
	ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(py, 9999, "setYPrecision") : ERROR;
	char tmp[20];
	sprintf(tmp, "%i", py);
	copyToMem(&YPRECISION, tmp);
}

void PhyphoxBleExperiment::Graph::setTimeOnX(bool b){
	char tmp[10];
	if(b){
		sprintf(tmp, "true");
	}else{
		sprintf(tmp, "false");
	}
	copyToMem(&TIMEONX, tmp);
}

void PhyphoxBleExperiment::Graph::setTimeOnY(bool b){
	char tmp[10];
	if(b){
		sprintf(tmp, "true");
	}else{
		sprintf(tmp, "false");
	}
	copyToMem(&TIMEONY, tmp);
}

void PhyphoxBleExperiment::Graph::setSystemTime(bool b){
	char tmp[10];
	if(b){
		sprintf(tmp, "true");
	}else{
		sprintf(tmp, "false");
	}
	copyToMem(&SYSTEMTIME, tmp);
}

void PhyphoxBleExperiment::Graph::setChannel(int x, int y)
{
	ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(x, numberOfChannels, "setChannel") : ERROR;
	ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(y, numberOfChannels, "setChannel") : ERROR;

	char tmpX[20];
	sprintf(tmpX, "CH%i", x);
	copyToMem(&FIRSTSUBGRAPH.INPUTX, tmpX);
	char tmpY[20];
	sprintf(tmpY, "CH%i", y);
	copyToMem(&FIRSTSUBGRAPH.INPUTY, tmpY);
	SUBGRAPHS[0]=&FIRSTSUBGRAPH;
}
void PhyphoxBleExperiment::Graph::addSubgraph(Subgraph& sg){
	for (int i = 0; i < phyphoxBleNChannel; i++){
		if(SUBGRAPHS[i]==nullptr){
			SUBGRAPHS[i] = &sg;
			if(ERROR.MESSAGE==NULL){
				ERROR = sg.ERROR;
			}
			break;
		}			
	}
}

void PhyphoxBleExperiment::Graph::setStyle(const char *s){
		FIRSTSUBGRAPH.setStyle(s);
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
	if (LABEL)  {
		strcat(buffArray," label=\"");
		strcat(buffArray,LABEL);
		strcat(buffArray,"\"");
	}else{
		strcat(buffArray," label=\"myLabel\"");
	}
	if (LABELX){
		strcat(buffArray, " labelX=\"");
		strcat(buffArray, LABELX);
		strcat(buffArray, "\"");
	} else {
		strcat(buffArray," labelX=\"label x\"");
	}
	if (LABELY){
		strcat(buffArray, " labelY=\"");
		strcat(buffArray, LABELY);
		strcat(buffArray, "\"");
	} else {
		strcat(buffArray," labelY=\"label y\"");
	}
	
	if(UNITX){
		strcat(buffArray, " unitX=\"");
		strcat(buffArray, UNITX);
		strcat(buffArray, "\"");
	}
	if(UNITY){
		strcat(buffArray, " unitY=\"");
		strcat(buffArray, UNITY);
		strcat(buffArray, "\"");
	}
	if (XPRECISION){
		strcat(buffArray," xPrecision=\"");
		strcat(buffArray,XPRECISION);
		strcat(buffArray,"\"");
	}
	if (YPRECISION){
		strcat(buffArray," yPrecision=\"");
		strcat(buffArray,YPRECISION);
		strcat(buffArray,"\"");
	}
	if (MINX) {strcat(buffArray,MINX);}
	if (MAXX) {strcat(buffArray,MAXX);}
	if (MINY) {strcat(buffArray,MINY);}
	if (MAXY) {strcat(buffArray,MAXY);}
	if (XMLAttribute) {strcat(buffArray,XMLAttribute);}

	if(TIMEONX){
		strcat(buffArray," timeOnX=\"");
		strcat(buffArray,TIMEONX);
		strcat(buffArray,"\"");
	}
	if(TIMEONY){
		strcat(buffArray," timeOnY=\"");
		strcat(buffArray,TIMEONY);
		strcat(buffArray,"\"");
	}
	if(SYSTEMTIME){
		strcat(buffArray," systemTime=\"");
		strcat(buffArray,SYSTEMTIME);
		strcat(buffArray,"\"");
	}	

	strcat(buffArray,">");

	for(int i=0;i<phyphoxBleNChannel; i++) {
		if(SUBGRAPHS[i]!=nullptr) {
			SUBGRAPHS[i]->getBytes(buffArray);
		}
	}

	strcat(buffArray, "\n\t\t</graph>\n");	
	
}