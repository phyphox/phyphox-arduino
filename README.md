# phyphox BLE

Plot your Arduino's sensor data on a phone, and control the Arduino from the phone — with the
free app [phyphox](https://phyphox.org) and three lines of code.

phyphox is a physics app from RWTH Aachen University that turns a phone's own sensors into
measuring instruments, with live graphs, data analysis and export. This library lets any
Bluetooth-capable Arduino-compatible board show up in that app as one more sensor: the board
tells the phone what to display, sends its data, and receives values the user enters.

```cpp
#include <phyphoxBle.h>

void setup() {
  PhyphoxBLE::start();              // start the Bluetooth server
}

void loop() {
  float value = analogRead(A0);
  PhyphoxBLE::write(value);         // plot it in phyphox
  delay(50);
  PhyphoxBLE::poll();
}
```

Open phyphox, tap **+** → *Bluetooth device*, pick your board: the graph appears.

> **Version 2.0** is a rewrite of the library. Existing sketches keep working; see
> [Migrating from 1.x](docs/migrating-from-1x.md) for the few things that changed. This branch
> is the 2.0 pre-release for testers — install it as a ZIP; the Library Manager still offers 1.x.

## Supported boards

| board | Bluetooth library | notes |
|---|---|---|
| ESP32 (all variants with a radio) | ESP32 core | nothing to install |
| Arduino Nano 33 BLE / Sense / Sense Rev2 | ArduinoBLE | installed automatically; call `PhyphoxBLE::poll()` in `loop()` |
| Arduino Nano 33 IoT, MKR WiFi 1010, Nano RP2040 Connect | ArduinoBLE | NINA firmware ≥ 3.0 (Firmware Updater); `poll()` in `loop()` |
| Arduino UNO R4 WiFi | ArduinoBLE | `poll()` in `loop()` |
| STM32WB and other STM32duinoBLE boards | STM32duinoBLE | install it; `poll()` in `loop()` |
| senseBox MCU / MCU-S2 with Bluetooth-Bee | NINA-B31 (serial) | `poll()` in `loop()` |
| Portenta H7, GIGA R1 WiFi, Nicla Sense ME | ArduinoBLE | compile-tested |

Details per board: [Getting started](docs/getting-started.md).

## Beyond three lines

Describe what the phone should show — tabs, graphs with labelled axes, values, sliders,
buttons — and get it back:

```cpp
PhyphoxBleExperiment exp("Voltmeter", "Arduino Experiments", "Voltage on A0 over time.");
PhyphoxBleExperiment::View  view("Data");
PhyphoxBleExperiment::Graph graph("Voltage");
graph.setChannel(0, 1);                       // time on x, channel 1 on y
graph.setLabelX("Time");  graph.setUnitX("s");
graph.setLabelY("Voltage"); graph.setUnitY("V");
PhyphoxBleExperiment::Slider gain("Gain", 1, 10, 1, /*input channel*/ 1);
gain.onChange([](float g) { gainFactor = g; });
view.addElement(graph).addElement(gain);
exp.addView(view);
PhyphoxBLE::start("Voltmeter", exp);
```

The `examples/` folder (File → Examples → phyphox BLE in the IDE) has a sketch for every feature.
The documentation is in [`docs/`](docs/README.md).

## Contributing, licence, contact

Issues and pull requests are welcome here. The library is released under the GNU Lesser General
Public Licence v3.0 or later (`COPYING.LESSER`). Contact: contact@phyphox.org. Developed by the
phyphox team at RWTH Aachen University; the original library was created by Alexander Krampe
(master thesis), maintained by Dominik Dorsel, and extended by Marcel Hagedorn and Edward Leier.
