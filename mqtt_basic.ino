void setup_mqtt() {                                                                              // Initialize MQTT and connect to the broker
  String strWillPayload = strClientID + "OFFLINE";                                               //
  bool boolWillRetain = true;                                                                    //
  int intWillQoS = 1;                                                                            //
  instMQTTClient.beginWill("node/status", strWillPayload.length(), boolWillRetain, intWillQoS);  //
  instMQTTClient.print(strWillPayload);                                                          //
  instMQTTClient.endWill();                                                                      //
  mqtt_connect();                                                                                // connect and handle handshake protocol
}

void mqtt_connect() {                                                  // TODO: should we add a section to troubleshoot underlying wifi issues, or is that built into MQTT?
  led.brightness(RGBLed::MAGENTA, 50);                                 // dim magenta/purple to indicate we're starting MQTT connection
  debugPrefix(defDebugMQTTConnect);                                    //
  debugPrint(defDebugMQTTConnect, "Attempting MQTT connection... ");   //
  while (!instMQTTClient.connected()) {                                // Loop until we're reconnected
    cmd_sendUDPmqttquery();                                            // send a UDP query
    if (Serial.available() > 0) { getSerialInput(); }                  //
    String strUDPresponse = strGetUDPReply();                          // hold response
    if (strUDPresponse.startsWith("192.168", 0)) {                     //
      strMQTTserver = strUDPresponse;                                  //
      debugPrint(defDebugMQTTConnect, "set MQTT from UDP");            //
    }                                                                  //
    if (instMQTTClient.connect(strMQTTserver.c_str(), intMQTTPort)) {  // Attempt to connect
      debugPrint(defDebugMQTTConnect, "MQTT Connected!\n");            //
      instMQTTClient.onMessage(onMqttMessage);                         // assign function call for incoming messages
      String strSubscribeTopic = "node/" + strClientID;                // assemble unique MQTT topic for this node to listen for.
      JSONVar objAnnouncePayload;                                      //
      defstandardheader(objAnnouncePayload);                           //
      objAnnouncePayload["version"] = defSoftwareVersion;              //
      objAnnouncePayload["function"] = defNodeFunction;                //
      objAnnouncePayload["commandtopic"] = strSubscribeTopic;          //
      objAnnouncePayload["reportint"] = intReportInterval;             //
      String jsonString = JSON.stringify(objAnnouncePayload);          //
      transmitmqttmessage("base/announce", jsonString, true, 1);       // simple announcement that will trigger the server to check its database if it's seen this node before
      debugPrint(defDebugMQTTConnect, "Subscribing to: \"");           // TODO: check if we need to subscribe every time we connect. What if we just disconnected from the broker and it's not an initial connection? Is the subscription retained?
      debugPrint(defDebugMQTTConnect, strSubscribeTopic);              // show the user
      instMQTTClient.setId(strClientID);                               //
      instMQTTClient.subscribe(strSubscribeTopic.c_str(), 1);          // subscribe to that topic with QoS 1: FOLLOWUP: why are we converting a String to char array for our own function?
    } else {                                                           // if we're here, we failed to connect
      led.brightness(RGBLed::RED, 100);                                // bright red fault indicator
      debugPrint(defDebugMQTTConnect, "MQTT connection failed, rc=");  //
      debugPrint(defDebugMQTTConnect, instMQTTClient.connectError());  //
      debugPrintln(defDebugMQTTConnect, " try again");                 //
      led.flash(RGBLed::MAGENTA, defBlinkLong, 200);                   // calling RGBLED flash this way turns the light on for defBlinkLong, then off for 200ms
      led.flash(RGBLed::RED, defBlinkLong, 200);                       // fault indicator
    }                                                                  //
  }                                                                    //
  led.brightness(RGBLed::MAGENTA, 100);                                // full brightness magenta for successful MQTT connection
}

void onMqttMessage(int intMessageSize) {                                               // Routine for handling incoming messages
  debugPrefix(defDebugMQTTReceive);                                                    //
  debugPrint(defDebugMQTTReceive, "Received a message with topic '");                  //
  debugPrint(defDebugMQTTReceive, instMQTTClient.messageTopic());                      //
  debugPrint(defDebugMQTTReceive, "', duplicate = ");                                  //
  debugPrint(defDebugMQTTReceive, instMQTTClient.messageDup() ? "true" : "false");     // TODO: skip processing of message if duplicate is true. DOUBLE CHECK DOCUMENTATION BEFOREHAND!
  debugPrint(defDebugMQTTReceive, ", QoS = ");                                         //
  debugPrint(defDebugMQTTReceive, instMQTTClient.messageQoS());                        //
  debugPrint(defDebugMQTTReceive, ", retained = ");                                    //
  debugPrint(defDebugMQTTReceive, instMQTTClient.messageRetain() ? "true" : "false");  //
  debugPrint(defDebugMQTTReceive, "', length = ");                                     //
  debugPrint(defDebugMQTTReceive, intMessageSize);                                     //
  debugPrintln(defDebugMQTTReceive, " bytes");                                         //
  String strPayload = "";                                                              //
  while (instMQTTClient.available()) {                                                 // read mqtt buffer into a string
    strPayload += (char)instMQTTClient.read();                                         // make sure to use the character represented, NOT the raw ASCII code
  }                                                                                    //
  debugPrintln(defDebugMQTTReceive, strPayload);                                       //
  JSONVar objJSONdoc = JSON.parse(strPayload);                                         //
  if (JSON.typeof(objJSONdoc) == "undefined") {                                        //
    debugPrint(defDebugMQTTReceive, "deserializeJson() failed\n");                     //
    return;                                                                            // if we have a deserialization error, then we want to abort this routine because we can't decipher what was sent
  }                                                                                    //
  String strRcvdCmd = objJSONdoc["command"];                                           //
  String strRcvdValue = objJSONdoc["value"];                                           // TODO: use enum and covert the mess below to a SWITCH CASE structure
  processreceivedcommand(strRcvdCmd, strRcvdValue);                                    // process what we received
}

void transmitmqttmessage(String strXmitTopic, String strXmitMsg, bool boolRetainMsg, int intQoS) {       // Single point for publishing messages.
  led.brightness(RGBLed::MAGENTA, 100);                                                                  //
  debugPrefix(defDebugMQTTTransmit);                                                                     //
  debugPrintln(defDebugMQTTTransmit, strXmitTopic);                                                      //
  debugPrintln(defDebugMQTTTransmit, strXmitMsg);                                                        //
  debugPrint(defDebugMQTTTransmit, "currRSSI:");                                                         //
  int intCurrRSSI = WiFi.RSSI();                                                                         // Get current RSSI value
  debugPrint(defDebugMQTTTransmit, intCurrRSSI);                                                         //
  intMsgCnt++;                                                                                           //
  debugPrint(defDebugMQTTTransmit, "\t SentMsgCount:");                                                  //
  debugPrintln(defDebugMQTTTransmit, intMsgCnt);                                                         //
  instMQTTClient.beginMessage(strXmitTopic.c_str(), strXmitMsg.length(), boolRetainMsg, intQoS, false);  // last position is duplicate flag. we don't use that feature, so no need to create a variable just for it.
  instMQTTClient.print(strXmitMsg);                                                                      //
  instMQTTClient.endMessage();                                                                           //
  led.brightness(RGBLed::WHITE, 50);                                                                     //
}