void setup_sensors() {                 // TODO: report sensor failure via MQTT and/or Serial Debug for ALL SENSOR TYPES
  debugPrefix(defDebugSensor);         //
  led.brightness(RGBLed::YELLOW, 50);  // indicator that we're starting up whichever sensors are enabled
#if (defFuncPacketFwd)
  mySerial.begin(115200);  // opening the second serial port for receiving data.
#endif
#if (defFuncINA3221)
  ina3221.begin();
  ina3221.reset();
  ina3221.setShuntRes(10, 10, 10);   // Set shunt resistors to 10 mOhm for all channels
  ina3221.setFilterRes(10, 10, 10);  // Set series filter resistors to 10 Ohm for all channels. They introduce error to the current measurement, but error can be estimated and corrected.
#endif
#if (defFuncNEO6_7)
  GPS.begin(4800);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);  // turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);     // 1 Hz update rate
  GPS.sendCommand(PGCMD_ANTENNA);                // Request updates on antenna status, comment out to keep quiet
  delay(1000);                                   // give time to adjust
  GPSSerial.println(PMTK_Q_RELEASE);             // ask for firmware version
#endif
#if (defFuncHMC5883)
  while (!compass.begin()) {
    debugSensor("Could not find a valid 5883 sensor, check wiring!\n");
    delay(500);
  }                                               //
  compass.setRange(QMC5883_RANGE_2GA);            //
  debugSensor("compass range is:");               //
  debugSensor(compass.getRange());                //
  compass.setMeasurementMode(QMC5883_CONTINOUS);  // Set/get measurement mode
  debugSensor("\ncompass measurement mode is:");
  debugSensor(compass.getMeasurementMode());
  compass.setDataRate(QMC5883_DATARATE_10HZ);  // Set/get the data collection frequency of the sensor
  debugSensor("\ncompass data rate is:");
  debugSensor(compass.getDataRate());
  compass.setSamples(QMC5883_SAMPLES_1);  //Get/get sensor status
  debugSensor("\ncompass samples is:");
  debugSensor(compass.getSamples());
  float fltMagneticDeviation = (-10 - (39 / 60)) * PI / 180;
  debugSensor("\nMagnetic Deviation: ");
  debugSensor(fltMagneticDeviation);
  debugSensor("\n");
  compass.setDeclinationAngle(fltMagneticDeviation);  // https://www.magnetic-declination.com/   mag dev for Dahlgren: -10deg39min  field strength: 50272.5nT, 50.53uT, or 0.505 gauss
#endif
#if (defFuncBMP280)
  unsigned status;
  status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
  if (!status) {
    debugSensor("Could not find a valid BMP280 sensor, check wiring or try a different address!");
    debugSensor("SensorID was: 0x");
    debugSensor(String(bmp.sensorID(), HEX));
    debugSensor("\n        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    debugSensor("   ID of 0x56-0x58 represents a BMP 280,\n");
    debugSensor("        ID of 0x60 represents a BME 280.\n");
    debugSensor("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);                           // TODO: modify this for RGB indicator
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,  /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,  /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16, /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,   /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500);
  } /* Standby time. */
#endif
#if (defFuncBMP680)

#endif
#if (defFuncMPU6050)
  if (!mpu.begin()) {                              //
    debugSensor("Failed to find MPU6050 chip\n");  //
    while (!mpu.begin()) {                         //
      led.brightness(RGBLed::RED, 100);            //
      delay(defBlinkShort);                        //
      led.brightness(RGBLed::YELLOW, 50);          //
      delay(defBlinkShort);                        //
    }                                              //
  }                                                //
  debugSensor("MPU6050 Found!\n");                 //
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);    //
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);         //
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);      //
#endif
#if (defFuncADXL345)

#endif
#if (defFuncAS6500)
  as5600.begin();                                 //
  as5600.setDirection(AS5600_COUNTERCLOCK_WISE);  // Platform only goes one way. Just so our data matches reality.
