#include "Serializer.h"
#include <string.h>

namespace phyphox {

// ---------------------------------------------------------------- text helpers

void writeEscaped(Sink& s, const char* text) {
    if (!text) return;
    for (const char* p = text; *p; ++p) {
        switch (*p) {
            case '&': s.write("&amp;"); break;
            case '<': s.write("&lt;"); break;
            case '>': s.write("&gt;"); break;
            case '"': s.write("&quot;"); break;
            default: s.write(*p);
        }
    }
}

void writeInt(Sink& s, int32_t v) {
    char buf[12];
    int i = 11;
    buf[i] = 0;
    uint32_t u = v < 0 ? (uint32_t)(-(int64_t)v) : (uint32_t)v;
    do { buf[--i] = (char)('0' + u % 10); u /= 10; } while (u);
    if (v < 0) buf[--i] = '-';
    s.write(buf + i);
}

static void writeUnsigned(Sink& s, uint32_t u) {
    char buf[12];
    int i = 11;
    buf[i] = 0;
    do { buf[--i] = (char)('0' + u % 10); u /= 10; } while (u);
    s.write(buf + i);
}

/// Fixed notation with up to 6 decimals (trailing zeros trimmed) for the range sketches use,
/// scientific with 6 significant digits outside it. No locale, no printf.
void writeFloat(Sink& s, float v) {
    if (v != v) { s.write("0"); return; }                 // NaN: the format has no spelling for it
    if (v < 0) { s.write('-'); v = -v; }
    if (v > 3.0e38f) { s.write("1e38"); return; }          // ±inf
    double d = v;
    if (v != 0 && (v >= 1e9f || v < 1e-4f)) {   // range test in float, so 1e-4f is still "0.0001"
        int exp = 0;
        while (d >= 10) { d /= 10; exp++; }
        while (d < 1) { d *= 10; exp--; }
        uint32_t m = (uint32_t)(d * 100000 + 0.5);         // 6 significant digits
        if (m >= 1000000) { m /= 10; exp++; }
        writeUnsigned(s, m / 100000);
        uint32_t frac = m % 100000;
        if (frac) {
            char buf[7]; int n = 5;
            for (int i = 4; i >= 0; --i) { buf[i] = (char)('0' + frac % 10); frac /= 10; }
            while (n > 0 && buf[n - 1] == '0') n--;
            buf[n] = 0;
            s.write('.'); s.write(buf);
        }
        s.write('e'); writeInt(s, exp);
        return;
    }
    uint32_t ip = (uint32_t)d;
    uint32_t frac = (uint32_t)((d - ip) * 1000000 + 0.5);
    if (frac >= 1000000) { ip++; frac -= 1000000; }
    writeUnsigned(s, ip);
    if (frac) {
        char buf[8]; int n = 6;
        for (int i = 5; i >= 0; --i) { buf[i] = (char)('0' + frac % 10); frac /= 10; }
        while (n > 0 && buf[n - 1] == '0') n--;
        buf[n] = 0;
        s.write('.'); s.write(buf);
    }
}

static const char* const kUuidSuffix = "-30f7-4671-8b43-5e40ba53514a";
static void writeHex2(Sink& s, uint8_t v) {
    const char* hex = "0123456789abcdef";
    s.write(hex[v >> 4]); s.write(hex[v & 15]);
}
void writeInputCharUuid(Sink& s, uint8_t channel) { s.write("cddf20"); writeHex2(s, channel); s.write(kUuidSuffix); }
void writeSensorCharUuid(Sink& s, uint8_t sensor) { s.write("cddf30"); writeHex2(s, sensor); s.write(kUuidSuffix); }
static const char* const kDataCharUuid = "cddf1002-30f7-4671-8b43-5e40ba53514a";

// ---------------------------------------------------------------- attribute helpers
namespace {

void attr(Sink& s, const char* name, const char* value) {
    if (!value) return;
    s.write(' '); s.write(name); s.write("=\""); writeEscaped(s, value); s.write('"');
}
void attrRaw(Sink& s, const char* name, const char* value) {   // value known to be safe
    s.write(' '); s.write(name); s.write("=\""); s.write(value); s.write('"');
}
void attrF(Sink& s, const char* name, float v) { s.write(' '); s.write(name); s.write("=\""); writeFloat(s, v); s.write('"'); }
void attrI(Sink& s, const char* name, int32_t v) { s.write(' '); s.write(name); s.write("=\""); writeInt(s, v); s.write('"'); }
void attrB(Sink& s, const char* name, bool v) { attrRaw(s, name, v ? "true" : "false"); }
void attrFlag(Sink& s, const GraphData& g, uint16_t flag, const char* name) {
    if (g.flagsSet & flag) attrB(s, name, (g.flags & flag) != 0);
}
const char* styleName(Style st) {
    switch (st) { case ST_LINES: return "lines"; case ST_DOTS: return "dots"; case ST_VBARS: return "vbars";
                  case ST_HBARS: return "hbars"; case ST_MAP: return "map"; default: return nullptr; }
}
const char* layoutName(Layout l) {
    switch (l) { case LY_AUTO: return "auto"; case LY_EXTEND: return "extend"; case LY_FIXED: return "fixed"; default: return nullptr; }
}
const char* alignName(Align a) {
    switch (a) { case AL_LEFT: return "left"; case AL_RIGHT: return "right"; case AL_CENTER: return "center"; default: return nullptr; }
}
const char* componentName(SensorComponent c) {
    switch (c) { case COMP_X: return "x"; case COMP_Y: return "y"; case COMP_Z: return "z"; case COMP_ABS: return "abs";
                 case COMP_ACCURACY: return "accuracy"; case COMP_T: return "t"; default: return "x"; }
}
void bufIn(Sink& s, uint8_t k) { s.write("in"); writeInt(s, k); }
void bufLast(Sink& s, uint8_t k) { s.write("last"); writeInt(s, k); }
void bufSend(Sink& s, uint8_t k) { s.write("send"); writeInt(s, k); }
void bufCh(Sink& s, uint8_t c) { s.write("CH"); writeInt(s, c); }
void bufSensor(Sink& s, uint8_t sensor, SensorComponent c) { s.write("sen"); writeInt(s, sensor + 1); s.write(componentName(c)); }
void visibility(Sink& s, const ElementData& e) {
    if (e.visibilityChannel) { s.write(" visibility=\""); bufIn(s, e.visibilityChannel); s.write('"'); }
}
void rawAttribute(Sink& s, const char* xml) { if (xml) { s.write(' '); s.write(xml); } }

/// Does a channel get the change detector? Buttons and sensor-fed channels do not; nor does
/// anything when the experiment asks for 1.x resends.
bool detector(const ExperimentStore& st, uint8_t k) {
    const InputChannelInfo& ci = st.input(k);
    return ci.used && !ci.isButton && !ci.fromSensor && !st.data().resendUnchanged;
}
/// The trigger id an edit uses to submit with button channel `k`, if any edit does.
bool buttonTriggers(const ExperimentStore& st, uint8_t k) {
    for (uint8_t j = 1; j <= st.inputChannelsUsed(); ++j)
        if (st.input(j).used && st.input(j).submitWithChannel == k) return true;
    return false;
}
void triggerId(Sink& s, uint8_t buttonChannel) { s.write("btn"); writeInt(s, buttonChannel); }

} // namespace

// ---------------------------------------------------------------- build and access

void Serializer::build(const ExperimentStore& store, const char* deviceName, uint16_t mtu) {
    store_ = &store;
    deviceName_ = deviceName ? deviceName : "phyphox-Arduino";
    mtu_ = mtu;
    partCount_ = (uint16_t)(store.data().viewCount + 2);
    partOffset_ = store.partTable();
    CountingSink cs;
    for (uint16_t p = 0; p < partCount_; ++p) {
        partOffset_[p] = (uint16_t)cs.count();
        emitPart(p, cs);
    }
    partOffset_[partCount_] = (uint16_t)cs.count();
    totalSize_ = (uint32_t)cs.count();
    crc_ = cs.crc();
}

void Serializer::emitPart(uint16_t part, Sink& sink) const {
    if (!store_) return;
    if (part == 0) emitPrologue(sink);
    else if (part <= store_->data().viewCount) emitView((uint8_t)(part - 1), sink);
    else emitEpilogue(sink);
}

void Serializer::writeAll(Sink& sink) const {
    for (uint16_t p = 0; p < partCount_; ++p) emitPart(p, sink);
}

uint32_t Serializer::read(uint32_t offset, uint8_t* out, uint32_t n) const {
    if (!store_ || offset >= totalSize_) return 0;
    if (offset + n > totalSize_) n = totalSize_ - offset;
    uint32_t produced = 0;
    for (uint16_t p = 0; p < partCount_ && produced < n; ++p) {
        uint32_t start = partOffset_[p], end = partOffset_[p + 1];
        uint32_t want = offset + produced;
        if (want >= end) continue;
        WindowSink w(out + produced, n - produced, want - start);
        emitPart(p, w);
        produced += (uint32_t)w.produced();
    }
    return produced;
}

// ---------------------------------------------------------------- prologue

void Serializer::emitPrologue(Sink& s) const {
    const ExperimentData& d = store_->data();
    s.write("<phyphox version=\"" PHYPHOX_BLE_FORMAT_VERSION "\">\n");
    s.write("<title>"); writeEscaped(s, d.title ? d.title : "Arduino-Experiment"); s.write("</title>\n");
    s.write("<category>"); writeEscaped(s, d.category ? d.category : "Arduino Experiments"); s.write("</category>\n");
    if (d.color) { s.write("<color>"); s.write(d.color); s.write("</color>\n"); }
    s.write("<description>");
    writeEscaped(s, d.description ? d.description : "An experiment created with the phyphox BLE library for Arduino-compatible micro controllers.");
    s.write("</description>\n");

    // data containers
    s.write("<data-containers>\n");
    for (uint8_t c = 0; c <= PHYPHOX_BLE_DATA_CHANNELS; ++c) {
        s.write("\t<container size=\"0\">"); bufCh(s, c); s.write("</container>\n");
    }
    for (uint8_t k = 1; k <= store_->inputChannelsUsed(); ++k) {
        const InputChannelInfo& ci = store_->input(k);
        if (!ci.used || ci.fromSensor) continue;
        s.write("\t<container size=\"1\"");
        if (ci.hasDefault && !ci.isButton) attrF(s, "init", ci.defaultValue);
        s.write('>'); bufIn(s, k); s.write("</container>\n");
        if (detector(*store_, k)) {
            s.write("\t<container size=\"1\"");
            if (ci.hasDefault) attrF(s, "init", ci.defaultValue);
            s.write('>'); bufLast(s, k); s.write("</container>\n");
            s.write("\t<container size=\"1\">"); bufSend(s, k); s.write("</container>\n");
        }
    }
    for (uint8_t i = 0; i < d.sensorCount; ++i)
        for (uint8_t c = 0; c < d.sensors[i].componentCount; ++c) {
            s.write("\t<container size=\"1\">"); bufSensor(s, i, d.sensors[i].components[c]); s.write("</container>\n");
        }
    s.write("</data-containers>\n");

    // input: the board's data and the phone's sensors
    s.write("<input>\n\t<bluetooth");
    attr(s, "name", deviceName_);
    attrRaw(s, "id", "phyphoxBLE");
    attrRaw(s, "mode", "notification");
    attrB(s, "subscribeOnStart", d.subscribeOnStart);
    if (mtu_ > PHYPHOX_BLE_DEFAULT_MTU) attrI(s, "mtu", mtu_ + 3);
    s.write(">\n");
    for (uint8_t c = 1; c <= PHYPHOX_BLE_DATA_CHANNELS; ++c) {
        s.write("\t\t<output"); attrRaw(s, "char", kDataCharUuid); attrRaw(s, "conversion", "float32LittleEndian");
        attrI(s, "offset", (c - 1) * 4);
        if (d.repeating > 0) attrI(s, "repeating", d.repeating);
        s.write('>'); bufCh(s, c); s.write("</output>\n");
    }
    s.write("\t\t<output"); attrRaw(s, "char", kDataCharUuid); attrRaw(s, "extra", "time"); s.write(">CH0</output>\n");
    s.write("\t</bluetooth>\n");
    for (uint8_t i = 0; i < d.sensorCount; ++i) {
        const SensorData& sd = d.sensors[i];
        s.write("\t<sensor"); attr(s, "type", sd.type ? sd.type : "accelerometer");
        if (sd.rate) attrI(s, "rate", sd.rate);
        if (sd.averageSet) attrB(s, "average", sd.average);
        if (sd.rateStrategy) attrRaw(s, "rateStrategy", sd.rateStrategy);
        if (sd.stride) attrI(s, "stride", sd.stride);
        if (sd.nameFilter) attr(s, "nameFilter", sd.nameFilter);
        if (sd.typeFilter >= 0) attrI(s, "typeFilter", sd.typeFilter);
        rawAttribute(s, sd.xmlAttribute);
        s.write(">\n");
        for (uint8_t c = 0; c < sd.componentCount; ++c) {
            s.write("\t\t<output"); attrRaw(s, "component", componentName(sd.components[c])); s.write('>');
            bufSensor(s, i, sd.components[c]); s.write("</output>\n");
        }
        s.write("\t</sensor>\n");
    }
    s.write("</input>\n");

    // output: what the phone writes to the board
    s.write("<output>\n\t<bluetooth"); attrRaw(s, "id", "phyphoxBLE"); attr(s, "name", deviceName_); s.write(">\n");
    for (uint8_t k = 1; k <= store_->inputChannelsUsed(); ++k) {
        const InputChannelInfo& ci = store_->input(k);
        if (!ci.used || ci.fromSensor) continue;
        s.write("\t\t<input char=\""); writeInputCharUuid(s, k); s.write('"');
        attrRaw(s, "conversion", "float32LittleEndian");
        bool det = detector(*store_, k);
        if (det || ci.isButton) attrB(s, "keep", false);
        if (ci.submitWithChannel) { s.write(" triggerId=\""); triggerId(s, ci.submitWithChannel); s.write('"'); }
        s.write('>');
        if (det) bufSend(s, k); else bufIn(s, k);
        s.write("</input>\n");
    }
    for (uint8_t i = 0; i < d.sensorCount; ++i) {
        const SensorData& sd = d.sensors[i];
        for (uint8_t c = 0; c < sd.componentCount; ++c) {
            s.write("\t\t<input char=\""); writeSensorCharUuid(s, i + 1); s.write('"');
            attrRaw(s, "conversion", "float32LittleEndian"); attrI(s, "offset", c * 4); s.write('>');
            bufSensor(s, i, sd.components[c]); s.write("</input>\n");
        }
    }
    s.write("\t</bluetooth>\n</output>\n");

    // analysis: the change detector per input channel. No onUserInput: the analysis runs at
    // its default rate anyway (10 ms is irrelevant for user input), and the attribute would
    // not help sensor data the phone sends (maintainer, 2026-09-13).
    // keep="true" on every input: an analysis input defaults to keep="false" in the format,
    // i.e. the module EMPTIES the container after reading it (spec/analysis.yml, common input
    // attributes). Without it the edit's own buffer and the "last sent" buffer vanished every
    // cycle — found on both phones on the first phase-5 run (2026-09-13).
    s.write("<analysis sleep=\"0\">\n");
    for (uint8_t k = 1; k <= store_->inputChannelsUsed(); ++k) {
        if (!detector(*store_, k)) continue;
        s.write("\t<if less=\"true\" greater=\"true\">\n\t\t<input as=\"a\" keep=\"true\">"); bufIn(s, k);
        s.write("</input>\n\t\t<input as=\"b\" keep=\"true\">"); bufLast(s, k);
        s.write("</input>\n\t\t<input as=\"true\" keep=\"true\">"); bufIn(s, k);
        s.write("</input>\n\t\t<output as=\"result\">"); bufSend(s, k); s.write("</output>\n\t</if>\n");
        s.write("\t<append>\n\t\t<input as=\"in\" keep=\"true\">"); bufIn(s, k);
        s.write("</input>\n\t\t<output as=\"out\">"); bufLast(s, k); s.write("</output>\n\t</append>\n");
    }
    s.write("</analysis>\n<views>\n");
}

// ---------------------------------------------------------------- views and elements

void Serializer::emitView(uint8_t view, Sink& s) const {
    const ViewData& v = store_->data().views[view];
    s.write("\t<view"); attr(s, "label", v.label ? v.label : "View"); rawAttribute(s, v.xmlAttribute); s.write(">\n");
    for (uint8_t i = 0; i < v.elementCount; ++i) emitElement(v.elements[i], s);
    s.write("\t</view>\n");
}

static void emitOutputBuffer(Sink& s, uint8_t k) { s.write("\t\t\t<output>"); bufIn(s, k); s.write("</output>\n"); }

void Serializer::emitElement(const ElementData& e, Sink& s) const {
    switch (e.type) {
        case EL_GRAPH: {
            const GraphData& g = e.graph;
            s.write("\t\t<graph"); attr(s, "label", e.label);
            attr(s, "labelX", g.labelX); attr(s, "labelY", g.labelY);
            attr(s, "unitX", g.unitX); attr(s, "unitY", g.unitY); attr(s, "unitYperX", g.unitYperX);
            if (g.xPrecision >= 0) attrI(s, "xPrecision", g.xPrecision);
            if (g.yPrecision >= 0) attrI(s, "yPrecision", g.yPrecision);
            if (g.scaleMinX) { attrRaw(s, "scaleMinX", layoutName(g.scaleMinX)); attrF(s, "minX", g.minX); }
            if (g.scaleMaxX) { attrRaw(s, "scaleMaxX", layoutName(g.scaleMaxX)); attrF(s, "maxX", g.maxX); }
            if (g.scaleMinY) { attrRaw(s, "scaleMinY", layoutName(g.scaleMinY)); attrF(s, "minY", g.minY); }
            if (g.scaleMaxY) { attrRaw(s, "scaleMaxY", layoutName(g.scaleMaxY)); attrF(s, "maxY", g.maxY); }
            attrFlag(s, g, GraphData::F_TIME_ON_X, "timeOnX"); attrFlag(s, g, GraphData::F_TIME_ON_Y, "timeOnY");
            attrFlag(s, g, GraphData::F_SYSTEM_TIME, "systemTime"); attrFlag(s, g, GraphData::F_LINEAR_TIME, "linearTime");
            attrFlag(s, g, GraphData::F_LOG_X, "logX"); attrFlag(s, g, GraphData::F_LOG_Y, "logY");
            attrFlag(s, g, GraphData::F_FOLLOW_X, "followX"); attrFlag(s, g, GraphData::F_PARTIAL_UPDATE, "partialUpdate");
            attrFlag(s, g, GraphData::F_SHOW_COLOR_SCALE, "showColorScale"); attrFlag(s, g, GraphData::F_HIDE_TIME_MARKERS, "hideTimeMarkers");
            attrFlag(s, g, GraphData::F_SUPPRESS_SCI, "suppressScientificNotation");
            if (g.aspectRatio > 0) attrF(s, "aspectRatio", g.aspectRatio);
            visibility(s, e); rawAttribute(s, e.xmlAttribute);
            s.write(">\n");
            for (uint8_t i = 0; i < g.subgraphCount; ++i) {
                const SubgraphData& c = g.subgraphs[i];
                s.write("\t\t\t<input axis=\"x\">"); bufCh(s, c.channelX); s.write("</input>\n");
                s.write("\t\t\t<input axis=\"y\"");
                if (c.color) attrRaw(s, "color", c.color);
                if (c.style) attrRaw(s, "style", styleName(c.style));
                if (c.lineWidth > 0) attrF(s, "lineWidth", c.lineWidth);
                s.write('>'); bufCh(s, c.channelY); s.write("</input>\n");
            }
            s.write("\t\t</graph>\n");
            break;
        }
        case EL_VALUE: {
            const ValueData& v = e.value;
            s.write("\t\t<value"); attr(s, "label", e.label);
            if (v.precision >= 0) attrI(s, "precision", v.precision);
            attr(s, "unit", v.unit);
            if (v.color) attrRaw(s, "color", v.color);
            if (v.size > 0) attrF(s, "size", v.size);
            if (v.factor != 0) attrF(s, "factor", v.factor);
            if (v.scientific) attrB(s, "scientific", true);
            visibility(s, e); rawAttribute(s, e.xmlAttribute);
            s.write(">\n\t\t\t<input>"); bufCh(s, v.channel); s.write("</input>\n");
            for (uint8_t i = 0; i < v.mapCount; ++i) {
                const MapEntry& m = v.maps[i];
                s.write("\t\t\t<map");
                if (m.flags & MapEntry::HAS_MIN) attrF(s, "min", m.a);
                if (m.flags & MapEntry::HAS_MAX) attrF(s, "max", m.b);
                s.write('>'); writeEscaped(s, m.text); s.write("</map>\n");
            }
            s.write("\t\t</value>\n");
            break;
        }
        case EL_EDIT: {
            const EditData& d = e.edit;
            s.write("\t\t<edit"); attr(s, "label", e.label);
            if (d.in.hasDefault) attrF(s, "default", d.in.defaultValue);
            if (d.signedSet) attrB(s, "signed", d.isSigned);
            if (d.decimalSet) attrB(s, "decimal", d.isDecimal);
            if (d.minSet) attrF(s, "min", d.min);
            if (d.maxSet) attrF(s, "max", d.max);
            attr(s, "unit", d.unit);
            if (d.factor != 0) attrF(s, "factor", d.factor);
            visibility(s, e); rawAttribute(s, e.xmlAttribute);
            s.write(">\n"); emitOutputBuffer(s, d.in.channel); s.write("\t\t</edit>\n");
            break;
        }
        case EL_SLIDER: {
            const SliderData& d = e.slider;
            s.write("\t\t<slider"); attr(s, "label", e.label);
            if (d.in.hasDefault) attrF(s, "default", d.in.defaultValue);
            if (d.minSet) attrF(s, "minValue", d.minValue);
            if (d.maxSet) attrF(s, "maxValue", d.maxValue);
            if (d.stepSet) attrF(s, "stepSize", d.stepSize);
            if (d.precision >= 0) attrI(s, "precision", d.precision);
            if (d.showValueSet) attrB(s, "showValue", d.showValue);
            if (d.color) attrRaw(s, "color", d.color);
            visibility(s, e); rawAttribute(s, e.xmlAttribute);
            s.write(">\n"); emitOutputBuffer(s, d.in.channel); s.write("\t\t</slider>\n");
            break;
        }
        case EL_DROPDOWN: {
            const DropdownData& d = e.dropdown;
            s.write("\t\t<dropdown"); attr(s, "label", e.label);
            if (d.in.hasDefault) attrF(s, "default", d.in.defaultValue);
            if (d.color) attrRaw(s, "color", d.color);
            visibility(s, e); rawAttribute(s, e.xmlAttribute);
            s.write(">\n"); emitOutputBuffer(s, d.in.channel);
            for (uint8_t i = 0; i < d.optionCount; ++i) {
                s.write("\t\t\t<map"); attrF(s, "value", d.options[i].a); s.write('>');
                writeEscaped(s, d.options[i].text); s.write("</map>\n");
            }
            s.write("\t\t</dropdown>\n");
            break;
        }
        case EL_TOGGLE: {
            const ToggleData& d = e.toggle;
            s.write("\t\t<toggle"); attr(s, "label", e.label);
            if (d.in.hasDefault) attrF(s, "default", d.in.defaultValue);
            visibility(s, e); rawAttribute(s, e.xmlAttribute);
            s.write(">\n"); emitOutputBuffer(s, d.in.channel); s.write("\t\t</toggle>\n");
            break;
        }
        case EL_BUTTON: {
            const ButtonData& d = e.button;
            s.write("\t\t<button"); attr(s, "label", e.label);
            visibility(s, e); rawAttribute(s, e.xmlAttribute);
            s.write(">\n\t\t\t<input type=\"value\">"); writeFloat(s, d.value); s.write("</input>\n");
            emitOutputBuffer(s, d.in.channel);
            if (buttonTriggers(*store_, d.in.channel)) { s.write("\t\t\t<trigger>"); triggerId(s, d.in.channel); s.write("</trigger>\n"); }
            s.write("\t\t</button>\n");
            break;
        }
        case EL_INFO: {
            const InfoData& d = e.info;
            s.write("\t\t<info"); attr(s, "label", e.label ? e.label : "");
            if (d.color) attrRaw(s, "color", d.color);
            if (d.size > 0) attrF(s, "size", d.size);
            if (d.align) attrRaw(s, "align", alignName(d.align));
            if (d.bold) attrB(s, "bold", true);
            if (d.italic) attrB(s, "italic", true);
            visibility(s, e); rawAttribute(s, e.xmlAttribute);
            s.write("></info>\n");
            break;
        }
        case EL_SEPARATOR: {
            const SeparatorData& d = e.separator;
            s.write("\t\t<separator");
            if (d.height > 0) attrF(s, "height", d.height);
            if (d.color) attrRaw(s, "color", d.color);
            visibility(s, e); rawAttribute(s, e.xmlAttribute);
            s.write("></separator>\n");
            break;
        }
        default: break;
    }
}

// ---------------------------------------------------------------- epilogue

static void emitError(Sink& s, const ErrorRecord& err) {
    s.write("\t\t<info label=\"ERROR FOUND: "); writeEscaped(s, errorText(err.code));
    if (err.origin) { s.write(", in "); writeEscaped(s, err.origin); s.write("()"); }
    s.write("\" color=\"ff0000\"></info>\n");
}

void Serializer::emitEpilogue(Sink& s) const {
    const ExperimentData& d = store_->data();
    if (store_->hasErrors()) {
        s.write("\t<view label=\"ERRORS\">\n");
        if (d.error.set()) emitError(s, d.error);
        for (uint8_t v = 0; v < d.viewCount; ++v)
            for (uint8_t i = 0; i < d.views[v].elementCount; ++i)
                if (d.views[v].elements[i].error.set()) emitError(s, d.views[v].elements[i].error);
        for (uint8_t i = 0; i < d.sensorCount; ++i) if (d.sensors[i].error.set()) emitError(s, d.sensors[i].error);
        s.write("\t\t<info label=\"Please check the documentation for more information about errors.\"></info>\n");
        s.write("\t</view>\n");
    }
    s.write("</views>\n<export>\n");
    if (d.exportSetCount == 0) {
        s.write("\t<set name=\"mySet\">\n");
        for (uint8_t c = 0; c <= PHYPHOX_BLE_DATA_CHANNELS; ++c) {
            s.write("\t\t<data name=\"myData"); writeInt(s, c); s.write("\">"); bufCh(s, c); s.write("</data>\n");
        }
        s.write("\t</set>\n");
    } else {
        for (uint8_t i = 0; i < d.exportSetCount; ++i) {
            const ExportSetData& set = d.exportSets[i];
            s.write("\t<set"); attr(s, "name", set.label ? set.label : "Data"); rawAttribute(s, set.xmlAttribute); s.write(">\n");
            for (uint8_t j = 0; j < set.entryCount; ++j) {
                const ExportDataData& e = set.entries[j];
                s.write("\t\t<data"); attr(s, "name", e.label ? e.label : "Value"); rawAttribute(s, e.xmlAttribute); s.write('>');
                bufCh(s, e.channel); s.write("</data>\n");
            }
            s.write("\t</set>\n");
        }
    }
    s.write("</export>\n</phyphox>\n");
}

} // namespace phyphox
