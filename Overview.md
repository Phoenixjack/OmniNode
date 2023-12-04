# NodeRed server Key Features:
## Receives reports
## Remotely reconfigures reporting units
## Tracks status of reporting units
## User defined Logging level 
      - Raw data (timestamp w/ packet contents; no correlation)
      - Correlated data that is indexed to a given reference (time interval or triggered by _____) (with user selection for columns)
      - Errors and discrepancies noted by Node Red analysis (remote node lost packets counter, num reboots, num WiFi or MQTT disconnects, WiFi RSSI min/max/avg, etc)
## Provides raw and derived analytics via Dashboard UI
      - Clearly display "RECORDING IN PROGRESS" and runtime counter
      - Request confirmation before stopping data capture
      - Raw or filtered data display
      - For given metric: current value, min, max, average
      - Sensor overview: List of nodes and color coded for NORMAL FUNCTION/OFFLINE/DEGRADED 
      - Node status
## Split logs
      - Divide logs into raw reported data, analytics (derived values), and diagnostics files.
      - Separate file for this test session: 
         + User defined colloquial name. Ex. "EngineXXX Test3" [error check to make sure the name is filename friendly]
         + Description. Ex. "Replaced reference antenna", "Repeating test due to suspected node malfunction", "Added 20dB attenuation to base receiver", "Repeating test with new antenna altitude: 25ft", etc
         + Sensors configuration: remote node reference, motion type, and offsets
         + Test setup reference notes. Ex. "Cactus antenna baseline: -115dB noise"
      - User defined log size/runtime: 
         + Start new file every X minutes/hours/days
         + Start new file every XXXX entries      
