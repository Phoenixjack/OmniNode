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
   o User-programmable accelerometer full-scale range of ±2g, ±4g, ±8g, and ±16g
   o [Datasheet](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf)
 + ADXL345 - 
   o 3 axis accelerometer
   o User-programmable accelerometer full-scale range of ±2g, ±4g, ±8g, and ±16g
   o 10-13 bit resolution
   o 32-256 LSB per g
   o ~±1% accuracy
   o Self-test feature using D7 of address 0x31; not supported by Adafruit library
   o [Datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/adxl345.pdf)
 + INA3221 - 
   o 3 channel
   o 13 bit ADCs
   o 0-26VDC
   o Bidirectional voltage/current sensor
   o High or Low side compatible
   o RANGE??? - 5mA max on input pins. 100mohm standard shunt resistors. ±163.8mV max shunt voltage
   o Accuracy - 8mV per LSB on bus / 40uV per LSB on shunt
   o [Datasheet](https://www.ti.com/lit/ds/symlink/ina3221.pdf?ts=1704300495483)
 + HMC5883/QMC5883
   o 3 axis magneto-resistive sensors
   o 12-bit ADC 
   o 1° to 2° compass heading accuracy
   o ±8 gauss range
   o 5 milli-gauss resolution
   o Continuous output rate: (HMC) 0.75 to 75 Hz / (QMC) 10 to 200 Hz 
   o [Datasheet](https://www.farnell.com/datasheets/1683374.pdf)
   o [Datasheet](https://datasheet.lcsc.com/lcsc/QST-QMC5883L-TR_C192585.pdf)
+ BMP280
   o 300-1100hPa (-500 to +9,000 meter equivalent) 
   o Relative accuracy: ±12 Pa, equiv. to ±1 m @ 25 °C
   o Absolute accuracy: ~±1 hPa
   o Temperature coefficient offset: ±1.5 Pa/K
   o Temperature range: -40 to +85 °C
   o [Datasheet](https://www.bosch-sensortec.com/media/boschsensortec/downloads/product_flyer/bst-bmp280-fl000.pdf)
+ BME680
   o 300-1100hPa (-500 to +9,000 meter equivalent) 
   o Relative accuracy: 0.12 Pa, equiv. to 1.7 cm
   o Absolute accuracy: ~±1 hPa
   o Relative humidity accuracy: ±3% r.H.
   o Temperature coefficient offset: ±1.3 Pa/K
   o Temperature range: -40 to +85 °C
   o [Datasheet](https://cdn-shop.adafruit.com/product-files/3660/BME680.pdf)
+ NEO-6M/-7M GPS
   o Max Navigation Update rate: 5Hz (NEO-6) / 10Hz (NEO-7)
   o Horizontal accuracy: 2.5 meter
   o Heading accuracy: 0.5 degrees
   o Velocity accuracy: 0.1 meters/sec
   o [Datasheet](https://content.u-blox.com/sites/default/files/products/documents/NEO-6_DataSheet_%28GPS.G6-HW-09005%29.pdf)
   o [Datasheet](https://content.u-blox.com/sites/default/files/products/documents/NEO-7_DataSheet_%28UBX-13003830%29.pdf)
