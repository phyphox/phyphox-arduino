// The builder: setters validate and store plain data; lists link objects until addExperiment().
#include "phyphoxBleExperiment.h"
#include <string.h>

using namespace phyphox;

// ---------------------------------------------------------------- validation helpers
namespace {

bool checkString(const char* s, ErrorRecord& err, const char* origin) {
    if (!s) { err.record(ERR_08_NULL_STRING, origin); return false; }
    if (strlen(s) > PHYPHOX_BLE_MAX_STRING_LENGTH) { err.record(ERR_01_TOO_LONG, origin); return false; }
    return true;
}

bool isHex(char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }

bool checkColor(const char* s, ErrorRecord& err, const char* origin) {
    if (!s) { err.record(ERR_08_NULL_STRING, origin); return false; }
    if (strlen(s) != 6) { err.record(ERR_03_NOT_HEX_COLOR, origin); return false; }
    for (int i = 0; i < 6; ++i) if (!isHex(s[i])) { err.record(ERR_03_NOT_HEX_COLOR, origin); return false; }
    return true;
}

Style parseStyle(const char* s) {
    if (!s) return STYLE_NONE;
    if (!strcmp(s, "lines")) return ST_LINES;
    if (!strcmp(s, "dots")) return ST_DOTS;
    if (!strcmp(s, "vbars")) return ST_VBARS;
    if (!strcmp(s, "hbars")) return ST_HBARS;
    if (!strcmp(s, "map")) return ST_MAP;
    return STYLE_NONE;
}

Layout parseLayout(const char* s) {
    if (!s) return LAYOUT_NONE;
    if (!strcmp(s, "auto")) return LY_AUTO;
    if (!strcmp(s, "extend")) return LY_EXTEND;
    if (!strcmp(s, "fixed")) return LY_FIXED;
    return LAYOUT_NONE;
}

Align parseAlign(const char* s) {
    if (!s) return ALIGN_NONE;
    if (!strcmp(s, "left")) return AL_LEFT;
    if (!strcmp(s, "right")) return AL_RIGHT;
    if (!strcmp(s, "center")) return AL_CENTER;
    return ALIGN_NONE;
}

SensorComponent parseComponent(const char* s) {
    if (!s) return COMP_NONE;
    if (!strcmp(s, "x")) return COMP_X;
    if (!strcmp(s, "y")) return COMP_Y;
    if (!strcmp(s, "z")) return COMP_Z;
    if (!strcmp(s, "abs")) return COMP_ABS;
    if (!strcmp(s, "accuracy")) return COMP_ACCURACY;
    if (!strcmp(s, "t")) return COMP_T;
    return COMP_NONE;
}

const char* const kSensorTypes[] = {
    "accelerometer", "linear_acceleration", "gravity", "gyroscope", "magnetic_field", "pressure",
    "light", "proximity", "temperature", "humidity", "attitude", "custom", nullptr };

bool validSensorType(const char* s) {
    if (!s) return false;
    for (int i = 0; kSensorTypes[i]; ++i) if (!strcmp(s, kSensorTypes[i])) return true;
    return false;
}

bool checkDataChannel(int c, ErrorRecord& err, const char* origin) {
    if (c < 0 || c > PHYPHOX_BLE_DATA_CHANNELS) { err.record(ERR_02_ABOVE_LIMIT, origin); return false; }
    return true;
}

bool checkInputChannel(int c, ErrorRecord& err, const char* origin) {
    if (c < 1 || c > PHYPHOX_BLE_MAX_INPUT_CHANNEL) { err.record(ERR_02_ABOVE_LIMIT, origin); return false; }
    return true;
}

void setFlag(GraphData& g, uint16_t flag, bool on) {
    g.flagsSet |= flag;
    if (on) g.flags |= flag; else g.flags &= ~flag;
}

void curveStyle(SubgraphData& d, ErrorRecord& err, const char* style) {
    Style st = parseStyle(style);
    if (st == STYLE_NONE) { err.record(ERR_04_INVALID_VALUE, "setStyle"); return; }
    d.style = st;
}

} // namespace

// ---------------------------------------------------------------- Element

