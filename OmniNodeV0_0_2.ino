// See Version History page for full details

#include <ESP8266WiFi.h>        // https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html
#include <ArduinoMqttClient.h>  // https://github.com/arduino-libraries/ArduinoMqttClient
#include <NTPClient.h>          // https://github.com/arduino-libraries/NTPClient
#include <WiFiUdp.h>            // required for NTPClient. This code does not include any calls directly to it.
#include <ArduinoJson.h>        // https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
#include <UnixTime.h>           // https://github.com/GyverLibs/UnixTime

const char* ssid = "Triton";               // Triton FBISecurityVan2 Verizon-MiFi8800L-75CE Verizon-MiFi8800L-Guest-75CF
const char* password = "az52p7fvnc";       // az52p7fvnc 773UT2PYWMBH75WD a2e63a23 586c9574
const char* mqtt_server = "192.168.0.17";  // 192.168.0.17
const int mqtt_port = 1883;                // set MQTT port

WiFiClient wifiClient;                                //
MqttClient mqttClient(wifiClient);                    //
WiFiUDP ntpUDP;                                       //
NTPClient timeClient(ntpUDP, mqtt_server, 0, 60000);  // local time offset is -14400; autoresync every 1000 seconds (~16 minutes)
UnixTime stamp(0);                                    //

#define DefineSoftwareVersion "OmniNodeV0_0_2"         // Used in MQTT config report.
#define DefineNodeFunction "mpu6500"                   // compile time definition to enable/disable code segments. Also used in MQTT config report
bool EnableMQTTEmulation = true;                       // global flag to bypass wifi & mqtt connections in favor of Serial input/output
const String clientId = String(ESP.getChipId(), HEX);  // global placeholder for my unique MAC ID.
const char inTopic[] = "arduino/in";                   // TODO: build off this to create enum of MQTT topics
unsigned long lastupdate = 0;                          // timestamp (in local millis) of last NTP sync. FOLLOWUP: does this need to be global or can we make it local/static?
unsigned long lastMsg = 0;                             // timestamp (in local millis) of last message sent. FOLLOWUP: does this need to be global or can we make it local/static?
int reportinterval = 1500;                             // default minimum time between reports in milliseconds. Leave global and variable
int messagecounter = 0;                                // for tracking number of outgoing messages. FOLLOWUP: will this be relevant once we go to QoS 1 or 2?
int avgrssi = 0;                                       // for tracking average WIFI strength. TODO: convert to static

void setup() {                                                                                      // initial setup
  Serial.begin(115200);                                                                             // initialize the Serial Monitor interface for debugging. TODO: Global debug flag to remove all Serial
  delay(1000);                                                                                      // delay for a while to give Serial Monitor time to catch up with board reset. TODO: remove for production code
  if (EnableMQTTEmulation) {                                                                        // is emulation enabled?
    Serial.println("WiFi and MQTT functions emulated.");                                            // if so, warn the user
    Serial.println("Enter compact JSON in Serial Monitor to simulate sending the node a message");  // and tell them how to interact with us
    Serial.println("");                                                                             //
    if (Serial.available() > 0) { getSerialInput(); }                                               //
  } else {                                                                                          // emulation isn't enabled, so we do a normal startup
    setup_wifi();                                                                                   // start wifi. TODO: Replace with wifi manager portal
    timeClient.begin();                                                                             // connect to NTP server.
    timeClient.update();                                                                            // force an update. FOLLOWUP: should this be moved to a separate function that's handled after we connect to the broker?
    lastupdate = millis();                                                                          // mark the time
    setup_mqtt();                                                                                   // start up MQTT
  }
}

void loop() {
  if (!EnableMQTTEmulation) {                                             // don't do mqtt stuff if we're emulating
    if (!mqttClient.connect(mqtt_server, mqtt_port)) { mqtt_connect(); }  // check if we're connected to MQTT broker. If not, reestablish.
    mqttClient.poll();                                                    // listens for MQTT messages AND sends keepalive messages to avoid disconnects; required for both purposes.
  }                                                                       //
  if (millis() - lastMsg > reportinterval) {                              // old if statement for sending regular reports. FOLLOWUP: what if we have multiple datapoints we want to send at different intervals? Could we convert to ArduinoTicker library?
    lastMsg = millis();                                                   // set timestamp to now. Placeholder for message packaging is below.
    Serial.print(CurrentUTCTime());                                       //
    Serial.print(" ");                                                    //
    Serial.println(reportinterval);                                       //
  }                                                                       //
}

