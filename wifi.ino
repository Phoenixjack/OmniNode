void setup_wifi() {
  debugprefix();                      //
  led.brightness(RGBLed::GREEN, 50);  //
  String strFileContents = readFile(strConfigFilename.c_str());
  debugoutput("Setup: Read from file: ");
  debugoutputln(strFileContents);
  DynamicJsonDocument json(1024);
  auto deserializeError = deserializeJson(json, strFileContents);
  if (!deserializeError) {
    debugoutput("\nSetup: Parsed config file JSON\n");
    strcpy(charMQTTServer, json["mqtt_server"]);
    strcpy(charMQTTPort, json["mqtt_port"]);
    debugoutputln(charMQTTServer);
    debugoutputln(charMQTTPort);
  }
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  bool res = wifiManager.autoConnect();  // auto generated AP name from chipid
  if (!res) {
    led.brightness(RGBLed::RED, 100);   // fault indicator
    led.brightness(RGBLed::GREEN, 50);  //
    debugoutput("Failed to connect\n");
  } else {
    debugoutput("WiFi connected!\n");    // if you get here you have connected to the WiFi
    led.brightness(RGBLed::GREEN, 100);  // bright green to indicate WiFi is connected
    serialprintwificonfig();
  }
}

void serialprintwificonfig() {
  debugprefix();  //
  // WiFi.printDiag(Serial);
  debugoutput("SSID: ");  //
  debugoutput(WiFi.SSID());
  debugoutput("\t RSSI: ");  //
  debugoutput(WiFi.RSSI());
  debugoutput("\nIPv4: ");
  debugoutput(WiFi.localIP());
  debugoutput("\tGATEWAY: ");
  debugoutput(WiFi.gatewayIP());
  debugoutput("\tSUBNET MASK: ");
  debugoutputln(WiFi.subnetMask());
  debugoutputln(strMQTTserver);
  debugoutputln(intMQTTPort);
}

void saveConfigCallback() {
  debugprefix();  //
  debugoutput("saveConfigCallback: Should save config\n");
  boolSaveWiFiConfig = true;
  String strSaveFileBuffer = "";
  debugoutput("saveConfigCallback: Saving config to ");
  debugoutput(strConfigFilename);
  debugoutput(": ");
  DynamicJsonDocument json(200);
  json["mqtt_server"] = custom_mqtt_server.getValue();
  json["mqtt_port"] = custom_mqtt_port.getValue();
  strcpy(charMQTTServer, custom_mqtt_server.getValue());
  strcpy(charMQTTPort, custom_mqtt_port.getValue());
  strMQTTserver = String(charMQTTServer);
  intMQTTPort = atoi(charMQTTPort);
  serializeJson(json, strSaveFileBuffer);
  debugoutput("saveConfigCallback: ");
  debugoutputln(strSaveFileBuffer);
  writeFile(strConfigFilename.c_str(), strSaveFileBuffer.c_str());
}