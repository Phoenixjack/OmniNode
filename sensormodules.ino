void setup_sensors() {                 // TODO: report sensor failure via MQTT and/or Serial Debug for ALL SENSOR TYPES
  debugprefix();                       //
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
    debugoutput("Could not find a valid 5883 sensor, check wiring!\n");
    delay(500);
  }                                                // HMC5883L_RANGE_0_88GA  HMC5883L_RANGE_1_3GA
  compass.setRange(QMC5883_RANGE_2GA);             // At 1.3GA range and heading 000, X:303 Y:12 Z:-1177. At 0.88GA range and heading 000, X:596 Y:-25 Z:-1275
  debugoutput("compass range is:");                //
  debugoutput(compass.getRange());                 //
  compass.setMeasurementMode(HMC5883L_CONTINOUS);  // Set/get measurement mode
  debugoutput("\ncompass measurement mode is:");
  debugoutput(compass.getMeasurementMode());
  compass.setDataRate(QMC5883_DATARATE_10HZ);  // Set/get the data collection frequency of the sensor
  debugoutput("\ncompass data rate is:");
  debugoutput(compass.getDataRate());
  compass.setSamples(QMC5883_SAMPLES_1);  //Get/get sensor status
  debugoutput("\ncompass samples is:");
  debugoutput(compass.getSamples());
  float fltMagneticDeviation = (-10 - (39 / 60)) * PI / 180;
  debugoutput("\nMagnetic Deviation: ");
  debugoutputln(fltMagneticDeviation);
  compass.setDeclinationAngle(fltMagneticDeviation);  // https://www.magnetic-declination.com/   mag dev for Dahlgren: -10deg39min  field strength: 50272.5nT, 50.53uT, or 0.505 gauss
#endif
#if (defFuncBMP280)
  unsigned status;
  status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
  if (!status) {
    debugoutput("Could not find a valid BMP280 sensor, check wiring or try a different address!");
    debugoutput("SensorID was: 0x");
    debugoutput(String(bmp.sensorID(), HEX));
    debugoutput("\n        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    debugoutput("   ID of 0x56-0x58 represents a BMP 280,\n");
    debugoutput("        ID of 0x60 represents a BME 280.\n");
    debugoutput("        ID of 0x61 represents a BME 680.\n");
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
    debugoutput("Failed to find MPU6050 chip\n");  //
    while (!mpu.begin()) {                         //
      led.brightness(RGBLed::RED, 100);            //
      delay(defBlinkShort);                        //
      led.brightness(RGBLed::YELLOW, 50);          //
      delay(defBlinkShort);                        //
    }                                              //
  }                                                //
  debugoutput("MPU6050 Found!\n");                 //
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
  debugprefix();                     //
  String strConfigSubStr = "";       // for compiling sub payloads
  String strXmitMsg = "";            // for compiling all relevant payloads
#if (defFuncPacketFwd)
  if (mySerial.available()) {                                          // check if there's anything in the secondary Serial buffer
    strConfigSubStr = strGetSerialData();                              // only then do we call the routine to retrieve the buffer data
    strConfigSubStr = strKeyValuePair("PacketRcvd", strConfigSubStr);  //
    strConfigSubStr = strJSONwrap(strConfigSubStr);                    //
    strXmitMsg += strKeyValuePair("PacketData", strConfigSubStr);      //
  }
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

#endif
#if (defFuncHMC5883)
  sVector_t mag = compass.readRaw();                                            //
  compass.getHeadingDegrees();                                                  //
  strConfigSubStr = strKeyValuePair("heading", String(mag.HeadingDegress, 1));  //
  strConfigSubStr = strJSONwrap(strConfigSubStr);                               //
  strXmitMsg += strKeyValuePair("xMC5883", strConfigSubStr);                    //
#endif
#if (defFuncBMP280)
  strConfigSubStr = strKeyValuePair("temperature", String(bmp.readTemperature())) + ",";               //
  strConfigSubStr += strKeyValuePair("pressure", String(bmp.readPressure() / 100)) + ",";              //
  strConfigSubStr += strKeyValuePair("altitude", String(bmp.readAltitude(intSeaLevelPressPa / 100)));  //
  strConfigSubStr = strJSONwrap(strConfigSubStr);                                                      //
  strXmitMsg += strKeyValuePair("BMP280", strConfigSubStr);                                            //
#endif
#if (defFuncBMP680)

#endif
#if (defFuncMPU6050)
  sensors_event_t a, g, temp;                                                       //
  mpu.getEvent(&a, &g, &temp);                                                      //
  strConfigSubStr = strKeyValuePair("gyroX", String(g.gyro.x, 4)) + ",";            //
  strConfigSubStr += strKeyValuePair("gyroY", String(g.gyro.y, 4)) + ",";           //
  strConfigSubStr += strKeyValuePair("gyroZ", String(g.gyro.z, 4)) + ",";           //
  strConfigSubStr += strKeyValuePair("Xaccel", String(a.acceleration.x, 4)) + ",";  //
  strConfigSubStr += strKeyValuePair("Yaccel", String(a.acceleration.y, 4)) + ",";  //
  strConfigSubStr += strKeyValuePair("Zaccel", String(a.acceleration.z, 4)) + ",";  //
  strConfigSubStr += strKeyValuePair("temperature", String(temp.temperature, 2));   // "m/s^2,rad/s,degC"
  strConfigSubStr = strJSONwrap(strConfigSubStr);                                   //
  strXmitMsg += strKeyValuePair("MPU6050", strConfigSubStr);                        //
#endif
#if (defFuncADXL345)

#endif
#if (defFuncAS6500)
  strConfigSubStr = strKeyValuePair("azimuth", String(as5600.rawAngle() * AS5600_RAW_TO_DEGREES, 1)) + ",";  //
  strConfigSubStr += strKeyValuePair("magnitude", String(as5600.readMagnitude()));                           //
  strConfigSubStr = strJSONwrap(strConfigSubStr);                                                            //
  strXmitMsg += strKeyValuePair("AS6500", strConfigSubStr);                                                  //
#endif
  led.brightness(RGBLed::BLUE, 100);                       // full brightness blue to indicate we have processed the data
  strXmitMsg = strJSONwrap(strXmitMsg);                    // wrap the payload
  strXmitMsg = strStandardMsg("data", strXmitMsg);         // add standard header with our assembled string as the payload
  transmitmqttmessage("node/data", strXmitMsg, false, 0);  // send it
  delay(200);                                              // pause to allow user to see bright indicator
  led.brightness(RGBLed::WHITE, 50);                       // return to dim white to indicate normal operations in progress
}

