void processreceivedcommand(String strRcvdCmd, String strRcvdValue) {           // process incoming messages. This has been separated from OnMQTTMessage so we can manually invoke on Serial inputs
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
}

void cmd_areyouthere() {                                                                    // simple response routine to respond to MQTT checkin messages.
  String strXmitMsg = strStandardMsg("lastcheckin", String(instNTPClient.getEpochTime()));  // MIGHT BE AVOIDABLE by querying the MQTT broker's KEEPALIVE stats. Should be doable and would reduce network traffic
  transmitmqttmessage("node/lastcheckin", strXmitMsg, true, 1);                             // send current UTC as our last checkin time
  strXmitMsg = strStandardMsg("status", "online");                                          // TODO: make the server update this topic after successfully processing our last checkin time
  transmitmqttmessage("node/status", strXmitMsg, true, 1);                                  //
}

void cmd_reportconfig() {                                                                // save on network traffic and packet size by reporting config ONLY when requested
  debugprefix();                                                                         //
  String strXmitMsg = strKeyValuePair("softwareversion", defSoftwareVersion) + ",";      // initialize msg payload holder; avoiding JSON calls to save memory
  strXmitMsg += strKeyValuePair("compiled", __DATE__) + ",";                             //
  //strXmitMsg += strKeyValuePair("IP", WiFi.localIP()) + ",";                     //
  strXmitMsg += strKeyValuePair("CurrReportInterval", String(intReportInterval)) + ",";  //
  strXmitMsg += strKeyValuePair("commandtopic", "node/config/" + strClientID);           //
  strXmitMsg = strJSONwrap(strXmitMsg);                                                  //
  strXmitMsg = strStandardMsg("diagnostics", strXmitMsg);                                // now, package that object into an object
  debugoutput("Reporting config: ");                                                     //
  debugoutputln(strXmitMsg);                                                             //
  transmitmqttmessage("node/config", strXmitMsg, true, 1);                               // send it
}

void cmd_reportdiagnostics() {                                                     // save on network traffic and packet size by reporting diagnostics ONLY when requested
  debugprefix();                                                                   //
  String strXmitMsg = strKeyValuePair("WiFiStr_Curr", String(WiFi.RSSI())) + ",";  // start assembling the payload
  strXmitMsg += strKeyValuePair("WiFiStr_Avg", String(intAvgRSSI)) + ",";          //
  strXmitMsg += strKeyValuePair("MsgSentToDate", String(intMsgCnt)) + ",";         //
  strXmitMsg += strKeyValuePair("UpTime", String(millis()));                       //
  strXmitMsg = strJSONwrap(strXmitMsg);                                            // wrap the assembled payload string into an object, then
  strXmitMsg = strStandardMsg("diagnostics", strXmitMsg);                          // add the standard msg header
  debugoutput("Reporting diagnostics: ");                                          //
  debugoutputln(strXmitMsg);                                                       //
  transmitmqttmessage("node/diagnostics", strXmitMsg, true, 1);                    // send it
}

void cmd_resetdiagnostics() {                           // reset any tracked diagnostics
  debugprefix();                                        //
  debugoutput("*** RESETTING DIAGNOSTICS INFO ***\n");  //
  intAvgRSSI = 0;                                       //
  intMsgCnt = 0;                                        //
}

void cmd_reboot() {                                           // act on a remote reboot command. WARNING: NO REPEATBACK OR CONFIRMATION
  debugprefix();                                              //
  debugoutput("------------REBOOTING------------\n\n");       // should we mqtt out a going offline message? adieu! farewell! Off wierder Shane! goodbye!
  String strXmitMsg = strStandardMsg("status", "REBOOTING");  //
  strXmitMsg = strJSONwrap(strXmitMsg);                       // make sure to wrap the entire thing before sending
  transmitmqttmessage("node/status", strXmitMsg, true, 1);    //
  ESP.restart();
}

void cmd_launchwifiportal() {                                       // launch the WiFiManager configuration portal
  debugprefix();                                                    //
  debugoutput("------LAUNCHING WIFI MANAGER PORTAL------\n\n");     // should we mqtt out a going offline message? adieu! farewell! Off wierder Shane! goodbye!
  String strXmitMsg = strStandardMsg("status", "LAUNCHINGPORTAL");  //
  strXmitMsg = strJSONwrap(strXmitMsg);                             // make sure to wrap the entire thing before sending
  transmitmqttmessage("node/status", strXmitMsg, true, 1);          //
  wifiManager.startConfigPortal();                                  //
}

