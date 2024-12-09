# OmniNode
## An adaptable expandable and flexible IoT sensor system leveraging NodeRed and ESP8266 nodes

## SUB TOPICS
[User Interface](/UserInterface.md)
[MQTT Topic Structure](/MQTTtopicStucture.md)
[Node hardware variations](/NodeHardware.md)

## Key Features:
[x] Define statements enable or disable functions (ie packet forwarding, AS5600, BMP280, etc)   
	[x] allows handling of single Node software suite                                            
	[x] define statements will exclude unneeded modules at compile time                            
	[x] software version will be reported via MQTT                                                 
[x] Connect to last known WiFi SSID saved in EEPROM
	[x] If failed, then:
		[x] initiate WiFi AP mode
		[x] serve up WiFi config page
[ ] At any point, if user presses button...
	[ ] and releases after >3 && <10 seconds, launch config portal
	[ ] and releases after >10 seconds, wipe EEPROM and launch config portal
        [ ] Leverage interrupt!
[ ] Use RGB LED to indicate status
	- WHITE = WiFi
	- CYAN = WiFiMan portal active
	- MAGENTA (purple) = MQTT
	- GREEN = Normal Operation
	- BLUE = Outgoing data
	- YELLOW = Received valid command
	- RED = Fault
	- Blink settings??? Long / slow; Leverage Ticker library for "multithreading"
[ ] NodeRed server handshake protocol:
	[x] This unit announces on base/announce topic.
	[ ] NodeRed server checks database of current nodes.
		[ ] If new, then: initiates query of report formats and accepted commands
[ ] If not, then: unit has rebooted during test and we'll conduct an expedited addition to the system, making sure to track unit dropout incidents
	[x] Synchronize NTP:
		[x] NodeRed server doubles as NTP server.
		[x] This unit reports successful NTP sync to base
	[x] Primary loop... 
		[x] Leverage ArduinoJSON library to assemble packets
		[x] Adhere to msg conventions for standardized data handling
		[x] Report data at commanded interval OR on receiving data
		[x] Report all data with Unix epoch timestamp, ?local packet counter?, WiFi RSSI
		[x] Listen for reconfig commands and report completion/implementation
			[x] to change report interval
			[x] to change sensor parameters -> ie BMP280 SLP, AS5600 hysteresis, ADXL345 sensitivity and max range, etc
			[x] to reboot

## REFERENCES
[JSON Editor](https://jsoneditoronline.org/#right=local.codope)
[Markdown How To](https://docs.github.com/en/get-started/writing-on-github/getting-started-with-writing-and-formatting-on-github/quickstart-for-writing-on-github)
