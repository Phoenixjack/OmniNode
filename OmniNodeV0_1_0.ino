/*
OmniNode
Author: Chris McCreary
A program to use ESP8266 modules to collect data and feed back to a Node Red server via MQTT
See VersionHistory.md for full details

DESIGN PHILOSOPHY
Overall emphasis will be on minimal user setup and troubleshooting.
Visual indicators are needed to allow easy monitoring of node status.
Autodiscovery is preferred, and the goal will be to accept valid commands by any means the user desires. Security will unfortunately suffer, but this is expected.
Code needs to remain flexible, yet support varying levels of debugging and diagnostic monitoring.
Diagnostics and remote commands will enable the Node Red server to identify faults, autocorrect, and/or alert users to faults.
Wherever possible, offload processing to the server to free up the node for data collection.

NAMING CONVENTIONS
The follow prefixes will be used for consistency and clarity:
Defined values ... defName
Objects .......... objName
Instances ........ instName
Unsigned Long .... ulongName
Integer .......... intName
String ........... strName
EXCEPTIONS: when a struct/enum/class clearly restrains and defines the nature of the object/instance/variable
*/

#define defSoftwareVersion "OmniNodeV0_1_0"  // See VersionHistory.md for full details
#define defDebugSerialMonitor true           // Global flag to enable/disable Serial Monitor debugging. Will override any other debug flag
#define defDebugJSON true                    // handles JSON debugging
#define defDebugSetup true                   // debug initial setup only
#define defDebugLoop false                   // debug main loop only; WARNING: GETS VERBOSE QUICK
#define defDebugVCC true                     // Enables reading onboard VCC, but conflicts with use of ADC input / A0
#if (defDebugVCC)
ADC_MODE(ADC_VCC);  // required to use built in ADC to measure onboard voltage. WARNING: THIS WILL DISABLE THE ADC input / A0 pin
#endif
#include <PhoenixJack_debugheader.h>  // custom library for handling debug statements and global switching
#include <PhoenixJack_filehandler.h> // custom wrapper for dealing with file I/O 
#include "datastructures.h"           // a separate header file for typedefs, enums, and structs; strictly for neatness
#include <Arduino_JSON.h>             // https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
FileHandler objFS;

void setup() {
#if (defDebugSerialMonitor)
  Serial.begin(115200);                  // initialize the Serial Monitor interface for debugging
  while (!Serial) delay(100);            // wait for Serial Monitor
  debugPF(defDebugSetup, "Compiled: ");  //
  debugPF(defDebugSetup, __DATE__);      //
  debugPF(defDebugSetup, " ");           //
  debugPFln(defDebugSetup, __TIME__);    //
  delay(1000);                           // delay for a while to give Serial Monitor time to catch up with board reset. TODO: remove for production code
#endif
  bool successfulboot = initialboot();

  // file system and get saved info
  // apply saved info
  // start sensors
  // connect wifi, mqtt, ntp, rest
}

void loop() {
}

bool initialboot() {
  if (loadconfigfromfile()) {
    return true;  // works for now. need to start wifi, mqtt, etc after loading config
  } else {
    return false;
  }
}

void composefile() {
  debugheader(defDebugSetup);
  JSONVar objSaveFile;
  int selectedsavedref[] = { 0, 1, 2, 8, 9 };
  int numvals = sizeof(selectedsavedref) / sizeof(selectedsavedref[0]);
  debugPVln(defDebugSetup, numvals);
  for (int i = 0; i < numvals; i++) {
    int thisindex = selectedsavedref[i];
    String thiskeyvalue = CMD_LIST[thisindex].title;
    switch (CMD_LIST[thisindex].datatype) {
      case (DATATYPE::STRING):
        objSaveFile[thiskeyvalue] = CMD_LIST[thisindex].text;
        break;
      case (DATATYPE::INTEGER):
        objSaveFile[thiskeyvalue] = CMD_LIST[thisindex].def_val;
        break;
      case (DATATYPE::BOOLEAN):
        objSaveFile[thiskeyvalue] = CMD_LIST[thisindex].def_val;
        break;
    }  // need to expand to include floats and option lists
    debugPVln(defDebugSetup, objSaveFile[thiskeyvalue]);
  }
  objSaveFile["pinky"] = "narf";
  objSaveFile["brain"] = "takeover";
  debugPFln(defDebugSetup, "");
  String jsonString = JSON.stringify(objSaveFile);
  debugPVln(defDebugSetup, jsonString);
  objFS.writeFile(objFS.ConfigFile, jsonString.c_str());
  debugPFln(defDebugSetup, "");
  debugPFln(defDebugSetup, "");
  debugPFln(defDebugSetup, "");
  String readback;
  objFS.readFile(objFS.ConfigFile, &readback);
  JSONVar myObject = JSON.parse(readback);
  Serial.println(myObject.keys());
  //Serial.print("myObject = ");
  //Serial.println(myObject);
}

