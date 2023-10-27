// See Version History page for full details

#include <ESP8266WiFi.h>        // https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html
#include <ArduinoMqttClient.h>  // https://github.com/arduino-libraries/ArduinoMqttClient
#include <NTPClient.h>          // https://github.com/arduino-libraries/NTPClient
#include <WiFiUdp.h>            // required for NTPClient. This code does not include any calls directly to it.
#include <ArduinoJson.h>        // https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
#include <UnixTime.h>           // https://github.com/GyverLibs/UnixTime   TODO: find a better library?

const char* charrSSID = "Triton";              // Triton FBISecurityVan2 Verizon-MiFi8800L-75CE Verizon-MiFi8800L-Guest-75CF
const char* charrWiFiPass = "az52p7fvnc";      // az52p7fvnc 773UT2PYWMBH75WD a2e63a23 586c9574
const char* charrMQTTServer = "192.168.0.17";  // 192.168.0.17
const int intMQTTPort = 1883;                  // set MQTT port

WiFiClient instWiFiClient;                                    //
MqttClient instMQTTClient(instWiFiClient);                    //
WiFiUDP instUDP;                                              //
NTPClient instNTPClient(instUDP, charrMQTTServer, 0, 60000);  // local time offset is -14400; autoresync every 1000 seconds (~16 minutes)
UnixTime instTimeConvert(0);                                  // time conversion instance with no time zone offset

#define defSoftwareVersion "OmniNodeV0_0_2"               // Used in MQTT config report.
#define defNodeFunction "mpu6500"                         // compile time definition to enable/disable code segments. Also used in MQTT config report
bool boolMQTTEmulated = true;                             // global flag to bypass wifi & mqtt connections in favor of Serial input/output
const String strClientID = String(ESP.getChipId(), HEX);  // global placeholder for my unique MAC ID.
unsigned long ul_lastupdate = 0;                          // timestamp (in local millis) of last NTP sync. FOLLOWUP: does this need to be global or can we make it local/static?
unsigned long ul_lastMsg = 0;                             // timestamp (in local millis) of last message sent. FOLLOWUP: does this need to be global or can we make it local/static?
int intReportInterval = 1500;                             // default minimum time between reports in milliseconds. Leave global and variable
int intMsgCnt = 0;                                        // for tracking number of outgoing messages. FOLLOWUP: will this be relevant once we go to QoS 1 or 2?
int intAvgRSSI = 0;                                       // for tracking average WIFI strength. FOLLOWUP: does this need to be global or can we make it local/static?
//const char inTopic[] = "arduino/in";                   // TODO: build off this to create enum of MQTT topics

void setup() {                                                                                      // initial setup
  Serial.begin(115200);                                                                             // initialize the Serial Monitor interface for debugging. TODO: Global debug flag to remove all Serial
  delay(1000);                                                                                      // delay for a while to give Serial Monitor time to catch up with board reset. TODO: remove for production code
  if (boolMQTTEmulated) {                                                                           // is emulation enabled?
    Serial.println("WiFi and MQTT functions emulated.");                                            // if so, warn the user
    Serial.println("Enter compact JSON in Serial Monitor to simulate sending the node a message");  // and tell them how to interact with us
    Serial.println("");                                                                             //
  } else {                                                                                          // emulation isn't enabled, so we do a normal startup
    setup_wifi();                                                                                   // start wifi. TODO: Replace with wifi manager portal
    instNTPClient.begin();                                                                          // connect to NTP server.
    instNTPClient.update();                                                                         // force an update. FOLLOWUP: should this be moved to a separate function that's handled after we connect to the broker?
    ul_lastupdate = millis();                                                                       // mark the time
    setup_mqtt();                                                                                   // start up MQTT
  }
}

