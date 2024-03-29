
// See Version History page for full details
#include <FS.h>                 // SPIFFS outdated, but apparently used as a basis for LittleFS
#include <LittleFS.h>           // https://github.com/esp8266/Arduino/tree/master/libraries/LittleFS
#include <Wire.h>               // https://www.arduino.cc/en/Reference/wire
#include <WiFiManager.h>        // https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>        // https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html
#include <ESP8266WebServer.h>   // https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer
#include <DNSServer.h>          // https://github.com/esp8266/Arduino/blob/master/libraries/DNSServer/examples/DNSServer/DNSServer.ino
#include <ArduinoMqttClient.h>  // https://github.com/arduino-libraries/ArduinoMqttClient
#include <NTPClient.h>          // https://github.com/arduino-libraries/NTPClient
#include <WiFiUdp.h>            // required for NTPClient. This code does not include any calls directly to it.
#include <ArduinoJson.h>        // https://arduinojson.org/?utm_source=meta&utm_medium=library.properties

//#define FORMAT_LITTLEFS_IF_FAILED true                    // global flag to enable formatting of filesystem if mount fails
#define defSoftwareVersion "OmniNodeV0_0_3"               // Used in MQTT config report.
#define defNodeFunction "mpu6500"                         // compile time definition to enable/disable code segments. Also used in MQTT config report
const String strClientID = String(ESP.getChipId(), HEX);  // global placeholder for my unique MAC ID
const String strConfigFilename = "/config.json";          // filename for saving our config
bool boolMQTTEmulated = false;                            // global flag to bypass wifi & mqtt connections in favor of Serial input/output
bool boolSaveWiFiConfig = false;                          // flag for saving WiFiManager data
String strMQTTserver = "192.168.0.17";                    // global placeholder for mqtt server address; to be overwritten by data from file or user via the config portal
int intMQTTPort = 1883;                                   // set default value, but leave subject to change by user
char charMQTTServer[40];
char charMQTTPort[6] = "1883";
unsigned long ul_lastupdate = 0;  // timestamp (in local millis) of last NTP sync. FOLLOWUP: does this need to be global or can we make it local/static?
unsigned long ul_lastMsg = 0;     // timestamp (in local millis) of last message sent. FOLLOWUP: does this need to be global or can we make it local/static?
int intReportInterval = 1500;     // default minimum time between reports in milliseconds. Leave global and variable
int intMsgCnt = 0;                // for tracking number of outgoing messages. FOLLOWUP: will this be relevant once we go to QoS 1 or 2?
int intAvgRSSI = 0;               // for tracking average WIFI strength. FOLLOWUP: does this need to be global or can we make it local/static?
const char broker[] = "192.168.0.17";
int port = 1883;

WiFiClient instWiFiClient;                                          //
MqttClient instMQTTClient(instWiFiClient);                          //
WiFiUDP instUDP;                                                    //
WiFiManager wifiManager;                                            // global intialization. FOLLOWUP: Does this need to be global or can we move to the portal handling subroutine? Variables it needs can be at a higher scope.
NTPClient instNTPClient(instUDP, strMQTTserver.c_str(), 0, 60000);  // local time offset is -14400; autoresync every 1000 seconds (~16 minutes)
WiFiManagerParameter custom_mqtt_server("server", "mqtt server", charMQTTServer, 40);
WiFiManagerParameter custom_mqtt_port("port", "mqtt port", charMQTTPort, 6);

void setup() {                                                 // initial setup
  Serial.begin(115200);                                        // initialize the Serial Monitor interface for debugging. TODO: Global debug flag to remove all Serial
  cmd_mountfilesystem();                                       // mount file system. will format it if mounting fails
  delay(1000);                                                 // delay for a while to give Serial Monitor time to catch up with board reset. TODO: remove for production code
  if (boolMQTTEmulated) {                                      // is emulation enabled?
    Serial.println("WiFi and MQTT functions emulated.");       // if so, warn the user
    Serial.print("Enter compact JSON in Serial Monitor ");     // and tell them how to interact with us
    Serial.println("to simulate sending the node a message");  //
    Serial.println("");                                        //
  } else {                                                     // emulation isn't enabled, so we do a normal startup
    setup_wifi();                                              //
    instNTPClient.begin();                                     // connect to NTP server.
    instNTPClient.update();                                    // force an update. FOLLOWUP: should this be moved to a separate function that's handled after we connect to the broker?
    ul_lastupdate = millis();                                  // mark the time
    setup_mqtt();                                              // start up MQTT
  }
}

void loop() {
  if (Serial.available() > 0) { getSerialInput(); }                     // regardless of whether we're emulating, allow serial inputs TODO: replace with REST library
  if (!boolMQTTEmulated) {                                              // only do mqtt stuff if we're not emulating
    instMQTTClient.poll();                                              // listens for MQTT messages AND sends keepalive messages to avoid disconnects; required for both purposes.}
  }                                                                     //
  if (millis() - ul_lastMsg > intReportInterval) {                      // old if statement for sending regular reports. FOLLOWUP: what if we have multiple datapoints we want to send at different intervals? Could we convert to ArduinoTicker library?
    ul_lastMsg = millis();                                              // set timestamp to now. Placeholder for message packaging is below.
    //Serial.print(CurrentUTCTime());                             //
    //Serial.print(" ");                                          //
    Serial.print(boolMQTTEmulated);     //
    Serial.print(" ");                  //
    Serial.println(intReportInterval);  //
  }                                     //
}