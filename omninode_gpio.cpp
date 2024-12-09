/*
OmniNode_gpio.cpp
Author: Chris McCreary
Version: 0.0.1
Updated: 2024-02-06

A wrapper for all GPIO operations
Features:
[ ] 
[ ] 
[ ] 


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





#include <Wire.h>               // https://www.arduino.cc/en/Reference/wire
#include <RGBLed.h>             // https://github.com/wilmouths/RGBLed

#define defRED_PIN 13                                                       //
#define defGREEN_PIN 14                                                     //
#define defBLUE_PIN 12                                                      //
#define defBlinkShort 500                                                   // time in ms
#define defBlinkLong 750                                                    // time in ms
RGBLed led(defRED_PIN, defGREEN_PIN, defBLUE_PIN, RGBLed::COMMON_CATHODE);  // led instance. Red 13, Green 12, Blue 14, common ground LED / GPIOs are active high

const int intOnBoardLED = LED_BUILTIN;                                                 //

/* 
  led.brightness(RGBLed::WHITE, 50);                                               // boot with dim white state indicator until we get things working
  pinMode(16, OUTPUT);                                                             //ground for pin during T/S. TODO: REMOVE
  digitalWrite(16, LOW);                                                           //TODO: REMOVE
*/