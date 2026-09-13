# phyphox BLE 2.0 — what changes, in one page

The phyphox Arduino library is being rewritten as version 2.0. It is a new implementation
of the same idea: your sketch describes what phyphox should show, sends data, and receives
what the user enters. Existing sketches keep working with very few exceptions, listed first.
We would like your input before the release — on the breaks, on boards you use, and on
anything you have always wanted the library to do.

## What breaks for existing projects

- **One renamed constant.** `SENSOR_MAGNETOMETER` becomes `SENSOR_MAGNETIC_FIELD`. The old
  name never worked (it was not a phyphox sensor name) and still compiles in 2.0 as an alias.
- **`PhyphoxBLE::poll()` in `loop()` is required on every board.** New only for the
  Nano 33 BLE, which moves from the Mbed Bluetooth stack to ArduinoBLE; on the ESP32 it is a
  no-op, so old ESP32 sketches without it keep working.
- **The Nano 33 BLE needs the ArduinoBLE library** (installed automatically by the Library
  Manager). On the Nano 33 IoT and MKR WiFi 1010, ArduinoBLE 2.x needs the board's NINA
  firmware 3.0 or newer — the IDE's Firmware Updater does it once.
- **Values from the phone arrive only when they change.** 1.x sent every input's value on every
  analysis cycle; 2.0 sends a value when it changed and a button press once per press. A
  sketch that relied on the periodic writes can switch the old behaviour back on with
  `experiment.setResendUnchanged(true)`.
- **The phone needs phyphox 1.2.1 or newer** (2025). The generated experiment declares the
  current file-format version; older app versions refuse it. Boards still running 1.x are
  unaffected.
- **Strings are stored by pointer, not copied.** Pass literals or static buffers to setters,
  not temporaries. No published sketch we know of does otherwise.
- Small visible differences: the default experiment (no `addExperiment`) is graph plus value
  on every board; separator colour, line width and value factor now take effect (1.x emitted
  them under misspelled names); labels containing an ampersand, a less-than sign or a quotation mark now load.

## What you gain

- **Buttons.** `PhyphoxBleExperiment::Button` with `onPress(callback)` on the board; an edit
  field can be submitted with a button.
- **Callbacks instead of polling.** `slider.onChange([](float v) { … })` on every input
  element; `read()` still works and returns the element's default before the first change.
- **A reliable experiment transfer.** 1.x sent packets at a fixed pace and silently dropped
  those its own Bluetooth stack refused — a third of the transfers to an iPhone failed that
  way. 2.0 sends as fast as the stack accepts and retries what it refuses; slow links get slow
  transfers, not broken ones. Connection parameters are ones iOS accepts.
- **Far less RAM, and no limits.** No 16 KB XML buffer, no heap allocation per string, no
  fixed maximum number of views or elements: the description is copied once into a block of
  exactly its size and the XML is streamed packet by packet. A typical experiment needs one to
  two kilobytes. Every example fits the default ESP32 partition.
- **All twelve phone sensors** the file format knows (gravity, attitude, custom sensors by
  name, …), with rate strategy and stride.
- **More of the format, cheaply:** value maps (show "OK"/"HOT" instead of a number), element
  visibility bound to a toggle, logarithmic axes, follow-x, time axes, colours and line widths
  per curve, info text size and alignment.
- **Shorter sketches** through constructors: `Graph graph("Voltage")`,
  `Slider gain("Gain", 1, 10, 1)`. Every 1.x setter remains.
- **More boards, fewer special cases:** all ESP32 variants (classic, S3, C3, C6, …), Nano 33
  BLE/IoT, UNO R4 WiFi, MKR WiFi 1010, Nano RP2040 Connect, Portenta, GIGA, STM32 with
  STM32duinoBLE, senseBox with the Bluetooth-Bee. One rule for all: call `poll()`.
- **Documentation in the repository** for Arduino users who do not know phyphox, a migration
  page, and an API reference generated from the headers.
- **Tests.** Host tests of the core, the generated documents validated against the phyphox
  file-format specification on every change, a compile matrix over every example and board,
  and a hardware bench where every board is exercised against a scripted phone and then
  against real Android and iOS phones.

## Major technical differences

- **One portable core, thin transports.** The XML generation, the transfer and the channel
  handling are one Arduino-independent implementation, tested on a PC. Per Bluetooth library
  there is one small transport: ArduinoBLE (also STM32duinoBLE), the ESP32 core library
  (Bluedroid and NimBLE variants), and the NINA-B31 serial protocol. 1.x had five parallel
  backends that had drifted apart.
- **Streaming serializer.** The document is never assembled; any packet is regenerated from
  the description on demand.
- **One characteristic per input channel** and a small change detector in the experiment's
  analysis block, which is how "send only on change" is achieved with the phyphox format as
  it is. Phone-sensor components share one characteristic per sensor.
- **The generated experiment always declares the latest file-format version** the library
  knows, and every construct the library emits is checked against the format specification
  automatically.

## How to try it and where to send input

The 2.0 branch (`v2`) can be installed as a ZIP library while it is in development; the
Library Manager keeps offering 1.x until the release. Please tell us which boards you use with
the library, which sketches you would want checked against 2.0, and what you are missing.
