// Which transport this build uses, from the board macros. Included by phyphoxBle.h and by every
// transport .cpp (the Arduino build compiles all of src/, so each transport guards its whole
// file with its macro). Boards ArduinoBLE supports but that are not listed can define
// PHYPHOX_BLE_TRANSPORT_ARDUINOBLE themselves.
#ifndef PHYPHOX_BLE_TRANSPORT_SELECT_H
#define PHYPHOX_BLE_TRANSPORT_SELECT_H

#if defined(PHYPHOX_BLE_TRANSPORT_ARDUINOBLE)
  #define PHYPHOX_BLE_USE_ARDUINOBLE 1
#elif defined(ARDUINO_SAMD_MKR1000) || defined(ARDUINO_SENSEBOX_MCU_ESP32S2)
  #define PHYPHOX_BLE_USE_NINAB31 1
#elif defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
  #define PHYPHOX_BLE_USE_ESP32 1
#elif defined(ARDUINO_ARCH_STM32)
  #define PHYPHOX_BLE_USE_ARDUINOBLE 1
  #define PHYPHOX_BLE_STM32DUINOBLE 1
#elif defined(ARDUINO_ARCH_MBED) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_RENESAS_UNO) || \
      defined(ARDUINO_ARCH_RENESAS) || defined(ARDUINO_ARCH_MBED_NANO) || defined(ARDUINO_ARCH_MBED_PORTENTA) || \
      defined(ARDUINO_ARCH_MBED_GIGA) || defined(ARDUINO_ARCH_MBED_NICLA) || defined(ARDUINO_ARCH_MEGAAVR) || \
      defined(ARDUINO_ARCH_RP2040)
  #define PHYPHOX_BLE_USE_ARDUINOBLE 1
#elif defined(ARDUINO)
  #error "phyphox BLE: unsupported board. See docs/getting-started.md; boards ArduinoBLE supports can define PHYPHOX_BLE_TRANSPORT_ARDUINOBLE."
#endif

#endif
