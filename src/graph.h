#ifndef GRAPH
#define GRAPH
#include "element.h"
#include <Arduino.h>
class Graph : public Element 
{
	public:
    	Graph() {};                     
    	Graph(const Graph&) = delete;         
    	Graph &operator=(const Graph &) = delete;
	~Graph() = default;

	char UNITX[50] = "";
	char UNITY[50] = "";
	char LABELX[50] = "Default Label X";
	char LABELY[50] = "Default Label Y";

	char INPUTX[5]="CH";
	char INPUTY[5]="CH";

	void setUnitX(char*);
	void setUnitY(char*);
	void setLabelX(char*);
	void setLabelY(char*);
	void setChannel(int, int);

	void phyphoxTimestamp();
	void getBytes(char*);
	private:


};
#endif
