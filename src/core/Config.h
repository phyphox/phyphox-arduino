// phyphox BLE 2.0 — compile-time capacities and constants.
//
// Everything in the library is statically sized (no heap): the experiment description lives in
// fixed pools whose sizes are set here. A sketch may override any PHYPHOX_BLE_MAX_* before
// including <phyphoxBle.h> to trade RAM for capacity; the defaults are generous for a phone
// screen and small for a microcontroller (see docs/concepts.md, "Memory").
#ifndef PHYPHOX_BLE_CORE_CONFIG_H
#define PHYPHOX_BLE_CORE_CONFIG_H

#include <stdint.h>
#include <stddef.h>

/// The phyphox file-format version every generated experiment declares. Always the latest
/// version this library knows (decision D1 of the 2.0 plan); bumped with library releases.
/// The conformance test checks that nothing the library emits is newer than this.
#define PHYPHOX_BLE_FORMAT_VERSION "1.20"

/// Library version as reported by PhyphoxBLE::version(). Must match library.properties.
#define PHYPHOX_BLE_VERSION "2.0.0"

#ifndef PHYPHOX_BLE_MAX_VIEWS
#define PHYPHOX_BLE_MAX_VIEWS 8            ///< tabs in the app
#endif
#ifndef PHYPHOX_BLE_MAX_ELEMENTS
#define PHYPHOX_BLE_MAX_ELEMENTS 24        ///< view elements in total, across all views
#endif
#ifndef PHYPHOX_BLE_MAX_SUBGRAPHS
#define PHYPHOX_BLE_MAX_SUBGRAPHS 4        ///< curves per graph, including the first
#endif
#ifndef PHYPHOX_BLE_MAX_OPTIONS
#define PHYPHOX_BLE_MAX_OPTIONS 6          ///< dropdown options or value-map entries per element
#endif
#ifndef PHYPHOX_BLE_MAX_EXPORT_SETS
#define PHYPHOX_BLE_MAX_EXPORT_SETS 4
#endif
#ifndef PHYPHOX_BLE_MAX_EXPORT_DATA
#define PHYPHOX_BLE_MAX_EXPORT_DATA 16     ///< data entries in total, across all export sets
#endif
#ifndef PHYPHOX_BLE_MAX_SENSORS
#define PHYPHOX_BLE_MAX_SENSORS 3          ///< phone sensors the board can request
#endif
#ifndef PHYPHOX_BLE_MAX_SENSOR_COMPONENTS
#define PHYPHOX_BLE_MAX_SENSOR_COMPONENTS 4 ///< components (x, y, z, abs, …) mapped per sensor
#endif

/// Values the board streams to the phone: write(f1 … f5). Fixed at 5 as in 1.x — the data
/// characteristic carries five float32 in one 20-byte notification. More bandwidth comes from
/// setMTU() plus the array write and setRepeating(), not from more channels (decision D12).
#define PHYPHOX_BLE_DATA_CHANNELS 5

#ifndef PHYPHOX_BLE_INPUT_CHANNELS
#define PHYPHOX_BLE_INPUT_CHANNELS 8       ///< values the phone can send to the board (1.x: 5)
#endif

/// Longest string a setter accepts (label, unit, description, …). Longer strings are an
/// ERR_01 configuration error. Strings are stored by pointer, never copied — see
/// docs/concepts.md, "Strings".
#ifndef PHYPHOX_BLE_MAX_STRING_LENGTH
#define PHYPHOX_BLE_MAX_STRING_LENGTH 250
#endif

/// Default MTU payload (ATT_MTU 23 − 3). setMTU() raises it where the transport can negotiate.
#define PHYPHOX_BLE_DEFAULT_MTU 20
#define PHYPHOX_BLE_MAX_MTU 512

/// Transfer flow control: how often a refused notification is re-sent before the transfer is
/// abandoned, and the pause before each retry (milliseconds).
#define PHYPHOX_BLE_TRANSFER_RETRIES 50
#define PHYPHOX_BLE_TRANSFER_RETRY_DELAY_MS 5

#endif
