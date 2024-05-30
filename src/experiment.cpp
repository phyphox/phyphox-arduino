#include "phyphoxBleExperiment.h"
#include "copyToMem.h"
#include "phyphoxBle.h"

int PhyphoxBleExperiment::numberOfChannels = 5;


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

void PhyphoxBleExperiment::addSensor(Sensor& s){
	for(int i=0; i<phyphoxBleNSensors; i++)
	{
		if(SENSORS[i]==nullptr){
			SENSORS[i] = &s;

			//add view to show raw data in last view
			if(VIEWS[phyphoxBleNViews-1]==nullptr){
				View* RawDataSmartoneSensor = new View();
				RawDataSmartoneSensor->setLabel("SENSOR RAW DATA");
				VIEWS[phyphoxBleNViews-1] = RawDataSmartoneSensor;
				InfoField* SensorType = new InfoField();
				SensorType->INFO = s.TYPE;
				for (int j = 0; j < phyphoxBleNElements; j++){
					if(VIEWS[phyphoxBleNViews-1]->ELEMENTS[j]==nullptr){
						VIEWS[phyphoxBleNViews-1]->ELEMENTS[j] = SensorType;
						break;
					}
				}

			}
			
			for(int mappedCH = 0; mappedCH<5; mappedCH++){
				if( s.CHANNEL[mappedCH]!=nullptr){
					for (int j = 0; j < phyphoxBleNElements; j++){
						if(VIEWS[phyphoxBleNViews-1]->ELEMENTS[j]==nullptr){
							PhyphoxBleExperiment::Value* newValueField = new Value();
							newValueField->INPUTVALUE = s.CHANNEL[mappedCH];
							newValueField->LABEL = s.COMPONENT[mappedCH];
							VIEWS[phyphoxBleNViews-1]->ELEMENTS[j]=newValueField;
							break;
						}
					}
				}
			}

			break;
		}
	}
}

void PhyphoxBleExperiment::setTitle(const char *t){
	copyToMem(&TITLE, t);
}

void PhyphoxBleExperiment::setCategory(const char *c){
	copyToMem(&CATEGORY, c);
}

void PhyphoxBleExperiment::setDescription(const char *d){
	copyToMem(&DESCRIPTION, d);
}

void PhyphoxBleExperiment::setColor(const char *c){
	copyToMem(&COLOR, (std::string(c)).c_str());
}

void PhyphoxBleExperiment::setRepeating(const int r){
	PhyphoxBleExperiment::repeating = r;
}
void PhyphoxBleExperiment::setSubscribeOnStart(bool b) {
	if(b) copyToMem(&SUBSCRIBEONSTART, "true");
    else copyToMem(&SUBSCRIBEONSTART, "false");
}

