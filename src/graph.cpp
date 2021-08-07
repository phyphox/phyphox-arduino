#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::Graph::setUnitX(const char *ux){
	ux = err_checkLength(ux, 20);
	memset(&UNITX[0], 0, sizeof(UNITX));
	strcat(UNITX, " unitX=\"");
	strcat(UNITX, ux);
	strcat(UNITX, "\"");
}

void PhyphoxBleExperiment::Graph::setUnitY(const char *uy){
	uy = err_checkLength(uy, 20);
	memset(&UNITY[0], 0, sizeof(UNITY));
	strcat(UNITY, " unitY=\"");
	strcat(UNITY, uy);
	strcat(UNITY, "\"");
}

void PhyphoxBleExperiment::Graph::setLabelX(const char *lx){
	lx = err_checkLength(lx, 39);
	memset(&LABELX[0], 0, sizeof(LABELX));
	strcat(LABELX, " labelX=\"");
	strcat(LABELX, lx);
	strcat(LABELX, "\"");
}

void PhyphoxBleExperiment::Graph::setLabelY(const char *ly){
	ly = err_checkLength(ly, 39);
	memset(&LABELY[0], 0, sizeof(LABELY));
	strcat(LABELY, " labelY=\"");
	strcat(LABELY, ly);
	strcat(LABELY, "\"");
}

void PhyphoxBleExperiment::Graph::setColor(const char *c){
	c = err_checkHex(c);
	memset(&COLOR[0], 0, sizeof(COLOR));
	strcat(COLOR, " color=\"");
	strcat(COLOR, c);
	strcat(COLOR, "\"");
}

void PhyphoxBleExperiment::Graph::setXPrecision(int px){
	px = err_checkUpper(px, 9999);
	sprintf(XPRECISION, " xPrecision=\"%i\"", px);
}

void PhyphoxBleExperiment::Graph::setYPrecision(int py){
	py = err_checkUpper(py, 9999);
	sprintf(YPRECISION, " yPrecision=\"%i\"", py);
}

void PhyphoxBleExperiment::Graph::setChannel(int x, int y)
{
	x = err_checkUpper(x, 5);
	y = err_checkUpper(y, 5);
	sprintf(INPUTX, "CH%i", x);
	sprintf(INPUTY, "CH%i", y);
}

void PhyphoxBleExperiment::Graph::setStyle(const char *s){
	s = err_checkStyle(s);
	memset(&STYLE[0], 0, sizeof(STYLE));
	strcat(STYLE, " style=\"");
	strcat(STYLE, s);
	strcat(STYLE, "\"");
}

void PhyphoxBleExperiment::Graph::setXMLAttribute(const char *xml){
	xml = err_checkLength(xml, 98);
	memset(&XMLAttribute[0], 0, sizeof(XMLAttribute));
	strcat(XMLAttribute, " ");
	strcat(XMLAttribute, xml);
}

void PhyphoxBleExperiment::Graph::getBytes(char *buffArray)
{
	


	strcat(buffArray,"\t\t<graph");
	strcat(buffArray,LABEL);
	strcat(buffArray,LABELX);
	strcat(buffArray,LABELY);
	strcat(buffArray,"labelZ=\"\"");
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
