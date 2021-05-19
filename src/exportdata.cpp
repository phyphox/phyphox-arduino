#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::ExportData::setName(const char *n){
	memset(&NAME[0], 0, sizeof(NAME));
	strcat(NAME, n);
}

void PhyphoxBleExperiment::ExportData::setDatachannel(int d){
	const char *b;
	switch (d) {
	case 1:
		b = "CH1"; break;
	case 2:
		b = "CH2"; break;
	case 3:
		b = "CH3"; break;
	case 4:
		b = "CH4"; break;
	case 5:
		b = "CH5"; break;
	default:
		b = "CH1"; break;
	}
	memset(&BUFFER[0], 0, sizeof(BUFFER));
	strcat(BUFFER, b);
}

void PhyphoxBleExperiment::ExportData::getBytes(char *buffArray)
{
	
	strcat(buffArray,"\t\t<data name=\"");
    strcat(buffArray, NAME);
    strcat(buffArray, "\">");
    strcat(buffArray, BUFFER);
    strcat(buffArray, "</data>\n");
	
}