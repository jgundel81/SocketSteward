// Date and time functions using a PCF8523 RTC connected via I2C and Wire lib
/*
*     RTC.ino
*    File to handle Calling RTC 
*/
#include "RTClib.h"

RTC_PCF8523 rtc;


/*
* intitialization function for the RTC
*
*/
void initRTC(void)
{
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (!rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC is NOT initialized, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    //
    // Note: allow 2 seconds after inserting battery or applying external power
    // without battery before calling adjust(). This gives the PCF8523's
    // crystal oscillator time to stabilize. If you call adjust() very quickly
    // after the RTC is powered, lostPower() may still return true.
  }
  else
  {
    Serial.println("RTC was running, but seting RTC time to compile time because we don't yet have a better way."); //this should not be needed... but it did. Can't we use network time when USB is connected?
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // remove this if we find a way to update RTC via network time on USB
  }
}

/*
*   RTC Task gets called every second to query the RTC
*/
void RTC_task(void) 
{

  //Only init once
  static bool RTCInited = false;
  if (false == RTCInited) {
    RTCInited = true;
    initRTC();
  }
  //Set the Time from the RTC into Global now Variables
  now = rtc.now();
}


