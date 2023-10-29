/*
date and time functions
TODO: add our own Epoch to DTG conversion function
*/

unsigned long CurrentEpochTime() {        // return either the current NTP or a fake epoch time
  if (boolMQTTEmulated) {                 //
    return 1698339326;                    // 2023-10-26
  } else {                                //
    return instNTPClient.getEpochTime();  //
  }                                       //
}

String CurrentUTCTime() {                           // function to return human readable UTC time as YYYY-MM-DD HH-MM-SS
  instTimeConvert.getDateTime(CurrentEpochTime());  //
  String strDTG = String(instTimeConvert.year);     // initialize string starting with year
  strDTG += "-";                                    //
  strDTG += String(instTimeConvert.month);          //
  strDTG += "-";                                    //
  strDTG += String(instTimeConvert.day);            //
  strDTG += " ";                                    //
  strDTG += String(instTimeConvert.hour);           // TODO: Correct this to local timezone
  strDTG += ":";                                    //
  strDTG += String(instTimeConvert.minute);         //
  strDTG += ":";                                    //
  strDTG += String(instTimeConvert.second);         //
  return strDTG;                                    // give it back
}


// create separate functions to return HH:MM:SS.S vs YYYY-MM-DD_HH_MM_SS.S
// add leading zeroes for month, hour, min, second
// round seconds to X places

/*
// Major shoutout to Alex Guyver: https://github.com/GyverLibs/UnixTime
// His UnixTime library formed much of the basis for my customized code.
// I used his UTC conversions for my human readable DTG functions

class DateTime {
public:
  DateTime(int8_t gmt) {  // Indicate GMT
    _gmt = gmt;            // FOLLOWUP: Why do we need to copy a value that's been passed to us?
  }                        //


  uint32_t getUnix() {                                                                      // get the UnixEpoch for a given day, month, year, hour, min, sec...
    int8_t my = (month >= 3) ? 1 : 0;                                                       // is the month greater than 3, yes or no? Why do we care about the month when we're dealing with years?
    uint16_t y = year + my - 1970;                                                          // apply that offset and remember that we're counting since 1970
    uint16_t juliandate = 0;                                                                // initialize a counter for the Julian date
    for (int i = 0; i < month - 1; i++) {                                                   // step through the months that have passed, but we stop short of counting the days in the current month
      juliandate += (i < 7) ? ((i == 1) ? 28 : ((i & 1) ? 30 : 31)) : ((i & 1) ? 31 : 30);  // a convoluted, but compressed method of determining how many days are in the month we're evaluating
    }                                                                                       // then we use a ridiculously long statement to convert that into milliseconds elapsed since the beginning of 1970
    return (((day - 1 + juliandate + ((y + 1) >> 2) - ((y + 69) / 100) + ((y + 369) / 100 / 4) + 365 * (y - my)) * 24ul + hour - _gmt) * 60ul + minute) * 60ul + second;
  }

  void getDateTime(uint32_t t) {   // convert unix stamp date and time (into class member variables)
    // http://howardhinnant.github.io/date_algorithms.html#civil_from_days
    t += _gmt * 3600ul;
    second = t % 60ul;
    t /= 60ul;
    minute = t % 60ul;
    t /= 60ul;
    hour = t % 24ul;
    t /= 24ul;
    dayOfWeek = (t + 4) % 7;
    if (!dayOfWeek) dayOfWeek = 7;
    uint32_t z = t + 719468;
    uint8_t era = z / 146097ul;
    uint16_t doe = z - era * 146097ul;
    uint16_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    uint16_t y = yoe + era * 400;
    uint16_t doy = doe - (yoe * 365 + yoe / 4 - yoe / 100);
    uint16_t mp = (doy * 5 + 2) / 153;
    day = doy - (mp * 153 + 2) / 5 + 1;
    month = mp + (mp < 10 ? 3 : -9);
    y += (month <= 2);
    year = y;
    
  }

  // setting the date and time (more convenient than writing variables manually)
  void setDateTime(uint16_t nyear, uint8_t nmonth, uint8_t nday, uint8_t nhour, uint8_t nminute, uint8_t nsecond) {
    year = nyear;
    month = nmonth;
    day = nday;
    hour = nhour;
    minute = nminute;
    second = nsecond;
  }

  // date and time variables
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t dayOfWeek;

private:
  int8_t _gmt = 0;
};
*/

/*  orphaned code from Alex:
//https://www.oryx-embedded.com/doc/date__time_8c_source.html
uint32_t a, b, c, d, e, f;
int h, j, k;
t += _gmt * 3600ul;
second = t % 60ul;
t /= 60;
minute = t % 60ul;
t /= 60;
hour = t % 24ul;
t /= 24;
a = (uint32_t)((4ul * t + 102032) / 146097 + 15);
b = (uint32_t)(t + 2442113 + a - (a / 4));
c = (20 * b - 2442) / 7305;
d = b - 365 * c - (c / 4);
e = d * 1000 / 30601;
f = d - e * 30 - e * 601 / 1000;
if (e <= 13) {
  c -= 4716;
  e -= 1;
} else {
  c -= 4715;
  e -= 13;
}
year = c;
month = e;
day = f;
if (e <= 2) {
  e += 12;
  c -= 1;
}
j = c / 100;
k = c % 100;
h = f + (26 * (e + 1) / 10) + k + (k / 4) + (5 * j) + (j / 4);  // Zeller equation
dayOfWeek = ((h + 5) % 7) + 1;
*/