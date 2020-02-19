# phyphox Arduino Nano 33 BLE 
Use phyphox to plot sensor data from your Arduino. 

The purpose of this library is to use the phyphox app (see www.phyphox.org) to plot sensor data on your phone. phyphox is much more more than only 'plotting your data'. You can also perform data analysis with it. 


What you will need:

1.) One of the following Boards:
    - Arduino Nano 33 Ble 
    - Arduino Nano Sense
    - Arduino Nano IoT
    - ESP32

2.) The drivers for the Arduino Nano 33 Ble/IoT should be installed automatically. After that navigate to Tools -> Boards -> Board Manager and search 'Arduino nano 33 ble'. A package with the word 'mbed os' should appear. It shoudl look like this. Install it.

![alt text](https://github.com/AlexanderKrampe/Mbed-ArduinoNano33BLE/blob/master/IDE.png)


Great. Now let's take a look at the phyphox.ino file to see how it works. 

- Let's look at the first two lines. 
In line 1 we include our phyphox library that establishes a Bluetooth Low Energy server and manages the data transfer. In line 2 we create a server. 

- The next step is to start our server. We do this in the setup() function by calling: server.start(). If you have apps like 'GattServer' or 'nrfConnect' on your phone and you should now see a bluetooth device named 'Arduino'. You can also check this with phyphox if you click on the '+' (bottom right) and then click on the bluetooth icon. A device named Arduino should appear in the list. 

- In line 12 we read in a voltage from something connected to Pin A1 (ok it's not a voltage yet...it's a value between 0 and 1023). In line 13 we map this value to an actual voltage. You can use a simple voltage divider or whatever you want. 


- OK now we want to send our voltage to phyphox. We do this by calling: server.write(voltage). The delay is optional you can also change the value. Now in phyphox connecet to Arduino. After the experiment has been transferred click on the 'play' symbol in the upper left corner to start your data capture. If everything worked fine we should see something like this. 

![alt text](https://github.com/AlexanderKrampe/Mbed-ArduinoNano33BLE/blob/master/exp.png)
