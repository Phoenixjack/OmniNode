#OmniNode
##An adaptable expandable and flexible IoT sensor system leveraging NodeRed and ESP8266 nodes
###TODO:
- [ ] Create Overview md
- [ ] Create MQTT topic stucture md
- [ ] Create Node hardware variations md
- [ ] Create user interface md
- [ ] 



OMNI 

MQTT Quality of Service
<http://www.steves-internet-guide.com/mqtt-which-qos-to-use/>
MQTT is an application level protocol and an application message can result in multiple TCP/IP packets
QOS0 Results in 1 message at the application level message. 
QOS1 results in 2 application level messages. Using QOS of 1 could queue the data when in a blackspot and send it later when connected. 
QOS2 results in 4 application level messages. Same as QOS1, but with more TCP/IP traffic
	QOS Publish	QOS Subscribe	OVERALL QOS
	0		0 or 1 or 2	0
	1		0		0
	1		1 or 2		1
	2		0		0
	2		1		1
	2		2		2


MQTT Topic structure:		<https://github.com/node-red/node-red/wiki/Node-msg-Conventions>

        ##### ALL MESSAGES SENT BY NODES WILL BE A JSON OBJECT PACKAGED WITH THEIR UNIT ID AS THE KEY #####
   ##### PAYLOADS MAY BE A NUMBER OR TEXT FOR SIMPLE TOPICS *OR* A JSON OBJECT WITH SEPARATE KEY/VALUE PAIRS #####
##### ALL MESSAGES SENT BY THE SERVER WILL BE STRING FORMATTED (EVEN NUMBERS) TO SIMPLIFY PROCESSING AT THE NODE #####
base/				Base topic for nodes to report in to OR for the server to publish general info (not implemented yet)
    /announce	QoS2/RETAIN	Receive remote unit IDs. Uses second half of MAC ID. Used to track unique nodes in setup
    /...TBD...

node/					Base topic for anything unique to that node
    /errors	 	QoS2/RETAIN	Receive error reports from nodes. Not fully implemented; currently only for message deserialization failures.
    /config/<MAC_ID>	QoS2/RETAIN	Topic for server to broadcast commands unique to that node. Commands will follow the reportcommands format provided by the node.
    /lastcheckin	QoS1/RETAIN	UTC Timestamp of last time the node checked in. Performed when MQTT connection is initially established OR when told to check in by the server.
    /status		QoS2/RETAIN	Text with status (ONLINE/REBOOTING/OFFLINE) of the unit. ONLINE and REBOOTING will be provided by the node. 
					OFFLINE will be updated by a server side watchdog routine if it fails to check in OR a LASTWILL message
    /ntpsync		QoS2/RETAIN	For tracking last NTP sync time from attached nodes; QOS1
    /config		QoS2/RETAIN	JSON object with MAC_ID, assigned function, software version, IP, current report interval... Example:
					{"ID":"565F0F","function":"mpu6050","softwareversion":"OMNIv1.0.0","IP":"192.168.0.10","CurrReportInterval":"1500","commandtopic":"node/config/565F0F"}
    /diagnostics	QoS2/RETAIN	JSON object with current diagnostic info. FUTURE ADDITION: RSSI min/max, ESP_VCC, ESP_REBOOT_REASON, QoS mismatches, WiFi/MQTT disconnects counter, etc.
					{"ID":"565F0F","WiFiStr_Curr":"-47","WiFiStr_Avg":"-45","MsgSentToDate":"125","UpTime":"58234"}
    /reportformat	QoS2/RETAIN	JSON formatted list of reports with data format [temp,humidity,azimuth,etc / integer,hex,string,etc / min,max values] (retained) QOS1
					{
					  "as5600fieldstrength": {"format": "number","resolution": "1","unit": "none"},
					  "as5600azimuth": {"format": "number","resolution": "0.1","unit": "degrees"},
					  "as5600rotation": {"format": "number","resolution": "0.1","unit": "RPM"},
					  "as5600hystersis": {"format": "boolean","value": "false"},
					  "ina3221voltage": {"format": "number","resolution": ".01","unit": "volts","channel":"1"},
					  "bmp280pressure": {"format": "number","resolution": "0.1","unit": "hPa"},
					  "bmp280altitude": {"format": "number","resolution": "0.01","unit": "m"},
					  "ina3221current": {"format": "number","resolution": ".01","unit": "milliamps","channel":"1"},
					  "adxl345gsensitivity": {"format": "string","value": "4G"}
					}
    /availablecommands	QoS2/RETAIN	JSON objects with formatted list of commands and available options.
					{
					  "reboot": {"format": "boolean"},
					  "reportin": {"format": "boolean"},
					  "ntpresync": {"format": "boolean"},
					  "reportinterval": {"format":"number","default":"1500","unit": "milliseconds","minimum":"500","maximum":"259200000"},
					  "reportcommands": {"format": "boolean"},
					  "reportconfig": {"format": "boolean"},
					  "reportdiagnostics": {"format": "boolean"},
					  "as5600hystersis": {"format": "boolean","default": "false"},
					  "adxl345gsensitivity": {"format": "csvlist","default": "4G","options":["2G","4G","8G","16G"]},
					  "bmp280sealevelreference": {"format":"number","default":"1013.25","unit": "hPa"}
					}

    /physical		QoS2/RETAIN	examples:	////// do we want to publish this to MQTT broker for external tracking or just keep it within Node Red? //////
					type of motion (stationary, revolve, free, etc)
					reference point (Earth, DUT, or undefined)
					offset from reference (X/Y/Z dist for DUT or lat/long/altitude for Earth)
					plain language assigned function (4300 #1, 4300 #2, spinny platform azimuth, etc)


JSON reference: <https://jsoneditoronline.org/#right=local.codope>

Key Features:
> Define statements enable or disable functions (ie packet forwarding, AS5600, BMP280, etc)   DONE
 - allows handling of single Node software suite                                              -feature-
 - define statements will exclude unneeded modules at compile time                            WORK IN PROGRESS
 - software version will be reported via MQTT                                                 PARTIALLY DONE
> Connect to last known WiFi SSID saved in EEPROM                                             DONE / UNTESTED
> If failed, then:                                                                            DONE / UNTESTED
 - initiate WiFi AP mode                                                                      DONE
 - serve up WiFi config page                                                                  DONE
> At any point, if user presses button...                                                     DONE / UNTESTED
  and releases after >3 && <10 seconds, launch config portal                                  DONE
  and releases after >10 seconds, wipe EEPROM and launch config portal                        DONE
> Use RGB LED to indicate status                                                              
 - WHITE = WiFi                                                                               DONE
 - CYAN = WiFiMan portal active                                                               DONE
 - MAGENTA (purple) = MQTT                                                                    WORK IN PROGRESS
 - GREEN = Normal Operation
 - BLUE = Outgoing data
 - YELLOW = Received valid command 
 - RED = Fault
 - Blink settings??? Long / slow
> NodeRed server handshake protocol:
 - This unit announces on base/announce topic.
 - NodeRed server checks database of current nodes. 
 - If new, then: initiates query of report formats and accepted commands
 - If not, then: unit has rebooted during test and we'll conduct an expedited addition to the system, making sure to track unit dropout incidents
 - Synchronize NTP:
   ) NodeRed server doubles as NTP server.
   ) This unit reports successful NTP sync to base/ntpsync topic
 - Primary loop... 
   * Leverage ArduinoJSON library to assemble packets
   * Adhere to msg conventions for standardized data handling
   * Report data at commanded interval OR on receiving data
   * Report all data with Unix epoch timestamp, ?local packet counter?, WiFi RSSI
   * Listen for reconfig commands and report completion/implementation
     - to change report interval
     - to change sensor parameters -> ie BMP280 SLP, AS5600 hysteresis, ADXL345 sensitivity and max range, etc
     - to reboot

