# phyphox Arduino BLE
Use phyphox to plot sensor data from your Microcontroller. 

The purpose of this library is to use the phyphox app (see www.phyphox.org) to plot sensor data on your phone with the open source app phyphox. In the other direction you can also use this library to access sensor data from your phone to use in your [Arduino](https://www.arduino.cc/) or ESP32 project.

## Supported boards
- Arduino Nano 33 Ble
- Arduino Nano Sense
- Arduino Nano 33 IoT (see note below)
- Arduino Uno R4 Wifi (see note below)
- senseBox MCU with NINA-B31 module
- ESP 32
- STM32 (e.g. STM32WB55)

Note: The Arduino Nano 33 IoT and the Arduino uno R4 are somewhat unusual. You will need to install the ArduinoBLE library to use it and you will need to call "PhyphoxBLE::poll()" periodically for it to work.

Note: When using the NINA-B31 module you must call PhyphoxBLE::poll() periodically (in loop() ) or the library will not work.
        The same applies to STM32

Note: to use STM32 BLE, you need the STM32duinoBLE library, and (at least for the STM32WBx5) the appropriate BLE stack (see Copro binaries)

## Concept of phyphox

Phyphox is an open source app that has been developed at the RWTH Aachen University. It is available on Android and iOS and primarily aims at making the phone's sensors available for physics experiments. However, the app is based on a very flexible file format that defines data sources, visualizations (i.e. values and graphs) and data analysis (from simple formulas to Fourier transforms). This file format also allows to define Bluetooth Low Energy devices to exchange data from and to.

This library generates an experiment configuration in this file format and allows phyphox to conenct to your microcontroller. It directly transfers the configuration (including graph configurations, axis labels, etc.) to phyphox and provides function to submit data to be plotted in phyphox or receive sensor data.

## Installation

You should be able to find this library in the library search of your Arduino IDE. There you should usually find the latest release from here, which has been tagged with a version number.

Alternatively, you can download this repository here as a zip file from github and install it with via the Arduino IDE's menu using the "Add ZIP library" entry. We recommend using the Arduino library manager, but directly installing a version from here might be relevant to try a specific branch or new feature.

You will may also need to install an BLE library specific to your board:
- ArduinoBLE for the Arduino Nano 33 IoT
- STM32duinoBLE for an STM32

## Usage

The easiest way to learn how to use this library is by looking at the examples in the `examples` folder. In most cases, you can simply connect to your Arduino running this library by scanning for Bluetooth devices via the "+"-button on the main screen of phyphox.

### randomNumbers.ino

This is our minimal example. It submits random numbers to phyphox. All you need to do to submit a value to phyphox is including this library, starting the server (i.e. in `setup()`) and writing your data to the server.

```arduino
#include <phyphoxBle.h> 

void setup() {
    PhyphoxBLE::start();                //Start the BLE server
}

void loop() {
    //uncomment next line if using senseBox MCU or Arduino Nano 33 IoT
    //PhyphoxBLE::poll();
    float randomNumber = random(0,100); //Generate random number in the range 0 to 100
    PhyphoxBLE::write(randomNumber);    //Send value to phyphox
    delay(50);                          //Shortly pause before repeating
}
```

### CreateExperiment.ino

This example shows how you can set a title, category and description as well as how to define graphs and setting axis labels and units. You can define one or multiple views (shown as tabs in phyphox), each of which can hold one or more graphs.

For each graph you need to call `setChannel(x, y)` with x and y being an index of your data set. This index corresponds to the order of the values that you transfer in a call to `server.write` while the index `0` is special and corresponds to the timestamp at which phyphox receives the value. At the moment `server.write` supports up to five values.

For example, let's assume you have the float values `foo` and `bar`. You can then call server.write(foo, bar) to send a set with both values. If you call `setChannel(0,1)`, your graph would plot `foo` on the y axis over time on the x axis. If you call `setChannel(2,1)`, your graph would plot `foo` on the y axis and `bar` on the x axis.

Here are some useful methods to create your own experiment:

| Target     | Method                   | Explanation                                                       |
| ---------- | ------------------------ | ----------------------------------------------------------------- |
| Experiment | setTitle(char*)          | Sets a title for the experiment                                   |
| Experiment | setCategory(char*)       | Sets a category for the experiment                                |
| Experiment | setDescription(char*)    | Sets a description for the experiment                             |
| Experiment | addView(View&)           | Adds a view to the corresponding experiment                       |
| Experiment | addExportSet(ExportSet&) | Adds an exportSet to the corresponding experiment                 |
| Experiment | addSensor(Sensor&)       | Data from smarpthone sensor can will be received (see getSensorDataFromSmartphone example)|
| View       | addElement(Element&)     | Adds an element to the corresponding view                         |
| View       | setLabel(char*)          | Sets a label for the view                                         |
| Graph      | setLabel(char*)          | Sets a label for the graph                                        |
| Graph      | setUnitX(char*)          | Sets the unit for x (similar with y)                              |
| Graph      | setLabelX(char*)         | Sets a label for x (similar with y)                               |
| Graph      | setXPrecision(int)       | Sets the amount of digits after the decimal point (similar with y)|
| Graph      | setChannel(int, int)     | As explained above (1-5)                                          |
| Graph      | addSubgraph(Subgraph)    | Adds an additional subgraph (see example "multigraph")            |
| Graph      | setStyle(char*)          | Sets the style (use defines: STYLE_LINES, STYLE_DOTS).            |
| Graph      | setColor(char*)          | Sets the line color of the graph (use a 6 digit hexadecimal code) |
| Graph      | setMinX(int, const char *)| Sets the min x value of the co-system and a layout (auto, extend and fixed) |
| Graph      | setMaxX(int, const char *)| Sets the max x value of the co-system and a layout (auto, extend and fixed) |
| Graph      | setMinY(int, const char *)| Sets the min y value of the co-system and a layout (auto, extend and fixed) |
| Graph      | setMaxY(int, const char *)| Sets the max y value of the co-system and a layout (auto, extend and fixed) |
| Graph      | setLineWidth(float)      | Sets the line width                                               |
| Separator  | setHeight(float)         | Creates a line to separate parts of the experiment                |
| Separator  | setColor(char*)          | Sets the color of the line (use a 6 digit hexadecimal code)       |
| Info       | setInfo(char*)           | Sets the infotext                                                 |
| Info       | setColor(char*)          | Sets the font color (use a 6 digit hexadecimal code)              |
| Value      | setLabel(char*)          | Sets a label for the displayed value                              |
| Value      | setPrecision(int)        | Sets the amount of digits after the decimal point                 |
| Value      | setUnit(char*)           | Sets a unit for the displayed value                               |
| Value      | setColor(char*)          | Sets the font color (use a 6 digit hexadecimal code)              |
| Value      | setChannel(int)          | As explained above, just with one parameter (1-5)                 |
| Edit       | setLabel(char*)          | Sets label for the editfield                                      |
| Edit       | setUnit(char*)           | Sets unit                                                         |
| Edit       | setSigned(bool)          | true = signed values allowed                                      |
| Edit       | setDecimal(bool)         | true = decimal values allowed                                     |
| Edit       | setChannel(int)          | As explained above, just with one parameter (1-5)                 |
| ExportSet  | setLabel(char*)          | Sets a label for the exportSet (Used to export to Excel, etc.)    |
| ExportData | setLabel(char*)          | Sets a label for the exportData                                   |
| ExportData | setDatachannel(int)      | Defines which channel should be exported for this dataset (1-5)   |
| Everything | setXMLAttribute(char*)   | Custom property e.g. setXMLAttribute("lineWidth=\"3\"")           |


#### Style and Layout oprtions for setStyle and setMax/setMin
STYLE_LINES, STYLE_DOTS, STYLE_VBARS, STYLE_HBARS, STYLE_MAP

LAYOUT_AUTO, LAYOUT_EXTEND, LAYOUT_FIXED

#### Error messages

If for some reason the app shows you an error in form of "ERROR FOUND: ERR_X", with different values for X, this could be the reason:
* ERR_01: The input was too long.
* ERR_02: The value exceeds the upper limit.
* ERR_03: The input was not a 6-digit hexadecimal code.
* ERR_04: The input does not match with a valid value.
* ERR_05: The layout must be auto, extend or fixed.

If you realize that the microcontroller is continiously rebooting, you maybe added too many elements.

### getDataFromSmartphone.ino

The phyphox file format is much more powerful than what is offered by this library's interface. In the example `getDataFromSmartphone.ino` you can see how a phyphox-file can be used to set up a sensor on the phone and retrieve its data on the Arduino. In contrast to other examples, the phyphox-file is not generated by the library but instead loaded in phyphox with a QR code. You can also convert your manually created phyphox file into a header file and provide it to this library so it is submitted via Bluetooth.

As the phyphox file format is extremely complex and powerful, please refer to the [phyphox wiki](https://phyphox.org/wiki/index.php/Phyphox_file_format) to learn about it and feel free to contact us if you are stuck or think that a specific aspect of the file format should be easily accessible through our Arduino library.

### Further documentation

For now, this library is rather lightweight. Feel free to browse the `.h` files to learn about the functions that are already available.

## Missing features

In the future we would like to see...
- Some memory optimization
- Compression for the transfer of the phyphox experiment generated on the Arduino
- Option to request a larger mtu size
- Addition graph settings
- Proper documentation

If you can help with this, we are happy to receive a pull request. You can contact us via contact@phyphox.org if you plan on a large addition to this library and want to make sure that it does not collide with anything we are already working on.

## Credits

This library has been developed by the phyphox team at the RWTH Aachen University. In particular, the foundations and basic concept was created by Alexander Krampe as part of his Master thesis. The library has been further improved and is now maintained by Dominik Dorsel, our PhD student who also supervised Alexander's thesis. The library was also further optimized and extended with new features by Marcel Hagedorn and Edward Leier.

## Licence

This library is released under the GNU Lesser General Public Licence v3.0 (or newer).

## Contact

Contact us any time at contact@phyphox.org and learn more about phyphox on https://phyphox.org.