void setup_wifi() {                        // Initialize WiFi
  Serial.print("Connecting to ");          //
  Serial.println(ssid);                    //
  WiFi.mode(WIFI_STA);                     //
  WiFi.begin(ssid, password);              //
  while (WiFi.status() != WL_CONNECTED) {  //
    delay(500);                            //
    Serial.print(".");                     //
  }                                        //
  Serial.println();                        //
  Serial.println(WiFi.macAddress());       //
  Serial.println(WiFi.gatewayIP());        //
  Serial.println(WiFi.localIP());          //
  WiFi.setHostname(clientId.c_str());      //
  delay(1000);                             //
}

void setup_mqtt() {                                                            // Initialize MQTT and connect to the broker
  const char willTopic[] = "node/staus";                                       //
  String willPayload = encapsulatemessage("OFFLINE");                          //
  bool willRetain = true;                                                      //
  int willQos = 1;                                                             //
  mqttClient.beginWill(willTopic, willPayload.length(), willRetain, willQos);  //
  mqttClient.print(willPayload);                                               //
  mqttClient.endWill();                                                        //
  mqttClient.connect(mqtt_server, mqtt_port);                                  // initialize MQTT instance
  mqttClient.onMessage(onMqttMessage);                                         // assign function call for incoming messages
  mqtt_connect();                                                              // connect and handle handshake protocol
}

void mqtt_connect() {                                           // TODO: should we add a section to troubleshoot underlying wifi issues, or is that built into MQTT pubsub?
  while (!mqttClient.connected()) {                             // Loop until we're reconnected
    Serial.print("Attempting MQTT connection...");              //
    if (mqttClient.connect(mqtt_server, mqtt_port)) {           // Attempt to connect
      Serial.println("connected");                              //
      transmitmqttmessage("base/announce", clientId, true, 1);  // simple announcement that will trigger the server to check its database if it's seen this node before
      cmd_areyouthere();                                        // keep it simple and just act like we were asked to checkin via msg
      String temptopic = "node/config/" + clientId;             // assemble unique MQTT topic for this node to listen for.
      Serial.print("Subscribing to: ");                         // TODO: check if we need to subscribe every time we connect. What if we just disconnected from the broker and it's not an initial connection? Is the subscription retained?
      Serial.println(temptopic);                                // show the user
      mqttClient.setId(clientId);                               //
      mqttClient.subscribe(temptopic.c_str(), 1);               // subscribe to that topic with QoS 1
    } else {                                                    //
      Serial.print("failed, rc=");                              //
      Serial.print(mqttClient.connectError());                  //
      Serial.println(" try again in 5 seconds");                //
      delay(5000);                                              // Wait 5 seconds before retrying
    }
  }
}

void onMqttMessage(int messageSize) {                                      // Routine for handling incoming messages
  StaticJsonDocument<200> doc;                                             // Should this be converted to dynamic? Or do we need a larger size?
  Serial.print("Received a message with topic '");                         //
  Serial.print(mqttClient.messageTopic());                                 //
  Serial.print("', duplicate = ");                                         //
  Serial.print(mqttClient.messageDup() ? "true" : "false");                //
  Serial.print(", QoS = ");                                                //
  Serial.print(mqttClient.messageQoS());                                   //
  Serial.print(", retained = ");                                           //
  Serial.print(mqttClient.messageRetain() ? "true" : "false");             //
  Serial.print("', length ");                                              //
  Serial.print(messageSize);                                               //
  Serial.println(" bytes:");                                               //
  String payload = "";                                                     //
  while (mqttClient.available()) {                                         // read mqtt buffer into a string
    payload += (char)mqttClient.read();                                    // make sure to use the character represented, NOT the raw ASCII code
  }                                                                        //
  DeserializationError error = deserializeJson(doc, payload);              //
  if (error) {                                                             //
    Serial.print("deserializeJson() failed: ");                            //
    Serial.println(error.f_str());                                         //
    String tempmsg = singleJSONobject(clientId, "deserialization error");  // assemble short message reporting a problem. TODO: is there a way to feedback a message identifier so the server even knows which message we're talking about?
    transmitmqttmessage("node/errors", tempmsg, true, 1);                  // send it to the error reporting topic
    return;                                                                // if we have a deserialization error, then we want to abort this routine because we can't decipher what was sent
  }                                                                        //
  String rcvdcmd = doc["command"];                                         //
  String rcvdvalue = doc["value"];                                         // TODO: use enum and covert the mess below to a SWITCH CASE structure
  processreceivedcommand(rcvdcmd, rcvdvalue);                              // process what we received
}

