void processreceivedcommand(String strRcvdCmd, String strRcvdValue) {           // process incoming messages. This has been separated from OnMQTTMessage so we can manually invoke on Serial inputs
  // {"command":"setmqtt","value":"192.168.1.68"}
  int intRcvdValue = strRcvdValue.toInt();                                      // convert to integer
  if (strRcvdCmd == "reboot") { cmd_reboot(); }                                 //
  if (strRcvdCmd == "reportin") { cmd_areyouthere(); }                          //
  if (strRcvdCmd == "ntpresync") { cmd_ntpresync(intRcvdValue); }               // force an NTP resync, and pass it the server provided timezone offset. For now, it's only for display purposes
  if (strRcvdCmd == "reportinterval") { cmd_newreportinterval(intRcvdValue); }  //
  if (strRcvdCmd == "setSLP") { intSeaLevelPressPa = intRcvdValue; }            // WARNING: no error checking, so must be in PASCALS as an INTEGER
  if (strRcvdCmd == "reportcommands") { cmd_listcommands(); }                   //
  if (strRcvdCmd == "reportconfig") { cmd_reportconfig(); }                     //
  if (strRcvdCmd == "sensorconfig") { cmd_GetSensorConfig(); }                  //
  if (strRcvdCmd == "reportdiagnostics") { cmd_reportdiagnostics(); }           //
  if (strRcvdCmd == "resetdiagnostics") { cmd_resetdiagnostics(); }             //
  if (strRcvdCmd == "wifiportal") { cmd_launchwifiportal(); }                   //
  if (strRcvdCmd == "setmqtt") { cmd_setmqtt(strRcvdValue); }                   //
}

void cmd_areyouthere() {                                                                    // simple response routine to respond to MQTT checkin messages.
  String strXmitMsg = strStandardMsg("lastcheckin", String(instNTPClient.getEpochTime()));  // MIGHT BE AVOIDABLE by querying the MQTT broker's KEEPALIVE stats. Should be doable and would reduce network traffic
  transmitmqttmessage("node/lastcheckin", strXmitMsg, true, 1);                             // send current UTC as our last checkin time
  strXmitMsg = strStandardMsg("status", "online");                                          // TODO: make the server update this topic after successfully processing our last checkin time
  transmitmqttmessage("node/status", strXmitMsg, true, 1);                                  //
}

void cmd_reportconfig() {                                                                        // save on network traffic and packet size by reporting config ONLY when requested
  debugPrefix(defDebugMQTTVerbose);                                                        //
  String strXmitMsg = strKeyValuePair("softwareversion", defSoftwareVersion) + ",";              // initialize msg payload holder; avoiding JSON calls to save memory. TODO: convert to ArduinoJSON. This has grown significantly.
  strXmitMsg += strKeyValuePair("compiled", __DATE__) + ",";                                     //
  strXmitMsg += strKeyValuePair("FlashChipSize", String(ESP.getFlashChipSize())) + ",";          // https://arduino-esp8266.readthedocs.io/en/latest/libraries.html
  strXmitMsg += strKeyValuePair("ESPSketchSize", String(ESP.getSketchSize())) + ",";             //
  strXmitMsg += strKeyValuePair("ESPFreeSketchSpace", String(ESP.getFreeSketchSpace())) + ",";   //
  strXmitMsg += strKeyValuePair("FreeHeap", String(ESP.getFreeHeap())) + ",";                    //
  strXmitMsg += strKeyValuePair("HeapFragmentation", String(ESP.getHeapFragmentation())) + ",";  //
  strXmitMsg += strKeyValuePair("Vcc", String(ESP.getVcc())) + ",";                              //
  strXmitMsg += strKeyValuePair("LastResetReason", ESP.getResetReason()) + ",";                  //
  strXmitMsg += strKeyValuePair("ESPCPUMHz", String(ESP.getCpuFreqMHz())) + ",";                 //
  strXmitMsg += strKeyValuePair("CurrReportInterval", String(intReportInterval)) + ",";          //
  strXmitMsg += strKeyValuePair("commandtopic", "node/config/" + strClientID);                   //
  strXmitMsg = strJSONwrap(strXmitMsg);                                                          //
  strXmitMsg = strStandardMsg("diagnostics", strXmitMsg);                                        // now, package that object into an object
  debugMQTTVerbose("Reporting config: ");                                                        //
  debugMQTTVerbose(strXmitMsg);                                                                  //
  debugMQTTVerbose("\n");                                                                        //
  transmitmqttmessage("node/config", strXmitMsg, true, 1);                                       // send it
  /*
  https://arduino-esp8266.readthedocs.io/en/latest/libraries.html
  
  ESP.getSketchSize() ulong 32bit
  ESP.getFreeSketchSpace() ulong 32bit
  ESP.getFreeHeap() uint
  ESP.getHeapFragmentation() uint
  ESP.getVcc() uint
  ESP.getResetReason() string
  ESP.getChipId() 32bit int
  ESP.getCpuFreqMHz() uint 8bit

  String(ESP.getFlashChipSize()
  String(fs_info.totalBytes)
  String(fs_info.usedBytes)
*/
}

