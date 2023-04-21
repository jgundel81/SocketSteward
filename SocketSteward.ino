
/*
 *  SocketSteward.ino
 *  Main Entry
 *
 */
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

Adafruit_AW9523 aw;

#define RMS_WINDOW 50   // rms window of 50 samples, means 3 periods @60Hz
#define INCLUDE_TIMESTAMP 0X01   //Used for writeTrace()
#define INCLUDE_SENSORS 0X02
#define INCLUDE_STATUS 0X04


DateTime now;
DateTime dataLoggingStartedTime;
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

#define LED_PIN 13
#define RELAY_PIN_IO_EXPANDER 13
//Chip Select pin connected to SDCard

const int chipSelect = 10;


//Flag so the Button Thread can Start/Stop DataLogging

bool dataloggingEnabled = false;
bool firstRun = false;
bool startLogging();
bool codeTracingEnabled = false;


bool startCodeTracing();
void writeEventLog(String messageText);  
void stopLogging();
float runImpedanceTest(bool flags);

bool gSDCardInited = false;
/********************* Scheduling Related Variables *************************/
#define INTERVAL_ALWAYS 0
#define INTERVAL_10ms 10
#define INTERVAL_100ms 100
#define INTERVAL_500ms 500
#define INTERVAL_1000ms 1000
#define INTERVAL_10S 10000

#define UPDATE_gAnalysis_impedance true

typedef struct _task {
  uint16_t interval;
  uint32_t lastTick;
  void (*func)(void);
} TaskType;


static TaskType *pTask = NULL;
static uint8_t taskIndex = 0;

/********************* Thread Prototypes used in table ***********************/
void data_logging(void);
void display_task(void);
void button_task(void);
void sensormonitor_task(void);
void RTC_task(void);
void control_task(void);
void blinkpattern_task(void);
void GetValues(void);


/*********    TASK Table (insert Tasks into Table **********************/
static TaskType Tasks[] = {
  { INTERVAL_1000ms, 0, RTC_task },
  { INTERVAL_500ms, 0, display_task },
  { INTERVAL_10ms, 0, button_task },
  { INTERVAL_100ms, 0, control_task },  //if changing control task, please also change static tick1 to maintain 3 second timer
  { INTERVAL_500ms, 0, sensormonitor_task },
  { INTERVAL_100ms, 0, blinkpattern_task}, 
  { INTERVAL_1000ms, 0, data_logging},
  
};

const uint8_t numOfTasks = sizeof(Tasks) / sizeof(*Tasks);

TaskType *getTable(void) {
  return Tasks;
}

Power acPower;  
float VoltRange = 2000.00; // the peak to peak AC volts that fills the ADC input range (the AC input board clips due to poor choice of op amp not having rail to rail capability. The trim pot is turned way down to eliminate clipping at 130 volts)
float acCurrRange = 500; // peak-to-peak current that fits the ADC input range 0 to 3.3v.
int acVoltADC;  // add "volatile" if ever value is accessed outside the timer ISR
int acCurrADC;


/*
*   SD Card Initialization Function 
*
*/
void initSDCard(void) {
  Serial.println(" Initializing SD card");

  // see if the card is present and can be initialized:
  if (! SD.begin(chipSelect))
  {
    writeEventLog("Card failed, or not present");

  }
  else
  {
    gSDCardInited = true;
    Serial.println("Card initialized.");
  }
  
}


/*
 *   Function Name: Setup 
 * 
 *   Description: Setup function, Initialized LED, Serial port and
 *     task scheduler structures
 */
void setup() 
{
   pinMode(LED_PIN, OUTPUT);
   Serial.begin(250000);

   while(! Serial);
   Serial.print("This is Socket Steward sy millis() is ");
   Serial.println(millis());


  //Initialize GPIO Expander.
   
   if (!aw.begin(0x58))
   {
    Serial.print(" AW9523 didnt initialize at millis = ");
    Serial.println(millis());
     }
   else{
     Serial.println("AW9523 GPOI Expander found, thank you");
   }
   
   initSDCard();

  pTask = getTable();
  if (NULL == pTask) {
    //Error
    while (1)
      ;
  }
  aw.pinMode(RELAY_PIN_IO_EXPANDER, OUTPUT);
  aw.digitalWrite(RELAY_PIN_IO_EXPANDER, LOW);
  
  writeTrace("RMS0", INCLUDE_SENSORS && INCLUDE_STATUS );
   
  acPower.begin(VoltRange, acCurrRange, RMS_WINDOW, ADC_10BIT, BLR_ON, CNT_SCAN);
  acPower.start(); //start measuring
  
  TC.startTimer(1000, GetValues); // 
  delay(1000);
  float val = runImpedanceTest(UPDATE_gAnalysis_impedance);
  Serial.print("ran Voltage Drop in startup:");
  Serial.println(val);
  TC.restartTimer(2000); // 2 msec 

  
 
}
  
/*
 *   Function Name: Loop 
 * 
 *   Description: Scheduler- Calls tasks at stated intervals
 *     
 */
void loop() {
  for (taskIndex = 0; taskIndex < numOfTasks; taskIndex++) {
    
    //Run primitive Scheduler
    if (0 == pTask[taskIndex].interval) {
      //run every loop
      (*pTask[taskIndex].func)();
    } else if ((millis() - pTask[taskIndex].lastTick) > pTask[taskIndex].interval) {
      (*pTask[taskIndex].func)();
      pTask[taskIndex].lastTick = millis();
    }
  }
}