void cmd_GetSensorConfig() {
  String strConfigSubStr = "";  // for compiling sub payloads
  String strXmitMsg = "";       // for compiling all relevant payloads
#if (defFuncPacketFwd)
  strConfigSubStr = strKeyValuePair("PacketFwding", "noconfig");   //
  strConfigSubStr = strJSONwrap(strConfigSubStr);                  //
  strXmitMsg += strKeyValuePair("PacketFwding", strConfigSubStr);  //
#endif
#if (defFuncINA3221)
  strConfigSubStr = strKeyValuePair("Ch1Setting1", String("settinglookup")) + ",";   //
  strConfigSubStr += strKeyValuePair("Ch1Setting2", String("settinglookup")) + ",";  //
  strConfigSubStr += strKeyValuePair("Ch1Setting3", String("settinglookup"));        //
  strConfigSubStr = strJSONwrap(strConfigSubStr);                                    //
  strXmitMsg += strKeyValuePair("INA3221", strConfigSubStr);                         //
#endif
#if (defFuncNEO6_7)
  strConfigSubStr = strKeyValuePair("NEO6_7 GPS", "noconfig");  //
  strConfigSubStr = strJSONwrap(strConfigSubStr);               //
  strXmitMsg += strKeyValuePair("NEO6_7", strConfigSubStr);     //
#endif
#if (defFuncHMC5883)
  strConfigSubStr = strKeyValuePair("range", String(compass.getRange())) + ",";                   //
  strConfigSubStr += strKeyValuePair("measuremode", String(compass.getMeasurementMode())) + ",";  //
  strConfigSubStr += strKeyValuePair("datarate", String(compass.getDataRate())) + ",";            //
  strConfigSubStr += strKeyValuePair("samples", String(compass.getSamples()));                    //
  strConfigSubStr = strJSONwrap(strConfigSubStr);                                                 //
  strXmitMsg += strKeyValuePair("HMC5883", strConfigSubStr);                                      //
#endif
#if (defFuncBMP280)
  strConfigSubStr = strKeyValuePair("Setting1", String("settinglookup")) + ",";   //
  strConfigSubStr += strKeyValuePair("Setting2", String("settinglookup")) + ",";  //
  strConfigSubStr += strKeyValuePair("Setting3", String("settinglookup"));        //
  strConfigSubStr = strJSONwrap(strConfigSubStr);                                 //
  strXmitMsg += strKeyValuePair("BMP280", strConfigSubStr);                       //
#endif
#if (defFuncBMP680)
  strConfigSubStr = strKeyValuePair("Setting1", String("settinglookup")) + ",";   //
  strConfigSubStr += strKeyValuePair("Setting2", String("settinglookup")) + ",";  //
  strConfigSubStr += strKeyValuePair("Setting3", String("settinglookup"));        //
  strConfigSubStr = strJSONwrap(strConfigSubStr);                                 //
  strXmitMsg += strKeyValuePair("BMP680", strConfigSubStr);                       //
#endif
#if (defFuncMPU6050)
  strConfigSubStr = strKeyValuePair("Setting1", String("settinglookup")) + ",";   //
  strConfigSubStr += strKeyValuePair("Setting2", String("settinglookup")) + ",";  //
  strConfigSubStr += strKeyValuePair("Setting3", String("settinglookup"));        //
  strConfigSubStr = strJSONwrap(strConfigSubStr);                                 //
  strXmitMsg += strKeyValuePair("MPU6050", strConfigSubStr);                      //
#endif
#if (defFuncADXL345)
  strConfigSubStr = strKeyValuePair("Setting1", String("settinglookup")) + ",";   //
  strConfigSubStr += strKeyValuePair("Setting2", String("settinglookup")) + ",";  //
  strConfigSubStr += strKeyValuePair("Setting3", String("settinglookup"));        //
  strConfigSubStr = strJSONwrap(strConfigSubStr);                                 //
  strXmitMsg += strKeyValuePair("ADXL345", strConfigSubStr);                      //
#endif
#if (defFuncAS6500)
  strConfigSubStr = strKeyValuePair("Setting1", String("settinglookup")) + ",";   //
  strConfigSubStr += strKeyValuePair("Setting2", String("settinglookup")) + ",";  //
  strConfigSubStr += strKeyValuePair("Setting3", String("settinglookup"));        //
  strConfigSubStr = strJSONwrap(strConfigSubStr);                                 //
  strXmitMsg += strKeyValuePair("AS6500", strConfigSubStr);                       //
#endif
  strXmitMsg = strJSONwrap(strXmitMsg);                            // wrap ALL the config data subobjects
  strXmitMsg = strStandardMsg("sensorconfig", strXmitMsg);         // add standard header
  transmitmqttmessage("node/sensorconfig", strXmitMsg, false, 0);  // send it
}