#MQTT Topic Stucture

+ ALL MESSAGES SENT BY NODES WILL BE A JSON OBJECT PACKAGED WITH THEIR UNIT ID AS THE KEY
+ PAYLOADS MAY BE A NUMBER OR TEXT FOR SIMPLE TOPICS **OR** A JSON OBJECT WITH SEPARATE KEY/VALUE PAIRS
+ ALL MESSAGES SENT BY THE SERVER WILL BE STRING FORMATTED (EVEN NUMBERS) TO SIMPLIFY PROCESSING AT THE NODE

##BASE

| level0   | level1      | level2        | FORMAT        | Description |
| -------  | -------     | -------       | -------       | -------     |
| 'base/'  | '___'       | '___'         | 'N/A'         | Base topic for nodes to report in to OR for the server to publish general info           |
| 'base/'  | 'announce'  | 'N/A'         | 'QoS2/RETAIN' | Receive remote unit IDs. Uses second half of MAC ID. Used to track unique nodes in setup |

 
##NODE
| level0   | level1        | level2        | FORMAT        | Description |
| -------  | -------       | -------       | -------       | -------     |
| 'node/'  | _MAC-ID_      | '___'         | 'N/A'         | Base topic for anything unique to that node           |
| 'node/'  | _MAC-ID_      | 'errors'      | 'QoS2/RETAIN' | Receive error reports from nodes. Not fully implemented; currently only for message deserialization failures. |
| 'node/'  | _MAC-ID_      | 'lastcheckin' | 'QoS1/RETAIN' | UTC Timestamp of last time the node checked in. Performed when MQTT connection is initially established OR when told to check in by the server. |
| 'node/'  | _MAC-ID_      | 'status'      | 'QoS2/RETAIN' | Text with status ('ONLINE'/'REBOOTING'/'OFFLINE') of the unit. 'ONLINE' and 'REBOOTING' will be provided by the node. 'OFFLINE' by LastWill |
| 'node/'  | _MAC-ID_      | 'ntpsync'     | 'QoS2/RETAIN' | For tracking last NTP sync time from attached nodes |
| 'node/'  | _MAC-ID_      | 'config'      | 'QoS1/RETAIN' | JSON object with MAC_ID, assigned function, software version, IP, current report interval... Example: {"ID":"565F0F","function":"mpu6050","softwareversion":"OMNIv1.0.0","IP":"192.168.0.10","CurrReportInterval":"1500","commandtopic":"node/config/565F0F"}|
| 'node/'  | _MAC-ID_      | 'diagnostics' | 'QoS1/RETAIN' | JSON object with current diagnostic info. Example: {"ID":"565F0F","WiFiStr_Curr":"-47","WiFiStr_Avg":"-45","MsgSentToDate":"125","UpTime":"58234"} **FUTURE ADDITION:** RSSI min/max, ESP_VCC, ESP_REBOOT_REASON, QoS mismatches, WiFi/MQTT disconnects counter, etc. |
| 'node/'  | _MAC-ID_      | 'reportformat'| 'QoS1/RETAIN' | JSON formatted list of reports with data format [temp,humidity,azimuth,etc / integer,hex,string,etc / min,max values] Example: 
                                                             '''
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
                                                             ''' |
| 'node/'  | _MAC-ID_      | 'availcommands' | 'QoS2/RETAIN' | JSON objects with formatted list of commands and available options.
                                                             '''
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
                                                              '''    |
| 'node/'  | _MAC-ID_      | 'physical'      | 'QoS2/RETAIN' | _do we want to publish this to MQTT broker for external tracking or just keep it within Node Red?_ Examples:
                                                               '''
                                                               type of motion (stationary, revolve, free, etc)
                                                               reference point (Earth, DUT, or undefined)
                                                               offset from reference (X/Y/Z dist for DUT or lat/long/altitude for Earth)
                                                               plain language assigned function (4300 #1, 4300 #2, spinny platform azimuth, etc)
                                                               ''' |


##References

[Reference]([https://pages.github.com/](https://github.com/node-red/node-red/wiki/Node-msg-Conventions))