void loop() {
  if (Serial.available() > 0) { getSerialInput(); }                                 // regardless of whether we're emulating, allow serial inputs TODO: replace with REST library
  if (!boolMQTTEmulated) {                                                          // only do mqtt stuff if we're not emulating
    if (!instMQTTClient.connect(charrMQTTServer, intMQTTPort)) { mqtt_connect(); }  // check if we're connected to MQTT broker. If not, reestablish.
    instMQTTClient.poll();                                                          // listens for MQTT messages AND sends keepalive messages to avoid disconnects; required for both purposes.}
  }                                                                                 //
  if (millis() - ul_lastMsg > intReportInterval) {                                  // old if statement for sending regular reports. FOLLOWUP: what if we have multiple datapoints we want to send at different intervals? Could we convert to ArduinoTicker library?
    ul_lastMsg = millis();                                                          // set timestamp to now. Placeholder for message packaging is below.
    Serial.print(CurrentUTCTime());                                                 //
    Serial.print(" ");                                                              //
    Serial.print(boolMQTTEmulated);                                                 //
    Serial.print(" ");                                                              //
    Serial.println(intReportInterval);                                              //
  }                                                                                 //
}

void setup_wifi() {                        // Initialize WiFi
  Serial.print("Connecting to ");          //
  Serial.println(charrSSID);               //
  WiFi.mode(WIFI_STA);                     //
  WiFi.begin(charrSSID, charrWiFiPass);    //
  while (WiFi.status() != WL_CONNECTED) {  //
    delay(500);                            //
    Serial.print(".");                     //
  }                                        //
  Serial.println();                        //
  Serial.println(WiFi.macAddress());       //
  Serial.println(WiFi.gatewayIP());        //
  Serial.println(WiFi.localIP());          //
  WiFi.setHostname(strClientID.c_str());   //
  delay(1000);                             //
}

void setup_mqtt() {                                                                               // Initialize MQTT and connect to the broker
  const char charrWillTopic[] = "node/staus";                                                     //
  String strWillPayload = encapsulatemessage("OFFLINE");                                          //
  bool boolWillRetain = true;                                                                     //
  int intWillQoS = 1;                                                                             //
  instMQTTClient.beginWill(charrWillTopic, strWillPayload.length(), boolWillRetain, intWillQoS);  //
  instMQTTClient.print(strWillPayload);                                                           //
  instMQTTClient.endWill();                                                                       //
  instMQTTClient.connect(charrMQTTServer, intMQTTPort);                                           // initialize MQTT instance
  instMQTTClient.onMessage(onMqttMessage);                                                        // assign function call for incoming messages
  mqtt_connect();                                                                                 // connect and handle handshake protocol
}

void mqtt_connect() {                                              // TODO: should we add a section to troubleshoot underlying wifi issues, or is that built into MQTT pubsub?
  while (!instMQTTClient.connected()) {                            // Loop until we're reconnected
    Serial.print("Attempting MQTT connection...");                 //
    if (instMQTTClient.connect(charrMQTTServer, intMQTTPort)) {    // Attempt to connect
      Serial.println("connected");                                 //
      transmitmqttmessage("base/announce", strClientID, true, 1);  // simple announcement that will trigger the server to check its database if it's seen this node before
      cmd_areyouthere();                                           // keep it simple and just act like we were asked to checkin via msg
      String strSubscribeTopic = "node/config/" + strClientID;     // assemble unique MQTT topic for this node to listen for.
      Serial.print("Subscribing to: ");                            // TODO: check if we need to subscribe every time we connect. What if we just disconnected from the broker and it's not an initial connection? Is the subscription retained?
      Serial.println(strSubscribeTopic);                           // show the user
      instMQTTClient.setId(strClientID);                           //
      instMQTTClient.subscribe(strSubscribeTopic.c_str(), 1);      // subscribe to that topic with QoS 1
    } else {                                                       //
      Serial.print("failed, rc=");                                 //
      Serial.print(instMQTTClient.connectError());                 //
      Serial.println(" try again in 5 seconds");                   //
      delay(5000);                                                 // Wait 5 seconds before retrying
    }
  }
}

