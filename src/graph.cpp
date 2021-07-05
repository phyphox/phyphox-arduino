#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::Graph::setUnitX(const char *ux){
	memset(&UNITX[0], 0, sizeof(UNITX));
	strcat(UNITX, ux);
}

void PhyphoxBleExperiment::Graph::setUnitY(const char *uy){
	memset(&UNITY[0], 0, sizeof(UNITY));
	strcat(UNITY, uy);
}

void PhyphoxBleExperiment::Graph::setLabelX(const char *lx){
	memset(&LABELX[0], 0, sizeof(LABELX));
	strcat(LABELX, lx);
}

void PhyphoxBleExperiment::Graph::setLabelY(const char *ly){
	memset(&LABELY[0], 0, sizeof(LABELY));
	strcat(LABELY, ly);
}

void PhyphoxBleExperiment::Graph::setColor(const char *c){
	sprintf(COLOR, c);
}

void PhyphoxBleExperiment::Graph::setXPrecision(int px){
	sprintf(XPRECISION, "%d", px);
}

void PhyphoxBleExperiment::Graph::setYPrecision(int py){
	sprintf(YPRECISION, "%d", py);
}

void PhyphoxBleExperiment::Graph::setChannel(int x, int y)
{
	sprintf(INPUTX, "CH%i", x);
	sprintf(INPUTY, "CH%i", y);
}

void PhyphoxBleExperiment::Graph::setStyle(const char *s){
	memset(&STYLE[0], 0, sizeof(STYLE));
	strcat(STYLE, s);
}

void PhyphoxBleExperiment::Graph::setWild(const char *w){
	memset(&WILD[0], 0, sizeof(WILD));
	strcat(WILD, w);
}

void PhyphoxBleExperiment::Graph::getBytes(char *buffArray)
{
	


	strcat(buffArray,"\t\t<graph label=\"");
	strcat(buffArray, LABEL);
	strcat(buffArray,"\" labelX=\"");
	strcat(buffArray,LABELX);
	strcat(buffArray,"\" labelY=\"");
	strcat(buffArray,LABELY);
	strcat(buffArray,"\" labelZ=\"\" unitX=\"");
	strcat(buffArray,UNITX);
	strcat(buffArray,"\" unitY=\"");
	strcat(buffArray,UNITY);
	strcat(buffArray,"\" xPrecision=\"");
	strcat(buffArray,XPRECISION);
	strcat(buffArray,"\" yPrecision=\"");
	strcat(buffArray,YPRECISION);
	strcat(buffArray,"\" style=\"");
	strcat(buffArray,STYLE);
	strcat(buffArray,"\" color=\"");
	strcat(buffArray,COLOR);
	strcat(buffArray,"\" ");
	strcat(buffArray, WILD);
	strcat(buffArray,">\n");

	strcat(buffArray, "\t\t\t<input axis=\"x\">");
	strcat(buffArray, INPUTX);
	strcat(buffArray, "</input>\n\t\t\t<input axis=\"y\">");
	strcat(buffArray, INPUTY);
	strcat(buffArray, "</input>\n\t\t</graph>\n");

	
}