void processreceivedcommand(String command, String value) {                       // function to process incoming messages. This has been separated from OnMQTTMessage so we can manually invoke on Serial inputs
  if (rcvdcmd == "reboot") { cmd_reboot(); }                                      //
  if (rcvdcmd == "reportin") { cmd_areyouthere(); }                               //
  if (rcvdcmd == "ntpresync") { cmd_ntpresync(rcvdvalue.toInt()); }               // force an NTP resync, and pass it the server provided timezone offset. For now, it's only for display purposes
  if (rcvdcmd == "reportinterval") { cmd_newreportinterval(rcvdvalue.toInt()); }  //
  if (rcvdcmd == "reportcommands") { cmd_listcommands(); }                        //
  if (rcvdcmd == "reportconfig") { cmd_reportconfig(); }                          //
  if (rcvdcmd == "reportdiagnostics") { cmd_reportdiagnostics(); }                //
  if (rcvdcmd == "resetdiagnostics") { cmd_resetdiagnostics(); }                  //
}

void getSerialInput() {
  String tempstring = Serial.readString();
  
}

void cmd_areyouthere() {                                                   // simple response routine to respond to MQTT checkin messages.
  String tempmsg = encapsulatemessage(String(timeClient.getEpochTime()));  // MIGHT BE AVOIDABLE by querying the MQTT broker's KEEPALIVE stats. Should be doable and would reduce network traffic
  transmitmqttmessage("node/lastcheckin", tempmsg, true, 1);               // send current UTC as our last checkin time
  tempmsg = encapsulatemessage("online");                                  // TODO: make the server update this topic after successfully processing our last checkin time
  transmitmqttmessage("node/status", tempmsg, true, 1);                    //
}

void cmd_reportconfig() {                                // save on network traffic and packet size by reporting config ONLY when requested
  String tempmsg = "";                                   // initialize msg payload holder
  DynamicJsonDocument doc(1024);                         // initialize blank JSON doc
  doc["ID"] = clientId;                                  // explicitly define my ID as my MAC
  doc["function"] = DefineNodeFunction;                  // declare my assigned sensor function
  doc["softwareversion"] = DefineSoftwareVersion;        // report this software version
  doc["IP"] = WiFi.localIP();                            // not really necessary with MQTT, but for the detail loving humans
  doc["CurrReportInterval"] = String(reportinterval);    // repeatback for the record. Should be used server side in the watchdog function.
  doc["commandtopic"] = "node/config/" + clientId;       // store this node's unique command topic in the server database
  serializeJson(doc, tempmsg);                           // assemble basic JSON object into tempmsg
  tempmsg = encapsulatemessage(tempmsg);                 // now, package that object into an object, and setting the clientId as the key. redundant, but makes server side processing easier
  Serial.print("Reporting config: ");                    //
  Serial.println(tempmsg);                               // diagnostic readout
  transmitmqttmessage("node/config", tempmsg, true, 1);  // send it
}

void cmd_reportdiagnostics() {                                // save on network traffic and packet size by reporting diagnostics ONLY when requested
  String tempmsg = "";                                        // initialize msg payload holder
  DynamicJsonDocument doc(1024);                              // initialize blank JSON doc
  doc["ID"] = clientId;                                       // explicitly define my ID as my MAC
  doc["WiFiStr_Curr"] = WiFi.RSSI();                          // report curr WiFiStr
  doc["WiFiStr_Avg"] = avgrssi;                               // for trend tracking and troubleshooting
  doc["MsgSentToDate"] = messagecounter;                      // because why not?
  doc["UpTime"] = millis();                                   //
  serializeJson(doc, tempmsg);                                // assemble basic JSON object into tempmsg
  tempmsg = encapsulatemessage(tempmsg);                      // now, package that object into an object, and setting the clientId as the key. redundant, but makes server side processing easier
  Serial.print("Reporting diagnostics: ");                    //
  Serial.println(tempmsg);                                    // diagnostic readout
  transmitmqttmessage("node/diagnostics", tempmsg, true, 1);  // send it
}

