#ifndef PHYPHOX_BLE_EXPERIMENT
#define PHYPHOX_BLE_EXPERIMENT

#define phyphoxBleNViews 5
#define phyphoxBleNElements 5
#define phyphoxBleNExportSets 5

#include <Arduino.h>

class PhyphoxBleExperiment
{

public:
	PhyphoxBleExperiment(){};
	PhyphoxBleExperiment(const PhyphoxBleExperiment &) = delete;
	PhyphoxBleExperiment &operator=(const PhyphoxBleExperiment &) = delete;
	~PhyphoxBleExperiment() = default;

	class Errorhandler
	{
	public:
		Errorhandler(){};
		Errorhandler(const Errorhandler &) = delete;
		Errorhandler &operator=(const Errorhandler &) = delete;
		~Errorhandler() = default;

		const char* err_checkLength(const char *, int);
		int err_checkUpper(int, int);
		const char* err_checkHex(const char *);
		const char* err_checkStyle(const char *);
	};

	class Element : public Errorhandler
	{
	public:
		Element(){};
		Element(const Element &) = delete;
		Element &operator=(const Element &) = delete;
		~Element() = default;

		int typeID = 0;

		char LABEL[50] = "";

		void setLabel(const char *);
		virtual void getBytes(char *);

	private:
	};

	class Graph : public Element
	{
	public:
		Graph(){};
		Graph(const Graph &) = delete;
		Graph &operator=(const Graph &) = delete;
		~Graph() = default;

		char UNITX[30] = "";
		char UNITY[30] = "";
		char LABELX[50] = "";
		char LABELY[50] = "";
		char COLOR[17] = "";
		char XPRECISION[20] = "";
		char YPRECISION[20] = "";

		char INPUTX[5] = "CH0";
		char INPUTY[5] = "CH1";

		char STYLE[17] = "";

		char XMLAttribute[100] = "";

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
		View(){};
		View(const View &) = delete;
		View &operator=(const View &) = delete;
		~View() = default;

		void setLabel(const char *);
		void getBytes(char *, uint8_t);
		void addElement(Element &);
		void setXMLAttribute(const char *);

		char LABEL[50] = "";
		char XMLAttribute[100] = "";
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

		char BUFFER[5] = "CH1";
		char XMLAttribute[100] = "";

		void setDatachannel(int);
		void setXMLAttribute(const char *);

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

		void setLabel(const char *);
		void getBytes(char *);
		void addElement(Element &);
		void setXMLAttribute(const char *);

		char LABEL[50] = "";
		char XMLAttribute[100] = "";
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

		char INFO[200] = "";
		char COLOR[17] = "";
		char XMLAttribute[100] = "";

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

		char COLOR[17] = "";
		char HEIGHT[20] = " height=\"0.1\"";
		char XMLAttribute[100] = "";

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

		char PRECISION[15] = "";
		char UNIT[20] = "";
		char COLOR[17] = "";
		char INPUTVALUE[5] = "CH3";
		char XMLAttribute[100] = "";

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

		char UNIT[18] = "";
		char SIGNED[17] = "";
		char DECIMAL[17] = "";
		char XMLAttribute[100] = "";
		char BUFFER[5] = "CH5";

	private:
	};

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

	char TITLE[50] = "Arduino-Experiment";
	char CATEGORY[50] = "Arduino Experiments";
	char DESCRIPTION[500] = "An experiment created with the phyphox BLE library for Arduino-compatible micro controllers.";
	char CONFIG[8] = "000000";

	View *VIEWS[phyphoxBleNViews] = {nullptr};
	ExportSet *EXPORTSETS[phyphoxBleNExportSets] = {nullptr};
};

#endif
