#User Interface

##Layout
+ [Collapsible windows](https://stevesnoderedguide.com/using-control-ui-node-flow-example) (except persistent alerts)
  
##Persistent Test Status Window
+ Always on top
+ Indicator: Test Not Configured / Test Ready / Test In Progress
+ Runtime indicator
+ Number of correlated datapoints
+ Autosave indicator

##Workspace Configuration
+ List and edit saved reference points
+ List and edit node functions
+ Edit global software parameters
  - Enable full lockout of server & node parameters
    + Change sensor parameters: Accelerometer sensitivity, Altimeter Sea Level Pressure Reference, etc
    + Start/Stop Relay toggling
    + Node Control remote reboot, launch config portal, forced checkin, etc
  - Manual Stop Test: confirm or immediate stop
  - Force set NTP server date/time
  - Reset node statistics / diagnostics

##Node Control
+ List detected nodes
+ Select node:
  - List current statistics / diagnosics
    + MAC ID
    + Software Version
    + Compiled Date
    + Loaded sensor(s)
    + Last NTP sync
    + Current Report Interval
    + Missed Report Counter
    + Average/Min/Max WiFi RSSI
    + Transmitted Packets Counter
    + Dropped Packets Counter
  - List available commands
    + Reboot
    + NTP Force Resync
    + Change Report Interval
    + Force Checkin
    + Change sensor parameter (BMP280 SLP, AS5600 direction, MPU6050 G-scale, etc)
    + Check user input against availablecommands validation parameters before sending

##Node Status
Abbreviated status of all detected nodes
+ Red/Yellow/Green indicator for OFFLINE, missed report in window/packet counter mismatch/etc, ONLINE and reporting data on schedule
+ Brief alert: "Dropped Packets Count: 21" "No data received" "Device rebooted" "Below average WiFi strength"

##User Workflow
+ Choose test setup:
  - Create new
  - Select saved test setup
  - Clone and edit saved setup
+ Edit test setup
  - List required reference points EXAMPLE: Test Site Origin, Device Carousel, Device Under Test, Equip Trailer, Equip Trailer Antenna
  - Configure reference points: Device Carousel at Test Site Origin, DUT 1.5 meters above Device Carousel, Equip Trailer 21 meters away from Origin at 300°, Antenna 5 meters above Trailer
  - List required functions EXAMPLE: AS5600, RF Receive Monitor, Altitude Sensor, Start/Stop Relay
  - Configure nodes EXAMPLE: AS5600 and Start/Stop Relay at Device Carousel, RF Receive Monitor and Altitude Sensor on Trailer Antenna
+ Detect and Assign nodes
  - List detected nodes
  - Map each node to an assigned function in the test
  - Configure data recording:
    + RUNTIME: predetermined runtime, predetermined number of datapoints, manual stop
    + PARSING: collate data on specified data point (ie each RF Receive Monitor report), timed (most recent data from all nodes every X seconds), or raw (uncollated) feed with timestamps
  - _OPTIONAL_: Bird's Eye Visualizer with Ref Point and Node placement
+ Start test
  - _FOLLOWUP_: Periodic or manual autosave in case of server power outage
+ Live Data Visualizer
  - RF Propagation Chart:
    + Round to X degrees
    + Sector data: average or most recent value
  - RF Running Line Chart: Include Min/Max/Avg lines?
+ Stop test OR fixed runtime OR fixed datapoint limit
  - Data Export Options:
    + _OPTIONAL_: Export diagnostics data?
    + Export test setup config data: ref points, node function/assignments, user notes
    + Export data: Single CSV OR parse by time/num datapoints?
