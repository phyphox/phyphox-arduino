#ifdef ESP32
#ifndef PHYPHOXBLE_ESP32_H
#define PHYPHOXBLE_ESP32_H



#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <esp_system.h>
#include <BLE2902.h>
using std::copy;


class phyphoxBleServer
{
    private:
        //const UUID customServiceUUID = UUID("cddf0001-30f7-4671-8b43-5e40ba53514a");
        //const UUID phyphoxUUID = UUID("cddf0002-30f7-4671-8b43-5e40ba53514a");
        //const string customServiceUUID = "cddf0001-30f7-4671-8b43-5e40ba53514a";
        //const string phyphoxUUID = "cddf0002-30f7-4671-8b43-5e40ba53514a";
        const char *customServiceUUID = "cddf0001-30f7-4671-8b43-5e40ba53514a";
        const char *phyphoxUUID = "cddf0002-30f7-4671-8b43-5e40ba53514a";        
        uint8_t data_package[20] = {0};
        const char DEVICE_NAME[20] = "Arduino";    
        //const UUID dataOneUUID = UUID("59f51a40-8852-4abe-a50f-2d45e6bd51ac");
        //const String dataOneUUID = "59f51a40-8852-4abe-a50f-2d45e6bd51ac";
        const char *dataOneUUID = "59f51a40-8852-4abe-a50f-2d45e6bd51ac";

        BLEServer *myServer;
        BLEService *myService;
        BLEDescriptor *myExperimentDescriptor;
        BLEDescriptor *myDataDescriptor;
        BLECharacteristic *dataCharacteristic;
        BLECharacteristic *experimentCharacteristic;
        BLEAdvertising *myAdvertising;

        
   //     void when_connected();

   //     virtual void onDisconnectionComplete();
   //     virtual void onConnectionComplete();
        uint8_t* data = nullptr; //this pointer points to the data the user wants to write in the characteristic
        uint8_t* p_exp = nullptr; //this pointer will point to the byte array which holds an experiment
        size_t exp_len = 0; //try o avoid this maybe use std::array or std::vector

    public:
        phyphoxBleServer() {};
        phyphoxBleServer(const phyphoxBleServer&) = delete; //there is no need to copy a BleServer once established
        phyphoxBleServer &operator=(const phyphoxBleServer&) = delete; //there is no need to assign a BleServer to a BleServer
        ~phyphoxBleServer() = default; //no dynamic memory allocation 
        void start(uint8_t* p = nullptr, size_t n = 0); 
        void when_subscription_received();

        void write(float&);
        void write(float&, float&);
        void write(float&, float&, float&);
        void write(float&, float&, float&, float&);
        void write(float&, float&, float&, float&, float&);
};

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
#endif