#endif
  led.brightness(RGBLed::YELLOW, 100);  // indicate successful sensor initialization
}




void cmd_getsensordata() {           // generic 'read the sensor' function. keeps the main loop and calling functions clean by containing ifdef statements here
  led.brightness(RGBLed::BLUE, 50);  // dim blue to indicate processing data
  debugPrefix(defDebugSensor);       //
  JSONVar objData;                   //
  defstandardheader(objData);        //
#if (defFuncPacketFwd)
  if (mySerial.available()) {                            // check if there's anything in the secondary Serial buffer
    objData["data"]["PacketData"] = strGetSerialData();  // only then do we call the routine to retrieve the buffer data
  }
#endif
#if (defFuncFauxFwd)
  // H,1,8,4,20.1,3.25,-33,-113,-120,-32,0
  // {"Loc":"H","PacketNum":73,"GroupID":8,"UnitID":4,"TempC":28.6,"Voltage":3.312,"RcvNoise":-191,"RcvRSSI":-42,"LastRSSI":-29,"LastNoise":-129,"GPS":0}
  String strConfigSubStr = "H," + String(intMsgCnt) + ",8,4,";                                 // position, packetnum, group, unit,
  strConfigSubStr += String(random(24, 30)) + "," + String((float)ESP.getVcc() / 1000) + ",";  // temp,vcc,
  strConfigSubStr += String(random(-190, -180)) + ",";                                         // rcvnoise,
  strConfigSubStr += String(random(-50, -38)) + ",";                                           // rcvrssi,
  strConfigSubStr += String(random(-50, -38)) + ",";                                           // lastrssi,
  strConfigSubStr += String(random(-190, -180)) + ",0";                                        // lastnoise,gps
  objData["data"]["PacketData"] = strConfigSubStr;                                             //
#endif
#if (defFuncINA3221)

#endif
#if (defFuncNEO6_7)
  char c = GPS.read();  // read data from the GPS
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trying to print out data
    if (!GPS.parse(GPS.lastNMEA()))  // this also sets the newNMEAreceived() flag to false
      return;                        // we can fail to parse a sentence in which case we should just wait for another
  }
  objData["data"]["GPS"] = String(c);  //
#endif
#if (defFuncHMC5883)
  sVector_t mag = compass.readRaw();  //
  compass.getHeadingDegrees();        //
  debugSensor("heading: ");
  debugSensor(mag.HeadingDegress);
  debugSensor("\n");
  objData["data"]["xMC5883"]["heading"] = mag.HeadingDegress;  //
#endif
#if (defFuncBMP280)
  objData["data"]["BMP280"]["temperature"] = bmp.readTemperature();                      //
  objData["data"]["BMP280"]["pressure"] = (bmp.readPressure() / 100);                    //
  objData["data"]["BMP280"]["altitude"] = (bmp.readAltitude(intSeaLevelPressPa / 100));  //
#endif
#if (defFuncBMP680)

#endif
#if (defFuncMPU6050)
  sensors_event_t a, g, temp;                                                 //
  mpu.getEvent(&a, &g, &temp);                                                //
  objData["data"]["MPU6050"]["gyroX"] = String(g.gyro.x, 4);                  //
  objData["data"]["MPU6050"]["gyroY"] = String(g.gyro.y, 4);                  //
  objData["data"]["MPU6050"]["gyroZ"] = String(g.gyro.z, 4);                  //
  objData["data"]["MPU6050"]["Xaccel"] = String(a.acceleration.x, 4);         //
  objData["data"]["MPU6050"]["Yaccel"] = String(a.acceleration.y, 4);         //
  objData["data"]["MPU6050"]["Zaccel"] = String(a.acceleration.z, 4);         //
  objData["data"]["MPU6050"]["temperature"] = String(temp.temperature, 2);   // "m/s^2,rad/s,degC"