void onMqttMessage(int intMessageSize) {                                         // Routine for handling incoming messages
  StaticJsonDocument<200> objJSONdoc;                                            // Should this be converted to dynamic? Or do we need a larger size?
  Serial.print("Received a message with topic '");                               //
  Serial.print(instMQTTClient.messageTopic());                                   //
  Serial.print("', duplicate = ");                                               //
  Serial.print(instMQTTClient.messageDup() ? "true" : "false");                  //
  Serial.print(", QoS = ");                                                      //
  Serial.print(instMQTTClient.messageQoS());                                     //
  Serial.print(", retained = ");                                                 //
  Serial.print(instMQTTClient.messageRetain() ? "true" : "false");               //
  Serial.print("', length ");                                                    //
  Serial.print(intMessageSize);                                                  //
  Serial.println(" bytes:");                                                     //
  String strPayload = "";                                                        //
  while (instMQTTClient.available()) {                                           // read mqtt buffer into a string
    strPayload += (char)instMQTTClient.read();                                   // make sure to use the character represented, NOT the raw ASCII code
  }                                                                              //
  DeserializationError boolJSONError = deserializeJson(objJSONdoc, strPayload);  //
  if (boolJSONError) {                                                           //
    Serial.print("deserializeJson() failed: ");                                  //
    Serial.println(boolJSONError.f_str());                                       //
    String strXmitMsg = singleJSONobject(strClientID, "deserialization error");  // assemble short message reporting a problem. TODO: is there a way to feedback a message identifier so the server even knows which message we're talking about?
    transmitmqttmessage("node/errors", strXmitMsg, true, 1);                     // send it to the error reporting topic
    return;                                                                      // if we have a deserialization error, then we want to abort this routine because we can't decipher what was sent
  }                                                                              //
  String strRcvdCmd = objJSONdoc["command"];                                     //
  String strRcvdValue = objJSONdoc["value"];                                     // TODO: use enum and covert the mess below to a SWITCH CASE structure
  processreceivedcommand(strRcvdCmd, strRcvdValue);                              // process what we received
}

void processreceivedcommand(String strRcvdCmd, String strRcvdValue) {           // process incoming messages. This has been separated from OnMQTTMessage so we can manually invoke on Serial inputs
  int intRcvdValue = strRcvdValue.toInt();                                      // convert to integer
  if (strRcvdCmd == "reboot") { cmd_reboot(); }                                 //
  if (strRcvdCmd == "reportin") { cmd_areyouthere(); }                          //
  if (strRcvdCmd == "ntpresync") { cmd_ntpresync(intRcvdValue); }               // force an NTP resync, and pass it the server provided timezone offset. For now, it's only for display purposes
  if (strRcvdCmd == "reportinterval") { cmd_newreportinterval(intRcvdValue); }  //
  if (strRcvdCmd == "reportcommands") { cmd_listcommands(); }                   //
  if (strRcvdCmd == "reportconfig") { cmd_reportconfig(); }                     //
  if (strRcvdCmd == "reportdiagnostics") { cmd_reportdiagnostics(); }           //
  if (strRcvdCmd == "resetdiagnostics") { cmd_resetdiagnostics(); }             //
}

void getSerialInput() {                                                          // reads raw JSON from Serial input. WARNING: NO INPUT VALIDATION!
  String strPayload = Serial.readString();                                       //
  StaticJsonDocument<200> objJSONdoc;                                            //
  DeserializationError boolJSONError = deserializeJson(objJSONdoc, strPayload);  //
  if (boolJSONError) {                                                           //
    Serial.print("deserializeJson() failed: ");                                  //
    Serial.println(boolJSONError.f_str());                                       //
    String strXmitMsg = singleJSONobject(strClientID, "deserialization error");  // assemble short message reporting a problem. TODO: is there a way to feedback a message identifier so the server even knows which message we're talking about?
    transmitmqttmessage("node/errors", strXmitMsg, true, 1);                     // send it to the error reporting topic
    return;                                                                      // if we have a deserialization error, then we want to abort this routine because we can't decipher what was sent
  }                                                                              //
  String strRcvdCmd = objJSONdoc["command"];                                     //
  String strRcvdValue = objJSONdoc["value"];                                     // TODO: use enum and covert the mess below to a SWITCH CASE structure
  processreceivedcommand(strRcvdCmd, strRcvdValue);                              // process what we received
}

void cmd_areyouthere() {                                                         // simple response routine to respond to MQTT checkin messages.
  String strXmitMsg = encapsulatemessage(String(instNTPClient.getEpochTime()));  // MIGHT BE AVOIDABLE by querying the MQTT broker's KEEPALIVE stats. Should be doable and would reduce network traffic
  transmitmqttmessage("node/lastcheckin", strXmitMsg, true, 1);                  // send current UTC as our last checkin time
  strXmitMsg = encapsulatemessage("online");                                     // TODO: make the server update this topic after successfully processing our last checkin time
  transmitmqttmessage("node/status", strXmitMsg, true, 1);                       //
}

