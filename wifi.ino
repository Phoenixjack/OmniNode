void setup_wifi() {                        // Initialize WiFi
  Serial.print("Connecting to ");          //
  Serial.println(charrSSID);               //
  WiFi.mode(WIFI_STA);                     //
  WiFi.begin(charrSSID, charrWiFiPass);    //
  while (WiFi.status() != WL_CONNECTED) {  //
    delay(500);                            //
    Serial.print(".");                     //
  }                                        //
  Serial.println();                        //
  Serial.println(WiFi.macAddress());       //
  Serial.println(WiFi.gatewayIP());        //
  Serial.println(WiFi.localIP());          //
  WiFi.setHostname(strClientID.c_str());   //
  delay(1000);                             //
}

void initialize_wifi_manager() {                                            // start wifi portal for autoconfig
  Serial.print("");                                                         // SPIFFS.format();  // clean FS, for testing
  Serial.println("mounting FS...");                                         // read configuration from FS json
  if (SPIFFS.begin()) {                                                     //
    Serial.println("mounted file system");                                  //
    if (SPIFFS.exists("/config.json")) {                                    //
      Serial.println("reading config file");                                // file exists, reading and loading
      File configFile = SPIFFS.open("/config.json", "r");                   //
      if (configFile) {                                                     //
        Serial.println("opened config file");                               //
        size_t size = configFile.size();                                    //
        std::unique_ptr<char[]> buf(new char[size]);                        // Allocate a buffer to store contents of the file.
        configFile.readBytes(buf.get(), size);                              //
        DynamicJsonDocument objJSONconfig(1024);                            //
        auto deserializeError = deserializeJson(objJSONconfig, buf.get());  //
        serializeJson(objJSONconfig, Serial);                               //
        if (!deserializeError) {                                            //
          Serial.println("\nparsed json");                                  //
          strcpy(mqtt_server, objJSONconfig["mqtt_server"]);                //
          strcpy(mqtt_port, objJSONconfig["mqtt_port"]);                    //
          strcpy(mqtt_user, objJSONconfig["mqtt_user"]);                    //
          strcpy(mqtt_password, objJSONconfig["mqtt_password"]);            //
        } else {                                                            //
          Serial.println("failed to load json config");                     //
        }                                                                   //
      }                                                                     //
    }                                                                       //
  } else {                                                                  //
    Serial.println("failed to mount FS");                                   //
  }                                                                         // end read
  Serial.println(mqtt_user);                                                //
  Serial.println(mqtt_password);                                            //
  Serial.println(mqtt_server);                                              //
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 5);
  WiFiManagerParameter custom_mqtt_user("mqtt user", "MQTT username", mqtt_user, 34);
  WiFiManagerParameter custom_mqtt_password("mqtt password", "MQTT password", mqtt_password, 34);
  WiFiManager wifiManager;                                       // Local intialization. Once its business is done, there is no need to keep it around
  wifiManager.setSaveConfigCallback(saveConfigCallback);         // set config save notify callback
  wifiManager.addParameter(&custom_mqtt_server);                 // add all your parameters here
  wifiManager.addParameter(&custom_mqtt_port);                   //
  wifiManager.addParameter(&custom_mqtt_user);                   //
  wifiManager.addParameter(&custom_mqtt_password);               //
  Serial.print("");                                              // wifiManager.resetSettings();   //reset settings - for testing
  wifiManager.setMinimumSignalQuality();                         // set minimum quality of signal so it ignores AP's under that quality defaults to 8%
  if (!wifiManager.autoConnect()) {                              // fetches ssid and pass and tries to connect
    Serial.println("failed to connect and hit timeout");         // if it does not connect it starts an access point with the specified name
    delay(3000);                                                 // and goes into a blocking loop awaiting configuration
    ESP.restart();                                               // reset and try again, or maybe put it to deep sleep
    delay(5000);                                                 //
  }                                                              //
  Serial.println("connected!");                                  // if you get here you have connected to the WiFi
  strcpy(mqtt_server, custom_mqtt_server.getValue());            // read updated parameters
  strcpy(mqtt_port, custom_mqtt_port.getValue());                //
  strcpy(mqtt_user, custom_mqtt_user.getValue());                //
  strcpy(mqtt_password, custom_mqtt_password.getValue());        //
  if (boolSaveConfig) {                                          // save the custom parameters to FS
    Serial.println("saving config");                             //
    DynamicJsonDocument objJSONconfig(1024);                     //
    objJSONconfig["mqtt_server"] = mqtt_server;                  //
    objJSONconfig["mqtt_port"] = mqtt_port;                      //
    objJSONconfig["mqtt_user"] = mqtt_user;                      //
    objJSONconfig["mqtt_password"] = mqtt_password;              //
    objJSONconfig["ip"] = WiFi.localIP().toString();             //
    objJSONconfig["gateway"] = WiFi.gatewayIP().toString();      //
    objJSONconfig["subnet"] = WiFi.subnetMask().toString();      //
    File configFile = SPIFFS.open("/config.json", "w");          //
    if (!configFile) {                                           //
      Serial.println("failed to open config file for writing");  //
    }                                                            //
    serializeJson(objJSONconfig, Serial);                        //
    serializeJson(objJSONconfig, configFile);                    //
    configFile.close();                                          //end save
  }                                                              //
  Serial.println("local ip");                                    //
  Serial.println(WiFi.localIP());                                //
  Serial.println(WiFi.gatewayIP());                              //
  Serial.println(WiFi.subnetMask());                             //
}

void saveConfigCallback() {              // callback notifying us of the need to save config
  Serial.println("Should save config");  //
  boolSaveConfig = true;                 //
}