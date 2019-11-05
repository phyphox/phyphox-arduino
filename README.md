# Mbed-ArduinoNano33BLE
Use phyphox to plot acquired data from Arduino (easy peasy)

What it does:
The file writes data read from Analog Pin 1 to a characteristic and these values are then sent to phyphox app.

The measured data (I used a simple variable voltage divider aka potentiometer) are then visualized in the app phyphox which you can download from www.phyphox.org. 

You do not need any further knowledge in phyphox it's simply used as a plotter for the data (although it can do a lot more). 
So if you ever wanted to get your data to a nice app that visualizes the data download phyphox and get a NanoBle 33 and use this file.

What you need (this is still a pre alpha version but it works...):
In your Arduino IDE install:
1. CRC32 library
2. ARM mbed library for Nano33 BLE
(3. maybe add c++11 compiler for the compiler flag I used several c++11 things...)