void cmd_reportconfig() {                                        // save on network traffic and packet size by reporting config ONLY when requested
  String strXmitMsg = "";                                        // initialize msg payload holder
  DynamicJsonDocument objJSONdoc(1024);                          // initialize blank JSON doc
  objJSONdoc["ID"] = strClientID;                                // explicitly define my ID as my MAC
  objJSONdoc["function"] = defNodeFunction;                      // declare my assigned sensor function
  objJSONdoc["softwareversion"] = defSoftwareVersion;            // report this software version
  objJSONdoc["IP"] = WiFi.localIP();                             // not really necessary with MQTT, but for the detail loving humans
  objJSONdoc["CurrReportInterval"] = String(intReportInterval);  // repeatback for the record. Should be used server side in the watchdog function.
  objJSONdoc["commandtopic"] = "node/config/" + strClientID;     // store this node's unique command topic in the server database
  serializeJson(objJSONdoc, strXmitMsg);                         // assemble basic JSON object into tempmsg
  strXmitMsg = encapsulatemessage(strXmitMsg);                   // now, package that object into an object, and setting the ClientID as the key. redundant, but makes server side processing easier
  Serial.print("Reporting config: ");                            //
  Serial.println(strXmitMsg);                                    // diagnostic readout
  transmitmqttmessage("node/config", strXmitMsg, true, 1);       // send it
}

void cmd_reportdiagnostics() {                                   // save on network traffic and packet size by reporting diagnostics ONLY when requested
  String strXmitMsg = "";                                        // initialize msg payload holder
  DynamicJsonDocument objJSONdoc(1024);                          // initialize blank JSON doc
  objJSONdoc["ID"] = strClientID;                                // explicitly define my ID as my MAC
  objJSONdoc["WiFiStr_Curr"] = WiFi.RSSI();                      // report curr WiFiStr
  objJSONdoc["WiFiStr_Avg"] = intAvgRSSI;                        // for trend tracking and troubleshooting
  objJSONdoc["MsgSentToDate"] = intMsgCnt;                       // because why not?
  objJSONdoc["UpTime"] = millis();                               //
  serializeJson(objJSONdoc, strXmitMsg);                         // assemble basic JSON object into tempmsg
  strXmitMsg = encapsulatemessage(strXmitMsg);                   // now, package that object into an object, and setting the ClientID as the key. redundant, but makes server side processing easier
  Serial.print("Reporting diagnostics: ");                       //
  Serial.println(strXmitMsg);                                    // diagnostic readout
  transmitmqttmessage("node/diagnostics", strXmitMsg, true, 1);  // send it
}

void cmd_resetdiagnostics() {                            // reset any tracked diagnostics
  Serial.println("*** RESETTING DIAGNOSTICS INFO ***");  //
  intAvgRSSI = 0;                                        //
  intMsgCnt = 0;                                         //
}

void cmd_reboot() {                                                // act on a remote reboot command. WARNING: NO REPEATBACK OR CONFIRMATION
  Serial.println("------------REBOOTING------------");             // should we mqtt out a going offline message? adieu! farewell! Off wierder Shane! goodbye!
  String strXmitMsg = singleJSONobject(strClientID, "REBOOTING");  //
  transmitmqttmessage("node/status", strXmitMsg, true, 1);         //
  ESP.restart();
}

void cmd_ntpresync(int intTimeZoneOffset) {                            // force an NTP sync only if we're not emulating. TODO: implement time zone offset?
  if (!boolMQTTEmulated) { instNTPClient.update(); }                   //
  ul_lastupdate = millis();                                            //
  Serial.print("*NTP update \t");                                      // we're going to try to keep the Serial Monitor to a single line per event for processed messages
  Serial.print("My uptime:");                                          //
  Serial.print(ul_lastupdate);                                         //
  Serial.print("msec \t");                                             //
  Serial.print("DTG:");                                                //
  Serial.println(CurrentUTCTime());                                    // just displaying for the humans in the audience; TODO: remove in production code
  String strXmitMsg = encapsulatemessage(String(CurrentEpochTime()));  // wrap it before sending
  transmitmqttmessage("node/ntpsync", strXmitMsg, true, 1);            // then encapsulate that message and publish to ntpsync topic
}

