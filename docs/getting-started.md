# Getting started

## Install

Arduino IDE: *Sketch → Include Library → Manage Libraries…*, search for **phyphox BLE**, install.
The Library Manager installs **ArduinoBLE** with it, which the Arduino boards need.

Then install the board package for your microcontroller as usual (*Tools → Board → Boards
Manager…*): `esp32` for ESP32 boards, `Arduino Mbed OS Nano Boards` for the Nano 33 BLE,
`Arduino SAMD Boards` for the Nano 33 IoT and MKR WiFi 1010, `Arduino UNO R4 Boards` for the
UNO R4 WiFi.

## The first sketch

```cpp
#include <phyphoxBle.h>

void setup() {
  PhyphoxBLE::start();                 // advertise as "phyphox-Arduino"
}

void loop() {
  float value = analogRead(A0);
  PhyphoxBLE::write(value);            // channel 1
  delay(50);
  PhyphoxBLE::poll();                  // call it on every board; a no-op where not needed
}
```

Upload, open phyphox, tap **+** → *Bluetooth device*, pick `phyphox-Arduino`. The phone
receives the experiment from the board (a graph of channel 1 over time plus its current value),
and ▶ starts plotting.

## Per-board notes

Status as in the [README](../README.md#supported-boards): *tested* on the bench with both apps,
*compiled* in the continuous integration only, or *untested*.

| board | status | notes |
|---|---|---|
| ESP32 (any with a radio; not the S2) | tested: ESP32, ESP32-C3; compiled: ESP32-S3 | nothing to install beyond the board package. Works with the `esp32` board package 3.x and 4.x. Needs no `poll()`, but calling it is harmless |
| Nano 33 BLE / Sense / Sense Rev2 | tested (Nano 33 BLE) | uses ArduinoBLE (installed with this library). **`PhyphoxBLE::poll()` in `loop()` is required** |
| Nano 33 IoT, MKR WiFi 1010, Nano RP2040 Connect | tested: Nano 33 IoT; compiled: the others | ArduinoBLE 2.x needs the board's NINA-W102 firmware ≥ 3.0.0: run *Tools → Firmware Updater* once if the board is older. `poll()` required |
| UNO R4 WiFi | tested | `poll()` required |
| STM32WB and other STM32duinoBLE boards | untested | install **STM32duinoBLE** and the STM32 board package. `poll()` required |
| senseBox MCU / MCU-S2 with the Bluetooth-Bee (NINA-B31), MKR1000 with NINA-B31 | tested: senseBox MCU; compiled: MCU-S2, MKR1000 | `poll()` required. The module's UART is `Serial3` on the senseBox MCU core and `Serial1` on the MCU-S2; another port: `-DPHYPHOX_BLE_NINA_SERIAL=SerialX`. On the senseBox MCU the library power-cycles the Bee when it starts (about 2 s) so that the module's UART comes up reliably; `-DPHYPHOX_BLE_NINA_NO_POWER_CYCLE` skips that. If the board never appears in the app, call `PhyphoxBLE::printErrors(&Serial)` after the first `poll()` (the module is configured then): it says when the module did not answer. `PhyphoxBLE::begin(&Serial)` before `start()` (or `-DPHYPHOX_BLE_NINA_TRACE`) prints the whole AT dialogue with the module on `Serial` — the thing to send when reporting a problem |
| Nicla Sense ME | tested | uses ArduinoBLE (installed with this library). `poll()` required |
| Portenta H7, GIGA R1 WiFi, UNO WiFi Rev2 | untested | ArduinoBLE boards, `poll()` required. Define `PHYPHOX_BLE_TRANSPORT_ARDUINOBLE` for other ArduinoBLE boards |

Raspberry Pi Pico W is not supported (its Arduino core has no ArduinoBLE); support is planned for
a later version.

Two example notes: `getDataFromSmartphone` blinks `LED_BUILTIN` and falls back to GPIO 2 on boards
that do not define it (the generic `esp32:esp32:esp32` board, the senseBox MCU-S2); `CO2kit` and
`rangefinder` need their sensor libraries (SparkFun SCD30, VL53L0X) from the Library Manager.

Typical sizes (flash and static RAM, the `randomNumbers` example unless noted): ESP32 1,107 KB
and 42 KB with board package 3.x, 1,123 KB and 43 KB with 4.x — every example fits the default
partition; Nano 33 BLE 345 KB and 70 KB (`CreateExperiment`); Nano 33 IoT 79 KB and 6.2 KB.

## Next

[Concepts](concepts.md) explains what the phone actually receives; [Views and graphs](views.md)
shows how to label axes and add tabs; [User input](inputs.md) how to control the board from
the phone.