// void PhyphoxBleExperiment::setConfig(const char *c){
// 	copyToMem(&CONFIG, c);
// }

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
	strcat(buffArray, "<phyphox version=\"1.15\">\n");
	//build title
	strcat(buffArray,"<title>");
	if (!TITLE)  {strcat(buffArray,"Arduino-Experiment");} else {strcat(buffArray,TITLE);}
	strcat(buffArray,"</title>\n");
	
	//build category
	strcat(buffArray, "<category>");
	if (!CATEGORY)  {strcat(buffArray,"Arduino Experiments");} else {strcat(buffArray,CATEGORY);}
	strcat(buffArray, "</category>\n");	

	//build color
	/*
	if (!COLOR){
		strcat(buffArray, "<color>");
		strcat(buffArray,COLOR);
		strcat(buffArray, "</color>\n");
	}
	*/

	//build description
	strcat(buffArray, "<description>");
	if (!DESCRIPTION)  {
		strcat(buffArray,"An experiment created with the phyphox BLE library for Arduino-compatible micro controllers.");
	}else {
		strcat(buffArray,DESCRIPTION);
	}
	strcat(buffArray, "</description>\n");

	//build container
	strcat(buffArray, "<data-containers>\n");
	strcat(buffArray, "\t<container size=\"0\" static=\"false\">CH0</container>\n");
	strcat(buffArray, "\t<container size=\"0\" static=\"false\">CB1</container>\n");
	strcat(buffArray, "\t<container size=\"0\" static=\"false\">CB2</container>\n");
	strcat(buffArray, "\t<container size=\"0\" static=\"false\">CB3</container>\n");
	strcat(buffArray, "\t<container size=\"0\" static=\"false\">CB4</container>\n");
	strcat(buffArray, "\t<container size=\"0\" static=\"false\">CB5</container>\n");

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
		char add[10];
		sprintf(add, "\" mtu=\"%i", MTU);
		strcat(buffArray, add);
	}
	strcat(buffArray, "\" id=\"phyphoxBLE\" mode=\"notification\" subscribeOnStart=\"");
	if (!SUBSCRIBEONSTART)  {strcat(buffArray, "false");} else {strcat(buffArray, SUBSCRIBEONSTART);}
	strcat(buffArray, "\">\n\t\t");
	
	
	

	//build config
	//strcat(buffArray,"\t\t<config char=\"cddf1003-30f7-4671-8b43-5e40ba53514a\" conversion=\"hexadecimal\">");
	// if (!CONFIG)  {strcat(buffArray,"000000");} else {strcat(buffArray,CONFIG);}
    //strcat(buffArray,"</config>\n\t\t");


	if(repeating <= 0){
		for(int i=1; i<=numberOfChannels;i++){
			strcat(buffArray, "\t\t<output char=\"cddf1002-30f7-4671-8b43-5e40ba53514a\" conversion=\"float32LittleEndian\" ");
			char add[20];
			int k = (i-1)*4;
			sprintf(add, "offset=\"%i\">CH%i", k,i);
			strcat(buffArray, add);
			strcat(buffArray,"</output>\n");
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
	//build sensor input
	
	for (int i = 0; i < phyphoxBleNSensors; i++)
	{
		if(SENSORS[i] != nullptr){
			//strcat(buffArray, "\n\t\t<sensor rate=\"0\" average=\"false\" type=\"accelerometer\">CB1</sensor>");
			SENSORS[i]->getBytes(buffArray);
		}
	}
	strcat(buffArray, "</input>\n");
	

	//build output
	strcat(buffArray, "<output>\n");
	strcat(buffArray, "\t<bluetooth id=\"phyphoxBLE\" name=\"");
	strcat(buffArray, DEVICENAME);
	strcat(buffArray, "\">\n\t\t");

	// for(int i=1; i<=1;i++){
	// 	strcat(buffArray, "<input char=\"cddf1002-30f7-4671-8b43-5e40ba53514a\" conversion=\"float32LittleEndian\" ");
	// 	char add[20];
	// 	int k = (i-1)*4;
	// 	sprintf(add, "offset=\"%i\" >CB%i", k,i);
	// 	strcat(buffArray, add);
	// 	strcat(buffArray,"</input>\n\t\t");
	// }
	strcat(buffArray, "\t\t<input char=\"cddf1003-30f7-4671-8b43-5e40ba53514a\" conversion=\"float32LittleEndian\">CB1</input>\n");
	strcat(buffArray, "\t\t<input char=\"cddf1003-30f7-4671-8b43-5e40ba53514a\" conversion=\"float32LittleEndian\" offset=\"4\">CB2</input>\n");
	strcat(buffArray, "\t\t<input char=\"cddf1003-30f7-4671-8b43-5e40ba53514a\" conversion=\"float32LittleEndian\" offset=\"8\">CB3</input>\n");
	strcat(buffArray, "\t\t<input char=\"cddf1003-30f7-4671-8b43-5e40ba53514a\" conversion=\"float32LittleEndian\" offset=\"12\">CB4</input>\n");
	strcat(buffArray, "\t\t<input char=\"cddf1003-30f7-4671-8b43-5e40ba53514a\" conversion=\"float32LittleEndian\" offset=\"16\">CB5</input>\n");

	strcat(buffArray, "\t</bluetooth>\n");
	strcat(buffArray, "</output>\n");

	//build analysis
	strcat(buffArray, "<analysis sleep=\"0\"  onUserInput=\"false\"></analysis>\n");

	//build views
	strcat(buffArray, "<views>\n");

	//errorhandling
	for(int i=0;i<phyphoxBleNViews;i++) {
		for(int j=0;j<phyphoxBleNElements;j++) {
			if(VIEWS[i]!= nullptr && errors<=2){
				if(VIEWS[i]->ELEMENTS[j]!=nullptr){
					if(VIEWS[i]->ELEMENTS[j]->ERROR.MESSAGE != NULL) {
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
	
	for(int i=0; i<phyphoxBleNSensors;i++){
		if(SENSORS[i]!=nullptr && SENSORS[i]->ERROR.MESSAGE !=NULL){
			if(errors == 0){
				strcat(buffArray, "\t<view label=\"ERRORS\"> \n");
			}
			SENSORS[i]->ERROR.getBytes(buffArray);
			errors++;
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