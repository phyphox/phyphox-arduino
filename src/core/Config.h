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

/// Transfer flow control: a refused notification is re-sent after RETRY_DELAY_MS, the pause
/// doubling with every consecutive refusal up to RETRY_MAX_MS. There is no retry count — a
/// congested stack can refuse for hundreds of milliseconds in a row and the packet must still
/// go out; only the stall watchdog below abandons a transfer.
#define PHYPHOX_BLE_TRANSFER_RETRY_DELAY_MS 5
#define PHYPHOX_BLE_TRANSFER_RETRY_MAX_MS 50
/// A transfer that has not moved (no packet accepted) for this long is abandoned, so a stuck
/// session can never block the next trigger. The app's own transfer timeout is longer.
#define PHYPHOX_BLE_TRANSFER_STALL_MS 3000
/// The first packet waits this long after the trigger. The apps subscribe and then at once
/// write 1 to the control characteristic; a board that floods notifications the moment the
/// subscription arrives starves the stack's response to that write, and Android abandons the
/// transfer after 5 s with "could not write" (seen on a Pixel 9 Pro, 2026-09-13).
#define PHYPHOX_BLE_TRANSFER_START_DELAY_MS 150
/// Packets sent back to back per pump before the stack gets a breather (the ESP32 task pumps
/// every 2 ms). Far faster than any link, but leaves room for the phone's writes and reads.
#define PHYPHOX_BLE_TRANSFER_BURST 8

#endif
