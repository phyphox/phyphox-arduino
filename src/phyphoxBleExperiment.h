// phyphox BLE 2.0 — the experiment description API.
//
// A sketch describes what the phyphox app should show (views, graphs, values, user inputs,
// buttons, export sets) and which phone sensors it wants, then hands the description to
// PhyphoxBLE::addExperiment(). The library generates the phyphox XML from it on demand and
// transfers it when a phone connects.
//
// Lifetime rules (the same as 1.x): build everything in setup(), call addExperiment() while all
// objects are still alive — it copies the description into one exactly-sized block — and pass
// string literals or static buffers, never temporaries (docs/concepts.md, "Strings").
//
// There are no capacity limits: views, elements, curves, options, export sets and sensors are
// linked into intrusive lists (each object carries one `next` pointer), so an object can be in
// one list at a time and the block is sized from what was actually added.
//
// This header is Arduino-independent so the core can be unit-tested on a host.
#ifndef PHYPHOX_BLE_EXPERIMENT
#define PHYPHOX_BLE_EXPERIMENT

#include <stdint.h>
#include "core/Config.h"
#include "core/ElementData.h"

/// Style and layout constants accepted by setStyle() and setMin*/setMax*(). Same strings as 1.x.
#define STYLE_LINES "lines"
#define STYLE_DOTS "dots"
#define STYLE_VBARS "vbars"
#define STYLE_HBARS "hbars"
#define STYLE_MAP "map"
#define LAYOUT_AUTO "auto"
#define LAYOUT_EXTEND "extend"
#define LAYOUT_FIXED "fixed"

class PhyphoxBleExperiment {
public:
    PhyphoxBleExperiment() = default;
    /// Shortcut for setTitle/setCategory/setDescription.
    PhyphoxBleExperiment(const char* title, const char* category = nullptr, const char* description = nullptr);
    PhyphoxBleExperiment(const PhyphoxBleExperiment&) = delete;
    PhyphoxBleExperiment& operator=(const PhyphoxBleExperiment&) = delete;

    // ------------------------------------------------------------------ view elements

    /// Base of every view element. Holds the plain data the serializer reads.
    class Element {
    public:
        Element(const Element&) = delete;
        Element& operator=(const Element&) = delete;
        virtual ~Element() {}
        virtual void setLabel(const char*);
        /// Raw attribute text appended verbatim to the element's start tag, e.g.
        /// setXMLAttribute("logY=\"true\""). The escape hatch for anything the API lacks; the
        /// library does not validate it.
        void setXMLAttribute(const char*);
        /// Show the element only while the last value in that input channel is > 0
        /// (a toggle's channel, typically). Format 1.20.
        void setVisibility(int inputChannel);
        phyphox::ElementData& data() { return data_; }
        const phyphox::ElementData& data() const { return data_; }
        virtual bool isExportData() const { return false; }
        Element* next = nullptr;      ///< intrusive list link, set by View::addElement()
    protected:
        explicit Element(phyphox::ElementType t) { data_.init(t); }
        phyphox::ElementData data_;
    };

    class Graph : public Element {
    public:
        /// One additional curve in a graph. Configure, then Graph::addSubgraph(); the graph keeps
        /// a pointer, so the Subgraph must live until addExperiment().
        class Subgraph {
        public:
            Subgraph() = default;
            Subgraph(const Subgraph&) = delete;
            void setChannel(int x, int y);        ///< 0 = time, 1…5 = data channels
            void setColor(const char* hex6);
            void setStyle(const char* style);
            void setLinewidth(float);
            void setLineWidth(float w) { setLinewidth(w); }
            phyphox::SubgraphData data;
            phyphox::ErrorRecord error;
            Subgraph* next = nullptr;
        };

        Graph() : Element(phyphox::EL_GRAPH) {}
        explicit Graph(const char* label) : Graph() { setLabel(label); }

        // 1.x setters
        void setUnitX(const char*);
        void setUnitY(const char*);
        void setLabelX(const char*);
        void setLabelY(const char*);
        void setXPrecision(int);
        void setYPrecision(int);
        void setTimeOnX(bool);
        void setTimeOnY(bool);
        void setSystemTime(bool);
        void setChannel(int x, int y);            ///< the first curve
        void addSubgraph(Subgraph&);
        void addChannel(int x, int y, const char* color); ///< 1.x shortcut: add a curve in one call
        void setStyle(const char*);               ///< first curve
        void setColor(const char*);               ///< first curve
        void setLinewidth(float);                 ///< first curve
        void setLineWidth(float w) { setLinewidth(w); }
        void setMinX(float, const char* layout);
        void setMaxX(float, const char* layout);
        void setMinY(float, const char* layout);
        void setMaxY(float, const char* layout);
        // new in 2.0 (D16, D17)
        void setLinearTime(bool);
        void setLogX(bool);
        void setLogY(bool);
        void setFollowX(bool);
        void setPartialUpdate(bool);
        void setHideTimeMarkers(bool);
        void setSuppressScientificNotation(bool);
        void setShowColorScale(bool);
        void setAspectRatio(float);
        void setUnitYperX(const char*);

