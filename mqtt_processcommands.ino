void processreceivedcommand(String strRcvdCmd, String strRcvdValue) {  // process incoming messages. This has been separated from OnMQTTMessage so we can manually invoke on Serial inputs
  // {"command":"setmqtt","value":"192.168.1.68"}
  int intRcvdValue = strRcvdValue.toInt();                                      // convert to integer
  if (strRcvdCmd == "reboot") { cmd_reboot(); }                                 //
  if (strRcvdCmd == "ntpresync") { cmd_ntpresync(intRcvdValue); }               // force an NTP resync, and pass it the server provided timezone offset. For now, it's only for display purposes
  if (strRcvdCmd == "reportinterval") { cmd_newreportinterval(intRcvdValue); }  //
  if (strRcvdCmd == "setSLP") { intSeaLevelPressPa = intRcvdValue; }            // WARNING: no error checking, so must be in PASCALS as an INTEGER
  if (strRcvdCmd == "reportconfig") { cmd_reportconfig(); }                     //
  if (strRcvdCmd == "sensorconfig") { cmd_GetSensorConfig(); }                  //
  if (strRcvdCmd == "reportdiagnostics") { cmd_reportdiagnostics(); }           //
  if (strRcvdCmd == "resetdiagnostics") { cmd_resetdiagnostics(); }             //
  if (strRcvdCmd == "wifiportal") { cmd_launchwifiportal(); }                   //
  if (strRcvdCmd == "setmqtt") { cmd_setmqtt(strRcvdValue); }                   //
}

void cmd_reportconfig() {                                               // save on network traffic and packet size by reporting config ONLY when requested
  JSONVar objReport;                                                    //
  defstandardheader(objReport);                                         //
  objReport["sensorconfig"]["softwareversion"] = defSoftwareVersion;    //
  objReport["sensorconfig"]["compiledate"] = __DATE__;                  //
  objReport["sensorconfig"]["compiletime"] = __TIME__;                  //
  objReport["sensorconfig"]["currreportinterval"] = intReportInterval;  //
  objReport["sensorconfig"]["commandtopic"] = "node/" + strClientID;    // ESP.getChipId() 32bit int
  debugPrefix(defDebugMQTTTransmit);                                    //
  String jsonString = JSON.stringify(objReport);                        //
  debugPrint(defDebugMQTTTransmit, "Reporting config: ");               //
  debugPrintln(defDebugMQTTTransmit, jsonString);                       //
  transmitmqttmessage("node/config", jsonString, true, 1);              // send it
  /* Little_FS...
  String(fs_info.totalBytes)
  String(fs_info.usedBytes)
  */
}

void cmd_setmqtt(String strNewMQTT) {                  // routine to apply a new MQTT broker without re-invoking the web portal. Currently, can only be executed by Serial Monitor. TODO: REST API input
  strMQTTserver = strNewMQTT;                          // Apply new broker. TODO: we should save the new broker address to file
  debugPrefix(defDebugMQTTConnect);                    //
  debugPrint(defDebugMQTTConnect, "NEW MQTT SERVER");  // read it out
  debugPrintln(defDebugMQTTConnect, strMQTTserver);    // read it out
  mqtt_connect();                                      // force an update based on new info
}

void cmd_reportdiagnostics() {                                                  // save on network traffic and packet size by reporting diagnostics ONLY when requested
  debugPrefix(defDebugMQTTTransmit);                                            //
  JSONVar objReport;                                                            //
  defstandardheader(objReport);                                                 //
  objReport["uptime"] = millis();                                               //
  objReport["Vcc"] = ESP.getVcc();                                              // ESP.getVcc() uint
  objReport["LastResetReason"] = ESP.getResetReason();                          // ESP.getResetReason() string
  objReport["sensorconfig"]["FlashChipSize"] = ESP.getFlashChipSize();          // https://arduino-esp8266.readthedocs.io/en/latest/libraries.html
  objReport["sensorconfig"]["ESPSketchSize"] = ESP.getSketchSize();             //  ESP.getSketchSize() ulong 32bit
  objReport["sensorconfig"]["ESPFreeSketchSpace"] = ESP.getFreeSketchSpace();   // ESP.getFreeSketchSpace() ulong 32bit
  objReport["sensorconfig"]["FreeHeap"] = ESP.getFreeHeap();                    // ESP.getFreeHeap() uint
  objReport["sensorconfig"]["HeapFragmentation"] = ESP.getHeapFragmentation();  // ESP.getHeapFragmentation() uint
  objReport["sensorconfig"]["ESPCPUMHz"] = ESP.getCpuFreqMHz();                 // ESP.getCpuFreqMHz() uint 8bit
  String jsonString = JSON.stringify(objReport);                                //
  debugPrint(defDebugMQTTTransmit, "Reporting diagnostics: ");                  //
  debugPrintln(defDebugMQTTTransmit, jsonString);                               //
  transmitmqttmessage("node/diagnostics", jsonString, true, 1);                 // send it
}

void cmd_resetdiagnostics() {                                                  // reset any tracked diagnostics
  debugPrefix(defDebugMQTTTransmit);                                           //
  debugPrintln(defDebugMQTTTransmit, "*** RESETTING DIAGNOSTICS INFO ***\n");  //
  intMsgCnt = 0;                                                               //
}

