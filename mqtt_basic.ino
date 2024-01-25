void setup_mqtt() {                                                                              // Initialize MQTT and connect to the broker
  String strWillPayload = strStandardMsg("status", "OFFLINE");                                   //
  strWillPayload = strJSONwrap(strWillPayload);                                                  //
  bool boolWillRetain = true;                                                                    //
  int intWillQoS = 1;                                                                            //
  instMQTTClient.beginWill("node/status", strWillPayload.length(), boolWillRetain, intWillQoS);  //
  instMQTTClient.print(strWillPayload);                                                          //
  instMQTTClient.endWill();                                                                      //
  mqtt_connect();                                                                                // connect and handle handshake protocol
}

void mqtt_connect() {                                                  // TODO: should we add a section to troubleshoot underlying wifi issues, or is that built into MQTT?
  led.brightness(RGBLed::MAGENTA, 50);                                 // dim magenta/purple to indicate we're starting MQTT connection
  debugPrefix(defDebugMQTTTerse);                                      //
  debugMQTTTerse("Attempting MQTT connection... ");                    //
  while (!instMQTTClient.connected()) {                                // Loop until we're reconnected
    if (Serial.available() > 0) { getSerialInput(); }                  //
    if (instMQTTClient.connect(strMQTTserver.c_str(), intMQTTPort)) {  // Attempt to connect
      debugMQTTTerse("MQTT Connected!\n");                             //
      instMQTTClient.onMessage(onMqttMessage);                         // assign function call for incoming messages
      transmitmqttmessage("base/announce", strClientID, true, 1);      // simple announcement that will trigger the server to check its database if it's seen this node before
      cmd_areyouthere();                                               // keep it simple and just act like we were asked to checkin via msg
      String strSubscribeTopic = "node/" + strClientID;                // assemble unique MQTT topic for this node to listen for.
      debugMQTTTerse("Subscribing to: \"");                            // TODO: check if we need to subscribe every time we connect. What if we just disconnected from the broker and it's not an initial connection? Is the subscription retained?
      debugMQTTTerse(strSubscribeTopic);                               // show the user
      debugMQTTTerse("\n");                                            // since we didn't macro a terse println
      instMQTTClient.setId(strClientID);                               //
      instMQTTClient.subscribe(strSubscribeTopic.c_str(), 1);          // subscribe to that topic with QoS 1: FOLLOWUP: why are we converting a String to char array for our own function?
    } else {                                                           // if we're here, we failed to connect
      led.brightness(RGBLed::RED, 100);                                // bright red fault indicator
      debugMQTTTerse("MQTT connection failed, rc=");                   //
      debugMQTTTerse(instMQTTClient.connectError());                   //
      debugMQTTTerse(" try again\n");                                  //
      led.flash(RGBLed::MAGENTA, defBlinkLong, 200);                   // calling RGBLED flash this way turns the light on for defBlinkLong, then off for 200ms
      led.flash(RGBLed::RED, defBlinkLong, 200);                       // fault indicator
    }                                                                  //
  }                                                                    //
  led.brightness(RGBLed::MAGENTA, 100);                                // full brightness magenta for successful MQTT connection
}

void onMqttMessage(int intMessageSize) {                                         // Routine for handling incoming messages
  StaticJsonDocument<200> objJSONdoc;                                            // Should this be converted to dynamic? Or do we need a larger size?
  debugPrefix(defDebugMQTTTerse);                                                //
  debugMQTTTerse("Received a message with topic '");                             //
  debugMQTTTerse(instMQTTClient.messageTopic());                                 //
  debugMQTTTerse("', duplicate = ");                                             //
  debugMQTTTerse(instMQTTClient.messageDup() ? "true" : "false");                // TODO: skip processing of message if duplicate is true. DOUBLE CHECK DOCUMENTATION BEFOREHAND!
  debugMQTTTerse(", QoS = ");                                                    //
  debugMQTTTerse(instMQTTClient.messageQoS());                                   //
  debugMQTTTerse(", retained = ");                                               //
  debugMQTTTerse(instMQTTClient.messageRetain() ? "true" : "false");             //
  debugMQTTTerse("', length = ");                                                //
  debugMQTTTerse(intMessageSize);                                                //
  debugMQTTTerse(" bytes");                                                      //
  String strPayload = "";                                                        //
  while (instMQTTClient.available()) {                                           // read mqtt buffer into a string
    strPayload += (char)instMQTTClient.read();                                   // make sure to use the character represented, NOT the raw ASCII code
  }                                                                              //
  DeserializationError boolJSONError = deserializeJson(objJSONdoc, strPayload);  //
  if (boolJSONError) {                                                           //
    debugMQTTTerse("deserializeJson() failed: ");                                //
    debugMQTTTerse(boolJSONError.f_str());                                       //
    debugMQTTTerse("\n");                                                        //
    String strXmitMsg = strStandardMsg("error", "deserialization error");        // assemble short message reporting a problem. TODO: is there a way to feedback a message identifier so the server even knows which message we're talking about?
    strXmitMsg = strJSONwrap(strXmitMsg);                                        //
    transmitmqttmessage("node/errors", strXmitMsg, true, 1);                     // send it to the error reporting topic
    return;                                                                      // if we have a deserialization error, then we want to abort this routine because we can't decipher what was sent
  }                                                                              //
  String strRcvdCmd = objJSONdoc["command"];                                     //
  String strRcvdValue = objJSONdoc["value"];                                     // TODO: use enum and covert the mess below to a SWITCH CASE structure
  processreceivedcommand(strRcvdCmd, strRcvdValue);                              // process what we received
}

