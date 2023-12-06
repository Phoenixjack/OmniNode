void setup_sensors() {
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
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);  // 1 Hz update rate
  GPS.sendCommand(PGCMD_ANTENNA);
  delay(1000);
  GPSSerial.println(PMTK_Q_RELEASE);
#endif
#if (defFuncHMC5883)
  while (!compass.begin()) {
    Serial.printf("%s: %s: Could not find a valid 5883 sensor, check wiring!", __FILE__, __func__);
    delay(500);
  }
  compass.setRange(HMC5883L_RANGE_1_3GA);
  Serial.print("compass range is:");
  Serial.println(compass.getRange());
  compass.setMeasurementMode(HMC5883L_CONTINOUS);  //Set/get measurement mode
  Serial.print("compass measurement mode is:");
  Serial.println(compass.getMeasurementMode());
  compass.setDataRate(HMC5883L_DATARATE_15HZ);  //Set/get the data collection frequency of the sensor
  Serial.print("compass data rate is:");
  Serial.println(compass.getDataRate());
  compass.setSamples(HMC5883L_SAMPLES_8);  //Get/get sensor status
  Serial.print("compass samples is:");
  Serial.println(compass.getSamples());
#endif
#if (defFuncBMP280)

#endif
#if (defFuncBMP680)

#endif
#if (defFuncMPU6050)
  if (!mpu.begin()) {                               //
    Serial.println("Failed to find MPU6050 chip");  //
    while (!mpu.begin()) {                          //
      led.brightness(RGBLed::RED, 100);             //
      delay(defBlinkShort);                         //
      led.brightness(RGBLed::YELLOW, 50);           //
      delay(defBlinkShort);                         //
    }                                               //
  }                                                 //
  Serial.println("MPU6050 Found!");                 //
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);     //
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);          //
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);       //
#endif
#if (defFuncADXL345)

#endif
#if (defFuncAS6500)
  as5600.begin();                                 //
  as5600.setDirection(AS5600_COUNTERCLOCK_WISE);  // Platform only goes one way. Just so our data matches reality.
#endif
  led.brightness(RGBLed::YELLOW, 100);  // indicate successful sensor initialization
}

void cmd_getsensordata() {            // generic 'read the sensor' function. keeps the main loop and calling functions clean by containing ifdef statements here
  led.brightness(RGBLed::BLUE, 50);   // dim blue to indicate processing data
  led.brightness(RGBLed::BLUE, 100);  // full brightness blue to indicate we have processed the data
#if (defFuncPacketFwd)
  if (mySerial.available()) {
    cmd_GetSerialData();
  }
#endif
#if (defFuncINA3221)

#endif
#if (defFuncNEO6_7)

#endif
#if (defFuncHMC5883)
  float fltMagneticDeviation = (-10 - (39 / 60)) * PI / 180;  // https://www.magnetic-declination.com/   mag dev for Dahlgren: -10deg39min  field strength: 50272.5nT
  compass.setDeclinationAngle(fltMagneticDeviation);
  sVector_t mag = compass.readRaw();
  compass.getHeadingDegrees();
  Serial.print("X:");
  Serial.print(mag.XAxis);
  Serial.print(" Y:");
  Serial.print(mag.YAxis);
  Serial.print(" Z:");
  Serial.println(mag.ZAxis);
  Serial.print("Degress = ");
  Serial.println(mag.HeadingDegress);
  delay(100);
#endif
#if (defFuncBMP280)

#endif
#if (defFuncBMP680)

#endif
#if (defFuncMPU6050)

#endif
#if (defFuncADXL345)

#endif
#if (defFuncAS6500)
// String(as5600.rawAngle() * AS5600_RAW_TO_DEGREES);
// String(as5600.readMagnitude());
#endif
  led.brightness(RGBLed::WHITE, 50);  // return to dim white to indicate normal operations in progress
}

void cmd_GetSerialData() {
  /*  digitalWrite(MQTTConnpin, LOW);                                                                   // turn off MQTT indicator so we can clearly see the packet light
  digitalWrite(PacketProcesspin, HIGH);                                                             // Indicate to user that packet processing has started
  stopwatch = millis();                                                                             //
  char messBuffer[99];                                                                              //
  int messageSize = 0;                                                                              //
  char singlechar;                                                                                  //
  char timestampchar[8];                                                                            //
  char packetnumchar[8];                                                                            //
  char wifirssichar[5];                                                                             //
  msgcounter++;                                                                                     // counting the number of times we received a packet on MySerial
  ltoa(millis(), timestampchar, 10);                                                                //
  itoa(msgcounter, packetnumchar, 10);                                                              //
  itoa(WiFi.RSSI(), wifirssichar, 10);                                                              //
  for (int c = 0; c <= 99; c++) messBuffer[c] = 0;                                                  // clear messBuffer in prep for new message
  messageSize = 0;                                                                                  // set message size to zero
  while (mySerial.available()) {                                                                    // loop through while data is available
    long now = millis();                                                                            //
    if (now - lastMsg > 1.5) {                                                                      //
      lastMsg = now;                                                                                //
      singlechar = mySerial.read();                                                                 // get character
      messBuffer[messageSize] = singlechar;                                                         // append to messBuffer
      messageSize++;                                                                                // bump message size
    }                                                                                               //
  }                                                                                                 //
  char compiledpayload[60];                                                                         //
  sprintf(compiledpayload, "%s,%s,%s,%s", timestampchar, packetnumchar, wifirssichar, messBuffer);  //
  if (!mqttclient.connected()) {                                                                    // check if we're still connected to MQTT broker
    mqttconnect();                                                                                  // if not, redial
  }                                                                                                 //
  mqttclient.publish(mqtt_packet_topic.c_str(), compiledpayload, false);                            // send it
  Serial.println(compiledpayload);                                                                  //
  Serial.println(millis() - stopwatch);                                                             //
  delay(150);                                                                                       //
  digitalWrite(PacketProcesspin, LOW);                                                              // Packet processing finished and transmitted. Turn off the light
  digitalWrite(MQTTConnpin, HIGH);                                                                  // turn MQTT indicator back on now that we're done
*/
}

void cmd_GetMPU6050Data() {
  // sensors_event_t a, g, temp;           //
  // mpu.getEvent(&a, &g, &temp);          //
  // String(g.gyro.x, charTemp, 10, 4) + ",";          //
  // String(g.gyro.y, charTemp, 10, 4) + ",";          //
  // String(g.gyro.z, charTemp, 10, 4) + ",";          //
  // floatToString(a.acceleration.x, charTemp, 10, 4) + ",";  //
  // floatToString(a.acceleration.y, charTemp, 10, 4) + ",";  //
  // floatToString(a.acceleration.z, charTemp, 10, 4) + ",";  //
  // floatToString(temp.temperature, charTemp, 10, 2) + ",";  //
  // "m/s^2,rad/s,degC";                                        //
}
