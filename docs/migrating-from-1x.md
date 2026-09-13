# Migrating from 1.x

Almost every 1.x sketch compiles and runs unchanged. What differs:

## One renamed constant

`SENSOR_MAGNETOMETER` → `SENSOR_MAGNETIC_FIELD`. The 1.x name emitted "magnetometer", which is
not a phyphox sensor, so that sensor never delivered data. The old macro still compiles in 2.0
(it now emits the correct name) and is removed in 2.1.

## Behaviour

- **Input values arrive on change only.** 1.x received every input's value on every analysis
  cycle; 2.0 receives a channel when it changed, and a button press once per press. `read()`
  returns each element's default before the first change. If a sketch wants the old periodic
  writes: `experiment.setResendUnchanged(true)`.
- **`PhyphoxBLE::poll()` is required in `loop()` on every board.** It always was on the Nano 33
  IoT, UNO R4 and MKR boards; now also on the Nano 33 BLE (which moved from the Mbed BLE stack
  to ArduinoBLE). On the ESP32 it is a no-op, so old ESP32 sketches without it keep working.
- **ArduinoBLE is needed for the Nano 33 BLE** (installed automatically by the Library Manager),
  and ArduinoBLE 2.x needs NINA-W102 firmware ≥ 3.0.0 on the Nano 33 IoT / MKR WiFi 1010 /
  Nano RP2040 Connect — *Tools → Firmware Updater* once.
- **The generated experiment declares format 1.20.** A board running 2.0 needs phyphox 1.2.1
  (2025) or newer on the phone. Boards still running 1.x are unaffected.
- **Strings are not copied.** Pass literals or static buffers; a temporary would dangle (see
  [Concepts](concepts.md#strings)). No shipped example or published sketch did this.
- **The default experiment** (no `addExperiment()`) is graph + value on every board; on the
  Nano 33 BLE it used to be the graph alone.
- **Bugs that now take effect:** separator colour, line width and value factor were emitted
  under misspelled attribute names and silently ignored by the app; labels containing `&`,
  `<` or `"` broke the document. Both are fixed, so a project that set them sees them now.
- The ArduinoBLE-based boards also start the transfer on subscription, not only on the control
  write. Invisible from the app.

## Removed internals

`PhyphoxBleExperiment::Errorhandler`, the `getBytes()` methods and the `char* LABEL`-style
public fields were implementation details of the string-building 1.x generator and are gone;
no sketch used them. `PhyphoxBleExperiment::MTU` moved to `PhyphoxBLE::setMTU()`.
