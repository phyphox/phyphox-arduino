# Changelog

## 2.0.0

The library was rewritten from scratch. Existing sketches keep working with very few
exceptions, listed under *Breaking*; [Migrating from 1.x](docs/migrating-from-1x.md) has the
details.

### Breaking

- Call `PhyphoxBLE::poll()` in `loop()` on every board. This is new only for the Nano 33 BLE,
  which now uses the ArduinoBLE library (installed automatically). On the ESP32 `poll()` does
  nothing, so ESP32 sketches without it keep working.
- Nano 33 IoT, MKR WiFi 1010 and Nano RP2040 Connect: the current ArduinoBLE needs the board's
  radio firmware 3.0 or newer. Run *Tools → Firmware Updater* once if the board is older.
- Values from the phone arrive only when they change, and a button press arrives once. A
  sketch that relied on getting every value over and over can switch that back on with
  `experiment.setResendUnchanged(true)`.
- `SENSOR_MAGNETOMETER` is now `SENSOR_MAGNETIC_FIELD`. The old name never worked; it still
  compiles and now reads the magnetic field.
- Texts given to the library (labels, units, titles) are no longer copied. Write them in
  quotes, `"Voltage"`, or keep them in global variables — not in a temporary `String`.
- The phone needs phyphox 1.2.1 or newer.

### Added

- Buttons, and callbacks: a function of your sketch runs when the user moves a slider, edits a
  value or presses a button (`onChange`, `onPress`).
- All twelve phone sensors phyphox knows, including gravity and orientation, with control over
  how often they send.
- More ways to show data: a text instead of a number ("cold", "fine", "hot"), elements shown or
  hidden by a toggle, logarithmic and time axes, a colour and line width per curve, text size
  and alignment.
- Shorter sketches: `PhyphoxBleExperiment::Graph graph("Voltage");` instead of a separate
  `setLabel`. Every 1.x setter still works.
- More boards: all ESP32 variants with Bluetooth (board package 3.x and 4.x), UNO R4 WiFi,
  Nicla Sense ME, Nano RP2040 Connect, Portenta, GIGA, STM32 boards with STM32duinoBLE. The
  [README](README.md#supported-boards) says which have been tested.
- Help when something goes wrong: `PhyphoxBLE::printErrors(&Serial)` lists mistakes in the
  experiment description and problems starting Bluetooth.
- Documentation in [`docs/`](docs/README.md), and automated tests, including runs on real
  boards with Android and iOS phones before every release.

### Fixed

- The experiment loads reliably. 1.x sent it at a fixed pace and lost pieces whenever the
  Bluetooth connection could not keep up, so loading often failed — on iPhones about one try in
  three. A weak connection now only makes loading slower.
- The board asks for connection settings that iPhones accept.
- senseBox with the Bluetooth-Bee: the experiment loads reliably; 1.x could miss the phone's
  request and time out. The library restarts the Bee when the sketch starts (about 2 s), which
  makes sure the module answers. `PhyphoxBLE::begin(&Serial)` prints the conversation with the
  module, for bug reports.
- Separator colour, line width and the factor of a value now take effect; 1.x misspelled them
  and the app ignored them. Labels containing `&`, `<` or quotation marks no longer break the
  experiment.
- Much less memory: no fixed 16 KB buffer, and no fixed maximum number of views, elements,
  sensors or export sets. A typical experiment needs one to two kilobytes of RAM, and every
  example fits the ESP32's default *Partition Scheme*.

## 1.3.1 and earlier

See the [GitHub releases](https://github.com/phyphox/phyphox-arduino/releases).
