#include "RTClib.h"
#include <SPI.h>
#include <SD.h>
#include <string.h>
#include <TrueRMS.h>
#include <Adafruit_AW9523.h>
#include <math.h>
#include <wiring_analog.h>
#include <Wire.h>
#include "SAMD51_InterruptTimer.h"  //https://github.com/Dennis-van-Gils/SAMD51_InterruptTimer/releases/tag/v1.1.1 download zip, and do Sketch>include library>add zip library
#include <Ewma.h> 
// #include <AudioZero.h> throws errors

using namespace std;

#define INCLUDE_TIMESTAMP 0X01
#define INCLUDE_SENSORS 0X02
#define INCLUDE_STATUS 0X04




// Bitwise test macros. To clarify how bitwise tests are being used, please add new macros here and avoid in-line tests elsewhere. 
#define ANY_CONNECTED_STATUS(x) (x & (1UL << 4) )  
#define ANY_DISCONNECTED_STATUS(x) (x & (1UL << 5) ) 



//Types of events that occur  and result in a change of gPowerStatus.
//This needs to be in the same order
//as the tables below
typedef enum
{
  no_error,
  thermal_runaway,
  low_voltage,
  ground_fault,
  arc_fault,
  trp_no_indication,          //Trippped no indication
  trp_gfci_load_gf,           //Tripped GFCI load GF
  trp_series_arc,             //Tripped series ARC
  trp_parallel_arc,           //Tripped parallel ARC
  trp_overvoltage,            //Tripped overvoltage
  trp_af_slf_tst_fail,        //Tripped AF self test failure
  trp_gfci_slf_tst_fail,      //Tripped GFCI self test failure
  hot_attch_plug,             //Hot attachment plug
  hot_receptacle,             //Hot receptacle
  no_load_at_pwr_up,          //No load at power up
  full_cap_confirmed,         //Full capacity confirmed
  mrg_cap_confirmed,          //Marginal capacity confirmed
  appliance_blocked_vdrop,          //Load present at power
  ext_volt_dips_det,          //external volt DIPS detected
  boot_up,
  sdcard_error,
  test_load_error,            //SD Card Error
  unknown_trip,
  sensor_error,
  rtc_battery_error,
  NUM_OF_ERRORS     //This needs to be last 
} error_conditions_t;

void processEvent(error_conditions_t error);

//Global Error Variable 
//Used for menu display
error_conditions_t gLatestEvent = boot_up;
error_conditions_t gPreviousEvent = boot_up;  
int gPowerStatus = INITIALIZING;