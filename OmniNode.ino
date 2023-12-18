// See Version History page for full details
#define defDebugFull true                    // flag to turn on/off serial print statements for troubleshooting
#define defDebugFileFunction true            // flag to turn on/off serial printing of what file and function produced the output
#define defSoftwareVersion "OmniNodeV0_0_4"  // Used in MQTT config report.
#define defFuncPacketFwd false               // defNodeFunction bit 1 (1) (LSB)
#define defFuncINA3221 false                 // defNodeFunction bit 2 (2)
#define defFuncNEO6_7 false                  // defNodeFunction bit 3 (4)
#define defFuncHMC5883 true                  // defNodeFunction bit 4 (8)
#define defFuncBMP280 false                  // defNodeFunction bit 5 (16)
#define defFuncBMP680 false                  // defNodeFunction bit 6 (32)
#define defFuncMPU6050 false                 // defNodeFunction bit 7 (64)
#define defFuncADXL345 false                 // defNodeFunction bit 8 (128)
#define defFuncAS6500 false                  // defNodeFunction bit 9 (256)
// now for brevity, we assemble the flags using bitshifts into a single value
#define defNodeFunction ((defFuncAS6500 << 8) + (defFuncADXL345 << 7) + (defFuncMPU6050 << 6) + (defFuncBMP680 << 5) + (defFuncBMP280 << 4) + (defFuncHMC5883 << 3) + (defFuncNEO6_7 << 2) + (defFuncINA3221 << 1) + defFuncPacketFwd)
#if (defDebugFull)
#define debugoutput(x) Serial.print(x)
#define debugoutputln(x) Serial.println(x)
#else
#define debugoutput(x)
#define debugoutputln(x)
#endif
#if (defDebugFileFunction)
#define debugprefix() Serial.printf("%s: %s: \n", __FILE__, __func__)
#else
#define debugprefix()
#endif
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
#include <Adafruit_Sensor.h>    // https://github.com/adafruit/Adafruit_Sensor
#include <RGBLed.h>             // https://github.com/wilmouths/RGBLed

#define RED_PIN 13                                                 //
#define GREEN_PIN 14                                               //
#define BLUE_PIN 12                                                //
#define defBlinkShort 500                                          // time in ms
#define defBlinkLong 500                                           // time in ms
RGBLed led(RED_PIN, GREEN_PIN, BLUE_PIN, RGBLed::COMMON_CATHODE);  // led instance. Red 13, Green 12, Blue 14, common ground LED / GPIOs are active high

#if (defFuncPacketFwd)
#include <SoftwareSerial.h>     // https://docs.arduino.cc/learn/built-in-libraries/software-serial
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


const String strClientID = String(ESP.getChipId(), HEX);  // global placeholder for my unique MAC ID
const String strConfigFilename = "/config.json";          // filename for saving our config
bool boolMQTTEmulated = false;                            // global flag to bypass wifi & mqtt connections in favor of Serial input/output
bool boolSaveWiFiConfig = false;                          // flag for saving WiFiManager data
String strMQTTserver = "192.168.0.17";                    // global placeholder for mqtt server address; to be overwritten by data from file or user via the config portal
int intMQTTPort = 1883;                                   // set default value, but leave subject to change by user
char charMQTTServer[40];                                  //
char charMQTTPort[6] = "1883";                            //
int intSeaLevelPressPa = 101325;                          // standard sea level pressure = 101,325 Pa; can update via mqtt subscribe later
unsigned long ul_lastupdate = 0;                          // timestamp (in local millis) of last NTP sync. FOLLOWUP: does this need to be global or can we make it local/static?
unsigned long ul_lastMsg = 0;                             // timestamp (in local millis) of last message sent. FOLLOWUP: does this need to be global or can we make it local/static?
int intReportInterval = 1500;                             // default minimum time between reports in milliseconds. Leave global and variable
int intMsgCnt = 0;                                        // for tracking number of outgoing messages. FOLLOWUP: will this be relevant once we go to QoS 1 or 2?
int intAvgRSSI = 0;                                       // for tracking average WIFI strength. FOLLOWUP: does this need to be global or can we make it local/static?

WiFiClient instWiFiClient;                                                             //
MqttClient instMQTTClient(instWiFiClient);                                             //
WiFiUDP instUDP;                                                                       //
WiFiManager wifiManager;                                                               // global intialization.
NTPClient instNTPClient(instUDP, strMQTTserver.c_str(), 0, 600000);                    // local time offset is -14400 (if used); autoresync every 600,000 msec (10 min)
WiFiManagerParameter custom_mqtt_server("server", "mqtt server", charMQTTServer, 40);  // has to be declared globally
WiFiManagerParameter custom_mqtt_port("port", "mqtt port", charMQTTPort, 6);           // has to be declared globally


void setup() {           // initial setup
  Serial.begin(115200);  // initialize the Serial Monitor interface for debugging
#if (defDebugFull)
  while (!Serial) delay(100);  // wait for Serial Monitor. TODO: Add debug statement to disable waiting for serial readiness
#endif
  led.brightness(RGBLed::WHITE, 50);                          // boot with fault state indicator until we get things working
  delay(1000);                                                // delay for a while to give Serial Monitor time to catch up with board reset. TODO: remove for production code
  cmd_mountfilesystem();                                      // mount file system. will format it if mounting fails
  setup_sensors();                                            // function to initialize whatever sensor we're using
  if (boolMQTTEmulated) {                                     // is emulation enabled?
    debugprefix();                                            // and tell them how to interact with us
    debugoutput("WiFi and MQTT functions emulated\n");        //
    debugoutput("Enter compact JSON in Serial Monitor ");     //
    debugoutput("to simulate sending the node a message\n");  //
  } else {                                                    // emulation isn't enabled, so we do a normal startup
    setup_wifi();                                             //
    instNTPClient.begin();                                    // connect to NTP server.
    instNTPClient.update();                                   // force an update. FOLLOWUP: should this be moved to a separate function that's handled after we connect to the broker?
    ul_lastupdate = millis();                                 // mark the time
    setup_mqtt();                                             // start up MQTT
  }
}

void loop() {
  if (Serial.available() > 0) { getSerialInput(); }  // regardless of whether we're emulating, allow serial inputs TODO: replace with REST library
  if (!boolMQTTEmulated) {                           // only do mqtt stuff if we're not emulating
    instMQTTClient.poll();                           // listens for MQTT messages AND sends keepalive messages to avoid disconnects; required for both purposes.}
  }                                                  //
  if (millis() - ul_lastMsg > intReportInterval) {   // old if statement for sending regular reports. FOLLOWUP: what if we have multiple datapoints we want to send at different intervals? Could we convert to ArduinoTicker library?
    ul_lastMsg = millis();                           // set timestamp to now. Placeholder for message packaging is below.
    debugoutput("interval: ");                       // Serial monitor readout of report interval
    debugoutput(intReportInterval);                  //
    cmd_getsensordata();                             //
  }                                                  //
}