void PhyphoxBleExperiment::Element::setLabel(const char* l) {
    if (checkString(l, data_.error, "setLabel")) data_.label = l;
}
void PhyphoxBleExperiment::Element::setXMLAttribute(const char* a) {
    if (checkString(a, data_.error, "setXMLAttribute")) data_.xmlAttribute = a;
}
void PhyphoxBleExperiment::Element::setVisibility(int ch) {
    if (checkInputChannel(ch, data_.error, "setVisibility")) data_.visibilityChannel = (uint8_t)ch;
}

// ---------------------------------------------------------------- Graph and Subgraph

void PhyphoxBleExperiment::Graph::Subgraph::setChannel(int x, int y) {
    if (!checkDataChannel(x, error, "setChannel") || !checkDataChannel(y, error, "setChannel")) return;
    data.channelX = (uint8_t)x; data.channelY = (uint8_t)y; data.active = true;
}
void PhyphoxBleExperiment::Graph::Subgraph::setColor(const char* c) { if (checkColor(c, error, "setColor")) data.color = c; }
void PhyphoxBleExperiment::Graph::Subgraph::setStyle(const char* s) { curveStyle(data, error, s); }
void PhyphoxBleExperiment::Graph::Subgraph::setLinewidth(float w) { data.lineWidth = w; }

void PhyphoxBleExperiment::Graph::setUnitX(const char* s) { if (checkString(s, data_.error, "setUnitX")) data_.graph.unitX = s; }
void PhyphoxBleExperiment::Graph::setUnitY(const char* s) { if (checkString(s, data_.error, "setUnitY")) data_.graph.unitY = s; }
void PhyphoxBleExperiment::Graph::setUnitYperX(const char* s) { if (checkString(s, data_.error, "setUnitYperX")) data_.graph.unitYperX = s; }
void PhyphoxBleExperiment::Graph::setLabelX(const char* s) { if (checkString(s, data_.error, "setLabelX")) data_.graph.labelX = s; }
void PhyphoxBleExperiment::Graph::setLabelY(const char* s) { if (checkString(s, data_.error, "setLabelY")) data_.graph.labelY = s; }
void PhyphoxBleExperiment::Graph::setXPrecision(int p) { data_.graph.xPrecision = (int8_t)(p < 0 ? 0 : p); }
void PhyphoxBleExperiment::Graph::setYPrecision(int p) { data_.graph.yPrecision = (int8_t)(p < 0 ? 0 : p); }
void PhyphoxBleExperiment::Graph::setTimeOnX(bool b) { setFlag(data_.graph, GraphData::F_TIME_ON_X, b); }
void PhyphoxBleExperiment::Graph::setTimeOnY(bool b) { setFlag(data_.graph, GraphData::F_TIME_ON_Y, b); }
void PhyphoxBleExperiment::Graph::setSystemTime(bool b) { setFlag(data_.graph, GraphData::F_SYSTEM_TIME, b); }
void PhyphoxBleExperiment::Graph::setLinearTime(bool b) { setFlag(data_.graph, GraphData::F_LINEAR_TIME, b); }
void PhyphoxBleExperiment::Graph::setLogX(bool b) { setFlag(data_.graph, GraphData::F_LOG_X, b); }
void PhyphoxBleExperiment::Graph::setLogY(bool b) { setFlag(data_.graph, GraphData::F_LOG_Y, b); }
void PhyphoxBleExperiment::Graph::setFollowX(bool b) { setFlag(data_.graph, GraphData::F_FOLLOW_X, b); }
void PhyphoxBleExperiment::Graph::setPartialUpdate(bool b) { setFlag(data_.graph, GraphData::F_PARTIAL_UPDATE, b); }
void PhyphoxBleExperiment::Graph::setHideTimeMarkers(bool b) { setFlag(data_.graph, GraphData::F_HIDE_TIME_MARKERS, b); }
void PhyphoxBleExperiment::Graph::setSuppressScientificNotation(bool b) { setFlag(data_.graph, GraphData::F_SUPPRESS_SCI, b); }
void PhyphoxBleExperiment::Graph::setShowColorScale(bool b) { setFlag(data_.graph, GraphData::F_SHOW_COLOR_SCALE, b); }
void PhyphoxBleExperiment::Graph::setAspectRatio(float r) { data_.graph.aspectRatio = r; }
void PhyphoxBleExperiment::Graph::setChannel(int x, int y) { first.setChannel(x, y); data_.error = first.error.set() ? first.error : data_.error; }
void PhyphoxBleExperiment::Graph::setStyle(const char* s) { first.setStyle(s); if (first.error.set()) data_.error = first.error; }
void PhyphoxBleExperiment::Graph::setColor(const char* c) { first.setColor(c); if (first.error.set()) data_.error = first.error; }
void PhyphoxBleExperiment::Graph::setLinewidth(float w) { first.setLinewidth(w); }

