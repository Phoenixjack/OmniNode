void initialize_wifi_manager() {                                                            // start wifi portal for autoconfig. FOLLOWUP: code was carried over from previous working version. Why are we starting the portal if we have saved config data?
  Serial.print("Starting WiFi Manager system: ");                                           // notify user that we're spooling up wifi... one way or another
  String strSavedMQTTServer, strSavedMQTTPort;                                              // create placeholders for previously saved config
  cmd_ReadConfigFromFS(&strSavedMQTTServer, &strSavedMQTTPort);                             // start by reading saved config, if we can. TODO: split file system functions into separate file and functions
  WiFiManagerParameter custom_mqtt_server("mqttbroker", "mqtt server", strMQTTserver, 40);  // HTML label, placeholder text, variable to store data, default length
  WiFiManagerParameter custom_mqtt_port("port", "1883", strMQTTport, 5);                    //
  wifiManager.setSaveConfigCallback(saveConfigCallback);                                    //
  wifiManager.addParameter(&custom_mqtt_server);                                            // we're going to piggyback WiFiMan to host and save additional parameters
  wifiManager.addParameter(&custom_mqtt_port);                                              //
  wifiManager.setMinimumSignalQuality();                                                    // set minimum quality of signal so it ignores APs under that quality (defaults to 8%)
  if (wifiManager.autoConnect()) {                                                          // fetches ssid and pass and tries to connect
    Serial.println("WIFI CONNECTED!");                                                           // if you get here you have connected to the WiFi
    wifiManager.setHostname(strClientID);                                                   // manually set DHCP hostname as MAC for easy network troubleshooting
    strcpy(strMQTTserver, custom_mqtt_server.getValue());                                   // update global variables with portal variables
    intMQTTPort = atoi(custom_mqtt_port.getValue());                                        // remember to convert the string to an integer
    if (boolSaveConfig) {                                                                   // if save is enabled, call the routine
      cmd_SaveConfigToFS(custom_mqtt_server.getValue(), custom_mqtt_port.getValue());       //
    }                                                                                       //
  } else {                                                                                  //
    Serial.println("failed to connect and hit timeout");                                    // if it does not connect, it starts an access point with the specified name
    delay(3000);                                                                            // and goes into a blocking loop awaiting configuration
    ESP.restart();                                                                          // reset and try again
  }                                                                                         //
}

void cmd_ReadConfigFromFS(String* strRetreivedServer, String* strRetreivedPort) {  // read settings from file and update global variables
  Serial.print("Mounting File System...");                                         //
  if (SPIFFS.begin()) {                                                            // read configuration from FS json
    Serial.println("SUCCESS!");                                                    //
    if (SPIFFS.exists("/config.json")) {                                           //
      Serial.println("Opening config file: ");                                     // file exists, reading and loading
      File configFile = SPIFFS.open("/config.json", "r");                          //
      if (configFile) {                                                            //
        Serial.println("SUCCESS!");                                                //
        size_t size = configFile.size();                                           // TODO: STOP being a script kiddy and actually comprehend this line and the next
        std::unique_ptr<char[]> buf(new char[size]);                               // Allocate a buffer to store contents of the file.
        configFile.readBytes(buf.get(), size);                                     //
        DynamicJsonDocument objJSONconfig(1024);                                   //
        auto boolJSONError = deserializeJson(objJSONconfig, buf.get());            //
        serializeJson(objJSONconfig, Serial);                                      // lazy way of Serial readout of our config. I like lazy.
        Serial.print("Parsing JSON...");                                           //
        if (!boolJSONError) {                                                      //
          Serial.println("SUCCESS!");                                              //
          strcpy(strRetreivedServer, objJSONconfig["mqtt_server"]);                //
          strcpy(strRetreivedPort, objJSONconfig["mqtt_port"]);                    //
        } else {                                                                   //
          Serial.println("ERROR: failed to parse JSON ");                          //
        }                                                                          //
      } else {                                                                     //
        Serial.print("ERROR: config.json does not exist or ");                     //
        Serial.println("could not be opened for reading");                         //
      }                                                                            //
    }                                                                              //
  } else {                                                                         //
    Serial.println("FAILED");                                                      //
  }                                                                                // end read
}

void cmd_SaveConfigToFS(String strServer, String strPort) {    // save the current custom parameters to the file system
  Serial.print("Saving config: ");                             //
  DynamicJsonDocument objJSONconfig(1024);                     //
  objJSONconfig["ip"] = WiFi.localIP().toString();             //
  objJSONconfig["gateway"] = WiFi.gatewayIP().toString();      //
  objJSONconfig["subnet"] = WiFi.subnetMask().toString();      //
  objJSONconfig["mqtt_server"] = strServer;                    //
  objJSONconfig["mqtt_port"] = strPort;                        //
  File configFile = SPIFFS.open("/config.json", "w");          //
  if (!configFile) {                                           //
    Serial.println("failed to open config file for writing");  //
  }                                                            //
  serializeJson(objJSONconfig, configFile);                    //
  configFile.close();                                          //
  serializeJson(objJSONconfig, Serial);                        //
  Serial.println("");                                          //
}

void cmd_ResetWiFiMan() {       // FOLLOWUP: Does this delete our config settings or nuke the entire filesystem?
  wifiManager.resetSettings();  // reset settings - for testing
}

void cmd_WipeFS() {  // Nukes the internal filesystem
  SPIFFS.format();   // clean FS
}

void saveConfigCallback() {              // callback notifying us of the need to save config
  Serial.println("Should save config");  //
  boolSaveConfig = true;                 //
}