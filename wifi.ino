void setup_wifi() {
  led.brightness(RGBLed::GREEN, 50);                         //
  String strFileContents = readFile(strConfigFilename.c_str());
  Serial.print("Setup: Read from file: ");
  Serial.println(strFileContents);
  DynamicJsonDocument json(1024);
  auto deserializeError = deserializeJson(json, strFileContents);
  if (!deserializeError) {
    Serial.println("\nSetup: Parsed config file JSON");
    strcpy(charMQTTServer, json["mqtt_server"]);
    strcpy(charMQTTPort, json["mqtt_port"]);
    Serial.println(charMQTTServer);
    Serial.println(charMQTTPort);
  }
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  bool res = wifiManager.autoConnect();  // auto generated AP name from chipid
  if (!res) {
    led.brightness(RGBLed::RED, 100);                        // fault indicator
    led.brightness(RGBLed::GREEN, 50);                       //
    Serial.printf("%s : %s : Failed to connect\n", __FILE__, __func__);
    // Serial.println("Setup: Failed to connect");
  } else {
    Serial.printf("%s : %s : WiFi connected!\n", __FILE__, __func__);  // if you get here you have connected to the WiFi
    // Serial.println("Setup: WiFi connected!");
    led.brightness(RGBLed::GREEN, 100);                        // bright green to indicate WiFi is connected
    serialprintwificonfig();
  }
}

void serialprintwificonfig() {
  WiFi.printDiag(Serial);
  Serial.println(WiFi.SSID());
  Serial.println(WiFi.RSSI());
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.gatewayIP());
  Serial.println(WiFi.subnetMask());
  Serial.println(strMQTTserver);
  Serial.println(intMQTTPort);
}

void saveConfigCallback() {
  Serial.println("saveConfigCallback: Should save config");
  boolSaveWiFiConfig = true;
  String strSaveFileBuffer = "";
  Serial.print("saveConfigCallback: Saving config to ");
  Serial.print(strConfigFilename);
  Serial.print(": ");
  DynamicJsonDocument json(200);
  json["mqtt_server"] = custom_mqtt_server.getValue();
  json["mqtt_port"] = custom_mqtt_port.getValue();
  strcpy(charMQTTServer, custom_mqtt_server.getValue());
  strcpy(charMQTTPort, custom_mqtt_port.getValue());
  strMQTTserver = String(charMQTTServer);
  intMQTTPort = atoi(charMQTTPort);
  serializeJson(json, strSaveFileBuffer);
  Serial.print("saveConfigCallback: ");
  Serial.println(strSaveFileBuffer);
  writeFile(strConfigFilename.c_str(), strSaveFileBuffer.c_str());
}