        Subgraph first;                           ///< the curve setChannel/setStyle/setColor configure
        Subgraph* subgraphs = nullptr;            ///< added curves, in order
        Subgraph* lastSubgraph = nullptr;
    };

    class Value : public Element {
    public:
        Value() : Element(phyphox::EL_VALUE) {}
        explicit Value(const char* label, int channel = 1) : Value() { setLabel(label); setChannel(channel); }
        void setPrecision(int);
        void setUnit(const char*);
        void setColor(const char*);
        void setChannel(int);                     ///< data channel 1…5 shown
        // new in 2.0
        void setSize(float);
        void setFactor(float);
        void setScientific(bool);
        /// Show `text` instead of the number while min <= value <= max (D17); the first match
        /// wins. Up to PHYPHOX_BLE_INLINE_OPTIONS entries this way; setMaps() takes any number.
        void addMap(float min, float max, const char* text);
        void addMapBelow(float max, const char* text);
        void addMapAbove(float min, const char* text);
        void setMaps(int n, const float* mins, const float* maxs, const char* const texts[]);

        phyphox::MapEntry inlineMaps[PHYPHOX_BLE_INLINE_OPTIONS];
        const phyphox::MapEntry* mapsArray = nullptr;   ///< reserved
        const float* mapsMins = nullptr;                ///< setMaps(): the caller's arrays, read at addExperiment
        const float* mapsMaxs = nullptr;
        const char* const* mapsTexts = nullptr;
        uint8_t mapCount = 0;
    };

    /// Common API of the elements that send a value to the board.
    class InputElement : public Element {
    public:
        void setChannel(int);                     ///< input channel 1…PHYPHOX_BLE_INPUT_CHANNELS
        void setDefault(float);
        /// Called on the board whenever the phone delivers a new value for this channel.
        void onChange(phyphox::ChangeCallback);
    protected:
        explicit InputElement(phyphox::ElementType t) : Element(t) {}
    };

    class Button;

    class Edit : public InputElement {
    public:
        Edit() : InputElement(phyphox::EL_EDIT) {}
        explicit Edit(const char* label, int channel = 0) : Edit() { setLabel(label); if (channel) setChannel(channel); }
        void setUnit(const char*);
        void setSigned(bool);
        void setDecimal(bool);
        // new in 2.0
        void setMin(float);
        void setMax(float);
        void setFactor(float);
        /// Deliver this field only when `button` is pressed (format 1.20, triggerId).
        void submitWith(Button&);
    };

    class Toggle : public InputElement {
    public:
        Toggle() : InputElement(phyphox::EL_TOGGLE) {}
        explicit Toggle(const char* label, int channel = 0) : Toggle() { setLabel(label); if (channel) setChannel(channel); }
    };

    class Slider : public InputElement {
    public:
        Slider() : InputElement(phyphox::EL_SLIDER) {}
        Slider(const char* label, float minValue, float maxValue, float stepSize, int channel = 0);
        void setMinValue(float);
        void setMaxValue(float);
        void setStepSize(float);
        void setPrecision(int);
        void setShowValue(bool);
        void setColor(const char*);               ///< new in 2.0
    };

    class Dropdown : public InputElement {
    public:
        Dropdown() : InputElement(phyphox::EL_DROPDOWN) {}
        explicit Dropdown(const char* label, int channel = 0) : Dropdown() { setLabel(label); if (channel) setChannel(channel); }
        /// 1.x form: n options from two arrays (copied at addExperiment; the arrays may be locals).
        void setOptions(int n, const char* const labels[], const float* values);
        /// New in 2.0; up to PHYPHOX_BLE_INLINE_OPTIONS this way, any number via setOptions().
        void addOption(const char* label, float value);
        void setColor(const char*);

        phyphox::MapEntry inlineOptions[PHYPHOX_BLE_INLINE_OPTIONS];
        const char* const* optionLabels = nullptr;      ///< setOptions(): the caller's arrays
        const float* optionValues = nullptr;
        uint8_t optionCount = 0;
    };

    /// New in 2.0. Pressing the button writes `value` (default 1) into its input channel; the
    /// board's onPress() callback fires once per press. A button owns its channel.
    class Button : public InputElement {
    public:
        Button() : InputElement(phyphox::EL_BUTTON) {}
        explicit Button(const char* label, int channel = 0) : Button() { setLabel(label); if (channel) setChannel(channel); }
        void setValue(float);
        void onPress(phyphox::PressCallback);
    };