void cmd_setmqtt(String strNewMQTT) {  // routine to apply a new MQTT broker without re-invoking the web portal. Currently, can only be executed by Serial Monitor. TODO: REST API input
  strMQTTserver = strNewMQTT;          // Apply new broker. TODO: we should save the new broker address to file
  debugPrefix(defDebugMQTTVerbose);                                                        //
  debugMQTTVerbose(strMQTTserver);     // read it out
  debugMQTTVerbose("\n");              //
  mqtt_connect();                      // force an update based on new info
  cmd_areyouthere();                   // report in
}

void cmd_reportdiagnostics() {                                                     // save on network traffic and packet size by reporting diagnostics ONLY when requested
  debugPrefix(defDebugMQTTVerbose);                                                        //
  String strXmitMsg = strKeyValuePair("WiFiStr_Curr", String(WiFi.RSSI())) + ",";  // start assembling the payload
  strXmitMsg += strKeyValuePair("WiFiStr_Avg", String(intAvgRSSI)) + ",";          //
  strXmitMsg += strKeyValuePair("MsgSentToDate", String(intMsgCnt)) + ",";         //
  strXmitMsg += strKeyValuePair("UpTime", String(millis()));                       //
  strXmitMsg = strJSONwrap(strXmitMsg);                                            // wrap the assembled payload string into an object, then
  strXmitMsg = strStandardMsg("diagnostics", strXmitMsg);                          // add the standard msg header
  debugMQTTVerbose("Reporting diagnostics: ");                                     //
  debugMQTTVerbose(strXmitMsg);                                                    //
  debugMQTTVerbose("\n");                                                          //
  transmitmqttmessage("node/diagnostics", strXmitMsg, true, 1);                    // send it
}

void cmd_resetdiagnostics() {                                // reset any tracked diagnostics
  debugPrefix(defDebugMQTTVerbose);                                                        //
  debugMQTTVerbose("*** RESETTING DIAGNOSTICS INFO ***\n");  //
  intAvgRSSI = 0;                                            //
  intMsgCnt = 0;                                             //
}

void cmd_reboot() {                                           // act on a remote reboot command. WARNING: NO REPEATBACK OR CONFIRMATION
  debugPrefix(defDebugMQTTVerbose);                                                        //
  debugMQTTVerbose("------------REBOOTING------------\n\n");  // should we mqtt out a going offline message? adieu! farewell! Off wierder Shane! goodbye!
  String strXmitMsg = strStandardMsg("status", "REBOOTING");  //
  strXmitMsg = strJSONwrap(strXmitMsg);                       // make sure to wrap the entire thing before sending
  transmitmqttmessage("node/status", strXmitMsg, true, 1);    //
  ESP.restart();
}

void cmd_launchwifiportal() {                                         // launch the WiFiManager configuration portal
  debugPrefix(defDebugMQTTVerbose);                                                        //
  debugMQTTVerbose("------LAUNCHING WIFI MANAGER PORTAL------\n\n");  // should we mqtt out a going offline message? adieu! farewell! Off wierder Shane! goodbye!
  String strXmitMsg = strStandardMsg("status", "LAUNCHINGPORTAL");    //
  strXmitMsg = strJSONwrap(strXmitMsg);                               // make sure to wrap the entire thing before sending
  transmitmqttmessage("node/status", strXmitMsg, true, 1);            //
  wifiManager.startConfigPortal();                                    //
}

void cmd_ntpresync(int intTimeZoneOffset) {                                     // force an NTP sync only if we're not emulating. TODO: implement time zone offset?
  debugPrefix(defDebugMQTTVerbose);                                                        //
  intTimeZoneOffset = 0;                                                        //
  if (!boolMQTTEmulated) { instNTPClient.update(); }                            //
  ul_lastupdate = millis();                                                     //
  String strXmitMsg = strKeyValuePair("lastsync", String(CurrentEpochTime()));  // assemble payload
  strXmitMsg = strJSONwrap(strXmitMsg);                                         // make sure to wrap the entire thing before sending
  strXmitMsg = strStandardMsg("lastsync", strXmitMsg);                          // assemble standard message header
  debugMQTTVerbose(strXmitMsg);                                                 //
  debugMQTTVerbose("\n");                                                       //
  transmitmqttmessage("node/ntpsync", strXmitMsg, true, 1);                     // send it
}