> NodeRed server overview:
 - Receives reports
 - Tracks status of reporting units
 - Provides raw and derived analytics via Dashboard UI
 - Dashboard UI provides for 
   * Session start/stop:
     - Check for missing config data or notes before allowing data capture
     - Format files as {test_colloquial_name}_{rawdata/correlated/diagnostics}_{datetimestamp} [check for existing files OR create new folder for each?]
     - Record {test_colloquial_name}_configdata_{datetimestamp} file and close before proceeding. [to limit open file handlers]
     - Clearly display "RECORDING IN PROGRESS" and runtime counter
     - Request confirmation before stopping data capture
   * Logging level 
     - Raw data (timestamp w/ packet contents; no correlation)
     - Correlated data that is indexed to a given reference (time interval or triggered by _____) (with user selection for columns)
     - Errors and discrepancies noted by Node Red analysis (remote node lost packets counter, num reboots, num WiFi or MQTT disconnects, WiFi RSSI min/max/avg, etc)
   * Split logs
     - Divide logs into raw reported data, analytics (derived values), and diagnostics files.
     - Separate file for this test session: 
       * User defined colloquial name. Ex. "EngineXXX Test3" [error check to make sure the name is filename friendly]
       * Description. Ex. "Replaced reference antenna", "Repeating test due to suspected node malfunction", "Added 20dB attenuation to base receiver", "Repeating test with new antenna altitude: 25ft", etc
       * Sensors configuration: remote node reference, motion type, and offsets
       * Test setup reference notes. Ex. "Cactus antenna baseline: -115dB noise"
     - User defined log size/runtime: 
       * Start new file every X minutes/hours/days
       * Start new file every XXXX entries
   * Visualization:
     - Raw or filtered data display
     - For given metric: current value, min, max, average
     - Sensor overview: List of nodes and color coded for NORMAL FUNCTION/OFFLINE/DEGRADED 
     - Node status: for each node: 
       * Uptime
       * Time since last message
       * Missed packets counter
       * Num Reboots this session
       * Curr/Avg WiFi
       * User alert for reboot detected, WiFi disconnect, missed packets > XX consecutive, etc
       * <https://stevesnoderedguide.com/message-object-cloning>
       * <https://stevesnoderedguide.com/using-dashboard-template-node>
       * <https://www.w3schools.com/angular/>
     - Quick and dirty RF propagation radar chart w/ options for "group by X degrees" and "display current or average data" [optimized to avoid CPU loading] 
       * <https://stevesnoderedguide.com/using-the-node-red-chart-node>


   * User experience:
     - Dedicated settings page: w/ lockout (see below)
       * Manual reconfiguration/control of remote nodes: reboot, force NTP sync, change parameter (BMP280 SLP, AS5600 direction, MPU6050 G-scale, etc)
       * Error check any user entered values against {cmdavailable} parameters. Reject invalid inputs.
     - User reconfigurable tabs: collapsible OR disappearing? <https://stevesnoderedguide.com/using-control-ui-node-flow-example>
   * Protect user from accidents
     - Lockout of specific functions after starting data capture (spinny platform remote start/stop commands, remote node reboots, remote node change configurations, etc)
     - Full lockout OR Partial lockout with warnings?
