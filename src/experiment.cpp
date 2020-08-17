#include "experiment.h"

void Experiment::addView(View& v)
{
	for(int i=0; i<nViews; i++)
	{
		if(VIEWS[i]==nullptr){
			VIEWS[i] = &v;
			break;
		}
	}
}

void Experiment::setTitle(char *t){
	memset(&TITLE[0], 0, sizeof(TITLE));
	strcat(TITLE, t);
}

void Experiment::setCategory(char *c){
	memset(&CATEGORY[0], 0, sizeof(CATEGORY));	
	strcat(CATEGORY, c);
}

void Experiment::setDescription(char *d){
	memset(&DESCRIPTION[0], 0, sizeof(DESCRIPTION));
	strcat(DESCRIPTION, d);
}

void Experiment::getBytes(char *buffArray){
	//header
	strcat(buffArray, "<phyphox xmlns=\"http://phyphox.org/xml\" xmlns:editor=\"http://phyphox.org/editor/xml\" version=\"1.7\" editor:version=\"1.0\" locale=\"en\">\n");
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
	strcat(buffArray, "\t<container size=\"0\" static=\"false\">CH1</container>\n\t<container size=\"0\" static=\"false\">CH2</container>\n\t<container size=\"0\" static=\"false\">CH3</container>\n\t<container size=\"0\" static=\"false\">CH4</container>\n\t<container size=\"0\" static=\"false\">CH5</container>\n\t<container size=\"0\" static=\"false\">CH0</container>\n");
	strcat(buffArray, "</data-containers>\n");

	//build input
	strcat(buffArray, "<input>\n");
	strcat(buffArray, "\t<bluetooth mode=\"notification\" rate=\"1\" subscribeOnStart=\"false\">\n\t\t");

	for(int i=1; i<=5;i++){
		strcat(buffArray, "<output char=\"59f51a40-8852-4abe-a50f-2d45e6bd51ac\" conversion=\"float32LittleEndian\" ");
		char add[20];
		int k = (i-1)*4;
		sprintf(add, "offset=\"%i\" >CH%i", k,i);
		strcat(buffArray, add);
		strcat(buffArray,"</output>\n");
	}
	strcat(buffArray,"<output char=\"59f51a40-8852-4abe-a50f-2d45e6bd51ac\" extra=\"time\">CH0</output>");

	strcat(buffArray, "\t</bluetooth>\n");
	strcat(buffArray, "</input>\n");

	//build output
	strcat(buffArray, "<output></output>\n");

	//build analysis
	strcat(buffArray, "<analysis sleep=\"0\"  onUserInput=\"false\"></analysis>\n");
	
	//build views

	strcat(buffArray, "<views>\n");
	for(int i=0;i<nViews; i++){
		if(VIEWS[i]!=nullptr){
			VIEWS[i]->getBytes(buffArray);
		}
	}
	strcat(buffArray,"</views>\n");

	//build export
	strcat(buffArray, "<export></export>\n");
	
	//close
	strcat(buffArray, "</phyphox>");

}

