# Mbed-ArduinoNano33BLE
Use phyphox to plot acquired data from Arduino (easy peasy)

What it does:
The file writes data read from Analog Pin 1 to a characteristic and these values are then sent to phyphox app which visualizes the received data from your Arduino Nano33 BLE.

You can download the app from Appstores. It is availible for iOS and Android (look for phyphox or visit www.phyphox.org)

You do not need any further knowledge in phyphox. The app is simply used as a plotter for the data (although it can do a lot more). 
So if you ever wanted to get your data to an app that visualizes the data download phyphox and get a NanoBle 33 and use this file.

What you need (my code is still a pre alpha version but it works...):
In your Arduino IDE install:
1. CRC32 library
2. ARM mbed library for Nano33 BLE
3. Add c++11 compiler flag for the compiler. I used several c++11 things...


In phyphox type the '+' and then the Bluetooth symbol. A device with name 'Arduino' will appear in the list. Click on it here you go. 


Take note: At the moment this won't work because I need to implement my own CRC32 function. The official CRC32 library produces a compiler error because it defines 'PinModes' and overwrites the existing 'PinModes' from mbed library. IF you still want to compile it you need to delete the '#include <Arduino.h>' in the CRC32 library then it will work. Nasty stuff. 
