// The plain-data description of an experiment. This is what the sketch's builder classes in
// phyphoxBleExperiment.h fill in, what addExperiment() copies into one exactly-sized block,
// and what the serializer turns into XML — on demand, one part at a time, never as a whole
// document.
//
// Everything is plain data: copying is memcpy, variable-length parts (subgraphs, maps,
// options, elements, …) are counted pointers into that block, and strings are const char*
// pointers to the sketch's literals (docs/concepts.md, "Strings").
#ifndef PHYPHOX_BLE_CORE_ELEMENTDATA_H
#define PHYPHOX_BLE_CORE_ELEMENTDATA_H

#include <stdint.h>
#include "Config.h"
#include "Errors.h"

namespace phyphox {

/// Callback types. Plain function pointers (captureless lambdas convert), never std::function.
typedef void (*ChangeCallback)(float value);
typedef void (*PressCallback)();

enum ElementType : uint8_t {
    EL_NONE = 0, EL_GRAPH, EL_VALUE, EL_EDIT, EL_SLIDER, EL_DROPDOWN, EL_TOGGLE, EL_BUTTON,
    EL_INFO, EL_SEPARATOR
};

enum Style : uint8_t { STYLE_NONE = 0, ST_LINES, ST_DOTS, ST_VBARS, ST_HBARS, ST_MAP };
enum Layout : uint8_t { LAYOUT_NONE = 0, LY_AUTO, LY_EXTEND, LY_FIXED };
enum Align : uint8_t { ALIGN_NONE = 0, AL_LEFT, AL_RIGHT, AL_CENTER };

/// A colour is six hex digits, validated by the setter; nullptr means "not set".
typedef const char* Color;

/// One curve of a graph. Channel 0 is the phone's arrival time, 1…5 the data channels.
struct SubgraphData {
    uint8_t channelX = 0;
    uint8_t channelY = 1;
    Style style = STYLE_NONE;
    Color color = nullptr;
    float lineWidth = 0;         ///< 0 = not set
    bool active = false;
};

struct GraphData {
    const char* labelX = nullptr;
    const char* labelY = nullptr;
    const char* unitX = nullptr;
    const char* unitY = nullptr;
    const char* unitYperX = nullptr;
    int8_t xPrecision = -1;      ///< -1 = not set
    int8_t yPrecision = -1;
    float minX = 0, maxX = 0, minY = 0, maxY = 0;
    Layout scaleMinX = LAYOUT_NONE, scaleMaxX = LAYOUT_NONE, scaleMinY = LAYOUT_NONE, scaleMaxY = LAYOUT_NONE;
    float aspectRatio = 0;       ///< 0 = not set
    // boolean attributes, tri-state: bit set in `flagsSet` means "emit", bit in `flags` the value
    enum Flag : uint16_t { F_TIME_ON_X = 1, F_TIME_ON_Y = 2, F_SYSTEM_TIME = 4, F_LINEAR_TIME = 8,
                           F_LOG_X = 16, F_LOG_Y = 32, F_FOLLOW_X = 64, F_PARTIAL_UPDATE = 128,
                           F_SHOW_COLOR_SCALE = 256, F_HIDE_TIME_MARKERS = 512, F_SUPPRESS_SCI = 1024 };
    uint16_t flags = 0;
    uint16_t flagsSet = 0;
    const SubgraphData* subgraphs = nullptr;  ///< the first curve, then the added ones
    uint8_t subgraphCount = 0;
};

/// A value map entry (text shown while min <= value <= max) or a dropdown option (text +
/// value). Kept to 16 bytes on 32-bit targets: `a` is the map's min or the option's value,
/// `b` the map's max; `flags` says which bounds apply.
struct MapEntry {
    const char* text = nullptr;
    float a = 0;
    float b = 0;
    enum : uint8_t { HAS_MIN = 1, HAS_MAX = 2 };
    uint8_t flags = 0;
};

struct ValueData {
    uint8_t channel = 1;         ///< data channel shown (1.x default was CH3; 2.0 defaults to 1)
    const char* unit = nullptr;
    Color color = nullptr;
    int8_t precision = -1;
    float size = 0;
    float factor = 0;            ///< 0 = not set (format default 1.0)
    bool scientific = false;
    const MapEntry* maps = nullptr;
    uint8_t mapCount = 0;
};

/// Common part of every element that writes into an input channel (edit, slider, dropdown,
/// toggle, button). Shared so the store can treat them alike.
struct InputCommon {
    uint8_t channel = 0;         ///< 1…PHYPHOX_BLE_MAX_INPUT_CHANNEL; 0 = not set (ERR at add time)
    float defaultValue = 0;
    bool hasDefault = false;
    ChangeCallback onChange = nullptr;
    uint8_t submitWithChannel = 0; ///< a button's channel: send only when it is pressed (triggerId)
};

struct EditData {
    InputCommon in;
    const char* unit = nullptr;
    bool isSigned = true, isDecimal = true;
    bool signedSet = false, decimalSet = false;
    float min = 0, max = 0; bool minSet = false, maxSet = false;
    float factor = 0;
};

struct SliderData {
    InputCommon in;
    float minValue = 0, maxValue = 1, stepSize = 1;
    bool minSet = false, maxSet = false, stepSet = false;
    int8_t precision = -1;
    bool showValue = true, showValueSet = false;
    Color color = nullptr;
};

struct DropdownData {
    InputCommon in;
    const MapEntry* options = nullptr;
    uint8_t optionCount = 0;
    Color color = nullptr;
};

struct ToggleData {
    InputCommon in;
};

struct ButtonData {
    InputCommon in;              ///< channel receives `value` on every press; onChange unused
    float value = 1;             ///< the constant written on press
    PressCallback onPress = nullptr;
};

struct InfoData {
    Color color = nullptr;
    float size = 0;
    Align align = ALIGN_NONE;
    bool bold = false, italic = false;
};

struct SeparatorData {
    float height = 0;            ///< 0 = not set (format default 0.1)
    Color color = nullptr;
};

/// One view element. `label` and `xmlAttribute` are common to all; the union holds the rest.
struct ElementData {
    ElementType type = EL_NONE;
    const char* label = nullptr;
    const char* xmlAttribute = nullptr;   ///< raw attribute text appended verbatim (escape hatch)
    uint8_t visibilityChannel = 0;        ///< input channel whose last value shows/hides the element
    ErrorRecord error;
    union {
        GraphData graph;
        ValueData value;
        EditData edit;
        SliderData slider;
        DropdownData dropdown;
        ToggleData toggle;
        ButtonData button;
        InfoData info;
        SeparatorData separator;
    };
    ElementData() : graph() {}
    /// The input-channel part, if this element has one; nullptr otherwise.
    InputCommon* input();
    const InputCommon* input() const;
};

struct ViewData {
    const char* label = nullptr;
    const char* xmlAttribute = nullptr;
    const ElementData* elements = nullptr;
    uint8_t elementCount = 0;
};

struct ExportDataData {
    const char* label = nullptr;
    const char* xmlAttribute = nullptr;
    uint8_t channel = 1;         ///< data channel (0 = time)
};

struct ExportSetData {
    const char* label = nullptr;
    const char* xmlAttribute = nullptr;
    const ExportDataData* entries = nullptr;
    uint8_t entryCount = 0;
};

enum SensorComponent : uint8_t { COMP_NONE = 0, COMP_X, COMP_Y, COMP_Z, COMP_ABS, COMP_ACCURACY, COMP_T };

struct SensorData {
    const char* type = nullptr;  ///< a phyphox sensor type string (SENSOR_* in phyphoxBle.h)
    const char* xmlAttribute = nullptr;
    uint16_t rate = 0;           ///< 0 = not set (1.x emitted 80 by default; 2.0 emits nothing = fastest)
    bool average = false, averageSet = false;
    const char* rateStrategy = nullptr;  ///< auto | request | generate | limit (format 1.14)
    uint16_t stride = 0;         ///< use every n-th reading; 0 = not set
    const char* nameFilter = nullptr;    ///< for type "custom" or to pick one of several sensors (1.19)
    int32_t typeFilter = -1;     ///< Android sensor type constant; −1 = not set (1.19)
    SensorComponent components[PHYPHOX_BLE_SENSOR_COMPONENTS];
    uint8_t channels[PHYPHOX_BLE_SENSOR_COMPONENTS];  ///< input channel each component lands in
    uint8_t componentCount = 0;
    ErrorRecord error;
};

/// The whole experiment. The experiment-level fields are filled by PhyphoxBleExperiment (the
/// builder); the counted arrays point into the block ExperimentStore allocates at
/// PhyphoxBLE::addExperiment().
struct ExperimentData {
    const char* title = nullptr;
    const char* category = nullptr;
    const char* description = nullptr;
    Color color = nullptr;
    uint16_t repeating = 0;      ///< samples per data notification for the array write (0 = off)
    bool subscribeOnStart = false;
    bool resendUnchanged = false; ///< D14: drop the change detector, write inputs every cycle
    const ViewData* views = nullptr;
    uint8_t viewCount = 0;
    const ExportSetData* exportSets = nullptr;
    uint8_t exportSetCount = 0;
    const SensorData* sensors = nullptr;
    uint8_t sensorCount = 0;
    ErrorRecord error;           ///< experiment-level errors (capacity, channel conflicts)
};

} // namespace phyphox

#endif