void cmd_listcommands() {                      // TODO: convert command list to an enum so we can iterate through like an array AND maintain this command list in the global declarations
  String strAssembledMsg = "";                 //
  String strTopic = "node/availablecommands";  // node/availablecommands  with the following structure
  //"node1234":{"commands":{"reboot":{"cmd":"reboot"}},"ntpresync":{"cmd":"ntpresync"},"listcommands":{"cmd":"listcommands"},
  //"diagnostics":{"cmd":"diagnostics"},"setreportinterval":{"cmd":"setreportinterval","value":"integer","unit":"milliseconds",
  //"minimum":"250","maximum":"259200000"}}
  Serial.println(strTopic);                  //
  DynamicJsonDocument objJSONdoc(1024);      //
  objJSONdoc["ID"] = strClientID;            //
  objJSONdoc["function"] = defNodeFunction;  //
  //serializeJson(objJSONdoc, strAssembledMsg);           // serialize
}

void cmd_newreportinterval(int intNewReportInterval) {                   // display new interval (converted for readability) and apply to global variable.  TODO: Split off time conversion function to return a string
  if (intNewReportInterval > 500 && intNewReportInterval < 259200000) {  // TODO: When we convert commands to an enum, we need to make the min/max parameters a lookup
    Serial.print("New report interval: ");                               // new interval is valid, so parse to something human readable and display.
    int intDividend = intNewReportInterval;                              // example: 93,858,000 milliseconds = 1 day, 2 hours, 4 mins, 18 secs
    int intRemainder = 0;                                                //
    int intMultiplier = 0;                                               //
    int intDivisorMinute = 60 * 1000;                                    // 60 seconds converted to milliseconds = 6,000
    int intDivisorHour = intDivisorMinute * 60;                          // 60 minutes in an hour = 3,600,000
    int intDivisorDay = intDivisorHour * 24;                             // 24 hours in a day = 86,400,000
    if (intDividend > intDivisorDay) {                                   // 93,858,000 > 86,400,000: TRUE
      intMultiplier = intDividend / intDivisorDay;                       // inttemp = 93858000/86400000 = 1
      Serial.print(intMultiplier);                                       //
      Serial.print("days ");                                             //
      intRemainder = intDividend % (intMultiplier * intDivisorDay);      // tempremainder = 93858000 % (1 * 86400000) ... 93858000 % 86400000 = 7458000
      intDividend = intRemainder;                                        // tempdividend = 7458000 (2hrs4mins18secs)
    }                                                                    //
    if (intDividend > intDivisorHour) {                                  // tempdividend = 7,458,000 > 3,600,000: TRUE
      intMultiplier = intDividend / (intDivisorHour);                    // 7458000 / 3600000 = 2
      Serial.print(intMultiplier);                                       //
      Serial.print("hours ");                                            //
      intRemainder = intDividend % (intMultiplier * intDivisorHour);     // 7458000 % (2 * 3600000) ... 7458000 % 7200000 = 258000
      intDividend = intRemainder;                                        // tempdividend = 258000
    }                                                                    //
    if (intDividend > intDivisorMinute) {                                // 258000 > 6000: TRUE
      intMultiplier = intDividend / intDivisorMinute;                    // 258000 / 6000 = 4
      Serial.print(intMultiplier);                                       //
      Serial.print("mins ");                                             //
      intRemainder = intDividend % (intMultiplier * intDivisorMinute);   // 258000 % (4 * 6000) ... 258000 % 24000 = 18000
      intDividend = intRemainder;                                        // tempdividend = 18000
    }                                                                    //
    if (intDividend > 0) {                                               // 18000 > 0: TRUE
      float fltSeconds = intDividend / 1000;                             // 18000 / 1000 = 18 seconds
      Serial.print(fltSeconds, 2);                                       // display 18.00 seconds
      Serial.print("secs");                                              //
    }                                                                    //
    Serial.println();                                                    //
    intReportInterval = intNewReportInterval;                            // TODO: remove or disable all of the IF:TRUE section except this line. The rest is just human readability.
  } else {                                                               //
    Serial.print("Requested report interval outside ");                  //
    Serial.print("acceptable range. Keeping existing value: ");          //
    Serial.print(intReportInterval);                                     //
    Serial.println(" msecs");                                            //
  }
}

