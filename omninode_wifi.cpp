/*
OmniNode_WiFi.cpp
Author: Chris McCreary
Version: 0.0.1
Updated: 2024-02-06

A wrapper for all WiFi and MQTT handling functions
Features:
[ ] Uses tzapu's WiFiManager library to provide an AP and WiFi config portal if we fail to connect to a saved SSID
[ ] If WiFi succeeds but MQTT fails, use UDP to initiate MQTT broker discovery
[ ] Launch REST API listener to accept commands


#define defRestServerEnable false  // enables command input via WiFi REST API on port 81
#define defDebugWiFi false         // WiFi actions
#define defDebugServer false       // debugging of REST API
#define defDebugMQTTTerse true     // MQTT critical actions
#define defDebugMQTTVerbose true   // MQTT non-critical actions
#define defDebugMQTTTransmit true  //
#define defDebugMQTTReceive true   //
#define defDebugTime false         // time functions

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





#include <WiFiManager.h>        // https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>        // https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html
#include <ESP8266WebServer.h>   // https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer  https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/examples/WebServer#registering-simple-functions-to-implement-restful-services
#include <DNSServer.h>          // https://github.com/esp8266/Arduino/blob/master/libraries/DNSServer/examples/DNSServer/DNSServer.ino
#include <ArduinoMqttClient.h>  // https://github.com/arduino-libraries/ArduinoMqttClient
#include <NTPClient.h>          // https://github.com/arduino-libraries/NTPClient
#include <WiFiUdp.h>            // https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/udp-examples.html

const String strClientID = String(ESP.getChipId(), HEX);                               // global placeholder for my unique MAC ID
bool boolMQTTEmulated = false;                                                         // global flag to bypass wifi & mqtt connections in favor of Serial input/output
bool boolSaveWiFiConfig = false;                                                       // flag for saving WiFiManager data
String strMQTTserver = "192.168.1.68";                                                 // global placeholder for mqtt server address; to be overwritten by data from file or user via the config portal
int intMQTTPort = 1883;                                                                // set default value, but leave subject to change by user
char charMQTTServer[40];                                                               //
char charMQTTPort[6] = "1883";                                                         //

unsigned long ul_lastupdate = 0;                                                       // timestamp (in local millis) of last NTP sync. FOLLOWUP: does this need to be global or can we make it local/static?
unsigned long ul_lastMsg = 0;                                                          // timestamp (in local millis) of last message sent. FOLLOWUP: does this need to be global or can we make it local/static?
int intReportInterval = 3000;                                                          // default minimum time between reports in milliseconds. Leave global and variable
int intMsgCnt = 0;                                                                     // for tracking number of outgoing messages. FOLLOWUP: will this be relevant once we go to QoS 1 or 2?
int intAvgRSSI = 0;                                                                    // for tracking average WIFI strength. FOLLOWUP: does this need to be global or can we make it local/static?
WiFiClient instWiFiClient;                                                             //
MqttClient instMQTTClient(instWiFiClient);                                             //
IPAddress instBroadcast = IPAddress(224, 0, 1, 3);                                     // manually set UDP address for auto-negotiation
unsigned int uintLocalUDPPort = 8266;                                                  //
WiFiUDP instUDP;                                                                       //
WiFiManager wifiManager;                                                               // global intialization.
NTPClient instNTPClient(instUDP, strMQTTserver.c_str(), 0, 600000);                    // local time offset is -14400 (if used); autoresync every 600,000 msec (10 min)
WiFiManagerParameter custom_mqtt_server("server", "mqtt server", charMQTTServer, 40);  // has to be declared globally; FOLLOWUP: IF we want to display the previously saved data, we'll need to load the file and parse before this point
WiFiManagerParameter custom_mqtt_port("port", "mqtt port", charMQTTPort, 6);           // has to be declared globally
#if (defRestServerEnable)
ESP8266WebServer objRestServer(81);
#endif

#if (defRestServerEnable)
    objRestServer.on("/", handleRoot);
    objRestServer.on("/postplain/", handlePlain);
    objRestServer.on("/postform/", handleForm);
    objRestServer.onNotFound(handleNotFound);
    objRestServer.begin();
    objRestServer.println("HTTP server started");
#endif
*/