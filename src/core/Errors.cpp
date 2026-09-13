#include "Errors.h"

namespace phyphox {

const char* errorText(ErrorCode code) {
    switch (code) {
        case ERR_NONE: return "";
        case ERR_01_TOO_LONG: return "ERR_01 string too long";
        case ERR_02_ABOVE_LIMIT: return "ERR_02 value above limit";
        case ERR_03_NOT_HEX_COLOR: return "ERR_03 color is not 6 hex digits";
        case ERR_04_INVALID_VALUE: return "ERR_04 invalid value";
        case ERR_05_INVALID_LAYOUT: return "ERR_05 layout must be auto, extend or fixed";
        case ERR_06_CAPACITY: return "ERR_06 out of memory or inline limit";
        case ERR_07_CHANNEL_CONFLICT: return "ERR_07 button shares an input channel";
        case ERR_08_NULL_STRING: return "ERR_08 null string";
    }
    return "ERR_?";
}

} // namespace phyphox