void PhyphoxBleExperiment::Graph::addSubgraph(Subgraph& s) {
    s.next = nullptr;
    if (!subgraphs) subgraphs = &s; else lastSubgraph->next = &s;
    lastSubgraph = &s;
}

void PhyphoxBleExperiment::Graph::addChannel(int x, int y, const char* color) {
    // 1.x shortcut. Without an object of the caller's to link, the first free curve is the
    // main one; further curves need Subgraph objects (documented).
    if (!first.data.active) { setChannel(x, y); if (color) setColor(color); }
    else data_.error.record(ERR_06_CAPACITY, "addChannel");
}

static void setLayout(GraphData& g, Layout& target, float& value, float v, const char* layout, ErrorRecord& err, const char* origin) {
    Layout l = parseLayout(layout);
    if (l == LAYOUT_NONE) { err.record(ERR_05_INVALID_LAYOUT, origin); return; }
    (void)g; target = l; value = v;
}
void PhyphoxBleExperiment::Graph::setMinX(float v, const char* l) { setLayout(data_.graph, data_.graph.scaleMinX, data_.graph.minX, v, l, data_.error, "setMinX"); }
void PhyphoxBleExperiment::Graph::setMaxX(float v, const char* l) { setLayout(data_.graph, data_.graph.scaleMaxX, data_.graph.maxX, v, l, data_.error, "setMaxX"); }
void PhyphoxBleExperiment::Graph::setMinY(float v, const char* l) { setLayout(data_.graph, data_.graph.scaleMinY, data_.graph.minY, v, l, data_.error, "setMinY"); }
void PhyphoxBleExperiment::Graph::setMaxY(float v, const char* l) { setLayout(data_.graph, data_.graph.scaleMaxY, data_.graph.maxY, v, l, data_.error, "setMaxY"); }

// ---------------------------------------------------------------- Value

void PhyphoxBleExperiment::Value::setPrecision(int p) { data_.value.precision = (int8_t)(p < 0 ? 0 : p); }
void PhyphoxBleExperiment::Value::setUnit(const char* s) { if (checkString(s, data_.error, "setUnit")) data_.value.unit = s; }
void PhyphoxBleExperiment::Value::setColor(const char* c) { if (checkColor(c, data_.error, "setColor")) data_.value.color = c; }
void PhyphoxBleExperiment::Value::setChannel(int c) { if (checkDataChannel(c, data_.error, "setChannel")) data_.value.channel = (uint8_t)c; }
void PhyphoxBleExperiment::Value::setSize(float s) { data_.value.size = s; }
void PhyphoxBleExperiment::Value::setFactor(float f) { data_.value.factor = f; }
void PhyphoxBleExperiment::Value::setScientific(bool b) { data_.value.scientific = b; }
void PhyphoxBleExperiment::Value::addMap(float mn, float mx, const char* text) {
    if (!checkString(text, data_.error, "addMap")) return;
    if (mapCount >= PHYPHOX_BLE_INLINE_OPTIONS) { data_.error.record(ERR_06_CAPACITY, "addMap"); return; }
    MapEntry& m = inlineMaps[mapCount++];
    m.text = text; m.a = mn; m.b = mx; m.flags = MapEntry::HAS_MIN | MapEntry::HAS_MAX;
}
void PhyphoxBleExperiment::Value::addMapBelow(float mx, const char* text) {
    addMap(0, mx, text);
    if (mapCount) inlineMaps[mapCount - 1].flags = MapEntry::HAS_MAX;
}
void PhyphoxBleExperiment::Value::addMapAbove(float mn, const char* text) {
    addMap(mn, 0, text);
    if (mapCount) inlineMaps[mapCount - 1].flags = MapEntry::HAS_MIN;
}
void PhyphoxBleExperiment::Value::setMaps(int n, const float* mins, const float* maxs, const char* const texts[]) {
    if (n < 0 || !texts) { data_.error.record(ERR_08_NULL_STRING, "setMaps"); return; }
    mapsArray = nullptr; mapCount = (uint8_t)(n > 255 ? 255 : n);
    mapsMins = mins; mapsMaxs = maxs; mapsTexts = texts;
}

