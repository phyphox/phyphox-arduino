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
                                       // (write() services the stack too, so a sketch that
                                       // only writes keeps working — but call poll())
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
| senseBox MCU / MCU-S2 with the Bluetooth-Bee (NINA-B31) | `poll()` required. The module's UART is `Serial3` on the senseBox MCU core and `Serial1` on the MCU-S2; another port: `-DPHYPHOX_BLE_NINA_SERIAL=SerialX`. Supported with the senseBox colleagues' testing |
| Portenta H7, GIGA R1 WiFi, Nicla Sense ME, UNO WiFi Rev2 | compile-tested only; ArduinoBLE boards. Define `PHYPHOX_BLE_TRANSPORT_ARDUINOBLE` for other ArduinoBLE boards |

Raspberry Pi Pico W is not supported (its Arduino core has no ArduinoBLE); see the plan for the
future.

Two example notes: `getDataFromSmartphone` blinks `LED_BUILTIN`, which the generic `esp32:esp32:esp32`
board does not define — pick your DevKit variant or add `-DLED_BUILTIN=2`; `CO2kit` and
`rangefinder` need their sensor libraries (SparkFun SCD30, VL53L0X) from the Library Manager.

Sizes on 2.0 (the `randomNumbers` example unless noted): ESP32 1,107 KB flash on the default
partition and 42 KB static RAM (1.x: 1,148 KB on the huge partition, 58 KB); Nano 33 BLE 345 KB
and 70 KB (1.x: 531 KB and 78 KB, `CreateExperiment`); Nano 33 IoT 79 KB and 6.2 KB.

## Next

[Concepts](concepts.md) explains what the phone actually receives; [Views and graphs](views.md)
shows how to label axes and add tabs; [User input](inputs.md) how to control the board from
the phone.
