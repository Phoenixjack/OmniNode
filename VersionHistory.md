# MAJOR TODOs:
## User/Server Interface
  [ ] need to add feature to node to periodically reinvestigate wifi connection. currently, it can't tell if wifi was dropped or just MQTT
  [ ] save broker, report interval, and any other config changes to local file and retrieve on boot *before* WiFiManager
  [ ] update use of ArduionJSON to reflect updates. StaticJSON and DynamicJSON are deprecated.
  [ ] provide server with method for validating data
  [ ] provide server with method for providing valid commands
  [ ] execute valid commands
  [ ] recursively compile report and command formats into JSON package
  [ ] structure or array of variable types, min/max values, units, etc
  [ ] button - separate vs onboard reset
    o can we leverage the onboard reset button to identify rapid reboots in a series to spur some action?
## Variable handling:
  [ ] convert some values (led dim/full brightness level, etc) to variables
  [ ] enum array for commands, parameters, etc
  [ ] implement pointers for subroutines with multiple returns; REDUCE GLOBAL VARIABLES! bad programmer! BAD! no cookie.
  [ ] better handling of variable scopes!!! less global/more local! start using 'static' for persistent local variables!
## Functions and Subroutines: 
  [ ] make sure their names still make sense as the code has evolved
  [ ] implement classes to isolate sections of code, functions, and variables
  [ ] mirror the naming convention of what they return (boolean, int, string, etc)
  [ ] convert them to return error codes or booleans based on what happened
## Memory / Space management:
  [ ] look at how much space the compiled code takes
  [ ] look at how much runtime memory is used
  [ ] evaluate if we need to minimize usage of JSON library and strings
  [ ] flexible on the fly?
    o does the Wemos 4M have space available to activate all sensor types and be reconfigured without reflashing?
    o if so, can we include all libraries in compile and remove all the #if (defFuncAS5600) statements?
## Overall:
  [ ] Use __VA_ARGS__ for flexible defines


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

## V0_1_0:
### TOTAL REBUILD using headers and classes
### *NOT* backwards compatible with prior NodeRed flows
[X] debug flag overhaul
    + separated debug defines into standalone header
    + leveraged ifndef to set defaults that can be overriden by the calling code
    + used conditional defines to replace empty print statements with nulls/no operations
    + created separate defines using F() to keep static strings in storage and out of volatile memory
[ ] separation into classes using wrappers for simplified usage and variable scoping
[ ] redesigned message and topic structure
    + NodeRed allows use of full msg properties (msg.sender, msg.description, etc)
    + Current MQTT and JSON library do not
    + topic structure redesigned to include test names and allow concurrent use of multiple OmniNode system TODO: allow method for 
[ ] overhauled usage of ArduinoJSON to avoid deprecated calls

## V0_0_6:
[X] remove infinite loop for WiFi and MQTT. Added Serial Monitor input feature
[X] debug levels:
    + initial setup
    + file system
    + WiFi notices
    + MQTT notices (terse & verbose)
    + Sensor readings
    + JSON handling
    + time functions

## V0_0_5:
[X] further testing of compile time switches. identified bugs in compass configuration/data reporting
[X] better handling of variable scopes
[X] revamp handling of MQTT commands and report formats
[x] reviewed/fixed various variable & object names that didn't meet naming convention

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

# Documentation notes
## [Semantic Versioning](https://semver.org/)
Given a version number MAJOR.MINOR.PATCH, increment the:
+ MAJOR version when you make incompatible API changes
+ MINOR version when you add functionality in a backward compatible manner
+ PATCH version when you make backward compatible bug fixes