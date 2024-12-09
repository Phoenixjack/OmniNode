/*
datastructures.h
Author: Chris McCreary
Updated: 2024-02-12

Header file for typedefs, enums, and structs
Strictly for neatness

LAYOUT:
...TODO...
*/

#define defNumCommands 15

enum DATATYPE {  // a collection of "how to interpret the payload given to us". Used for incoming commands AND applying settings from file
  BOOLEAN,
  INTEGER,
  FLOAT,
  STRING,
  OPTIONLIST
};

enum TARGET {  // a collection of 'objects we can apply settings to differentiate similar commands. EXAMPLE: *restart* sensor VS *reboot* node or 'set datareport interval' VS 'total msg interval'
  SYSTEM,
  SENSOR,  // used as to reference all sensor functions VS a single
  PACKETFWD,
  INA3221,
  NEOGPS,
  XMC5883,
  BM_X80,
  MPU6050,
  ADXL345,
  AS6500
};

enum SYS_PARAM {  // a collection of system parameters that can be modified. FOLLOWUP: This may be redundant as we grow the structure
  TOT_MSG_INTERVAL,
  NTP_FORCERESYNC,
  SET_MQTT,
  SET_PORT,
  REBOOT_NOW,
  LAUNCH_PORTAL
};

struct CMD_LIST {               //
  uint8_t target;               // indexed to enum TARGET
  String title;                 // short text of the command
  String unit;                  // unit of this property
  uint8_t datatype;             // indexed to enum DATATYPE
  int32_t curr_val;             // current value
  int32_t min_val;              // minimum acceptable value
  int32_t max_val;              // maximum acceptable value
  int32_t def_val;              // default value if none specified OR invalid input
  String text;                  // contains string data OR CSV option list
} CMD_LIST[defNumCommands] = {  //
  { TARGET::SYSTEM, "MQTT_BROKER", "ipv4", DATATYPE::STRING, 0, 0, 0, 0, "192.168.1.6" },
  { TARGET::SYSTEM, "MQTT_PORT", "portnum", DATATYPE::INTEGER, 1883, 200, 9000, 1883, "" },
  { TARGET::SYSTEM, "TEST_NAME", "string", DATATYPE::STRING, 0, 0, 0, 0, "default" },
  { TARGET::SYSTEM, "NTP_RESYNC", "boolean", DATATYPE::BOOLEAN, 0, 0, 0, 0, "" },
  { TARGET::SYSTEM, "CMD_REBOOT", "boolean", DATATYPE::BOOLEAN, 0, 0, 0, 0, "" },
  { TARGET::SYSTEM, "CMD_PORTAL", "boolean", DATATYPE::BOOLEAN, 0, 0, 0, 0, "" },
  { TARGET::BM_X80, "SET_SLP", "Pa", DATATYPE::INTEGER, 101325, 30000, 110000, 101325, "" },
  { TARGET::AS6500, "SET_DIR", "boolean", DATATYPE::BOOLEAN, 0, 0, 0, 0, "CW=TRUE,CCW=FALSE" },
  { TARGET::SYSTEM, "SYSTEM_INTERVAL", "msec", DATATYPE::INTEGER, 2000, 600, 3600000, 2000, "Minimum time between system messages" },
  { TARGET::SENSOR, "SENSOR_INTERVAL", "msec", DATATYPE::INTEGER, 1000, 500, 1800000, 1000, "Minimum time between sensor readings/messages" }
};

class indexer {
public:
  size_t totalelements = sizeof(CMD_LIST) / sizeof(CMD_LIST[0]);

  size_t activeelements() {  // searches CMD_LIST array for non-null titles and returns total count.
    size_t filledelements = 0;
    for (int i = 0; i < totalelements; i++) {
      if (CMD_LIST[i].title != "") { filledelements++; }
    }
    return filledelements;
  }

  uint8_t lookup(String *searchkey) {  // returns the index of a specific lookup value OR (defNumCommands+1) for not found
    for (int i = 0; i < defNumCommands; i++) {
      if (*searchkey == CMD_LIST[i].title) {
        return i;  // if they match, then exit the loop and function, and return the current index
      }
    }
    return defNumCommands + 1;  // if we reach this point, then there's no matches
  }
};

struct nodeconfig {
  String ID = String(ESP.getChipId(), HEX);  // 6 character hex value of MAC address
  String version = defSoftwareVersion;       // used in MQTT config report.
  String function;                           // define later
} thisnode;

struct mqtttopic {                            // structure for holding all mqtt topics
  const char announce[14] = "base/announce";  // for announcing node online; if more than one NodeRed server on the same network, server will 'claim' the node by assigning it to that test
  char time[10] = "base/time";                // for ntp specific queries
  char data[10] = "node/data";                // for publishing sensor data
  char diag[10] = "node/diag";                // for providing diagnostic info
  char report[12] = "node/report";            // for publishing report/command formats AND reported data
  char response[14] = "node/response";        // for echoing command acknowledgement
  char cmnd[12];                              // "node/" + ID; will get populated during initialboot()
} mqtttopic;

struct msg {                    // for tracking outgoing messages sent
  unsigned long last_data = 0;  // tracks time of last SENSOR/DATA packet sent; for throttling data messages and sensor calls. Referenced against CMD::DATA_INTERVAL
  unsigned long last_msg = 0;   // tracks time of ANY message sent; for throttling total outgoing messages. Referenced against CMD::TOTAL_INTERVAL
  uint32_t cnt_total = 0;       // total messages sent; max value = 4,294,967,295
  uint16_t cnt_data = 0;        // for counting outgoing data messages; max value = 65,535
  uint16_t cnt_diag = 0;        // for counting outgoing diag messages; max value = 65,535
  uint16_t cnt_report = 0;      // for counting outgoing report messages; max value = 65,535
  uint16_t cnt_response = 0;    // for counting outgoing response messages; max value = 65,535
} msg;  uint16_t cnt_report = 0;      // for counting outgoing report messages; max value = 65,535
  uint16_t cnt_response = 0;    // for counting outgoing response messages; max value = 65,535
} msg;