// ---------------------------------------------------------------- input elements

void PhyphoxBleExperiment::InputElement::setChannel(int c) {
    if (checkInputChannel(c, data_.error, "setChannel")) data_.input()->channel = (uint8_t)c;
}
void PhyphoxBleExperiment::InputElement::setDefault(float v) { data_.input()->defaultValue = v; data_.input()->hasDefault = true; }
void PhyphoxBleExperiment::InputElement::onChange(ChangeCallback cb) { data_.input()->onChange = cb; }

void PhyphoxBleExperiment::Edit::setUnit(const char* s) { if (checkString(s, data_.error, "setUnit")) data_.edit.unit = s; }
void PhyphoxBleExperiment::Edit::setSigned(bool b) { data_.edit.isSigned = b; data_.edit.signedSet = true; }
void PhyphoxBleExperiment::Edit::setDecimal(bool b) { data_.edit.isDecimal = b; data_.edit.decimalSet = true; }
void PhyphoxBleExperiment::Edit::setMin(float v) { data_.edit.min = v; data_.edit.minSet = true; }
void PhyphoxBleExperiment::Edit::setMax(float v) { data_.edit.max = v; data_.edit.maxSet = true; }
void PhyphoxBleExperiment::Edit::setFactor(float f) { data_.edit.factor = f; }
void PhyphoxBleExperiment::Edit::submitWith(Button& b) {
    uint8_t ch = b.data().button.in.channel;
    if (!ch) { data_.error.record(ERR_02_ABOVE_LIMIT, "submitWith"); return; }
    data_.edit.in.submitWithChannel = ch;
}

PhyphoxBleExperiment::Slider::Slider(const char* label, float mn, float mx, float step, int channel) : Slider() {
    setLabel(label); setMinValue(mn); setMaxValue(mx); setStepSize(step);
    if (channel) setChannel(channel);
}
void PhyphoxBleExperiment::Slider::setMinValue(float v) { data_.slider.minValue = v; data_.slider.minSet = true; }
void PhyphoxBleExperiment::Slider::setMaxValue(float v) { data_.slider.maxValue = v; data_.slider.maxSet = true; }
void PhyphoxBleExperiment::Slider::setStepSize(float v) { data_.slider.stepSize = v; data_.slider.stepSet = true; }
void PhyphoxBleExperiment::Slider::setPrecision(int p) { data_.slider.precision = (int8_t)(p < 0 ? 0 : p); }
void PhyphoxBleExperiment::Slider::setShowValue(bool b) { data_.slider.showValue = b; data_.slider.showValueSet = true; }
void PhyphoxBleExperiment::Slider::setColor(const char* c) { if (checkColor(c, data_.error, "setColor")) data_.slider.color = c; }

void PhyphoxBleExperiment::Dropdown::setOptions(int n, const char* const labels[], const float* values) {
    if (n < 0 || !labels || !values) { data_.error.record(ERR_08_NULL_STRING, "setOptions"); return; }
    optionLabels = labels; optionValues = values; optionCount = (uint8_t)(n > 255 ? 255 : n);
}
void PhyphoxBleExperiment::Dropdown::addOption(const char* label, float value) {
    if (!checkString(label, data_.error, "addOption")) return;
    if (optionLabels) { data_.error.record(ERR_04_INVALID_VALUE, "addOption"); return; }   // mixing the two forms
    if (optionCount >= PHYPHOX_BLE_INLINE_OPTIONS) { data_.error.record(ERR_06_CAPACITY, "addOption"); return; }
    MapEntry& m = inlineOptions[optionCount++];
    m.text = label; m.a = value; m.b = 0; m.flags = 0;
}
void PhyphoxBleExperiment::Dropdown::setColor(const char* c) { if (checkColor(c, data_.error, "setColor")) data_.dropdown.color = c; }

void PhyphoxBleExperiment::Button::setValue(float v) { data_.button.value = v; }
void PhyphoxBleExperiment::Button::onPress(PressCallback cb) { data_.button.onPress = cb; }

// ---------------------------------------------------------------- info, separator

