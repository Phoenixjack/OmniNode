/*
***TODO: Split off TODOs, FOLLOWUPS, WARNINGS, etc into a ROADMAP


MAJOR TODOs:
- Memory / Space management:
  - look at how much space the compiled code takes
  - look at how much runtime memory is used
  - evaluate if we need to minimize usage of JSON library and strings
- Compile-time stuff:
  - DEBUG level disabling of Serial.print statements? -> even if only for processing during production
  - modify defNodeFunction to point to a global array of functions
    - with defNodeFunction containing bit flags
    - allow more than one function per node
    - example: defNodeFunction = b0000 0000 0000 1010 with bit 2 = compass & bit 4 = gps
- Variable handling:
  - enum array for commands, parameters, etc
  - implement pointers for subroutines with multiple returns; REDUCE GLOBAL VARIABLES! bad programmer! BAD! no cookie.
  - better handling of variable scopes!!! less global/more local! start using 'static' for persistent local variables!
  - this might be a little excessively OCD, but hear me out... vlul_ for variable, local, unsigned long; cgca_ for constant, global, char array; vssi_ for variable, static, signed integer...
- User Visual Indicators: integrate RGB library
- Functions and Subroutines: 
  - make sure their names still make sense as the code has evolved
  - possibly mirror the naming convention of what they return (boolean, int, string, etc)
  - convert them to return error codes or booleans based on what happened

V0_0_4:
- Initial sensor integration
  - INA3221 and HMC5883 for now
  - Implemented basic compile-time definitions to reduce unneeded code

V0_0_3:
- Split functions off from a single INO file so we can differentiate what commits mean
- Moving to LittleFS due to FS library depreciation.
- WiFiManager integration implemented.
- Added commands: reportdiagnostics, resetdiagnostics, wifiportal
- reportcommands function still incomplete.

V0_0_2:
Migrated from PubSubClient to ArduinoMQTTClient library to enable QoS
Created emulation mode to bypass WiFi and MQTT so we can continue coding without having a server available
Standardized naming conventions 
 - Moved away from any variable called TEMP. Naming is now more defined to the task. NOT COMPLETE!
 - compiler definitions are prefixed with def...
 - instances like WiFi, UDP, etc have inst...
 - objects like JSON have obj...
 - Strings have str...
 - char arrays have charr...
 - integers have int...
 - unsigned longs have ul_...
 - booleans have bool...

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