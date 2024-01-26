#MQTT Topic Stucture

[NodeRed Topic Best Practices](https://github.com/node-red/node-red/wiki/Node-msg-Conventions)

- msg.title : Node function (for base station: "SERVER" / for nodes, defNodeFunction value)
- msg.description : report or command ("reportin" "ntpresync" "reportcommands", "data")
- msg.sender : MAC ID of the sender
- msg.time : NTP timestamp of sent data
- msg.data 
- msg.payload
  + msg.payload.type : purpose of data
  + msg.payload.id : MAC ID of the sender (duplicate)
  + msg.payload.distance
  + msg.payload.duration
  + msg.payload.starttime
- msg.location 
  + msg.location.name : used for colloquial position when not connected directly to GPS; ie 'palletspinner' 'trailerantenna'
  + msg.location.lat
  + msg.location.lon
  + msg.location.ele
  

| level0   | level1          | FORMAT        | Description |
| -------  | -------         | -------       | -------     |
| 'base/'  | 'announce'      | 'QoS2/RETAIN' | Receive remote unit IDs. Uses second half of MAC ID. Used to track unique nodes in setup |
| 'node/'  | _MAC-ID_        | 'QoS2/RETAIN' | Single point for commands to that specific node. Incoming messages will be routed based on command payload. |
| 'node/'  | 'data'          | 'QoS1/RETAIN' | Single point for publishing data. Incoming traffic will be routed and parsed server side based on MAC ID and report format. |
| 'node/'  | 'errors'        | 'QoS2/RETAIN' | Receive error reports from nodes. Not fully implemented; currently only for message deserialization failures. |
| 'node/'  | 'lastcheckin'   | 'QoS1/RETAIN' | UTC Timestamp of last time the node checked in. Performed when MQTT connection is initially established OR when told to check in by the server. |
| 'node/'  | 'status'        | 'QoS2/RETAIN' | Text with status ('ONLINE'/'REBOOTING'/'OFFLINE') of the unit. 'ONLINE' and 'REBOOTING' will be provided by the node. 'OFFLINE' by LastWill |
| 'node/'  | 'ntpsync'       | 'QoS2/RETAIN' | For tracking last NTP sync time from attached nodes |
| 'node/'  | 'config'        | 'QoS1/RETAIN' | JSON object with MAC_ID, assigned function, software version, IP, current report interval... |
| 'node/'  | 'diagnostics'   | 'QoS1/RETAIN' | JSON object with current diagnostic info. **FUTURE ADDITION:** RSSI min/max, ESP_VCC, ESP_REBOOT_REASON, QoS mismatches, WiFi/MQTT disconnects counter, etc. |
| 'node/'  | 'reportformat'  | 'QoS1/RETAIN' | JSON formatted list of reports with data format [temp,humidity,azimuth,etc / integer,hex,string,etc / min,max values] EXAMPLE: {"as5600fieldstrength": {"format": "number","resolution": "1","unit": "none"},"as5600azimuth": {"format": "number","resolution": "0.1","unit": "degrees"},"as5600rotation": {"format": "number","resolution": "0.1","unit": "RPM"},"as5600hystersis": {"format": "boolean","value": "false"},"ina3221voltage": {"format": "number","resolution": ".01","unit": "volts","channel":"1"},"bmp280pressure": {"format": "number","resolution": "0.1","unit": "hPa"},"bmp280altitude": {"format": "number","resolution": "0.01","unit": "m"},"ina3221current": {"format": "number","resolution": ".01","unit": "milliamps","channel":"1"},"adxl345gsensitivity": {"format": "string","value": "4G"}} |
| 'node/'  | 'availcommands' | 'QoS2/RETAIN' | JSON objects with formatted list of commands and available options. EXAMPLE: {"reboot": {"format": "boolean"}, "reportin": {"format": "boolean"},"ntpresync": {"format": "boolean"}, "reportinterval": {"format":"number","default":"1500","unit": "milliseconds","minimum":"500","maximum":"259200000"}, "reportcommands": {"format": "boolean"}, "reportconfig": {"format": "boolean"}, "reportdiagnostics": {"format": "boolean"}, "as5600hystersis": {"format": "boolean","default": "false"}, "adxl345gsensitivity": {"format": "csvlist","default": "4G","options":["2G","4G","8G","16G"]}, "bmp280sealevelreference": {"format":"number","default":"1013.25","unit": "hPa"} } |
| 'node/'  | 'physical'      | 'QoS2/RETAIN' | _do we want to publish this to MQTT broker for external tracking or just keep it within Node Red?_ EXAMPLES:  + type of motion (stationary, revolve, free, etc)  + reference point (Earth, DUT, or undefined)  + offset from reference (X/Y/Z dist for DUT or lat/long/altitude for Earth)  + plain language assigned function (4300 #1, 4300 #2, spinny platform azimuth, etc) |

+ **FUTURE ADDITION:** aknowledgement of messages if we can't use the underlying software for validation
+ ALL MESSAGES SENT BY THE SERVER WILL BE STRING FORMATTED (EVEN NUMBERS) TO SIMPLIFY PROCESSING AT THE NODE
