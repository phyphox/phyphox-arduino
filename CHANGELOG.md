# Changelog

## 2.0.0 — unreleased (branch `v2`)

A rewrite of the library. Existing sketches keep working; the full list of visible changes
with reasons is in [docs/whats-new-2.0.md](docs/whats-new-2.0.md), the migration steps in
[docs/migrating-from-1x.md](docs/migrating-from-1x.md).

### Breaking

- `SENSOR_MAGNETOMETER` is renamed `SENSOR_MAGNETIC_FIELD` (the old name never matched a
  phyphox sensor); the old name remains as a deprecated alias.
- `PhyphoxBLE::poll()` in `loop()` is required on every board; new only for the Nano 33 BLE,
  which moves from the Mbed Bluetooth stack to ArduinoBLE (a no-op on the ESP32).
- Values from the phone arrive only when they change, a button press once per press;
  `experiment.setResendUnchanged(true)` restores the periodic writes of 1.x.
- The generated experiment declares file-format version 1.20; the phone needs phyphox 1.2.1
  or newer.
- Strings passed to setters are stored by pointer, not copied: pass literals or static buffers.

### Added

- Buttons (`PhyphoxBleExperiment::Button`, `onPress`), and `onChange` callbacks on every
  input element.
- All twelve phone sensors of the file format, with rate strategy and stride.
- Value maps, element visibility, logarithmic and time axes, follow-x, per-curve colours and
  line widths, info text size and alignment.
- Constructors for every element (`Graph graph("Voltage")`), every 1.x setter kept.
- Boards: all ESP32 variants (Bluedroid and NimBLE), UNO R4 WiFi, Nano RP2040 Connect,
  Portenta, GIGA, STM32duinoBLE boards, senseBox with the Bluetooth-Bee (NINA-B31).
- NINA-B31 (senseBox Bluetooth-Bee): the serial line is read by one line reader that queues the
  module's unsolicited events while a command waits for its response, so the phone's control
  write and connection events are never lost to a data notification in flight (1.x flushed the
  input before every command); the transfer also starts on subscription; data notifications are
  not sent before the phone subscribes; read requests are answered; `PhyphoxBLE::begin(&Serial)` (or
  `-DPHYPHOX_BLE_NINA_TRACE`) mirrors the AT dialogue on `Serial`. A transport that does not come up is tried once and
  reported by `printErrors()` instead of being retried from every `poll()`.
- Documentation in `docs/`, a migration page, a Doxygen reference; host tests, conformance of
  the generated XML against the phyphox file-format specification, a compile matrix, and a
  hardware bench with real Android and iOS phones.

### Fixed

- The experiment transfer no longer drops packets its own Bluetooth stack refused (a third of
  the transfers to an iPhone failed that way in 1.x) and, on the classic ESP32, packets the
  Bluedroid stack discarded after accepting them. Retries are paced by the stack, no fixed
  delays; a stalled transfer is abandoned after 3 s and the next request starts afresh.
- Separator colour, line width and value factor take effect (1.x emitted them under
  misspelled attribute names); labels containing an ampersand, a less-than sign or a quotation mark load.
- Connection-parameter requests use values iOS accepts.
- No 16 KB XML buffer, no per-string heap allocation, no fixed limits on views, elements,
  sensors or export sets: the description is copied once into a block of exactly its size and
  the XML is streamed packet by packet. Every example fits the default ESP32 partition.

## 1.3.1 and earlier

See the [GitHub releases](https://github.com/phyphox/phyphox-arduino/releases).
