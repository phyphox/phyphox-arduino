// phyphox BLE 2.0 — the public include.
//
//   #include <phyphoxBle.h>
//   void setup() { PhyphoxBLE::start(); }
//   void loop()  { PhyphoxBLE::write(analogRead(A0) * 3.3f / 4096); delay(50); PhyphoxBLE::poll(); }
//
// Three lines add phyphox to a project: this is the simple API and the library's main feature.
// Everything else — describing views, graphs, user inputs and buttons, requesting phone
// sensors — is in phyphoxBleExperiment.h and optional.
//
// The transport (which BLE library talks to the radio) is chosen here from the board macros.
#ifndef PHYPHOXBLE
#define PHYPHOXBLE

#include <Arduino.h>
#include "phyphoxBleExperiment.h"
#include "core/Server.h"

// ---------------------------------------------------------------- phone sensor types
// The strings are phyphox file-format sensor names (phyphox-docs spec/input.yml).
#define SENSOR_ACCELEROMETER "accelerometer"
#define SENSOR_ACCELEROMETER_WITHOUT_G "linear_acceleration"
#define SENSOR_LINEAR_ACCELERATION "linear_acceleration"
#define SENSOR_GRAVITY "gravity"
#define SENSOR_GYROSCOPE "gyroscope"
#define SENSOR_MAGNETIC_FIELD "magnetic_field"
#define SENSOR_PRESSURE "pressure"
#define SENSOR_LIGHT "light"
#define SENSOR_PROXIMITY "proximity"
#define SENSOR_TEMPERATURE "temperature"
#define SENSOR_HUMIDITY "humidity"
#define SENSOR_ATTITUDE "attitude"
/// A sensor phyphox does not name: select it with Sensor::setNameFilter() / setTypeFilter()
/// (Android; the app's device info lists the names). Format 1.19.
#define SENSOR_CUSTOM "custom"
/// 1.x spelled the magnetometer "magnetometer", which is not a phyphox sensor name, so that
/// sensor never worked. Deprecated alias: emits the correct name; removed in 2.1.
#define SENSOR_MAGNETOMETER SENSOR_MAGNETIC_FIELD

// ---------------------------------------------------------------- colours (1.x)
#define COLOR_RED "fe005d"
#define COLOR_BLUE "39a2ff"
#define COLOR_GREEN "2bfb4c"
#define COLOR_ORANGE "ff7e22"
#define COLOR_WHITE "ffffff"
#define COLOR_YELLOW "edf668"
#define COLOR_MAGENTA "eb46f4"

// ---------------------------------------------------------------- transport selection
#include "transport/Select.h"
#if defined(PHYPHOX_BLE_USE_ESP32)
  #include "transport/Esp32CoreBleTransport.h"
  #define PHYPHOX_BLE_TRANSPORT phyphox::Esp32CoreBleTransport
#elif defined(PHYPHOX_BLE_USE_NINAB31)
  #include "transport/NinaB31Transport.h"
  #define PHYPHOX_BLE_TRANSPORT phyphox::NinaB31Transport
#elif defined(PHYPHOX_BLE_USE_ARDUINOBLE)
  #include "transport/ArduinoBleTransport.h"
  #define PHYPHOX_BLE_TRANSPORT phyphox::ArduinoBleTransport
#endif

// ---------------------------------------------------------------- the simple API

/// Static façade over one phyphox::Server. Every method is a one-liner forwarding to it; the
/// class exists so that `PhyphoxBLE::start()` and `PhyphoxBLE::write(x)` stay the whole story
/// for the simplest sketches.
class PhyphoxBLE {
public:
    // ---- start
    /// Advertise as `deviceName` ("phyphox-Arduino" by default). Without addExperiment() the
    /// default experiment is served: a graph of channel 1 over time plus its value.
    static void start(const char* deviceName = "phyphox-Arduino");
    /// Start and serve `experiment` in one call.
    static void start(const char* deviceName, PhyphoxBleExperiment& experiment);
    /// Serve a hand-written phyphox document instead of a generated one (1.x form).
    static void start(const char* deviceName, uint8_t* xml, size_t len);
    static void start(uint8_t* xml, size_t len) { start("phyphox-Arduino", xml, len); }
    /// Serve this description (copies it). May be called before or after start().
    static void addExperiment(PhyphoxBleExperiment& experiment);

    // ---- every loop()
    /// Gives the BLE stack CPU time and drives the transfer. Required on every board; a no-op
    /// where the stack runs by itself (ESP32), so calling it never hurts.
    static void poll();
    static void poll(int timeoutMs);   ///< 1.x signature; the timeout is ignored

    // ---- data to the phone (channels 1…5; channel 0 is the phone's arrival time)
    // (const references: 1.x took float&, which rejected write(analogRead(A0) * k); const& takes both)
    static void write(const float& v1);
    static void write(const float& v1, const float& v2);
    static void write(const float& v1, const float& v2, const float& v3);
    static void write(const float& v1, const float& v2, const float& v3, const float& v4);
    static void write(const float& v1, const float& v2, const float& v3, const float& v4, const float& v5);
    /// Raw arrays, up to the MTU payload; with setRepeating(n) the app unpacks n samples.
    static void write(uint8_t* bytes, unsigned int len);
    static void write(float* values, unsigned int count);

    // ---- data from the phone (the last value of input channels 1…5)
    static void read(float& v1);
    static void read(float& v1, float& v2);
    static void read(float& v1, float& v2, float& v3);
    static void read(float& v1, float& v2, float& v3, float& v4);
    static void read(float& v1, float& v2, float& v3, float& v4, float& v5);
    static void read(uint8_t* bytes, unsigned int len);   ///< user-XML mode: the raw cddf1003 value
    static float readChannel(int channel);                 ///< any input channel
    static bool changed(int channel);                      ///< true once per new value
    /// Attach a callback to an input channel after the experiment was added (the element's
    /// onChange() does the same before).
    static void onChange(int channel, phyphox::ChangeCallback);
    static void onPress(int channel, phyphox::PressCallback);
    /// 1.x hook: called after any value arrived from the phone (any channel, any sensor).
    static void (*configHandler)();

    // ---- events (start/pause/clear/sync from the app)
    static void (*experimentEventHandler)();
    static uint8_t eventType;          ///< phyphox::EventType of the last event
    static int64_t experimentTime;     ///< ms since the experiment was started (−1 on SYNC)
    static int64_t systemTime;         ///< Unix time in ms as the phone sees it
    static const phyphox::ExperimentEvent& lastEvent();

    // ---- radio parameters (before start())
    /// Ask for an MTU payload up to 512 bytes (default 20). Emits the mtu attribute; the phone
    /// negotiates what it can (iOS always the maximum). Bandwidth path with the array write.
    static void setMTU(uint16_t payload);
    /// Connection parameters in BLE units (1.25 ms, 1.25 ms, events, 10 ms). Defaults are
    /// Apple-compliant (15–30 ms); a request iOS cannot accept is ignored by iOS.
    static uint16_t minConInterval, maxConInterval, slaveLatency, timeout;

    // ---- introspection and debugging
    static void printXML(Print* out);            ///< the document as the phone receives it
    static void printErrors(Print* out);         ///< configuration errors, if any
    static const phyphox::ServerStats& stats();
    static uint16_t currentConnections;          ///< 1.x field, kept up to date
    static bool isSubscribed;                    ///< the phone subscribed to the data characteristic
    static const char* version() { return PHYPHOX_BLE_VERSION; }
    static const char* transportName();
    static void begin(Print* debugOut);          ///< 1.x: route debug output here (compiles now)

    static phyphox::Server& server();
};

#endif
