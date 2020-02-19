#ifndef PHYBLE_H
#define PHYBLE_H

#ifndef DATASIZE
#define DATASIZE 20
#endif

#ifndef CONFIGSIZE
#define CONFIGSIZE 20
#endif

#if defined(ARDUINO_ARCH_MBED)
#include "phyphoxBLE_NRF52.h"

#elif defined(ESP32)
#include "phyphoxBLE_ESP32.h"
#endif

#endif
