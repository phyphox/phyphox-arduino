#ifndef PHYPHOX_BLE_EXPERIMENT
#define PHYPHOX_BLE_EXPERIMENT

#define phyphoxBleNViews 5
#define phyphoxBleNElements 5
#define phyphoxBleNExports 5

#include <Arduino.h>

class PhyphoxBleExperiment {

	public:

	PhyphoxBleExperiment() {};                     
	PhyphoxBleExperiment(const PhyphoxBleExperiment&) = delete;         
	PhyphoxBleExperiment &operator=(const PhyphoxBleExperiment &) = delete;
	~PhyphoxBleExperiment() = default;

    class Element {
	    public:

        	Element() {};                     
        	Element(const Element&) = delete;         
        	Element &operator=(const Element &) = delete;
		    ~Element() = default;

		    int typeID = 0;

		    char LABEL[50] = "Graph";

		    void setLabel(const char*);
		    virtual void getBytes(char*);

	    private:


    };

    class Graph : public Element 
    {
	    public:
        	Graph() {};                     
        	Graph(const Graph&) = delete;         
        	Graph &operator=(const Graph &) = delete;
	    ~Graph() = default;

	    char UNITX[50] = "s";
	    char UNITY[50] = "";
	    char LABELX[50] = "time";
	    char LABELY[50] = "value";

	    char INPUTX[5] = "CH0";
	    char INPUTY[5] = "CH1";

	    char STYLE[8] = "lines";

	    void setUnitX(const char*);
	    void setUnitY(const char*);
	    void setLabelX(const char*);
	    void setLabelY(const char*);
	    void setChannel(int, int);
	    void setStyle(const char*);

	    void phyphoxTimestamp();
	    void getBytes(char*);
	    private:


    };

    class View
    {
	    public:
        	View() {};                     
        	View(const View&) = delete;         
        	View &operator=(const View &) = delete;
	    ~View() = default;


	    void setLabel(const char*);
	    void getBytes(char*);
	    void addElement(Element&);

	    char LABEL[50] = "Data";
	    Element *ELEMENTS[phyphoxBleNViews]={nullptr};
	    private: 
    };

	class Exportdata : public Element 
    {
	    public:
        	Exportdata() {};                     
        	Exportdata(const Exportdata&) = delete;         
        	Exportdata &operator=(const Exportdata &) = delete;
	    ~Exportdata() = default;

	    char NAME[50] = "name";
		char BUFFER[50] = "buffer";

	    char INPUTX[5] = "CH0";
	    char INPUTY[5] = "CH1";

	    char STYLE[8] = "lines";

	    void setName(const char*);
		void setBuffer(const char*);
	    void setChannel(int, int);
	    void setStyle(const char*);

	    void phyphoxTimestamp();
	    void getBytes(char*);
	    private:


    };

	class Export
    {
	    public:
        	Export() {};                     
        	Export(const Export&) = delete;         
        	Export &operator=(const Export &) = delete;
	    ~Export() = default;


	    void setName(const char*);
	    void getBytes(char*);
	    void addElement(Element&);

	    char NAME[50] = "Name";
	    Element *ELEMENTS[phyphoxBleNExports]={nullptr};
	    private: 
    };

	void setTitle(const char*);
	void setCategory(const char*);
	void setDescription(const char*);

	void getBytes(char*);
	void addView(View&);
	void addExport(Export&);


	char TITLE[50] = "Arduino-Experiment";
	char CATEGORY[50] = "Arduino Experiments";
	char DESCRIPTION[500] = "An experiment created with the phyphox BLE library for Arduino-compatible micro controllers.";

	View *VIEWS[phyphoxBleNViews] = {nullptr};
	Export *EXPORTS[phyphoxBleNExports] = {nullptr};

};

#endif
