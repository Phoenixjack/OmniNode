# Node Hardware

## Common / Standard hardware
 + I2C and TX/RX standard pins are used
 + Reconfig button
   - TBD...
 + RGB Indicator 
   - Uses GPIO 12,13,14. 
   - Common Cathode/ground: Could use GPIO15 or GPIO16; 16 (D0) goes HIGH at boot, but otherwise good; 15 (D8) boot will fail if pulled HIGH during startup
   - General guidance:
     + Colors
       o RED - fault
       o GREEN - WiFi
       o MAGENTA (purple) - MQTT
       o YELLOW - sensor initialization
       o BLUE - retrieving data from sensor
     + Dim light when starting a feature
     + Full brightness when feature succeeds
     + Solid red for terminal fault preventing further code execution
     + Alternating red and ___ for error while waiting (wifi/mqtt connection, sensor not ready, etc)
     + __FUTURE IMPLEMENTATION__: Long and short pulses for additional information
     + __FUTURE IMPLEMENTATION__: Timers for asynchronous ("multithreaded") indications VS delay statements
 + **__CAUTION__**
   - [Avoid problematic pins](https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/)
     + ESP8266 D3, D4, D8, RX, TX (GPIO0, 2, 15, 1, 3)
   - Most pins 20mA maximum, with 100 or 200mA max across all pins
   - Most pins 5V tolerant UNLESS wired directly to ESP8266 or ESP01

## Hardware Variations
 + MPU6050 - 
   o 6 Degree of Freedom gyro/acceleration sensor (includes gravity)
   o ±250, ±500, ±1000, and ±2000 °/sec (dps)
   o user-programmable accelerometer full-scale range of ±2g, ±4g, ±8g, and ±16g.
 + INA3221 - 
   o 3 channel
   o 13 bit ADCs
   o 26VDC max
   o bidirectional voltage/current sensor
   o high/low side compatible
   o RANGE??? - 5mA max on input pins. 100mohm standard shunt resistors. 163.8mV max shunt voltage
 + HMC5883
   o 12-bit ADC 
   o 1° to 2° compass heading accuracy
   o Measures milli-gauss to 8 gauss
   o 5 milli-gauss resolution
   o 0.75 up to 75 Hz continuous output rate

