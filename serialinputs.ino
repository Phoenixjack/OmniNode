String strGetSerialData() {     // reads incoming data from the secondary Serial port for packet forwarding
  String strSerialPacket = "";  //
  debugPrefix(defDebugFull);    //
#if (defFuncPacketFwd)
  unsigned long ulLastMsg, ulNow;          //
  while (mySerial.available()) {           // loop through while data is available
    ulNow = millis();                      //
    if (ulNow - ulLastMsg > 1.5) {         // 1.5 msecs appears to be the sweet spot
      ulLastMsg = ulNow;                   //
      strSerialPacket += mySerial.read();  // get character and append
    }                                      //
  }                                        //
#endif
  debugoutputln(strSerialPacket);  //
  return strSerialPacket;          //
}

void getSerialInput() {                     // reads raw JSON from Serial input. WARNING: NO INPUT VALIDATION!
  String strPayload = Serial.readString();  //
  JSONVar objJSONdoc = JSON.parse(strPayload);
  if (JSON.typeof(objJSONdoc) == "undefined") {                            //
    debugPrefix(defDebugJSON);                                             //
    debugJSON("deserializeJson() failed: ");                               //
    debugJSON(boolJSONError.f_str());                                      //
    debugJSON("\n");                                                       //
    //String strXmitMsg = strStandardMsg("error", "deserialization error");  // assemble short message reporting a problem. TODO: is there a way to feedback a message identifier so the server even knows which message we're talking about?
    //strXmitMsg = strJSONwrap(strXmitMsg);                                  //
    //transmitmqttmessage("node/errors", strXmitMsg, true, 1);               // send it to the error reporting topic
    return;                                                                // if we have a deserialization error, then we want to abort this routine because we can't decipher what was sent
  }                                                                        //
  String strRcvdCmd = objJSONdoc["command"];                               //
  String strRcvdValue = objJSONdoc["value"];                               // TODO: use enum and covert the mess below to a SWITCH CASE structure
  processreceivedcommand(strRcvdCmd, strRcvdValue);                        // process what we received
}

String strParsedGPSData() {  // returns CSV string of GPS data; NOT WRAPPED!
  String strGPSData = "";
  String strSubAssy = "";
#if (defFuncNEO6_7)
  // time
  if (GPS.hour < 10) { strSubAssy = '0'; }
  strSubAssy += String(GPS.hour, DEC) + ':';
  if (GPS.minute < 10) { strSubAssy += '0'; }
  strSubAssy += String(GPS.minute, DEC) + ':';
  if (GPS.seconds < 10) { strSubAssy += '0'; }
  strSubAssy += String(GPS.seconds, DEC) + '.';
  if (GPS.milliseconds < 10) {
    strSubAssy += String("00");
  } else if (GPS.milliseconds > 9 && GPS.milliseconds < 100) {
    strSubAssy += String("0");
  }
  strSubAssy += String(GPS.milliseconds);
  strGPSData = strKeyValuePair("time", strSubAssy);
  // date
  strSubAssy = "20" + String(GPS.year, DEC) + '-';
  strSubAssy += String(GPS.month, DEC) + '-';
  strSubAssy += String(GPS.day, DEC) + '-';
  strGPSData += ',' + strKeyValuePair("date", strSubAssy);
  // fix and quality
  strSubAssy = String(GPS.fixquality);
  strGPSData += ',' + strKeyValuePair("fixquality", strSubAssy);
  if (GPS.fix) {
    String strLocSubAssy = String(GPS.latitude, 4) + String(GPS.lat);
    strSubAssy = strKeyValuePair("latitude", strLocSubAssy);
    strLocSubAssy = String(GPS.longitude, 4) + String(GPS.lon);
    strSubAssy += ',' + strKeyValuePair("longitude", strLocSubAssy);
    strSubAssy += ',' + strKeyValuePair("speed", String(GPS.speed));
    strSubAssy += ',' + strKeyValuePair("angle", String(GPS.angle));
    strSubAssy += ',' + strKeyValuePair("altitude", String(GPS.altitude));
    strSubAssy += ',' + strKeyValuePair("speed", String(GPS.speed));
    strSubAssy += ',' + strKeyValuePair("satellites", String(GPS.satellites));
    strSubAssy += ',' + strKeyValuePair("antenna", String(GPS.antenna));
    strSubAssy = strJSONwrap(strSubAssy);
    strGPSData += ',' + strKeyValuePair("location", strSubAssy);
  }
#endif
  return strGPSData;
}