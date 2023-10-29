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