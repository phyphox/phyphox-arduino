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

	class Element
	{
	public:
		Element(){};
		Element(const Element &) = delete;
		Element &operator=(const Element &) = delete;
		~Element() = default;

		int typeID = 0;

		char LABEL[50] = "Graph";

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

		char UNITX[50] = "s";
		char UNITY[50] = "";
		char LABELX[50] = "time";
		char LABELY[50] = "value";
		char COLOR[8] = "ff7e22";
		char XPRECISION[5] = "3";
		char YPRECISION[5] = "3";

		char INPUTX[5] = "CH0";
		char INPUTY[5] = "CH1";

		char STYLE[8] = "lines";

		char WILD[100] = "";

		void setUnitX(const char *);
		void setUnitY(const char *);
		void setLabelX(const char *);
		void setLabelY(const char *);
		void setXPrecision(int);
		void setYPrecision(int);
		void setColor(const char *);
		void setChannel(int, int);
		void setStyle(const char *);
		void setWild(const char *);

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
		void setWild(const char *);

		char LABEL[50] = "Data";
		char WILD[100] = "";
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

		char NAME[50] = "Name";
		char BUFFER[50] = "CH1";
		char WILD[100] = "";

		void setName(const char *);
		void setDatachannel(int);
		void setWild(const char *);

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

		void setName(const char *);
		void getBytes(char *);
		void addElement(Element &);
		void setWild(const char *);

		char NAME[50] = "Name";
		char WILD[100] = "";
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
		void setWild(const char *);
		void getBytes(char *);

		char INFO[250] = "Info";
		char COLOR[8] = "ffffff";
		char NAME[50] = "Name";
		char WILD[100] = "";

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
		void setWild(const char *);
		void getBytes(char *);

		char COLOR[8] = "404040";
		char HEIGHT[6] = "0.1";
		char NAME[50] = "Name";
		char WILD[100] = "";

	private:
	};

	class Value : public Element
	{
	public:
		Value(){};
		Value(const Value &) = delete;
		Value &operator=(const Value &) = delete;
		~Value() = default;

		void setLabel(const char *);
		void setPrecision(int);
		void setUnit(const char *);
		void setColor(const char *);
		void getBytes(char *);
		void setChannel(int);
		void setWild(const char *);

		char LABEL[50] = "Value";
		char PRECISION[2] = "2";
		char UNIT[10] = "Unit";
		char COLOR[8] = "ffffff";
		char INPUTVALUE[5] = "CH1";
		char NAME[50] = "Name";
		char WILD[100] = "";

	private:
	};

	class Edit : public Element
	{
	public:
		Edit(){};
		Edit(const Edit &) = delete;
		Edit &operator=(const Edit &) = delete;
		~Edit() = default;

		void setLabel(const char *);
		void setUnit(const char *);
		void setSigned(bool);
		void setDecimal(bool);
		void setWild(const char *);
		void getBytes(char *);

		char LABEL[50] = "Edit";
		char UNIT[10] = "Unit";
		char SIGNED[7] = "true";
		char DECIMAL[7] = "true";
		char NAME[50] = "Name";
		char WILD[100] = "";

	private:
	};

	void setTitle(const char *);
	void setCategory(const char *);
	void setDescription(const char *);
	void setConfig(const char *);
	void setDeviceName(const char *);

	void getBytes(char *);
	void getFirstBytes(char *);
	void getViewBytes(char *, uint8_t, uint8_t);
	void getLastBytes(char *);
	void addView(View &);
	void addExportSet(ExportSet &);
	void setWild(const char *);

	char TITLE[50] = "Arduino-Experiment";
	char CATEGORY[50] = "Arduino Experiments";
	char DESCRIPTION[500] = "An experiment created with the phyphox BLE library for Arduino-compatible micro controllers.";
	char CONFIG[8] = "000000";
	char DEVICENAME[20] = "MyDevice";

	View *VIEWS[phyphoxBleNViews] = {nullptr};
	ExportSet *EXPORTSETS[phyphoxBleNExportSets] = {nullptr};
};

#endif
