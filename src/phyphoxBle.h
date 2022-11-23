#ifndef PHYPHOXBLE
#define PHYPHOXBLE

#include "Arduino.h"

static const char *phyphoxBleExperimentServiceUUID = "cddf0001-30f7-4671-8b43-5e40ba53514a";
static const char *phyphoxBleExperimentCharacteristicUUID = "cddf0002-30f7-4671-8b43-5e40ba53514a";
static const char *phyphoxBletControlCharacteristicUUID = "cddf0003-30f7-4671-8b43-5e40ba53514a";
static const char *phyphoxBleEventCharacteristicUUID = "cddf0004-30f7-4671-8b43-5e40ba53514a";

static const char *phyphoxBleDataServiceUUID = "cddf1001-30f7-4671-8b43-5e40ba53514a";
static const char *phyphoxBleDataCharacteristicUUID = "cddf1002-30f7-4671-8b43-5e40ba53514a";
static const char *phyphoxBleConfigCharacteristicUUID = "cddf1003-30f7-4671-8b43-5e40ba53514a";

static const char *deviceName = "phyphox-Arduino";

#define SENSOR_ACCELEROMETER "accelerometer"
#define SENSOR_ACCELEROMETER_WITHOUT_G "linear_acceleration"
#define SENSOR_GYROSCOPE "gyroscope"
#define SENSOR_MAGNETOMETER "magnetometer"
#define SENSOR_PRESSURE "pressure"
#define SENSOR_TEMPERATURE "temperature"
#define SENSOR_LIGHT "light"
#define SENSOR_HUMIDITY "humidity"
#define SENSOR_PROXIMITY "proximity"

#define STYLE_DOTS "dots"
#define STYLE_LINES "lines"
#define STYLE_VBARS "vbars"
#define STYLE_HBARS "hbars"
#define STYLE_MAP "map"

#define LAYOUT_AUTO "auto"
#define LAYOUT_EXTEND "extend"
#define LAYOUT_FIXED "fixed"

#ifndef CONFIGSIZE
#define CONFIGSIZE 20
#endif
#if defined(ARDUINO_SAMD_MKR1000)
    #include "phyphoxBLE_NINAB31.h"
#elif defined(ARDUINO_ARCH_MBED)
    #include "phyphoxBLE_NRF52.h"


#elif defined(ESP32)
    #include "phyphoxBLE_ESP32.h"
#elif defined(ARDUINO_SAMD_NANO_33_IOT)
    #include <ArduinoBLE.h>
    #include "phyphoxBLE_NanoIOT.h"
#else
#error "Unsupported board selected!"
#endif

#include "Arduino.h"

struct phyphoxBleCrc32
{
    static void generate_table(uint32_t(&table)[256])
    {
        uint32_t polynomial = 0xEDB88320;
        for (uint32_t i = 0; i < 256; i++) 
        {
            uint32_t c = i;
            for (size_t j = 0; j < 8; j++) 
            {
                if (c & 1) {
                    c = polynomial ^ (c >> 1);
                }
                else {
                    c >>= 1;
                }
            }
            table[i] = c;
        }
    }

    static uint32_t update(uint32_t (&table)[256], uint32_t initial, const uint8_t* buf, size_t len)
    {
        uint32_t c = initial ^ 0xFFFFFFFF;
        const uint8_t* u = static_cast<const uint8_t*>(buf);
        for (size_t i = 0; i < len; ++i) 
        {
            c = table[(c ^ u[i]) & 0xFF] ^ (c >> 8);
        }
        return c ^ 0xFFFFFFFF;
    }
};

static int64_t swap_int64( int64_t val ){
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
    return (val << 32) | ((val >> 32) & 0xFFFFFFFFULL);
}

#endif