bool loadconfigfromfile() {
  debugheader(defDebugSetup);
  FileHandler objFS;
  if (objFS.mount()) {
    String filecontents;  // {"MQTT_BROKER":"192.168.1.6","MQTT_PORT":1883,"TEST_NAME":"default","TOTAL_INTERVAL":4000,"SET_SLP":101325,"pinky":"narf","brain":"takeover"}
    if (objFS.readFile(objFS.ConfigFile, &filecontents)) {
      JSONVar objJSONdoc = JSON.parse(filecontents);
      if (JSON.typeof(objJSONdoc) == "undefined") {
        debugPFln(defDebugJSON, "Parsing config file failed!");
      } else {                                            // it was parsed
        String stringconvert;                             // a temp variable for converting strings to char arrays, ints, floats, etc
        JSONVar savedkeys = objJSONdoc.keys();            // extract all JSON keys from the file; example: ["MQTT_BROKER","MQTT_PORT","TEST_NAME","TOTAL_INTERVAL","SET_SLP","pinky","brain"]
        indexer objCmdArray; // initialize an instance of the indexer class to perform lookups for us
        for (int i = 0; i < savedkeys.length(); i++) {    // iterate through however many there are and match them to the indexes
          JSONVar value = objJSONdoc[savedkeys[i]];       // why do we need to create a new variable just to reference savedkeys[i]? All keys should be strings
          stringconvert = String(savedkeys[i]);           //
          size_t indexval = objCmdArray.lookup(&stringconvert); // call saved function; returns index of first match OR defNumCommands+1 (out of range) if not found
          if (indexval > defNumCommands) {
            // if the key wasn't found in the commandlist, then we'll ignore it. When we go to write an updated file, this key won't be included
            // objJSONdoc[[savedkeys[i]] = undefined; // this will tell JSON library to explicitly delete it
          } else {
            //CMD_FORMAT[indexval] = i;  // associate the index of the saved keys to CMD_FORMAT
            // then we'll need to verify the input against acceptable parameters
            // and apply if it's valid
          }
        }
        /*
        if (objJSONdoc.hasOwnProperty("mqttbroker")) {
          // TODO: once wifi/UDP/MQTT library up, need to ping and verify, then VERIFY before applying
          stringconvert = objJSONdoc["mqttbroker"];
          thisnode.mqttbroker = stringconvert.c_str();
        }
        if (objJSONdoc.hasOwnProperty("mqttport")) {
          if (JSON.typeof_(objJSONdoc["mqttport"]) == "string") {
            stringconvert = objJSONdoc["mqttport"];
            thisnode.mqttport = stringconvert.toInt();
          } else if (JSON.typeof_(objJSONdoc["mqttport"]) == "number") {
            thisnode.mqttport = objJSONdoc["mqttport"];
          }
        }
        if (objJSONdoc.hasOwnProperty("ReportInterval")) {
          int intervalvalidation;
          if (JSON.typeof_(objJSONdoc["ReportInterval"]) == "string") {
            stringconvert = objJSONdoc["ReportInterval"];
            intervalvalidation = stringconvert.toInt();
          } else if (JSON.typeof_(objJSONdoc["ReportInterval"]) == "number") {
            intervalvalidation = objJSONdoc["ReportInterval"];
          }
          if (intervalvalidation < 500 || intervalvalidation > 3600000) {
            intervalvalidation = 2000;
          }
          thisnode.interval = intervalvalidation;
        }
        if (objJSONdoc.hasOwnProperty("testname")) {
          stringconvert = objJSONdoc["testname"];
          thisnode.testname = stringconvert.c_str();
        }
        // proceed with loading into config
        */
      }
    }
    return true;
  } else {
    return false;
  }
}

int jsontocmdlookup(String searchkey) {
  for (int i = 0; i < defNumCommands; i++) {
    if (searchkey == command[i]) {
      return i;  // if they match, then exit the loop and function, and return the current index
    }
  }
  return -1;  // if we reach this point, then there's no matches
}