void PhyphoxBleExperiment::InfoField::setInfo(const char* s) { if (checkString(s, data_.error, "setInfo")) data_.label = s; }
void PhyphoxBleExperiment::InfoField::setColor(const char* c) { if (checkColor(c, data_.error, "setColor")) data_.info.color = c; }
void PhyphoxBleExperiment::InfoField::setSize(float s) { data_.info.size = s; }
void PhyphoxBleExperiment::InfoField::setAlign(const char* a) {
    Align al = parseAlign(a);
    if (al == ALIGN_NONE) { data_.error.record(ERR_04_INVALID_VALUE, "setAlign"); return; }
    data_.info.align = al;
}
void PhyphoxBleExperiment::InfoField::setBold(bool b) { data_.info.bold = b; }
void PhyphoxBleExperiment::InfoField::setItalic(bool b) { data_.info.italic = b; }

void PhyphoxBleExperiment::Separator::setHeight(float h) { data_.separator.height = h; }
void PhyphoxBleExperiment::Separator::setColor(const char* c) { if (checkColor(c, data_.error, "setColor")) data_.separator.color = c; }

// ---------------------------------------------------------------- View

void PhyphoxBleExperiment::View::setLabel(const char* l) { if (checkString(l, error, "setLabel")) data.label = l; }
void PhyphoxBleExperiment::View::setXMLAttribute(const char* a) { if (checkString(a, error, "setXMLAttribute")) data.xmlAttribute = a; }
PhyphoxBleExperiment::View& PhyphoxBleExperiment::View::addElement(Element& e) {
    if (e.isExportData()) { error.record(ERR_04_INVALID_VALUE, "addElement"); return *this; }
    e.next = nullptr;
    if (!elements) elements = &e; else lastElement->next = &e;
    lastElement = &e;
    return *this;
}

// ---------------------------------------------------------------- export

void PhyphoxBleExperiment::ExportData::setDatachannel(int c) {
    if (checkDataChannel(c, data_.error, "setDatachannel")) exportData.channel = (uint8_t)c;
}
void PhyphoxBleExperiment::ExportData::setLabel(const char* l) { if (checkString(l, data_.error, "setLabel")) exportData.label = l; }

void PhyphoxBleExperiment::ExportSet::setLabel(const char* l) { if (checkString(l, error, "setLabel")) data.label = l; }
void PhyphoxBleExperiment::ExportSet::setXMLAttribute(const char* a) { if (checkString(a, error, "setXMLAttribute")) data.xmlAttribute = a; }
PhyphoxBleExperiment::ExportSet& PhyphoxBleExperiment::ExportSet::addElement(ExportData& d) {
    d.nextEntry = nullptr;
    if (!entries) entries = &d; else lastEntry->nextEntry = &d;
    lastEntry = &d;
    return *this;
}
void PhyphoxBleExperiment::ExportSet::addElement(Element& e) {
    if (!e.isExportData()) { error.record(ERR_04_INVALID_VALUE, "addElement"); return; }
    addElement(static_cast<ExportData&>(e));
}

// ---------------------------------------------------------------- Sensor

void PhyphoxBleExperiment::Sensor::setType(const char* t) {
    if (!validSensorType(t)) { data.error.record(ERR_04_INVALID_VALUE, "setType"); return; }
    data.type = t;
}
void PhyphoxBleExperiment::Sensor::setAverage(bool b) { data.average = b; data.averageSet = true; }
void PhyphoxBleExperiment::Sensor::setRate(int hz) {
    if (hz < 0) { data.error.record(ERR_02_ABOVE_LIMIT, "setRate"); return; }
    data.rate = (uint16_t)(hz > 65535 ? 65535 : hz);
}
void PhyphoxBleExperiment::Sensor::setRateStrategy(const char* s) {
    if (!s || (strcmp(s, "auto") && strcmp(s, "request") && strcmp(s, "generate") && strcmp(s, "limit"))) {
        data.error.record(ERR_04_INVALID_VALUE, "setRateStrategy"); return;
    }
    data.rateStrategy = s;
}
void PhyphoxBleExperiment::Sensor::setStride(int n) { if (n < 0) { data.error.record(ERR_02_ABOVE_LIMIT, "setStride"); return; } data.stride = (uint16_t)n; }
void PhyphoxBleExperiment::Sensor::setNameFilter(const char* s) { if (checkString(s, data.error, "setNameFilter")) data.nameFilter = s; }
void PhyphoxBleExperiment::Sensor::setTypeFilter(int t) { data.typeFilter = t; }
void PhyphoxBleExperiment::Sensor::mapChannel(const char* component, int ch) {
    SensorComponent c = parseComponent(component);
    if (c == COMP_NONE) { data.error.record(ERR_04_INVALID_VALUE, "mapChannel"); return; }
    if (!checkInputChannel(ch, data.error, "mapChannel")) return;
    if (data.componentCount >= PHYPHOX_BLE_SENSOR_COMPONENTS) { data.error.record(ERR_06_CAPACITY, "mapChannel"); return; }
    data.components[data.componentCount] = c;
    data.channels[data.componentCount] = (uint8_t)ch;
    data.componentCount++;
}
void PhyphoxBleExperiment::Sensor::setComponent(const char* component) { mapChannel(component, data.componentCount + 1); }
void PhyphoxBleExperiment::Sensor::setXMLAttribute(const char* a) { if (checkString(a, data.error, "setXMLAttribute")) data.xmlAttribute = a; }

