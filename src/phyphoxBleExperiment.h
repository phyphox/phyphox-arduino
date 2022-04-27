#ifndef PHYPHOX_BLE_EXPERIMENT
#define PHYPHOX_BLE_EXPERIMENT

#define phyphoxBleNViews 5
#define phyphoxBleNElements 20
#define phyphoxBleNExportSets 5

#include <Arduino.h>

class PhyphoxBleExperiment {

	public:

	PhyphoxBleExperiment() = default;                     
	PhyphoxBleExperiment(const PhyphoxBleExperiment&) = delete;         
	PhyphoxBleExperiment &operator=(const PhyphoxBleExperiment &) = delete;
	~PhyphoxBleExperiment() = default;

	static uint16_t MTU;

	class Error {
	public:
		char* MESSAGE = "";
		void getBytes(char *);
	};


	class Errorhandler {
	public:
		virtual void copyToMem(char **, const char *);
		
		virtual Error err_checkLength(const char *, int, const char *);
		virtual Error err_checkUpper(int, int, const char *);
		virtual Error err_checkHex(const char *, const char *);
		virtual Error err_checkStyle(const char *, const char *);
	};

	class Element : public Errorhandler {
	public:
		Element() = default;
		Element(const Element &) = delete;
		Element &operator=(const Element &) = delete;
		~Element() = default;

		int typeID = 0;

		char LABEL[50] = " label=\"label\"";

		Error ERROR;

		virtual void setLabel(const char *);
		virtual void getBytes(char *)=0;
		virtual void copyToMem(char **, const char *);

	private:
	};

	class Graph : public Element
	{
	public:
		Graph() = default;
		Graph(const Graph &) = delete;
		Graph &operator=(const Graph &) = delete;
		~Graph() = default;

		char* UNITX = "";
		char* UNITY = "";
		char* LABELX = " labelX=\"label x\"";
		char* LABELY = " labelY=\"label y\"";
		char* COLOR = "";
		char* XPRECISION = "";
		char* YPRECISION = "";

		char* INPUTX = "CH0";
		char* INPUTY = "CH1";

		char* STYLE = "";

		char* XMLAttribute = "";

		void setUnitX(const char *);
		void setUnitY(const char *);
		void setLabelX(const char *);
		void setLabelY(const char *);
		void setXPrecision(int);
		void setYPrecision(int);
		void setColor(const char *);
		void setChannel(int, int);
		void setStyle(const char *);
		void setXMLAttribute(const char *);

		void phyphoxTimestamp();
		void getBytes(char *);

	private:
	};

	class View
	{
	public:
		View() = default;
		View(const View &) = delete;
		View &operator=(const View &) = delete;
		~View() = default;

		void copyToMem(char **, const char *);

		void setLabel(const char *);
		void getBytes(char *, uint8_t);
		void addElement(Element &);
		void setXMLAttribute(const char *);

		char* LABEL = " label=\"label\"";
		char* XMLAttribute = "";

		Element *ELEMENTS[phyphoxBleNElements] = {nullptr};

	private:
	};

	class ExportData : public Element
	{
	public:
		ExportData(){};
		ExportData(const ExportData &) = delete;
		ExportData &operator=(const ExportData &) = delete;
		~ExportData() = default;

		char* BUFFER = "CH1";
		char* XMLAttribute = "";
		void setDatachannel(int);
		void setXMLAttribute(const char *);
		void setLabel(const char *);
		void getBytes(char *);

	private:
	};

	class ExportSet
	{
	public:
		ExportSet(){};
		ExportSet(const ExportSet &) = delete;
		ExportSet &operator=(const ExportSet &) = delete;
		~ExportSet() = default;

		void copyToMem(char **, const char *);

		void setLabel(const char *);
		void getBytes(char *);
		void addElement(Element &);
		void setXMLAttribute(const char *);

		char* LABEL = "";
		char* XMLAttribute = "";
		Element *ELEMENTS[phyphoxBleNExportSets] = {nullptr};

	private:
	};

	class InfoField : public Element
	{
	public:
		InfoField(){};
		InfoField(const InfoField &) = delete;
		InfoField &operator=(const InfoField &) = delete;
		~InfoField() = default;

		void setInfo(const char *);
		void setColor(const char *);
		void setXMLAttribute(const char *);
		void getBytes(char *);

		char* INFO = "";
		char* COLOR = "";
		char* XMLAttribute = "";

	private:
	};

	class Separator : public Element
	{
	public:
		Separator(){};
		Separator(const Separator &) = delete;
		Separator &operator=(const Separator &) = delete;
		~Separator() = default;

		void setHeight(float);
		void setColor(const char *);
		void setXMLAttribute(const char *);
		void getBytes(char *);

		char* COLOR = "";
		char* HEIGHT = " height=\"0.1\"";
		char* XMLAttribute = "";

	private:
	};

	class Value : public Element
	{
	public:
		Value(){};
		Value(const Value &) = delete;
		Value &operator=(const Value &) = delete;
		~Value() = default;

		void setPrecision(int);
		void setUnit(const char *);
		void setColor(const char *);
		void getBytes(char *);
		void setChannel(int);
		void setXMLAttribute(const char *);

		char* PRECISION = "";
		char* UNIT = "";
		char* COLOR = "";
		char* INPUTVALUE = "CH3";
		char* XMLAttribute = "";

	private:
	};

	class Edit : public Element
	{
	public:
		Edit(){};
		Edit(const Edit &) = delete;
		Edit &operator=(const Edit &) = delete;
		~Edit() = default;

		void setUnit(const char *);
		void setSigned(bool);
		void setDecimal(bool);
		void setXMLAttribute(const char *);
		void setChannel(int);
		void getBytes(char *);

		char* UNIT = "";
		char* SIGNED = "";
		char* DECIMAL = "";
		char* XMLAttribute = "";
		char* BUFFER = "CH5";
	
	private:
	};

	void copyToMem(char **, const char *);

	void setTitle(const char *);
	void setCategory(const char *);
	void setDescription(const char *);
	void setConfig(const char *);

	void getBytes(char *);
	void getFirstBytes(char *, const char *);
	void getViewBytes(char *, uint8_t, uint8_t);
	void getLastBytes(char *);
	void addView(View &);
	void addExportSet(ExportSet &);

	char* TITLE = "Arduino-Experiment";
	char* CATEGORY = "Arduino Experiments";
	char* DESCRIPTION = "An experiment created with the phyphox BLE library for Arduino-compatible micro controllers.";
	char* CONFIG = "000000";

	View *VIEWS[phyphoxBleNViews] = {nullptr};
	ExportSet *EXPORTSETS[phyphoxBleNExportSets] = {nullptr};

	static int numberOfChannels;
	int repeating = 0;

	
};

#endif
