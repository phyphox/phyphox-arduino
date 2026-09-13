// Configuration errors. A setter that receives something the format cannot express records the
// first error per element; the generated experiment then shows every error as a red info
// element (the one channel a user without a serial monitor has), and PhyphoxBLE::printErrors()
// lists them for those with one. Codes keep the 1.x numbering the README documents.
#ifndef PHYPHOX_BLE_CORE_ERRORS_H
#define PHYPHOX_BLE_CORE_ERRORS_H

#include <stdint.h>

namespace phyphox {

enum ErrorCode : uint8_t {
    ERR_NONE = 0,
    ERR_01_TOO_LONG = 1,        ///< a string exceeds PHYPHOX_BLE_MAX_STRING_LENGTH
    ERR_02_ABOVE_LIMIT = 2,     ///< a number exceeds its upper limit (channel, rate, count)
    ERR_03_NOT_HEX_COLOR = 3,   ///< a colour is not six hexadecimal digits
    ERR_04_INVALID_VALUE = 4,   ///< a style, component, sensor type or option is not one the format has
    ERR_05_INVALID_LAYOUT = 5,  ///< a layout is not auto, extend or fixed
    // new in 2.0
    ERR_06_CAPACITY = 6,        ///< a pool is full (raise the PHYPHOX_BLE_MAX_* constant)
    ERR_07_CHANNEL_CONFLICT = 7,///< a button shares an input channel with another element
    ERR_08_NULL_STRING = 8,     ///< a null pointer was passed where a string is required
};

/// The first error recorded on an element, with the setter it came from.
struct ErrorRecord {
    ErrorCode code = ERR_NONE;
    const char* origin = nullptr;   ///< setter name, e.g. "setColor"
    bool set() const { return code != ERR_NONE; }
    void record(ErrorCode c, const char* from) { if (!set()) { code = c; origin = from; } }
};

const char* errorText(ErrorCode code);   ///< short English description for the info element

} // namespace phyphox

#endif
