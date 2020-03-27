#ifndef GENERATEEXPERIMENT_H
#define GENERATEEXPERIMENT_H

#include <Arduino.h>



class myExperiment{


	private:

	void title(int&);
	void category(int&);
	void description(int&);
	void container(int&);
	void input(int&);
	void output(int&);
	void analysis(int&);
	void views(int&);
	void exportData(int&);

	public:
	myExperiment() {};
	myExperiment(const myExperiment&) = delete;
	myExperiment &operator=(const myExperiment&) = delete;
	~myExperiment() = default;


	uint8_t GENERATEDEXPERIMENT[4096] = {0};// block some storage

	void buildExperiment(int&);

	char TITLE[20] = "Default Title";
	char CATEGORY[100] = "Arduino Experiment";
	char DESCRIPTION[100]="This phyphox experiment is creatend on a Arduino";
};

#endif
