// A sketch that does nothing — flashed to a board that must stay off the air while another
// board on the same desk is tested (tools/bench/examples.py --silence). A board still running
// an example advertises that example's name, and two advertisers with the same name make the
// phone connect to whichever it sees first (found 2026-09-13: the Nano sweep talked to the ESP32).
// The UNO R4 WiFi's radio is its ESP32-S3 bridge, which a re-flash of the main MCU does not
// reset: it keeps advertising the last sketch's name until the HCI reset of BLE.begin().
#if defined(ARDUINO_UNOR4_WIFI)
#include <ArduinoBLE.h>
void setup() { BLE.begin(); BLE.end(); }
#else
void setup() {}
#endif
void loop() { delay(1000); }