String strKeyValuePair(String strKey, String strValue) {  // simple function for a single key/value pair
  String strAssembledString;
  if (strValue.startsWith("{")) {
    strAssembledString = "\"" + strKey + "\":" + strValue;  // "key":{"key":"value"} Don't enclose JSON objects in quotes, even when they're values
  } else {
    strAssembledString = "\"" + strKey + "\":\"" + strValue + "\"";  // "key":"value"
  }
  return strAssembledString;  // give it back
}

String strJSONwrap(String value) {              // simple function to wrap an object
  String strWrappedObject = "{" + value + "}";  // {object}
  return strWrappedObject;                      // give it back
}

String strStandardMsg(String strDesc, String strPayload) {                              // returns an UNwrapped JSON object with standard meta info
  String strAssembledString = strKeyValuePair("title", String(defNodeFunction)) + ",";  // we're avoiding JSON library calls to save memory
  strAssembledString += strKeyValuePair("sender", strClientID) + ",";                   // TODO: implement pointers to reduce 'copying' variables during all these function calls
  strAssembledString += strKeyValuePair("description", strDesc) + ",";                  //
  strAssembledString += strKeyValuePair("time", String(CurrentEpochTime())) + ",";      //
  strAssembledString += strKeyValuePair("payload", strPayload);                         //
  return strAssembledString;                                                            //
}

void transmitmqttmessage(String strXmitTopic, String strXmitMsg, bool boolRetainMsg, int intQoS) {         // Single point for publishing messages. Ensures messages are counted and avg RSSI is tracked. TODO: add anomaly tracking, QoS, and message retention
  led.brightness(RGBLed::MAGENTA, 100);                                                                    //
  debugPrefix(defDebugMQTTVerbose);                                                                        //
  debugMQTTVerbose(strXmitTopic);                                                                          //
  debugMQTTVerbose("\n");                                                                                  //
  debugMQTTVerbose(strXmitMsg);                                                                            //
  debugMQTTVerbose("\n");                                                                                  //
  int intCurrRSSI;                                                                                         //
  strXmitMsg = strJSONwrap(strXmitMsg);                                                                    // make sure to wrap the entire thing before sending
  if (boolMQTTEmulated) {                                                                                  //
    debugMQTTVerbose("fakeRSSI:");                                                                         // emulated, so fake it
    intCurrRSSI = random(-50, -30);                                                                        // fake rssi value
  } else {                                                                                                 //
    debugMQTTVerbose("currRSSI:");                                                                         //
    intCurrRSSI = WiFi.RSSI();                                                                             // Get current RSSI value
  }                                                                                                        //
  debugMQTTVerbose(intCurrRSSI);                                                                           //
  intAvgRSSI = (intCurrRSSI + (intMsgCnt * intAvgRSSI)) / (intMsgCnt + 1);                                 // calculate average. Keep it in this order so we don't have a divide by zero
  debugMQTTVerbose("\t avgRSSI:");                                                                         //
  debugMQTTVerbose(intAvgRSSI);                                                                            //
  intMsgCnt++;                                                                                             //
  debugMQTTVerbose("\t SentMsgCount:");                                                                    //
  debugMQTTVerbose(intMsgCnt);                                                                             //
  debugMQTTVerbose("\n");                                                                                  //
  if (!boolMQTTEmulated) {                                                                                 //
    instMQTTClient.beginMessage(strXmitTopic.c_str(), strXmitMsg.length(), boolRetainMsg, intQoS, false);  // last position is duplicate flag. we don't use that feature, so no need to create a variable just for it.
    instMQTTClient.print(strXmitMsg);                                                                      //
    instMQTTClient.endMessage();                                                                           //
  }                                                                                                        //
  led.brightness(RGBLed::WHITE, 50);                                                                       //
}