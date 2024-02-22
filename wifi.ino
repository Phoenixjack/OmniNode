void setup_wifi() {
  debugPrefix(defDebugWiFi);                                          //
  led.brightness(RGBLed::GREEN, 50);                                  //
  String strFileContents = readFile(strConfigFilename.c_str());       // copy contents of file into string variable; FOLLOWUP: Can we read directly from memory or is this just simpler? Granted, it's a small file, but still
  JSONVar objJSONdoc = JSON.parse(strFileContents);
  if (JSON.typeof(objJSONdoc) != "undefined") {                                               // IF we were able to read the file AND parse the contents, then load those settings
    debugJSON("\nParsed config file JSON\n");                         //
    strcpy(charMQTTServer, objJSONdoc["mqtt_server"]);                //
    strcpy(charMQTTPort, objJSONdoc["mqtt_port"]);                    //
    debugJSON(charMQTTServer);                                        //
    debugJSON("\n");                                                  //
    debugJSON(charMQTTPort);                                          //
    debugJSON("\n");                                                  //
  }                                                                   //
  wifiManager.addParameter(&custom_mqtt_server);                      // make wifimanager add input boxes on the portal
  wifiManager.addParameter(&custom_mqtt_port);                        //
  wifiManager.setSaveConfigCallback(saveConfigCallback);              // attach the function to save the user's entry to file
  bool boolWMConnect = wifiManager.autoConnect();                     // auto generated AP name from chipid
  debugWiFi("WiFi ");                                                 //
  if (!boolWMConnect) {                                               //
    debugWiFi("failed to connect\n");                                 //
    while (boolWMConnect) {                                           // infinite loop since we can't wifi
      if (Serial.available() > 0) { getSerialInput(); }               //
      led.flash(RGBLed::RED, defBlinkLong, 200);                      // fault indicator
      led.flash(RGBLed::GREEN, defBlinkLong, 200);                    // calling RGBLED flash this way turns the light on for defBlinkLong, then off for 20ms
    }                                                                 //
  } else {                                                            //
    debugWiFi("connected!\n");                                        // if you get here you have connected to the WiFi
    led.brightness(RGBLed::GREEN, 100);                               // bright green to indicate WiFi is connected
    serialprintwificonfig();                                          //
  }
}


void cmd_sendUDPmqttquery() {
  static unsigned long ul_lastUDPmqttquery;  // will default to zero
  Serial.print("called UDP xmt. last UDP query: ");
  Serial.print(ul_lastUDPmqttquery);
  Serial.print("... ");
  //if (ul_lastUDPmqttquery = 0 || millis() < ul_lastUDPmqttquery + 3000) {  // if it's the first time OR been more than 3 seconds,
  Serial.println("sent UDP");
  instUDP.beginPacketMulticast(instBroadcast, uintLocalUDPPort, WiFi.localIP());  // then we'll allow it to be sent
  instUDP.write("mqtt-query");
  instUDP.endPacket();
  ul_lastUDPmqttquery = millis();
  //}
}

String strGetUDPReply() {
  // {"command":"setmqtt","value":"192.168.1.68"}
  Serial.print("called UDP rcv. Length: ");
  int packetSize = instUDP.parsePacket();
  Serial.println(packetSize);
  String strReceivedUDPData = "";
  char charrIncomingPacket[255];  // buffer for incoming packets
  if (packetSize) {
    Serial.println("received UDP");
    // receive incoming UDP packets
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, instUDP.remoteIP().toString().c_str(), instUDP.remotePort());
    int len = instUDP.read(charrIncomingPacket, 255);
    if (len > 0) {
      //incomingPacket[len] = 0;
    }
    //Serial.printf("UDP packet contents: %s\n", charrIncomingPacket);
    //delay(2000);
    // send back a reply, to the IP address and port we got the packet from
    //instUDP.beginPacket(instUDP.remoteIP(), instUDP.remotePort());
    //instUDP.write(charrIncomingPacket);
    //instUDP.endPacket();
    strReceivedUDPData = String(charrIncomingPacket);
  }
  return strReceivedUDPData;
}

void serialprintwificonfig() {  // Print out our wifi config info
  debugPrefix(defDebugWiFi);    //
  debugWiFi("\nWiFi CONFIG\n");
  debugWiFi("  SSID: ");
  debugWiFi(WiFi.SSID());
  debugWiFi("\t RSSI: ");
  debugWiFi(WiFi.RSSI());
  debugWiFi("\n  IPv4: ");
  debugWiFi(WiFi.localIP());
  debugWiFi("\tGATEWAY: ");
  debugWiFi(WiFi.gatewayIP());
  debugWiFi("\tSUBNET MASK: ");
  debugWiFi(WiFi.subnetMask());
  debugWiFi("\n  MQTT BROKER: ");
  debugWiFi(strMQTTserver);
  debugWiFi(":");
  debugWiFi(intMQTTPort);
  debugWiFi("\n\n");
}

void saveConfigCallback() {                                         // function to save input from the wifimanager portal to a file
  debugPrefix(defDebugWiFi);                                        //
  debugWiFi("Should save config\n");                                //
  boolSaveWiFiConfig = true;                                        // FOLLOWUP: do we even need this boolean? It's carried over from the wifimanager example code.
  String strSaveFileBuffer = "";                                    //
  JSONVar objJSONdoc;                                               //
  objJSONdoc["mqtt_server"] = custom_mqtt_server.getValue();        //
  objJSONdoc["mqtt_port"] = custom_mqtt_port.getValue();            //
  strcpy(charMQTTServer, custom_mqtt_server.getValue());            //
  strcpy(charMQTTPort, custom_mqtt_port.getValue());                //
  strMQTTserver = String(charMQTTServer);                           //
  intMQTTPort = atoi(charMQTTPort);                                 //
  strSaveFileBuffer = JSON.stringify(objJSONdoc);                   //
  writeFile(strConfigFilename.c_str(), strSaveFileBuffer.c_str());  //
}