void cmd_resetdiagnostics() {                            // function to reset any tracked diagnostics
  Serial.println("*** RESETTING DIAGNOSTICS INFO ***");  //
  avgrssi = 0;                                           //
  messagecounter = 0;                                    //
}

void cmd_reboot() {                                                                       // remote reboot command
  Serial.println("-----------------------------REBOOTING-----------------------------");  // should we mqtt out a going offline message? adieu! farewell! Off wierder Shane! goodbye!
  String tempmsg = singleJSONobject(clientId, "REBOOTING");                               //
  transmitmqttmessage("node/status", tempmsg, true, 1);                                   //
  ESP.restart();
}

void cmd_ntpresync(int timezoneoffset) {                     // force an NTP sync only if we're not emulating
  if (!EnableMQTTEmulation) { timeClient.update(); }         //
  lastupdate = millis();                                     //
  Serial.print("*NTP update \t");                            // we're going to try to keep the Serial Monitor to a single line per event for processed messages
  Serial.print("My uptime:");                                //
  Serial.print(lastupdate);                                  //
  Serial.print("msec \t");                                   //
  String tempstring = String(CurrentEpochTime());            //
  Serial.print("Epoch: ");                                   //
  Serial.print(tempstring);                                  //
  Serial.print("\t DTG:");                                   //
  tempstring = encapsulatemessage(tempstring);               // wrap it before sending
  transmitmqttmessage("node/ntpsync", tempstring, true, 1);  // then encapsulate that message and publish to ntpsync topic
  Serial.println(CurrentUTCTime());                          // just displaying for the humans in the audience; TODO: remove in production code
}

void cmd_listcommands() {                       // TODO: convert command list to an enum so we can iterate through like an array AND maintain this command list in the global declarations
  String temptopic = "node/availablecommands";  // node/availablecommands  with the following structure
  //"node1234":{"commands":{"reboot":{"cmd":"reboot"}},"ntpresync":{"cmd":"ntpresync"},"listcommands":{"cmd":"listcommands"},
  //"diagnostics":{"cmd":"diagnostics"},"setreportinterval":{"cmd":"setreportinterval","value":"integer","unit":"milliseconds",
  //"minimum":"250","maximum":"259200000"}}
  Serial.println(temptopic);             //
  DynamicJsonDocument doc(1024);         //
  doc["ID"] = clientId;                  //
  doc["function"] = DefineNodeFunction;  //
  //serializeJson(doc, tempmsg);           // serialize
}

void cmd_newreportinterval(int newreportinterval) {                // display new interval (converted for readability) and apply to global variable.
  if (newreportinterval > 500 && newreportinterval < 259200000) {  // TODO: When we convert commands to an enum, we need to make the min/max parameters a lookup
    Serial.print("New report interval: ");                         // new interval is valid, so parse to something human readable and display.
    int tempdividend = newreportinterval;                          // example: 93,858,000 milliseconds = 1 day, 2 hours, 4 mins, 18 secs
    int tempremainder = 0;                                         //
    int inttemp = 0;                                               //
    int divisorminute = 60 * 1000;                                 // 60 seconds converted to milliseconds = 6,000
    int divisorhour = divisorminute * 60;                          // 60 minutes in an hour = 3,600,000
    int divisorday = divisorhour * 24;                             // 24 hours in a day = 86,400,000
    if (tempdividend > divisorday) {                               // 93,858,000 > 86,400,000: TRUE
      inttemp = tempdividend / divisorday;                         // inttemp = 93858000/86400000 = 1
      Serial.print(inttemp);                                       //
      Serial.print("days ");                                       //
      tempremainder = tempdividend % (inttemp * divisorday);       // tempremainder = 93858000 % (1 * 86400000) ... 93858000 % 86400000 = 7458000
      tempdividend = tempremainder;                                // tempdividend = 7458000 (2hrs4mins18secs)
    }                                                              //
    if (tempdividend > divisorhour) {                              // tempdividend = 7,458,000 > 3,600,000: TRUE
      inttemp = tempdividend / (divisorhour);                      // 7458000 / 3600000 = 2
      Serial.print(inttemp);                                       //
      Serial.print("hours ");                                      //
      tempremainder = tempdividend % (inttemp * divisorhour);      // 7458000 % (2 * 3600000) ... 7458000 % 7200000 = 258000
      tempdividend = tempremainder;                                // tempdividend = 258000
    }                                                              //
    if (tempdividend > divisorminute) {                            // 258000 > 6000: TRUE
      inttemp = tempdividend / divisorminute;                      // 258000 / 6000 = 4
      Serial.print(inttemp);                                       //
      Serial.print("mins ");                                       //
      tempremainder = tempdividend % (inttemp * divisorminute);    // 258000 % (4 * 6000) ... 258000 % 24000 = 18000
      tempdividend = tempremainder;                                // tempdividend = 18000
    }                                                              //
    if (tempdividend > 0) {                                        // 18000 > 0: TRUE
      float tempfloat = tempdividend / 1000;                       // 18000 / 1000 = 18 seconds
      Serial.print(tempfloat, 2);                                  // display 18.00 seconds
      Serial.print("secs");                                        //
    }                                                              //
    Serial.println();                                              //
    reportinterval = newreportinterval;                            // TODO: remove or disable all of the IF:TRUE section except this line. The rest is just human readability.
  } else {                                                         //
    Serial.print("Requested report interval outside ");            //
    Serial.print("acceptable range. Keeping existing value: ");    //
    Serial.print(reportinterval);                                  //
    Serial.println(" msecs");                                      //
  }
}

