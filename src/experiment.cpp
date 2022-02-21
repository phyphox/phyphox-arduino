#include "phyphoxBleExperiment.h"

void PhyphoxBleExperiment::addView(View& v)
{
	for(int i=0; i<phyphoxBleNViews; i++)
	{
		if(VIEWS[i]==nullptr){
			VIEWS[i] = &v;
			break;
		}
	}
}

void PhyphoxBleExperiment::setTitle(const char *t){
	memset(&TITLE[0], 0, sizeof(TITLE));
	strcat(TITLE, t);
}

void PhyphoxBleExperiment::setCategory(const char *c){
	memset(&CATEGORY[0], 0, sizeof(CATEGORY));	
	strcat(CATEGORY, c);
}

void PhyphoxBleExperiment::setDescription(const char *d){
	memset(&DESCRIPTION[0], 0, sizeof(DESCRIPTION));
	strcat(DESCRIPTION, d);
}

void PhyphoxBleExperiment::setConfig(const char *t){
	memset(&CONFIG[0], 0, sizeof(CONFIG));
	strcat(CONFIG, t);
}

void PhyphoxBleExperiment::addExportSet(ExportSet& e)
{
	for(int i=0; i<phyphoxBleNExportSets; i++)
	{
		if(EXPORTSETS[i]==nullptr){
			EXPORTSETS[i] = &e;
			break;
		}
	}
}