void cmd_listcommands() {                      // TODO: convert command list to an enum so we can iterate through like an array AND maintain this command list in the global declarations
  debugPrefix(defDebugMQTTVerbose);                                                        //
  String strAssembledMsg = "";                 //
  String strTopic = "node/availablecommands";  //
  debugMQTTVerbose(strTopic);                  //
  debugMQTTVerbose("\n");                      //
  // DynamicJsonDocument objJSONdoc(1024);      //
  // objJSONdoc["ID"] = strClientID;            //
  // objJSONdoc["function"] = defNodeFunction;  //
  // serializeJson(objJSONdoc, strAssembledMsg);           // serialize
}

void cmd_newreportinterval(int intNewReportInterval) {                   // display new interval (converted for readability) and apply to global variable.  TODO: Split off time conversion function to return a string
  debugPrefix(defDebugMQTTVerbose);                                                        //
  if (intNewReportInterval > 500 && intNewReportInterval < 259200000) {  // TODO: When we convert commands to an enum, we need to make the min/max parameters a lookup
    intReportInterval = intNewReportInterval;                            //
#if (defDebugFull)
    // if debugging isn't enabled, we can skip this entire section. don't do math you don't have to i always say
    debugMQTTVerbose("New report interval: ");                          // new interval is valid, so parse to something human readable and display.
    int intDividend = intNewReportInterval;                             // example: 93,858,000 milliseconds = 1 day, 2 hours, 4 mins, 18 secs
    int intRemainder = 0;                                               //
    int intMultiplier = 0;                                              //
    int intDivisorMinute = 60 * 1000;                                   // 60 seconds converted to milliseconds = 6,000
    int intDivisorHour = intDivisorMinute * 60;                         // 60 minutes in an hour = 3,600,000
    int intDivisorDay = intDivisorHour * 24;                            // 24 hours in a day = 86,400,000
    if (intDividend > intDivisorDay) {                                  // 93,858,000 > 86,400,000: TRUE
      intMultiplier = intDividend / intDivisorDay;                      // inttemp = 93858000/86400000 = 1
      debugMQTTVerbose(intMultiplier);                                  //
      debugMQTTVerbose("days ");                                        //
      intRemainder = intDividend % (intMultiplier * intDivisorDay);     // tempremainder = 93858000 % (1 * 86400000) ... 93858000 % 86400000 = 7458000
      intDividend = intRemainder;                                       // tempdividend = 7458000 (2hrs4mins18secs)
    }                                                                   //
    if (intDividend > intDivisorHour) {                                 // tempdividend = 7,458,000 > 3,600,000: TRUE
      intMultiplier = intDividend / (intDivisorHour);                   // 7458000 / 3600000 = 2
      debugMQTTVerbose(intMultiplier);                                  //
      debugMQTTVerbose("hours ");                                       //
      intRemainder = intDividend % (intMultiplier * intDivisorHour);    // 7458000 % (2 * 3600000) ... 7458000 % 7200000 = 258000
      intDividend = intRemainder;                                       // tempdividend = 258000
    }                                                                   //
    if (intDividend > intDivisorMinute) {                               // 258000 > 6000: TRUE
      intMultiplier = intDividend / intDivisorMinute;                   // 258000 / 6000 = 4
      debugMQTTVerbose(intMultiplier);                                  //
      debugMQTTVerbose("mins ");                                        //
      intRemainder = intDividend % (intMultiplier * intDivisorMinute);  // 258000 % (4 * 6000) ... 258000 % 24000 = 18000
      intDividend = intRemainder;                                       // tempdividend = 18000
    }                                                                   //
    if (intDividend > 0) {                                              // 18000 > 0: TRUE
      float fltSeconds = intDividend / 1000;                            // 18000 / 1000 = 18 seconds
      debugMQTTVerbose(String(fltSeconds, 2));                          // display 18.00 seconds
      debugMQTTVerbose("secs");                                         //
    }                                                                   //
    debugMQTTVerbose("\n");                                             //
#endif                                                                  //
  } else {                                                              //
    debugMQTTVerbose("Requested report interval outside ");             //
    debugMQTTVerbose("acceptable range. Keeping existing value: ");     //
    debugMQTTVerbose(intReportInterval);                                //
    debugMQTTVerbose(" msecs\n");                                       //
  }
}