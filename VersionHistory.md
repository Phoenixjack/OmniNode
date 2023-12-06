# MAJOR TODOs:
## Memory / Space management:
  [ ] look at how much space the compiled code takes
  [ ] look at how much runtime memory is used
  [ ] evaluate if we need to minimize usage of JSON library and strings
## Compile-time stuff:
  [ ] DEBUG level(s) to disable all or some Serial.print statements? -> even if only for reducing processor time for production code
  [ ] modify defNodeFunction to point to a global array of functions
    [ ] with defNodeFunction containing bit flags
    [ ] allow more than one function per node
    [ ] example: defNodeFunction = 12 = b0000 0000 0000 1010 with bit 2 = compass & bit 4 = gps
## Variable handling:
  [ ] enum array for commands, parameters, etc
  [ ] implement pointers for subroutines with multiple returns; REDUCE GLOBAL VARIABLES! bad programmer! BAD! no cookie.
  [ ] better handling of variable scopes!!! less global/more local! start using 'static' for persistent local variables!
  [ ] this might be a little excessively OCD, but hear me out... 
      - Position 1: 'g' global, 'l' local
      - Position 2: 'v' variable, 's' for static variable, 'c' constant
      - Position 3: 'i' integer, 'd' double, 'l' long, 'c' char, 's' string, 'b' boolean, etc
      - Position 4: 's' signed for numbers, 'u' unsigned for numbers, 'a' for char arrays, 's' for single char
      - EXAMPLE: int gvis_MQTTPort = 1883, static int lsiu_XmitPacketCnt++, gcs_ClientID = String(ESP.getChipId(), HEX)
## User Visual Indicators: integrate RGB library
## Functions and Subroutines: 
  [ ] make sure their names still make sense as the code has evolved
  [ ] possibly mirror the naming convention of what they return (boolean, int, string, etc)
  [ ] convert them to return error codes or booleans based on what happened

## V0_0_4:
[x] Implemented C++ macro to save compile timestamp
[x] Implemented basic compile-time definitions to reduce unneeded code
[x] defNodeFunction that aggregates multiple functions
[x] Initial implementation of RGB library for user notification **USES GPIO 12,13,14** __Avoid these pins for future hardware__
[ ] Cleanup / reduce Serial print calls
[ ] Debug level defines for Serial prints
[ ] Implement Serial printf calls indicating what filename and function generated the action
[ ] Initial sensor integration **NOT full integration; sensor initialization and basic data pulls only**
  [x] Packet Forwarding via second serial port
  [x] INA3221 
  [x] HMC5883
  [x] MPU6050
  [ ] NEO6/7
  [x] AS5600

## V0_0_3:
[x] Split functions off from a single INO file so we can differentiate what commits mean
[x] Moving to LittleFS due to FS library depreciation.
[x] WiFiManager integration implemented.
[x] Added commands: reportdiagnostics, resetdiagnostics, wifiportal
[ ] reportcommands function still incomplete.

## V0_0_2:
[x] Migrated from PubSubClient to ArduinoMQTTClient library to enable QoS
[x] Created emulation mode to bypass WiFi and MQTT so we can continue coding without having a server available
[x] Standardized naming conventions 
 [x] Moved away from any variable called TEMP. Naming is now more defined to the task. NOT COMPLETE!
 [x] compiler definitions are prefixed with def...
 [x] instances like WiFi, UDP, etc have inst...
 [x] objects like JSON have obj...
 [x] Strings have str...
 [x] char arrays have charr...
 [x] integers have int...
 [x] unsigned longs have ul_...
 [x] booleans have bool...

## V0_0_1:
[x] Connects to statically assigned WiFi SSID and MQTT broker
[x] Announces name to MQTT announce topic
[x] Listens and responds to commands:
  [x] reboot
  [x] reportin
  [x] ntpresync
  [x] reportinterval
  [x] reportcommands <INCOMPLETE>
  [x] reportconfig
  [x] reportdiagnostics