void PhyphoxBleExperiment::getFirstBytes(char *buffArray, const char *DEVICENAME){

	int errors = 0;

	//header
	strcat(buffArray, "<phyphox version=\"1.10\">\n");
	//build title
	strcat(buffArray,"<title>");
	strcat(buffArray, TITLE);
	strcat(buffArray,"</title>\n");
	
	//build category
	strcat(buffArray, "<category>");
	strcat(buffArray, CATEGORY);
	strcat(buffArray, "</category>\n");	

	//build description
	strcat(buffArray, "<description>");
	strcat(buffArray, DESCRIPTION);
	strcat(buffArray, "</description>\n");

	//build container
	strcat(buffArray, "<data-containers>\n");
	strcat(buffArray, "\t<container size=\"0\" static=\"false\">CH0</container>\n");
	strcat(buffArray, "\t<container size=\"0\" static=\"false\">CB1</container>\n");
	for(int i=0; i<numberOfChannels;i++){
		strcat(buffArray, "\t<container size=\"0\" static=\"false\">CH");
		char add[20];
		sprintf(add, "%i", i+1);
		strcat(buffArray, add);
		strcat(buffArray, "</container>\n");
	}
	strcat(buffArray, "</data-containers>\n");

	//build input
	strcat(buffArray, "<input>\n");
	strcat(buffArray, "\t<bluetooth name=\"");
	strcat(buffArray, DEVICENAME);
	
	if(MTU!=20){
		char add[0];
		sprintf(add, "\" mtu=\"%i", MTU);
		strcat(buffArray, add);
	}
	strcat(buffArray, "\" id=\"phyphoxBLE\" mode=\"notification\" rate=\"1\" subscribeOnStart=\"false\">\n");
	
	
	

	//build config
	//strcat(buffArray,"\t\t<config char=\"cddf1003-30f7-4671-8b43-5e40ba53514a\" conversion=\"hexadecimal\">");
	//strcat(buffArray, CONFIG);
    //strcat(buffArray,"</config>\n\t\t");


	if(repeating <= 0){
		for(int i=1; i<=numberOfChannels;i++){
			strcat(buffArray, "<output char=\"cddf1002-30f7-4671-8b43-5e40ba53514a\" conversion=\"float32LittleEndian\" ");
			char add[20];
			int k = (i-1)*4;
			sprintf(add, "offset=\"%i\" >CH%i", k,i);
			strcat(buffArray, add);
			strcat(buffArray,"</output>\n\t\t");
		}
	}else{
		for(int i=1; i<=numberOfChannels;i++){
			strcat(buffArray, "<output char=\"cddf1002-30f7-4671-8b43-5e40ba53514a\" conversion=\"float32LittleEndian\" ");
			char add[40];
			int k = (i-1)*4;
			sprintf(add, "offset=\"%i\" repeating=\"%i\" >CH%i", k,repeating,i);
			strcat(buffArray, add);
			strcat(buffArray,"</output>\n\t\t");
		}
	}
	
	strcat(buffArray,"<output char=\"cddf1002-30f7-4671-8b43-5e40ba53514a\" extra=\"time\">CH0</output>");

	strcat(buffArray, "\n\t</bluetooth>\n");
	strcat(buffArray, "</input>\n");

	//build output
	strcat(buffArray, "<output>\n");
	strcat(buffArray, "\t<bluetooth id=\"phyphoxBLE\" name=\"");
	strcat(buffArray, DEVICENAME);
	strcat(buffArray, "\">\n");

	// for(int i=1; i<=1;i++){
	// 	strcat(buffArray, "<input char=\"cddf1002-30f7-4671-8b43-5e40ba53514a\" conversion=\"float32LittleEndian\" ");
	// 	char add[20];
	// 	int k = (i-1)*4;
	// 	sprintf(add, "offset=\"%i\" >CB%i", k,i);
	// 	strcat(buffArray, add);
	// 	strcat(buffArray,"</input>\n\t\t");
	// }
	strcat(buffArray, "\t\t<input char=\"cddf1003-30f7-4671-8b43-5e40ba53514a\" conversion=\"float32LittleEndian\">CB1</input>\n");

	strcat(buffArray, "\t</bluetooth>\n");
	strcat(buffArray, "</output>\n");

	//build analysis
	strcat(buffArray, "<analysis sleep=\"0.1\"  onUserInput=\"false\"></analysis>\n");

	//build views
	strcat(buffArray, "<views>\n");

	//errorhandling
	for(int i=0;i<phyphoxBleNViews;i++) {
		for(int j=0;j<phyphoxBleNElements;j++) {
			if(VIEWS[i]!= nullptr && errors<=2){
				if(VIEWS[i]->ELEMENTS[j]!=nullptr){
					if(strcmp(VIEWS[i]->ELEMENTS[j]->ERROR.MESSAGE, "") != 0) {
						if(errors == 0) {
							strcat(buffArray, "\t<view label=\"ERRORS\"> \n");
						}
						VIEWS[i]->ELEMENTS[j]->ERROR.getBytes(buffArray);
						errors++;
					}
				}
			}
		}
	}
	if(errors>0) {
		strcat(buffArray,"\t\t<info  label=\"DE: Siehe Dokumentation für mehr Informationen zu Fehlern.\">\n");
		//strcat(buffArray,"\" color=\"ff0000\">\n");
		strcat(buffArray,"\t\t</info>\n");
		strcat(buffArray,"\t\t<info  label=\"EN: Please check the documentation for more information about errors.\">\n");
		//strcat(buffArray,"\" color=\"ff0000\">\n");
		strcat(buffArray,"\t\t</info>\n");
		strcat(buffArray,"\t</view>\n");
	}
}

void PhyphoxBleExperiment::getViewBytes(char *buffArray, uint8_t view, uint8_t element){

	if(VIEWS[view]!=nullptr && view<phyphoxBleNViews){
		VIEWS[view]->getBytes(buffArray,element);
	}
	
}

void PhyphoxBleExperiment::getLastBytes(char *buffArray){
	bool noExports = true;

	strcat(buffArray,"</views>\n");

	//build export
	strcat(buffArray, "<export>\n");
	for(int i=0;i<phyphoxBleNExportSets; i++){
		if(EXPORTSETS[i]!=nullptr){
			EXPORTSETS[i]->getBytes(buffArray);
			noExports = false;
		}
	}
	if(noExports) {
		strcat(buffArray,"\t<set name=\"mySet\">\n");

		for(int i=0;i<numberOfChannels+1;i++){
			strcat(buffArray,"\t\t<data name=\"myData");	
			char add[20];
			sprintf(add, "%i\">CH%i", i,i);
			strcat(buffArray, add);
			strcat(buffArray,"</data>\n");	
		}
		strcat(buffArray,"\t</set>\n");
	}
	strcat(buffArray, "</export>\n");
	
	//close
	strcat(buffArray, "</phyphox>");
}



