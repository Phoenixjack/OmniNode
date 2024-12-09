/*
OmniNode_serial.cpp
Author: Chris McCreary
Version: 0.0.1
Updated: 2024-02-06

A wrapper for all Serial I/O functions
Features:
[ ] Monitors default Serial Monitor for JSON command packets
[ ] Monitors second Serial port for incoming data (part of Packet Forwarding)


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

#include <SoftwareSerial.h>     // https://docs.arduino.cc/learn/built-in-libraries/software-serial


*/