void cmd_ntpresync(int intTimeZoneOffset) {                                     // force an NTP sync only if we're not emulating. TODO: implement time zone offset?
  debugprefix();                                                                //
  intTimeZoneOffset = 0;                                                        //
  if (!boolMQTTEmulated) { instNTPClient.update(); }                            //
  ul_lastupdate = millis();                                                     //
  String strXmitMsg = strKeyValuePair("lastsync", String(CurrentEpochTime()));  // assemble payload
  strXmitMsg = strJSONwrap(strXmitMsg);                                         // make sure to wrap the entire thing before sending
  strXmitMsg = strStandardMsg("lastsync", strXmitMsg);                          // assemble standard message header
  debugoutputln(strXmitMsg);                                                    //
  transmitmqttmessage("node/ntpsync", strXmitMsg, true, 1);                     // send it
}

void cmd_listcommands() {                      // TODO: convert command list to an enum so we can iterate through like an array AND maintain this command list in the global declarations
  debugprefix();                               //
  String strAssembledMsg = "";                 //
  String strTopic = "node/availablecommands";  //
  debugoutputln(strTopic);                     //
  // DynamicJsonDocument objJSONdoc(1024);      //
  // objJSONdoc["ID"] = strClientID;            //
  // objJSONdoc["function"] = defNodeFunction;  //
  // serializeJson(objJSONdoc, strAssembledMsg);           // serialize
}

void cmd_newreportinterval(int intNewReportInterval) {                   // display new interval (converted for readability) and apply to global variable.  TODO: Split off time conversion function to return a string
  debugprefix();                                                         //
  if (intNewReportInterval > 500 && intNewReportInterval < 259200000) {  // TODO: When we convert commands to an enum, we need to make the min/max parameters a lookup
    intReportInterval = intNewReportInterval;                            //
#if (defDebugFull)
    // if debugging isn't enabled, we can skip this entire section. don't do math you don't have to i always say
    debugoutput("New report interval: ");                               // new interval is valid, so parse to something human readable and display.
    int intDividend = intNewReportInterval;                             // example: 93,858,000 milliseconds = 1 day, 2 hours, 4 mins, 18 secs
    int intRemainder = 0;                                               //
    int intMultiplier = 0;                                              //
    int intDivisorMinute = 60 * 1000;                                   // 60 seconds converted to milliseconds = 6,000
    int intDivisorHour = intDivisorMinute * 60;                         // 60 minutes in an hour = 3,600,000
    int intDivisorDay = intDivisorHour * 24;                            // 24 hours in a day = 86,400,000
    if (intDividend > intDivisorDay) {                                  // 93,858,000 > 86,400,000: TRUE
      intMultiplier = intDividend / intDivisorDay;                      // inttemp = 93858000/86400000 = 1
      debugoutput(intMultiplier);                                       //
      debugoutput("days ");                                             //
      intRemainder = intDividend % (intMultiplier * intDivisorDay);     // tempremainder = 93858000 % (1 * 86400000) ... 93858000 % 86400000 = 7458000
      intDividend = intRemainder;                                       // tempdividend = 7458000 (2hrs4mins18secs)
    }                                                                   //
    if (intDividend > intDivisorHour) {                                 // tempdividend = 7,458,000 > 3,600,000: TRUE
      intMultiplier = intDividend / (intDivisorHour);                   // 7458000 / 3600000 = 2
      debugoutput(intMultiplier);                                       //
      debugoutput("hours ");                                            //
      intRemainder = intDividend % (intMultiplier * intDivisorHour);    // 7458000 % (2 * 3600000) ... 7458000 % 7200000 = 258000
      intDividend = intRemainder;                                       // tempdividend = 258000
    }                                                                   //
    if (intDividend > intDivisorMinute) {                               // 258000 > 6000: TRUE
      intMultiplier = intDividend / intDivisorMinute;                   // 258000 / 6000 = 4
      debugoutput(intMultiplier);                                       //
      debugoutput("mins ");                                             //
      intRemainder = intDividend % (intMultiplier * intDivisorMinute);  // 258000 % (4 * 6000) ... 258000 % 24000 = 18000
      intDividend = intRemainder;                                       // tempdividend = 18000
    }                                                                   //
    if (intDividend > 0) {                                              // 18000 > 0: TRUE
      float fltSeconds = intDividend / 1000;                            // 18000 / 1000 = 18 seconds
      debugoutput(String(fltSeconds, 2));                               // display 18.00 seconds
      debugoutput("secs");                                              //
    }                                                                   //
    debugoutput("\n");                                                  //
#endif                                                                  //
  } else {                                                              //
    debugoutput("Requested report interval outside ");                  //
    debugoutput("acceptable range. Keeping existing value: ");          //
    debugoutput(intReportInterval);                                     //
    debugoutput(" msecs\n");                                            //
  }
}