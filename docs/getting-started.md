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
  PhyphoxBLE::poll();                  // required on every board; a no-op where not needed
}
```

Upload, open phyphox, tap **+** → *Bluetooth device*, pick `phyphox-Arduino`. The phone
receives the experiment from the board (a graph of channel 1 over time plus its current value),
and ▶ starts plotting.

## Per-board notes

| board | notes |
|---|---|
| ESP32 (any with a radio; not the S2) | nothing to install beyond the board package. Needs no `poll()` but calling it is harmless |
| Nano 33 BLE / Sense / Sense Rev2 | uses ArduinoBLE (installed with this library). **`PhyphoxBLE::poll()` in `loop()` is required** |
| Nano 33 IoT, MKR WiFi 1010, Nano RP2040 Connect | ArduinoBLE 2.x needs the board's NINA-W102 firmware ≥ 3.0.0: run *Tools → Firmware Updater* once if the board is older. `poll()` required |
| UNO R4 WiFi | `poll()` required |
| STM32WB and other STM32duinoBLE boards | install **STM32duinoBLE** and the STM32 board package. `poll()` required |
| senseBox MCU / MCU-S2 with the Bluetooth-Bee (NINA-B31) | `poll()` required. Supported with the senseBox colleagues' testing |
| Portenta H7, GIGA R1 WiFi, Nicla Sense ME, UNO WiFi Rev2 | compile-tested only; ArduinoBLE boards. Define `PHYPHOX_BLE_TRANSPORT_ARDUINOBLE` for other ArduinoBLE boards |

Raspberry Pi Pico W is not supported (its Arduino core has no ArduinoBLE); see the plan for the
future.

## Next

[Concepts](concepts.md) explains what the phone actually receives; [Views and graphs](views.md)
shows how to label axes and add tabs; [User input](inputs.md) how to control the board from
the phone.
