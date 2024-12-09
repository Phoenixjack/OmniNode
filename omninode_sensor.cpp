/*
OmniNode_sensor.cpp
Author: Chris McCreary
Version: 0.0.1
Updated: 2024-02-06

A wrapper for all sensor handling functions
Features:
[ ] 
[ ] 
[ ] 


#define defDebugSensor false                          // Sensor config and reading
#define defFuncPacketFwd false                        // defNodeFunction bit 1 (1) (LSB)
#define defFuncINA3221 false                          // defNodeFunction bit 2 (2)
#define defFuncNEO6_7 false                           // defNodeFunction bit 3 (4)
#define defFuncHMC5883 true                           // defNodeFunction bit 4 (8)
#define defFuncBMP280 false                           // defNodeFunction bit 5 (16)
#define defFuncBMP680 false                           // defNodeFunction bit 6 (32)
#define defFuncMPU6050 false                          // defNodeFunction bit 7 (64)
#define defFuncADXL345 false                          // defNodeFunction bit 8 (128)
#define defFuncAS6500 false                           // defNodeFunction bit 9 (256)
// now for brevity, we assemble the flags using bitshifts into a single value
#define defNodeFunction ((defFuncAS6500 << 8) + (defFuncADXL345 << 7) + (defFuncMPU6050 << 6) + (defFuncBMP680 << 5) + (defFuncBMP280 << 4) + (defFuncHMC5883 << 3) + (defFuncNEO6_7 << 2) + (defFuncINA3221 << 1) + defFuncPacketFwd)

#ifndef defDebugSerialMonitor
#define defDebugSerialMonitor true // Global flag to enable/disable Serial Monitor debugging. Will override any other debug flag
#endif
#ifndef debugPrefix
#define debugPrefix(x) (x & defDebugSerialMonitor) ? Serial.printf("%s, %s\n", __FILE__, __FUNCTION__) \
                           : Serial.print("");  // defined macro to conditionally print file and function produced the output; accepts boolean flags (defDebug___)
#endif
#ifndef debugPrint
#define debugPrint(x, y) (x & defDebugSerialMonitor) ? Serial.print(y) \
                             : Serial.print("");  // defined macro to conditionally print file and function produced the output; accepts boolean flags (defDebug___)
#endif
#ifndef debugPrintln
#define debugPrintln(x, y) (x & defDebugSerialMonitor) ? Serial.println(y) \
                               : Serial.println("");  // defined macro to conditionally print file and function produced the output; accepts boolean flags (defDebug___)
#endif


#include <Adafruit_Sensor.h>    // https://github.com/adafruit/Adafruit_Sensor
#if (defFuncPacketFwd)
SoftwareSerial mySerial(2, 0);  // RX, TX  (D4 connected to logger out); GPIO 0, 2 => D4, D3
#endif
#if (defFuncINA3221)
#include <INA3221.h>                  // https://github.com/Tinyu-Zhao/INA3221
INA3221 ina3221(INA3221_ADDR40_GND);  //
#endif
#if (defFuncNEO6_7)
#include <Adafruit_GPS.h>      // https://github.com/adafruit/Adafruit_GPS
#define GPSECHO false          // Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSSerial Serial1      //
Adafruit_GPS GPS(&GPSSerial);  //
#endif
#if (defFuncHMC5883)
#include <DFRobot_QMC5883.h>           // https://github.com/DFRobot/DFRobot_QMC5883  This library covers the QMC5883, HMC5883, and VMC5993
DFRobot_QMC5883 compass(&Wire, 0x0D);  // I manually defined the I2C address because the first batch of 5883s received were QMCs masquerading as HMCs
#endif
#if (defFuncBMP280)
#include <Adafruit_BMP280.h>  // https://www.ndbc.noaa.gov/station_page.php?station=COVM2  Cove Point, MD      https://www.ndbc.noaa.gov/station_page.php?station=SLIM2 Solomons Island, MD
Adafruit_BMP280 bmp;          // https://www.ndbc.noaa.gov/station_page.php?station=NCDV2  Dahlgren            https://www.ndbc.noaa.gov/station_page.php?station=LWTV2 Lewisetta, VA
#endif
#if (defFuncBMP680)

#endif
#if (defFuncMPU6050)
#include <Adafruit_MPU6050.h>  // https://github.com/adafruit/Adafruit_MPU6050
Adafruit_MPU6050 mpu;          //
#endif
#if (defFuncADXL345)

#endif
#if (defFuncAS6500)
#include "AS5600.h"  // https://github.com/RobTillaart/AS5600
AS5600 as5600;
#endif

int intSeaLevelPressPa = 101325;                                                       // standard sea level pressure = 101,325 Pa; can update via mqtt subscribe later
*/
