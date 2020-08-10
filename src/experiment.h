#ifndef EXPERIMENT
#define EXPERIMENT


#define nViews 5

#include "element.h"
#include "view.h"
#include "graph.h"


class Experiment
{
	public:

	Experiment() {};                     
	Experiment(const Experiment&) = delete;         
	Experiment &operator=(const Experiment &) = delete;
	~Experiment() = default;


	void setTitle(char*);
	void setCategory(char*);
	void setDescription(char*);

	void getBytes(char*);
	void addView(View&);


	char TITLE[50] = "Default Experiment Title";
	char CATEGORY[50] = "Arduino Experiments";
	char DESCRIPTION[500] = "This is the Default Description";

	View *VIEWS[nViews] = {nullptr};
	private:

};
#endif
