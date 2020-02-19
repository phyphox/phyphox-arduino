#ifndef PHYBLE_H
#define PHYBLE_H



#if defined(ARDUINO_ARCH_MBED)
#include "phyphoxBLE_NRF52.h"

#elif defined(ESP32)
#include "phyphoxBLE_ESP32.h"
#endif

#endif
