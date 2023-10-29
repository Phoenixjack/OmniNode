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
