#include "graph.h"

void Graph::setUnitX(char *ux){
	memset(&UNITX[0], 0, sizeof(UNITX));
	strcat(UNITX, ux);
}

void Graph::setUnitY(char *uy){
	memset(&UNITY[0], 0, sizeof(UNITY));
	strcat(UNITY, uy);
}

void Graph::setLabelX(char *lx){
	memset(&LABELX[0], 0, sizeof(LABELX));
	strcat(LABELX, lx);
}

void Graph::setLabelY(char *ly){
	memset(&LABELY[0], 0, sizeof(LABELY));
	strcat(LABELY, ly);
}

void Graph::setChannel(int x, int y)
{
	char xChar[1] = "";
	char yChar[1] = "";
	sprintf(xChar, "CH%i", x);
	sprintf(yChar, "CH%i", y);

	strcat(INPUTX, xChar);
	strcat(INPUTY, yChar);

}

void Graph::getBytes(char *buffArray)
{
	


	strcat(buffArray,"\t\t<graph label=\"");
	strcat(buffArray, UNITX);
	strcat(buffArray,"\" aspectRatio=\"2.5\" style=\"lines\" lineWidth=\"1\" color=\"ff7e22\" partialUpdate=\"false\" history=\"1\" labelX=\"");
	strcat(buffArray,LABELX);
	strcat(buffArray,"\" labelY=\"");
	strcat(buffArray,LABELY);
	strcat(buffArray,"\" labelZ=\"\" unitX=\"");
	strcat(buffArray,UNITX);
	strcat(buffArray,"\" unitY=\"");
	strcat(buffArray,UNITY);
	strcat(buffArray,"\" unitZ=\"\" logX=\"false\" logY=\"false\" logZ=\"false\" xPrecision=\"3\" yPrecision=\"3\" zPrecision=\"3\" scaleMinX=\"auto\" scaleMaxX=\"auto\" scaleMinY=\"auto\" scaleMaxY=\"auto\" scaleMinZ=\"auto\" scaleMaxZ=\"auto\" minX=\"0\" maxX=\"0\" minY=\"0\" maxY=\"0\" minZ=\"0\" maxZ=\"0\"  mapWidth=\"0\" >\n");

	strcat(buffArray, "\t\t\t<input axis=\"x\">");
	strcat(buffArray, INPUTX);
	strcat(buffArray, "</input>\n\t\t\t<input axis=\"y\">");
	strcat(buffArray, INPUTY);
	strcat(buffArray, "</input>\n\t\t</graph>\n");

	
}