#endif
#if (defFuncADXL345)
  objData["data"]["ADXL345"]["gyroX"] = "x";  //
  objData["data"]["ADXL345"]["gyroY"] = "y";  //
  objData["data"]["ADXL345"]["gyroZ"] = "z";  //
#endif
#if (defFuncAS6500)
  objData["data"]["AS6500"]["azimuth"] = String(as5600.rawAngle() * AS5600_RAW_TO_DEGREES, 1);  //
  objData["data"]["AS6500"]["magnitude"] = String(as5600.readMagnitude());                      //
#endif
  led.brightness(RGBLed::BLUE, 100);                       // full brightness blue to indicate we have processed the data
  String strXmitMsg = JSON.stringify(objData);             //
  debugPrintln(defDebugSensor, strXmitMsg);                //
  transmitmqttmessage("node/data", strXmitMsg, false, 0);  // send it
  delay(200);                                              // pause to allow user to see bright indicator
  led.brightness(RGBLed::WHITE, 50);                       // return to dim white to indicate normal operations in progress
}

void cmd_GetSensorConfig() {
  JSONVar objData;             //
  defstandardheader(objData);  //
#if (defFuncPacketFwd)
  objData["sensorconfig"]["PacketFwd"]["setting"] = "actual serial feed";
#endif
#if (defFuncFauxFwd)
  objData["sensorconfig"]["PacketFwd"]["setting"] = "simulated serial feed";
#endif
#if (defFuncINA3221)
  objData["sensorconfig"]["INA3221"]["Ch1Setting1"] = "settinglookup";
  objData["sensorconfig"]["INA3221"]["Ch1Setting2"] = "settinglookup";
  objData["sensorconfig"]["INA3221"]["Ch1Setting3"] = "settinglookup";
#endif
#if (defFuncNEO6_7)
  objData["sensorconfig"]["NEOGPS"]["setting"] = "noconfig");  //
#endif
#if (defFuncHMC5883)
  objData["sensorconfig"]["xMC5883"]["range"] = String(compass.getRange() ? "QMC5883_RANGE_2GA" : "QMC5883_RANGE_8GA");                //
  objData["sensorconfig"]["xMC5883"]["measuremode"] = String(compass.getMeasurementMode() ? "SINGLE" : "CONTINUOUS");                  //
  objData["sensorconfig"]["xMC5883"]["datarate"] = String(compass.getDataRate() ? "QMC5883_DATARATE_10HZ" : "QMC5883_DATARATE_50HZ");  //
  objData["sensorconfig"]["xMC5883"]["samples"] = String(compass.getSamples() ? "QMC5883_SAMPLES_1" : "QMC5883_SAMPLES_2");            //
#endif
#if (defFuncBMP280)
  objData["sensorconfig"]["BMP280"]["Setting1"] = "settinglookup";
  objData["sensorconfig"]["BMP280"]["Setting2"] = "settinglookup";
#endif
#if (defFuncBMP680)
  objData["sensorconfig"]["BMP680"]["Setting1"] = "settinglookup";
  objData["sensorconfig"]["BMP680"]["Setting2"] = "settinglookup";
#endif
#if (defFuncMPU6050)
  objData["sensorconfig"]["MPU6050"]["Setting1"] = "settinglookup";
  objData["sensorconfig"]["MPU6050"]["Setting2"] = "settinglookup";
#endif
#if (defFuncADXL345)
  objData["sensorconfig"]["ADXL345"]["Setting1"] = "settinglookup";
  objData["sensorconfig"]["ADXL345"]["Setting2"] = "settinglookup";
#endif
#if (defFuncAS6500)
  objData["sensorconfig"]["AS6500"]["Setting1"] = "settinglookup";
  objData["sensorconfig"]["AS6500"]["Setting2"] = "settinglookup";
#endif
  String strXmitMsg = JSON.stringify(objData);                     //
  transmitmqttmessage("node/sensorconfig", strXmitMsg, false, 0);  // send it
}