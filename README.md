#OmniNode
##An adaptable expandable and flexible IoT sensor system leveraging NodeRed and ESP8266 nodes

##SUB TOPICS
[User Interface](/UserInterface.md)
[MQTT Topic Structure](/MQTTtopicStucture.md)
[Node hardware variations](/NodeHardware.md)

##Key Features:
	Define statements enable or disable functions (ie packet forwarding, AS5600, BMP280, etc)   DONE
		  - allows handling of single Node software suite                                              -feature-
		  - define statements will exclude unneeded modules at compile time                            WORK IN PROGRESS
		  - software version will be reported via MQTT                                                 PARTIALLY DONE
	Connect to last known WiFi SSID saved in EEPROM                                             DONE / UNTESTED
		  - If failed, then:                                                                            DONE / UNTESTED
		  - initiate WiFi AP mode                                                                      DONE
		  - serve up WiFi config page                                                                  DONE
	At any point, if user presses button...                                                     DONE / UNTESTED
	  - and releases after >3 && <10 seconds, launch config portal                                  DONE
	  - and releases after >10 seconds, wipe EEPROM and launch config portal                        DONE
  Use RGB LED to indicate status                                                              
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

##REFERENCES

[JSON Editor](https://jsoneditoronline.org/#right=local.codope)