void cmd_reboot() {                                                             // act on a remote reboot command. WARNING: NO REPEATBACK OR CONFIRMATION
  debugPrefix(defDebugMQTTTransmit);                                            //
  debugPrintln(defDebugMQTTTransmit, "------------REBOOTING------------\n\n");  // should we mqtt out a going offline message? adieu! farewell! Off wierder Shane! goodbye!
  ESP.restart();
}

void cmd_launchwifiportal() {                                                           // launch the WiFiManager configuration portal
  debugPrefix(defDebugMQTTTransmit);                                                    //
  debugPrintln(defDebugMQTTTransmit, "------LAUNCHING WIFI MANAGER PORTAL------\n\n");  // should we mqtt out a going offline message? adieu! farewell! Off wierder Shane! goodbye!
  wifiManager.startConfigPortal();                                                      //
}

void cmd_ntpresync(int intTimeZoneOffset) {                  // force an NTP sync only if we're not emulating. TODO: implement time zone offset?
  debugPrefix(defDebugMQTTTransmit);                         //
  intTimeZoneOffset = 0;                                     //
  instNTPClient.update();                                    //
  ul_lastupdate = millis();                                  //
  JSONVar objReport;                                         //
  defstandardheader(objReport);                              //
  objReport["lastsync"] = CurrentEpochTime();                //
  String jsonString = JSON.stringify(objReport);             //
  debugPrint(defDebugMQTTTransmit, "NTP RESYNC: ");          //
  debugPrintln(defDebugMQTTTransmit, jsonString);            //
  transmitmqttmessage("node/ntpsync", jsonString, true, 1);  // send it
}

void cmd_newreportinterval(int intNewReportInterval) {                   // display new interval (converted for readability) and apply to global variable.  TODO: Split off time conversion function to return a string
  debugPrefix(defDebugMQTTConnect);                                      //
  if (intNewReportInterval > 500 && intNewReportInterval < 259200000) {  // TODO: When we convert commands to an enum, we need to make the min/max parameters a lookup
    intReportInterval = intNewReportInterval;                            //
#if (defDebugFull)
    // if debugging isn't enabled, we can skip this entire section. don't do math you don't have to i always say
    debugPrint(defDebugMQTTReceive, "NEW REPORT INTERVAL: ");                                  // new interval is valid, so parse to something human readable and display.
    int intDividend = intNewReportInterval;                                                    // example: 93,858,000 milliseconds = 1 day, 2 hours, 4 mins, 18 secs
    int intRemainder = 0;                                                                      //
    int intMultiplier = 0;                                                                     //
    int intDivisorMinute = 60 * 1000;                                                          // 60 seconds converted to milliseconds = 6,000
    int intDivisorHour = intDivisorMinute * 60;                                                // 60 minutes in an hour = 3,600,000
    int intDivisorDay = intDivisorHour * 24;                                                   // 24 hours in a day = 86,400,000
    if (intDividend > intDivisorDay) {                                                         // 93,858,000 > 86,400,000: TRUE
      intMultiplier = intDividend / intDivisorDay;                                             // inttemp = 93858000/86400000 = 1
      debugPrint(defDebugMQTTReceive, intMultiplier);                                          //
      debugPrint(defDebugMQTTReceive, "days ");                                                //
      intRemainder = intDividend % (intMultiplier * intDivisorDay);                            // tempremainder = 93858000 % (1 * 86400000) ... 93858000 % 86400000 = 7458000
      intDividend = intRemainder;                                                              // tempdividend = 7458000 (2hrs4mins18secs)
    }                                                                                          //
    if (intDividend > intDivisorHour) {                                                        // tempdividend = 7,458,000 > 3,600,000: TRUE
      intMultiplier = intDividend / (intDivisorHour);                                          // 7458000 / 3600000 = 2
      debugPrint(defDebugMQTTReceive, intMultiplier);                                          //
      debugPrint(defDebugMQTTReceive, "hours ");                                               //
      intRemainder = intDividend % (intMultiplier * intDivisorHour);                           // 7458000 % (2 * 3600000) ... 7458000 % 7200000 = 258000
      intDividend = intRemainder;                                                              // tempdividend = 258000
    }                                                                                          //
    if (intDividend > intDivisorMinute) {                                                      // 258000 > 6000: TRUE
      intMultiplier = intDividend / intDivisorMinute;                                          // 258000 / 6000 = 4
      debugPrint(defDebugMQTTReceive, intMultiplier);                                          //
      debugPrint(defDebugMQTTReceive, "mins ");                                                //
      intRemainder = intDividend % (intMultiplier * intDivisorMinute);                         // 258000 % (4 * 6000) ... 258000 % 24000 = 18000
      intDividend = intRemainder;                                                              // tempdividend = 18000
    }                                                                                          //
    if (intDividend > 0) {                                                                     // 18000 > 0: TRUE
      float fltSeconds = intDividend / 1000;                                                   // 18000 / 1000 = 18 seconds
      debugPrint(defDebugMQTTReceive, String(fltSeconds, 2));                                  // display 18.00 seconds
      debugPrintln(defDebugMQTTReceive, "secs");                                               //
    }                                                                                          //
#endif                                                                                         //
  } else {                                                                                     //
    debugPrint(defDebugMQTTReceive, "is outside acceptable range. Keeping existing value: ");  //
    debugPrint(defDebugMQTTReceive, intReportInterval);                                        //
    debugPrintln(defDebugMQTTReceive, " msecs\n");                                             //
  }
}