// ---------------------------------------------------------------- the experiment

PhyphoxBleExperiment::PhyphoxBleExperiment(const char* title, const char* category, const char* description) {
    if (title) setTitle(title);
    if (category) setCategory(category);
    if (description) setDescription(description);
}
void PhyphoxBleExperiment::setTitle(const char* s) { if (checkString(s, data.error, "setTitle")) data.title = s; }
void PhyphoxBleExperiment::setCategory(const char* s) { if (checkString(s, data.error, "setCategory")) data.category = s; }
void PhyphoxBleExperiment::setDescription(const char* s) { if (checkString(s, data.error, "setDescription")) data.description = s; }
void PhyphoxBleExperiment::setColor(const char* c) { if (checkColor(c, data.error, "setColor")) data.color = c; }
void PhyphoxBleExperiment::setRepeating(int r) { if (r < 0) { data.error.record(ERR_02_ABOVE_LIMIT, "setRepeating"); return; } data.repeating = (uint16_t)r; }
void PhyphoxBleExperiment::setSubscribeOnStart(bool b) { data.subscribeOnStart = b; }
void PhyphoxBleExperiment::setResendUnchanged(bool b) { data.resendUnchanged = b; }
PhyphoxBleExperiment& PhyphoxBleExperiment::addView(View& v) {
    v.next = nullptr;
    if (!views) views = &v; else lastView->next = &v;
    lastView = &v;
    return *this;
}
PhyphoxBleExperiment& PhyphoxBleExperiment::addSensor(Sensor& s) {
    s.next = nullptr;
    if (!sensors) sensors = &s; else lastSensor->next = &s;
    lastSensor = &s;
    return *this;
}
PhyphoxBleExperiment& PhyphoxBleExperiment::addExportSet(ExportSet& e) {
    e.next = nullptr;
    if (!exportSets) exportSets = &e; else lastExportSet->next = &e;
    lastExportSet = &e;
    return *this;
}

// ---------------------------------------------------------------- ElementData helpers

void ElementData::init(ElementType t) {
    type = t;
    // The types are trivially copyable, so assigning a default-constructed value activates
    // the union member with its defaults without placement new.
    switch (t) {
        case EL_GRAPH: graph = GraphData(); break;
        case EL_VALUE: value = ValueData(); break;
        case EL_EDIT: edit = EditData(); break;
        case EL_SLIDER: slider = SliderData(); break;
        case EL_DROPDOWN: dropdown = DropdownData(); break;
        case EL_TOGGLE: toggle = ToggleData(); break;
        case EL_BUTTON: button = ButtonData(); break;
        case EL_INFO: info = InfoData(); break;
        case EL_SEPARATOR: separator = SeparatorData(); break;
        default: break;
    }
}

InputCommon* ElementData::input() {
    switch (type) {
        case EL_EDIT: return &edit.in;
        case EL_SLIDER: return &slider.in;
        case EL_DROPDOWN: return &dropdown.in;
        case EL_TOGGLE: return &toggle.in;
        case EL_BUTTON: return &button.in;
        default: return nullptr;
    }
}
const InputCommon* ElementData::input() const { return const_cast<ElementData*>(this)->input(); }