    class InfoField : public Element {
    public:
        InfoField() : Element(phyphox::EL_INFO) {}
        explicit InfoField(const char* text) : InfoField() { setInfo(text); }
        void setInfo(const char*);                ///< the text (the info element's label)
        void setColor(const char*);
        // new in 2.0
        void setSize(float);
        void setAlign(const char* leftRightCenter);
        void setBold(bool);
        void setItalic(bool);
    };

    class Separator : public Element {
    public:
        Separator() : Element(phyphox::EL_SEPARATOR) {}
        void setHeight(float);
        void setColor(const char*);
    };

    /// A tab in the app. Links its elements into a list until addExperiment() copies them.
    class View {
    public:
        View() = default;
        explicit View(const char* label) { setLabel(label); }
        View(const View&) = delete;
        void setLabel(const char*);
        void setXMLAttribute(const char*);
        View& addElement(Element&);
        phyphox::ViewData data;
        Element* elements = nullptr;              ///< list head
        Element* lastElement = nullptr;
        View* next = nullptr;
        phyphox::ErrorRecord error;
    };

    // ------------------------------------------------------------------ export

    class ExportData : public Element {
    public:
        ExportData() : Element(phyphox::EL_NONE) {}
        ExportData(const char* label, int channel) : ExportData() { setLabel(label); setDatachannel(channel); }
        void setDatachannel(int);                 ///< 0 = time, 1…5 = data channels
        void setDataChannel(int c) { setDatachannel(c); }
        void setLabel(const char*) override;
        bool isExportData() const override { return true; }
        phyphox::ExportDataData exportData;
        ExportData* nextEntry = nullptr;
    };

    class ExportSet {
    public:
        ExportSet() = default;
        explicit ExportSet(const char* label) { setLabel(label); }
        ExportSet(const ExportSet&) = delete;
        void setLabel(const char*);
        void setXMLAttribute(const char*);
        ExportSet& addElement(ExportData&);
        void addElement(Element&);                ///< 1.x signature; must be an ExportData
        phyphox::ExportSetData data;
        ExportData* entries = nullptr;            ///< list head
        ExportData* lastEntry = nullptr;
        ExportSet* next = nullptr;
        phyphox::ErrorRecord error;
    };

    // ------------------------------------------------------------------ phone sensors

    /// A sensor of the phone whose readings the phone sends to the board.
    class Sensor {
    public:
        Sensor() = default;
        explicit Sensor(const char* type) { setType(type); }
        Sensor(const Sensor&) = delete;
        void setType(const char*);                ///< a SENSOR_* constant (phyphox file-format name)
        void setAverage(bool);
        void setRate(int hz);
        /// How the phone reconciles the requested rate with what its sensor offers:
        /// "auto", "request", "generate" or "limit" (format 1.14).
        void setRateStrategy(const char*);
        void setStride(int n);                    ///< use only every n-th reading (1.14)
        /// Select a sensor by name (the app's device info lists them) — needed for
        /// SENSOR_CUSTOM, useful when a phone has several sensors of one type (1.19, Android).
        void setNameFilter(const char*);
        void setTypeFilter(int androidSensorType);   ///< select by Android sensor type constant (1.19)
        /// Deliver component ("x", "y", "z", "abs", "accuracy", "t") into input channel `ch`.
        void mapChannel(const char* component, int ch);
        void setComponent(const char*);           ///< 1.x: like mapChannel(component, next free)
        void setXMLAttribute(const char*);
        phyphox::SensorData data;
        Sensor* next = nullptr;
    };

    // ------------------------------------------------------------------ the experiment

    void setTitle(const char*);
    void setCategory(const char*);
    void setDescription(const char*);
    void setColor(const char*);
    /// Samples per data notification when using the array write; emits the `repeating` attribute.
    void setRepeating(int);
    void setSubscribeOnStart(bool);
    /// 1.x behaviour: the phone writes every input's value every analysis cycle, not only on
    /// change (decision D14). Also handy as a heartbeat from the phone.
    void setResendUnchanged(bool);
    PhyphoxBleExperiment& addView(View&);
    PhyphoxBleExperiment& addSensor(Sensor&);
    PhyphoxBleExperiment& addExportSet(ExportSet&);

    phyphox::ExperimentData data;                 ///< experiment-level fields; arrays filled at addExperiment
    View* views = nullptr;                        ///< list heads
    View* lastView = nullptr;
    Sensor* sensors = nullptr;
    Sensor* lastSensor = nullptr;
    ExportSet* exportSets = nullptr;
    ExportSet* lastExportSet = nullptr;
};

#endif