String compilesimplemessage(String key1, String value1, String key2, String value2) {  // simple function for 2 key/value pairs
  StaticJsonDocument<200> objJSONdoc;                                                  // create jsondoc
  String strXmitMsg = "";                                                              //
  objJSONdoc[key1] = value1;                                                           // assemble doc
  objJSONdoc[key2] = value2;                                                           //
  serializeJson(objJSONdoc, strXmitMsg);                                               // serialize
  return strXmitMsg;                                                                   // return serialized string
}

String singleJSONobject(String key, String value) {           // simple function for a single key/value pair
  String strXmitMsg = "{\"" + key + "\":\"" + value + "\"}";  // {"key":"value"}
  return strXmitMsg;                                          // give it back
}

String encapsulatemessage(String message) {                    // reduce overhead in other steps when setting our ClientID as the key
  String strXmitMsg = singleJSONobject(strClientID, message);  //
  return strXmitMsg;                                           //
}

void transmitmqttmessage(String strXmitTopic, String strXmitMsg, bool boolRetainMsg, int intQoS) {                 // Single point for publishing messages. Ensures messages are counted and avg RSSI is tracked. TODO: add anomaly tracking, QoS, and message retention
  int intCurrRSSI;                                                                                                 //
  if (boolMQTTEmulated) {                                                                                          //
    Serial.print("fakeRSSI:");                                                                                     //
    intCurrRSSI = random(-30, -50);                                                                                // fake rssi value
  } else {                                                                                                         // emulated, so fake it
    Serial.print("currRSSI:");                                                                                     //
    intCurrRSSI = WiFi.RSSI();                                                                                     // Get current RSSI value
  }                                                                                                                //
  Serial.print(intCurrRSSI);                                                                                       //
  intAvgRSSI = (intCurrRSSI + (intMsgCnt * intAvgRSSI)) / (intMsgCnt + 1);                                         // calculate average. Keep it in this order so we don't have a divide by zero
  Serial.print("\t avgRSSI:");                                                                                     //
  Serial.print(intAvgRSSI);                                                                                        //
  intMsgCnt++;                                                                                                     //
  Serial.print("\t SentMsgCount:");                                                                                //
  Serial.println(intMsgCnt);                                                                                       //
  if (boolMQTTEmulated) {                                                                                          //
    Serial.print(strXmitTopic);                                                                                    //
    Serial.print("  ");                                                                                            //
    Serial.println(strXmitMsg);                                                                                    //
  } else {                                                                                                         // we're emulating, so just print what we would have sent out
    bool boolDuplicate = false;                                                                                    //
    instMQTTClient.beginMessage(strXmitTopic.c_str(), strXmitMsg.length(), boolRetainMsg, intQoS, boolDuplicate);  //
    instMQTTClient.print(strXmitMsg);                                                                              //
    instMQTTClient.endMessage();                                                                                   //
  }                                                                                                                //
}

unsigned long CurrentEpochTime() {        // return either the current NTP or a fake epoch time
  if (boolMQTTEmulated) {                 //
    return 1698339326;                    // 2023-10-26
  } else {                                //
    return instNTPClient.getEpochTime();  //
  }                                       //
}

String CurrentUTCTime() {                           // function to return human readable UTC time as YYYY-MM-DD HH-MM-SS
  instTimeConvert.getDateTime(CurrentEpochTime());  //
  String strDTG = String(instTimeConvert.year);     // initialize string starting with year
  strDTG += "-";                                    //
  strDTG += String(instTimeConvert.month);          //
  strDTG += "-";                                    //
  strDTG += String(instTimeConvert.day);            //
  strDTG += " ";                                    //
  strDTG += String(instTimeConvert.hour);           // TODO: Correct this to local timezone
  strDTG += ":";                                    //
  strDTG += String(instTimeConvert.minute);         //
  strDTG += ":";                                    //
  strDTG += String(instTimeConvert.second);         //
  return strDTG;                                    // give it back
}
