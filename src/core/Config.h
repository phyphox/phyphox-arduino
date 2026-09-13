// phyphox BLE 2.0 — compile-time constants.
//
// There are no capacity pools. The experiment description is copied into ONE exactly-sized
// block allocated at addExperiment() and never freed (the sketch's builder objects may then
// die), so RAM is what the experiment needs and nothing more, and a big experiment is not a
// compile-time setting. What remains fixed here is bounded by nature: the protocol, the
// format, or a sanity check.
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

/// Values the board streams to the phone: write(f1 … f5). Fixed at 5 as in 1.x — the data
/// characteristic carries five float32 in one 20-byte notification. More bandwidth comes from
/// setMTU() plus the array write and setRepeating(), not from more channels (decision D12).
#define PHYPHOX_BLE_DATA_CHANNELS 5

/// Highest input channel number a sketch may use. Every input channel is one characteristic,
/// and the NINA-B31 module keeps a bounded GATT table (plan §3.3), so this is a protocol cap,
/// not a pool: only the channels actually used cost anything. 1.x allowed 5.
#ifndef PHYPHOX_BLE_MAX_INPUT_CHANNEL
#define PHYPHOX_BLE_MAX_INPUT_CHANNEL 16
#endif

/// Components a phone sensor can deliver: x, y, z, abs, accuracy, t — the format has six.
#define PHYPHOX_BLE_SENSOR_COMPONENTS 6

/// Entries the incremental addMap()/addOption() setters keep inside the builder object before
/// addExperiment() copies them. Only a convenience limit: setOptions(n, labels, values) and
/// setMaps(n, …) take arrays of any length.
#ifndef PHYPHOX_BLE_INLINE_OPTIONS
#define PHYPHOX_BLE_INLINE_OPTIONS 8
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
