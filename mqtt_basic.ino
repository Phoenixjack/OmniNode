void setup_mqtt() {                                                                               // Initialize MQTT and connect to the broker
  const char charrWillTopic[] = "node/status";                                                    //
  String strWillPayload = encapsulatemessage("OFFLINE");                                          //
  bool boolWillRetain = true;                                                                     //
  int intWillQoS = 1;                                                                             //
  instMQTTClient.beginWill(charrWillTopic, strWillPayload.length(), boolWillRetain, intWillQoS);  //
  instMQTTClient.print(strWillPayload);                                                           //
  instMQTTClient.endWill();                                                                       //
  mqtt_connect();                                                                                 // connect and handle handshake protocol
}

void mqtt_connect() {                                                  // TODO: should we add a section to troubleshoot underlying wifi issues, or is that built into MQTT?
  Serial.print("Attempting MQTT connection... ");                      //
  while (!instMQTTClient.connected()) {                                // Loop until we're reconnected
    if (instMQTTClient.connect(strMQTTserver.c_str(), intMQTTPort)) {  // Attempt to connect
      Serial.println("MQTT Connected!");                               //
      instMQTTClient.onMessage(onMqttMessage);                         // assign function call for incoming messages
      transmitmqttmessage("base/announce", strClientID, true, 1);      // simple announcement that will trigger the server to check its database if it's seen this node before
      cmd_areyouthere();                                               // keep it simple and just act like we were asked to checkin via msg
      String strSubscribeTopic = "node/config/" + strClientID;         // assemble unique MQTT topic for this node to listen for.
      Serial.print("Subscribing to: ");                                // TODO: check if we need to subscribe every time we connect. What if we just disconnected from the broker and it's not an initial connection? Is the subscription retained?
      Serial.println(strSubscribeTopic);                               // show the user
      instMQTTClient.setId(strClientID);                               //
      instMQTTClient.subscribe(strSubscribeTopic.c_str(), 1);          // subscribe to that topic with QoS 1: FOLLOWUP: why are we converting a String to char array for our own function?
    } else {                                                           // if we're here, we failed to connect
      Serial.print("MQTT connection failed, rc=");                     //
      Serial.print(instMQTTClient.connectError());                     //
      Serial.println(" try again in 5 seconds");                       //
      delay(5000);                                                     // Wait 5 seconds before retrying
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