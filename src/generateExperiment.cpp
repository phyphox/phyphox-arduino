#include "generateExperiment.h"



void myExperiment::buildExperiment(int& expSize){
	int currentBytePosition=0;
	char header[] = "<phyphox xmlns=\"http://phyphox.org/xml\" xmlns:editor=\"http://phyphox.org/editor/xml\" version=\"1.7\" editor:version=\"1.0\" locale=\"en\">";



	const static int sizeOfHeader = sizeof(header);
	memcpy(&myExperiment::GENERATEDEXPERIMENT[0],&header[0],sizeOfHeader);
	currentBytePosition=currentBytePosition+sizeOfHeader;

	// BUILD TITLE
	title(currentBytePosition);

	//BUILD CATEGORY
	category(currentBytePosition);

	//BUILD DESCRIPTION
	description(currentBytePosition);

	//BUILD DATA CONTAINER
	container(currentBytePosition);

	//BUILD INPUT
	input(currentBytePosition);

	//BUILD OUTPUT
	output(currentBytePosition);

	//BUILD ANALYSIS
	analysis(currentBytePosition);

	//BUILD VIEWS/GRAPHS
	views(currentBytePosition);

	//BUILD EXPORT
	exportData(currentBytePosition);

	//END PHPHOX FILE
	char endPhyphox[]="</phyphox>";

	memcpy(&myExperiment::GENERATEDEXPERIMENT[currentBytePosition],&endPhyphox[0],sizeof(endPhyphox));
	currentBytePosition=currentBytePosition+sizeof(endPhyphox);
	expSize = currentBytePosition;

/*
	const static int sizeOfBody = sizeof(BODY);
	expSize = sizeOfHeader+sizeOfBody;
	memcpy(&myExperiment::GENERATEDEXPERIMENT[0],&header[0],sizeOfHeader);
	memcpy(&myExperiment::GENERATEDEXPERIMENT[0+sizeOfHeader],&BODY[0],sizeOfBody);
*/
}


void myExperiment::title(int& address){

	char open[] = "<title>";
	char close[] = "</title>";

	const size_t size = sizeof(open) + sizeof(close) + sizeof(myExperiment::TITLE);

	memcpy(&myExperiment::GENERATEDEXPERIMENT[address], &open[0], sizeof(open)); 
	memcpy(&myExperiment::GENERATEDEXPERIMENT[address+sizeof(open)], &myExperiment::TITLE[0], sizeof(myExperiment::TITLE)); 
	memcpy(&myExperiment::GENERATEDEXPERIMENT[address+sizeof(open)+sizeof(myExperiment::TITLE)], &close[0], sizeof(close)); 
	address = address + size;
}

void myExperiment::category(int& address){

	char open[] = "<category>";
	char close[] = "</category>";

	const size_t size = sizeof(open) + sizeof(close) + sizeof(myExperiment::CATEGORY);

	memcpy(&myExperiment::GENERATEDEXPERIMENT[address], &open[0], sizeof(open)); 
	memcpy(&myExperiment::GENERATEDEXPERIMENT[address+sizeof(open)], &myExperiment::CATEGORY[0], sizeof(myExperiment::CATEGORY)); 
	memcpy(&myExperiment::GENERATEDEXPERIMENT[address+sizeof(open)+sizeof(myExperiment::CATEGORY)], &close[0], sizeof(close)); 
	address = address + size;
}

void myExperiment::description(int& address){

	char open[] = "<description>";
	char close[] = "</description>";

	const size_t size = sizeof(open) + sizeof(close) + sizeof(myExperiment::DESCRIPTION);

	memcpy(&myExperiment::GENERATEDEXPERIMENT[address], &open[0], sizeof(open)); 
	memcpy(&myExperiment::GENERATEDEXPERIMENT[address+sizeof(open)], &myExperiment::DESCRIPTION[0], sizeof(myExperiment::DESCRIPTION)); 
	memcpy(&myExperiment::GENERATEDEXPERIMENT[address+sizeof(open)+sizeof(myExperiment::DESCRIPTION)], &close[0], sizeof(close)); 
	address = address + size;
}

void myExperiment::container(int& address){

	char open[] = "<data-containers>";
	char close[] = "</data-containers>";
	char content[] = "<container size=\"0\" static=\"false\">f1</container>\n<container size=\"0\" static=\"false\">f2</container>\n<container size=\"0\" static=\"false\">f3</container>\n<container size=\"0\" static=\"false\">f4</container>\n<container size=\"0\" static=\"false\">f5</container>\n<container size=\"0\" static=\"false\">t</container>\n";

	const size_t size = sizeof(open) + sizeof(close) + sizeof(content);

	memcpy(&myExperiment::GENERATEDEXPERIMENT[address], &open[0], sizeof(open)); 
	memcpy(&myExperiment::GENERATEDEXPERIMENT[address+sizeof(open)], &content, sizeof(content)); 
	memcpy(&myExperiment::GENERATEDEXPERIMENT[address+sizeof(open)+sizeof(content)], &close[0], sizeof(close)); 
	address = address + size;
}

