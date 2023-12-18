# MAJOR TODOs:
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
## Functions and Subroutines: 
  [ ] make sure their names still make sense as the code has evolved
  [ ] mirror the naming convention of what they return (boolean, int, string, etc)
  [ ] convert them to return error codes or booleans based on what happened
## Memory / Space management:
  [ ] look at how much space the compiled code takes
  [ ] look at how much runtime memory is used
  [ ] evaluate if we need to minimize usage of JSON library and strings

. Variables and constants in RAM (global, static), used 34540 / 80192 bytes (43%)
║   SEGMENT  BYTES    DESCRIPTION
╠══ DATA     1612     initialized variables
╠══ RODATA   5384     constants       
╚══ BSS      27544    zeroed variables
. Instruction RAM (IRAM_ATTR, ICACHE_RAM_ATTR), used 62039 / 65536 bytes (94%)
║   SEGMENT  BYTES    DESCRIPTION
╠══ ICACHE   32768    reserved space for flash instruction cache
╚══ IRAM     29271    code in IRAM    
. Code in flash (default, ICACHE_FLASH_ATTR), used 377024 / 1048576 bytes (35%)
║   SEGMENT  BYTES    DESCRIPTION
╚══ IROM     377024   code in flash   

## V0_0_4:
_BEWARE_: Switching defNodeFunction flags may cause unexpected errors. This is strictly an Arduino IDE issue where it tries to include & compile code that won't be called.
***BROKE***: cmd_reportconfig:fetch local IP address now fails. Cause unknown at this time.
[x] Implemented C++ macro to save compile timestamp
[x] Implemented basic compile-time definitions to reduce unneeded code
[x] defNodeFunction that aggregates multiple functions
[x] Initial implementation of RGB library for user notification **USES GPIO 12,13,14** __Avoid these pins for future hardware__
[x] Cleanup / reduce Serial print calls
[x] Debug level defines for Serial prints
[x] Implement Serial printf calls indicating what filename and function generated the action
[x] Initial sensor integration **NOT full integration; sensor initialization and basic data pulls only**
  [x] Packet Forwarding via second serial port
  [x] INA3221 
  [x] HMC5883/QMC5883
  [x] MPU6050
  [x] NEO6/7
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
