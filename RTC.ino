// Date and time functions using a PCF8523 RTC connected via I2C and Wire lib
#include "RTClib.h"

RTC_PCF8523 rtc;



void initRTC(void)
{
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

    Serial.println("Syncing the time.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println("Finished syncing the time.");
  }



void RTC_task(void) 
{

  //Only init once
  static bool isInited = false;
  if (false == isInited) {
    isInited = true;
    initRTC();
  }

  now = rtc.now();


}

