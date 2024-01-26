void setup_wifi() {
  debugPrefix(defDebugWiFi);                                          //
  led.brightness(RGBLed::GREEN, 50);                                  //
  String strFileContents = readFile(strConfigFilename.c_str());       // copy contents of file into string variable; FOLLOWUP: Can we read directly from memory or is this just simpler? Granted, it's a small file, but still
  DynamicJsonDocument objJSONdoc(1024);                               // FOLLOWUP: can we scale that JSON allocation down? What's the max size?
  auto boolJSONError = deserializeJson(objJSONdoc, strFileContents);  // FOLLOWUP: Should we add a conditional in case the file could NOT be loaded/read? Gonna be hard to parse 0% of nothing, carry the zero
  if (!boolJSONError) {                                               // IF we were able to read the file AND parse the contents, then load those settings
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
  DynamicJsonDocument objJSONdoc(200);                              //
  objJSONdoc["mqtt_server"] = custom_mqtt_server.getValue();        //
  objJSONdoc["mqtt_port"] = custom_mqtt_port.getValue();            //
  strcpy(charMQTTServer, custom_mqtt_server.getValue());            //
  strcpy(charMQTTPort, custom_mqtt_port.getValue());                //
  strMQTTserver = String(charMQTTServer);                           //
  intMQTTPort = atoi(charMQTTPort);                                 //
  serializeJson(objJSONdoc, strSaveFileBuffer);                     //
  writeFile(strConfigFilename.c_str(), strSaveFileBuffer.c_str());  //
}
