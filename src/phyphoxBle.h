#ifndef PHYPHOXBLE
#define PHYPHOXBLE

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

#include "Arduino.h"


struct crc32
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


#endif
