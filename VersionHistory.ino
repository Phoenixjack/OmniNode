/*

// MAJOR TODOs:
// WiFiManager! Stop saving credentials and config in source code
// DEBUG level disabling of Serial.print statements?
// compile time flags to enable/disable code segments -> DefineNodeFunction
// better handling of variable scopes!!! start using 'static' for local variables!
// standardize variable naming... you know, like you used to do. ulongLastUpdate charMsgBuffer intReportInterval strClientID
// integrate RGB library and visual indicator


V0_0_2:
Migrated from PubSubClient to ArduinoMQTTClient library to enable QoS
Created emulation mode to bypass WiFi and MQTT so we can continue coding without having a server available

V0_0_1:
Connects to statically assigned WiFi SSID and MQTT broker
Announces name to MQTT announce topic
Listens and responds to commands:
reboot
reportin
ntpresync
reportinterval
reportcommands <INCOMPLETE>
reportconfig
reportdiagnostics
*/