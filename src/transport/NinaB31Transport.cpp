#include "Select.h"
#if defined(PHYPHOX_BLE_USE_NINAB31)

// u-blox NINA-B31 over UART with u-connectXpress AT commands: the senseBox MCU / MCU-S2 with
// the Bluetooth-Bee, and the MKR1000 with a NINA-B31. A port of the 1.x backend by the
// senseBox side; Tier 2 (compiled in CI, tested on senseBox hardware). Both transfer
// triggers: the control write, and the CCCD write the module reports as a characteristic
// write on the client-configuration handle where it does.
#include <Arduino.h>
#include "NinaB31Transport.h"
#include <string.h>
#include <stdlib.h>

// The UART the module hangs on: Serial3 on the senseBox MCU's core (which identifies as an
// MKR1000), Serial1 on the MCU-S2. Override with -DPHYPHOX_BLE_NINA_SERIAL=SerialX.
#if defined(PHYPHOX_BLE_NINA_SERIAL)
  #define SerialBLE PHYPHOX_BLE_NINA_SERIAL
#elif defined(ARDUINO_SAMD_MKR1000)
  #define SerialBLE Serial3
#elif defined(ARDUINO_SENSEBOX_MCU_ESP32S2)
  #define SerialBLE Serial1
#endif

namespace phyphox {
namespace {

struct Entry { int handle; CharId id; };
Entry* entries = nullptr;
uint8_t entryCount = 0;
TransportListener* listener = nullptr;
bool isConnected = false;
String input;

const char* const kSuffixHex = "30F746718B435E40BA53514A";
// advertising payload with the experiment service UUID (cddf0001…), as 1.x sent it
const char* const kAdvertisement = "020A0605121800280011074A5153BA405E438B7146F7300100DFCD";

String uuidHex(uint8_t group, uint8_t index) {
    char buf[9]; snprintf(buf, sizeof(buf), "CDDF%02X%02X", group, index);
    return String(buf) + kSuffixHex;
}

bool checkResponse(const String& cmd, uint32_t timeout) {
    while (SerialBLE.available()) SerialBLE.read();
    SerialBLE.print(cmd); SerialBLE.write('\r'); SerialBLE.flush();
    String in;
    uint32_t start = millis();
    while (millis() - start < timeout) {
        if (SerialBLE.available()) {
            in += (char)SerialBLE.read();
            if (in.endsWith("ERROR\r")) return false;
            if (in.endsWith("OK\r")) return true;
        }
    }
    return false;
}

/// Send a command whose response carries a number after ':' (a handle); −1 on error.
int parseResponse(const String& cmd, uint32_t timeout) {
    while (SerialBLE.available()) SerialBLE.read();
    SerialBLE.print(cmd); SerialBLE.write('\r'); SerialBLE.flush();
    String in;
    uint32_t start = millis();
    while (millis() - start < timeout) {
        if (SerialBLE.available()) {
            in += (char)SerialBLE.read();
            if (in.endsWith("ERROR\r")) return -1;
            if (in.endsWith("OK\r")) {
                int colon = in.indexOf(':'), comma = in.indexOf(',');
                if (colon == -1) return -1;
                return (comma != -1 ? in.substring(colon + 1, comma) : in.substring(colon + 1)).toInt();
            }
        }
    }
    return -1;
}

bool configModule() {
    SerialBLE.print("AT+UMRS=115200,2,8,1,1\r");   // 115200 8N1, no flow control
    SerialBLE.print("AT&W0\r");
    SerialBLE.print("AT+CPWROFF\r");
    SerialBLE.flush();
    delay(2000);
    return checkResponse("ATE0", 500);
}

int addCharacteristic(uint8_t group, uint8_t index, CharId id) {
    // properties 1a = read | write | notify, as 1.x; the module ignores what it cannot do
    int h = parseResponse(String("AT+UBTGCHA=") + uuidHex(group, index) + ",1a,1,1", 1000);
    Entry& e = entries[entryCount++];
    e.handle = h; e.id = id;
    return h;
}

Entry* find(CharId id) {
    for (uint8_t i = 0; i < entryCount; ++i) if (entries[i].id.kind == id.kind && entries[i].id.index == id.index) return &entries[i];
    return nullptr;
}
Entry* find(int handle) {
    for (uint8_t i = 0; i < entryCount; ++i) if (entries[i].handle == handle) return &entries[i];
    return nullptr;
}

int hexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/// "+UUBTGRW:<conn>,<handle>,<hex>,<offset>" — a characteristic written by the phone.
void handleLine(const String& line) {
    if (line.indexOf("UUBTACLC:") != -1) { isConnected = true; if (listener) listener->onConnect(); return; }
    if (line.indexOf("UUBTACLD:") != -1) { isConnected = false; if (listener) listener->onDisconnect(); return; }
    int p = line.indexOf("UUBTGRW:");
    if (p == -1) return;
    int c1 = line.indexOf(',', p), c2 = c1 == -1 ? -1 : line.indexOf(',', c1 + 1), c3 = c2 == -1 ? -1 : line.indexOf(',', c2 + 1);
    if (c1 == -1 || c2 == -1) return;
    int handle = line.substring(c1 + 1, c2).toInt();
    String hex = c3 == -1 ? line.substring(c2 + 1) : line.substring(c2 + 1, c3);
    hex.trim();
    Entry* e = find(handle);
    if (!e || !listener) return;
    uint8_t buf[64]; uint16_t n = 0;
    for (unsigned i = 0; i + 1 < hex.length() && n < sizeof(buf); i += 2) {
        int hi = hexVal(hex[i]), lo = hexVal(hex[i + 1]);
        if (hi < 0 || lo < 0) return;
        buf[n++] = (uint8_t)((hi << 4) | lo);
    }
    listener->onWrite(e->id, buf, n);
}

} // namespace

NinaB31Transport& NinaB31Transport::instance() { static NinaB31Transport t; return t; }

bool NinaB31Transport::begin(const GattLayout& layout, TransportListener& li) {
    listener = &li;
    SerialBLE.begin(115200);
    delay(500);
    checkResponse("AT", 500);
    bool up = false;
    for (int i = 0; i < 3 && !up; ++i) up = checkResponse("ATE0", 500) || configModule();
    if (!up) return false;
    checkResponse("AT+UBTDM=1", 1000);                              // stop advertising while configuring
    checkResponse(String("AT+UBTAD=") + kAdvertisement, 1000);

    uint8_t total = 4 + layout.inputChannels + layout.sensors + (layout.legacyConfig ? 1 : 0);
    entries = (Entry*)calloc(total, sizeof(Entry));
    entryCount = 0;
    parseResponse(String("AT+UBTGSER=") + uuidHex(0x00, 0x01), 1000);
    addCharacteristic(0x00, 0x03, CharId{CH_CONTROL, 0});
    addCharacteristic(0x00, 0x02, CharId{CH_EXPERIMENT, 0});
    addCharacteristic(0x00, 0x04, CharId{CH_EVENT, 0});
    parseResponse(String("AT+UBTGSER=") + uuidHex(0x10, 0x01), 1000);
    addCharacteristic(0x10, 0x02, CharId{CH_DATA, 0});
    for (uint8_t k = 1; k <= layout.inputChannels; ++k) addCharacteristic(0x20, k, CharId{CH_INPUT, k});
    for (uint8_t s = 1; s <= layout.sensors; ++s) addCharacteristic(0x30, s, CharId{CH_SENSOR, s});
    if (layout.legacyConfig) addCharacteristic(0x10, 0x03, CharId{CH_LEGACY_CONFIG, 0});

    checkResponse(String("AT+UBTLN=\"") + layout.deviceName + "\"", 1000);
    checkResponse(String("AT+UBTAD=") + kAdvertisement, 1000);
    return checkResponse("AT+UBTDM=3", 1000);
}

bool NinaB31Transport::notify(CharId id, const uint8_t* data, uint16_t len) {
    Entry* e = find(id);
    if (!e || e->handle < 0 || !isConnected) return false;
    if (len > 20) len = 20;
    String msg = String("AT+UBTGSN=0,") + e->handle + ",";
    const char* h = "0123456789abcdef";
    for (uint16_t i = 0; i < len; ++i) { msg += h[data[i] >> 4]; msg += h[data[i] & 15]; }
    return checkResponse(msg, 1000);
}

void NinaB31Transport::setValue(CharId, const uint8_t*, uint16_t) {}
bool NinaB31Transport::connected() const { return isConnected; }
uint16_t NinaB31Transport::mtuPayload() const { return PHYPHOX_BLE_DEFAULT_MTU; }

void NinaB31Transport::poll() {
    while (SerialBLE.available()) {
        char c = (char)SerialBLE.read();
        input += c;
        if (c == '\r' || c == '\n') {
            handleLine(input);
            input = "";
        }
        if (input.length() > 200) input = "";
    }
}

void NinaB31Transport::restartAdvertising() { checkResponse("AT+UBTDM=3", 1000); }

} // namespace phyphox

#endif