void myExperiment::input(int& address){

	char open[] = "<input>";
	char close[] = "</input>";
	char content[] = "<bluetooth mode=\"notification\" rate=\"1\" subscribeOnStart=\"false\">\n<output char=\"59f51a40-8852-4abe-a50f-2d45e6bd51ac\" conversion=\"float32LittleEndian\">f1</output>\n</bluetooth>";

	const size_t size = sizeof(open) + sizeof(close) + sizeof(content);

	memcpy(&myExperiment::GENERATEDEXPERIMENT[address], &open[0], sizeof(open)); 
	memcpy(&myExperiment::GENERATEDEXPERIMENT[address+sizeof(open)], &content, sizeof(content)); 
	memcpy(&myExperiment::GENERATEDEXPERIMENT[address+sizeof(open)+sizeof(content)], &close[0], sizeof(close)); 
	address = address + size;
}

void myExperiment::output(int& address){

	char open[] = "<output>";
	char close[] = "</output>";
	char content[] = "";

	const size_t size = sizeof(open) + sizeof(close) + sizeof(content);

	memcpy(&myExperiment::GENERATEDEXPERIMENT[address], &open[0], sizeof(open)); 
	memcpy(&myExperiment::GENERATEDEXPERIMENT[address+sizeof(open)], &content, sizeof(content)); 
	memcpy(&myExperiment::GENERATEDEXPERIMENT[address+sizeof(open)+sizeof(content)], &close[0], sizeof(close)); 
	address = address + size;
}

void myExperiment::analysis(int& address){

	char open[] = "<analysis sleep=\"0\"  onUserInput=\"false\">";
	char close[] = "</analysis>";
	char content[] = "";

	const size_t size = sizeof(open) + sizeof(close) + sizeof(content);

	memcpy(&myExperiment::GENERATEDEXPERIMENT[address], &open[0], sizeof(open)); 
	memcpy(&myExperiment::GENERATEDEXPERIMENT[address+sizeof(open)], &content, sizeof(content)); 
	memcpy(&myExperiment::GENERATEDEXPERIMENT[address+sizeof(open)+sizeof(content)], &close[0], sizeof(close)); 
	address = address + size;
}
void myExperiment::views(int& address){

	char open[] = "<views>";
	char close[] = "</views>";
	char content[] = "<view label=\"New view\">\n<value label=\"view label\" size=\"1\" precision=\"2\" scientific=\"false\"  factor=\"1\" >\n<input>f1</input>\n</value>\n</view>";

	const size_t size = sizeof(open) + sizeof(close) + sizeof(content);

	memcpy(&myExperiment::GENERATEDEXPERIMENT[address], &open[0], sizeof(open)); 
	memcpy(&myExperiment::GENERATEDEXPERIMENT[address+sizeof(open)], &content, sizeof(content)); 
	memcpy(&myExperiment::GENERATEDEXPERIMENT[address+sizeof(open)+sizeof(content)], &close[0], sizeof(close)); 
	address = address + size;
}
void myExperiment::exportData(int& address){

	char open[] = "<export>";
	char close[] = "</export>";
	char content[] = "";

	const size_t size = sizeof(open) + sizeof(close) + sizeof(content);

	memcpy(&myExperiment::GENERATEDEXPERIMENT[address], &open[0], sizeof(open)); 
	memcpy(&myExperiment::GENERATEDEXPERIMENT[address+sizeof(open)], &content, sizeof(content)); 
	memcpy(&myExperiment::GENERATEDEXPERIMENT[address+sizeof(open)+sizeof(content)], &close[0], sizeof(close)); 
	address = address + size;
}


/*

	//unsigned char BODY[] =  "<title>New experiment</title>\n<category>no category</category>\n<description></description>\n<data-containers>\n<container size=\"1000\" static=\"false\">t</container>\n<container size=\"1000\" static=\"false\">abs</container>\n</data-containers>\n<input>\n<sensor rate=\"0\" average=\"false\" type=\"accelerometer\">\n<output component=\"t\">t</output>\n<output component=\"abs\">abs</output>\n</sensor>\n</input>\n<output></output>\n<analysis sleep=\"0\"  onUserInput=\"false\"></analysis>\n<views>\n<view label=\"New view\">\n<graph label=\"LABEL\" aspectRatio=\"2.5\" style=\"lines\" lineWidth=\"1\" color=\"ff7e22\" partialUpdate=\"false\" history=\"1\" labelX=\"labelX\" labelY=\"labely\" labelZ=\"\" unitX=\"unitx\" unitY=\"unitY\" unitZ=\"\" logX=\"false\" logY=\"false\" logZ=\"false\" xPrecision=\"3\" yPrecision=\"3\" zPrecision=\"3\" scaleMinX=\"auto\" scaleMaxX=\"auto\" scaleMinY=\"auto\" scaleMaxY=\"auto\" scaleMinZ=\"auto\" scaleMaxZ=\"auto\" minX=\"0\" maxX=\"0\" minY=\"0\" maxY=\"0\" minZ=\"0\" maxZ=\"0\"  mapWidth=\"0\" >\n<input axis=\"x\">t</input>\n<input axis=\"y\">abs</input>\n</graph>\n</view></views>\n<export></export>\n</phyphox>";

*/
        
