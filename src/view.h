#ifndef VIEW
#define VIEW
#define nElements 5
#include "graph.h"

class View
{
	public:
    	View() {};                     
    	View(const View&) = delete;         
    	View &operator=(const View &) = delete;
	~View() = default;


	void setLabel(char*);
	void getBytes(char*);
	void addElement(Graph&);

	char LABEL[50] = "Default View Label";
	Element *ELEMENTS[nElements]={nullptr};
	private: 



};
#endif
