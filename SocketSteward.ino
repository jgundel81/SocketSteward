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
Adafruit_AW9523 aw;

#define RMS_WINDOW 50   // rms window of 50 samples, means 3 periods @60Hz

DateTime now;
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

#define LED_PIN 13

//Flag so the Button Thread can Start/Stop DataLogging
bool dataloggingEnabled = false;
void startLogging();
void stopLogging();

/********************* Scheduling Related Variables *************************/
#define INTERVAL_ALWAYS 0
#define INTERVAL_10ms 10
#define INTERVAL_100ms 100
#define INTERVAL_500ms 500
#define INTERVAL_1000ms 1000

typedef struct _task {
  uint16_t interval;
  uint32_t lastTick;
  void (*func)(void);
} TaskType;


static TaskType *pTask = NULL;
static uint8_t taskIndex = 0;

/********************* Thread Prototypes used in table ***********************/
void data_logging(void);
void blinkLED(void);
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
  { INTERVAL_1000ms, 0, blinkLED },
  { INTERVAL_500ms, 0, display_task },
  { INTERVAL_10ms, 0, button_task },
  { INTERVAL_10ms, 0, GetValues },
  { INTERVAL_100ms, 0, control_task },
  { INTERVAL_500ms, 0, sensormonitor_task },
  { INTERVAL_100ms, 0, blinkpattern_task},
  { INTERVAL_500ms, 0, data_logging},
  
};

const uint8_t numOfTasks = sizeof(Tasks) / sizeof(*Tasks);

TaskType *getTable(void) {
  return Tasks;
}

Power acPower;  // create an instance of Power
float VoltRange = 2000.00; // The full scale value is set to 5.00 Volts but can be changed when using an
float acCurrRange = 130; // peak-to-peak current scaled down to 0-5V is 5A (=5/2*sqrt(2) = 1.77Arms max).



/*
 *   Function Name: Setup 
 * 
 *   Description: Setup function, Initialized LED, Serial port and
 *     task scheduler structures
 */
void setup() 
{
   pinMode(LED_PIN, OUTPUT);
   Serial.begin(9600);

  //Initialize GPIO Expander.
   if (! aw.begin(0x58))
   {
    Serial.println("AW9523 not found? Check wiring!");
   }
  pTask = getTable();
  if (NULL == pTask) {
    //Error
    while (1)
      ;
  }
  acPower.begin(VoltRange, acCurrRange, RMS_WINDOW, ADC_10BIT, BLR_ON, CNT_SCAN);
  acPower.start(); //start measuring


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


/*
 *   Function Name: blinkLED 
 * 
 *   Description: Blinks LED 1000ms
 *   
 */
void blinkLED(void) {
  static bool ledState = false;
  digitalWrite(LED_PIN, ledState);
  if (ledState == true)
    ledState = false;
  else
    ledState = true;
}