String compilesimplemessage(String key1, String value1, String key2, String value2) {  // function for small messages only.
  StaticJsonDocument<200> doc;                                                         // create jsondoc
  String tempmsg = "";                                                                 //
  doc[key1] = value1;                                                                  // assemble doc
  doc[key2] = value2;                                                                  //
  serializeJson(doc, tempmsg);                                                         // serialize
  return tempmsg;                                                                      // return serialized string
}

String singleJSONobject(String key, String value) {        // possibly more memory efficient than using ArduinoJSON library
  String tempmsg = "{\"" + key + "\":\"" + value + "\"}";  // {"key":"value"}
  return tempmsg;                                          // give it back
}

String encapsulatemessage(String message) {              // reduce overhead in other steps when setting our clientId as the key
  String tempmsg = singleJSONobject(clientId, message);  //
  return tempmsg;                                        //
}

void transmitmqttmessage(String topic, String message, bool retained, int qos) {  // Single point for publishing messages. Ensures messages are counted and avg RSSI is tracked. TODO: add anomaly tracking, QoS, and message retention
  int currrssi;
  if (!EnableMQTTEmulation) {                                                      //
    Serial.print("currRSSI:");                                                     //
    currrssi = WiFi.RSSI();                                                        // Get current RSSI value
  } else {                                                                         // emulated, so fake it
    Serial.print("fakeRSSI:");                                                     //
    currrssi = random(-30, -50);                                                   // fake rssi value
  }                                                                                //
  Serial.print(currrssi);                                                          //
  avgrssi = (currrssi + (messagecounter * avgrssi)) / (messagecounter + 1);        // calculate average. Keep it in this order so we don't have a divide by zero
  Serial.print("\t avgRSSI:");                                                     //
  Serial.print(avgrssi);                                                           //
  messagecounter++;                                                                //
  Serial.print("\t SentMsgCount:");                                                //
  Serial.println(messagecounter);                                                  //
  if (!EnableMQTTEmulation) {                                                      //
    bool dup = false;                                                              //
    mqttClient.beginMessage(topic.c_str(), message.length(), retained, qos, dup);  //
    mqttClient.print(message);                                                     //
    mqttClient.endMessage();                                                       //
  } else {                                                                         // we're emulating, so just print what we would have sent out
    Serial.print(topic);                                                           //
    Serial.print("  ");                                                            //
    Serial.println(message);                                                       //
  }                                                                                //
}

unsigned long CurrentEpochTime() {     // return either the current NTP or a fake epoch time
  if (!EnableMQTTEmulation) {          //
    return timeClient.getEpochTime();  //
  } else {                             //
    return 1698339326;                 // 2023-10-26
  }                                    //
}

String CurrentUTCTime() {                 // function to return human readable UTC time
  stamp.getDateTime(CurrentEpochTime());  //
  String tempstring = stamp.year;         // initialize string starting with year
  tempstring += "-";                      //
  tempstring += stamp.month;              //
  tempstring += "-";                      //
  tempstring += stamp.day;                //
  tempstring += " ";                      //
  tempstring += stamp.hour;               // TODO: Correct this to local timezone
  tempstring += ":";                      //
  tempstring += stamp.minute;             //
  tempstring += ":";                      //
  tempstring += stamp.second;             //
  return tempstring;                      // give it back
}
