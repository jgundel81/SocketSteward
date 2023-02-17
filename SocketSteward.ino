
#include "RTClib.h"
#include <SPI.h>
#include <SD.h>
#include <string.h>
#include <TrueRMS.h>

#define RMS_WINDOW 50   // rms window of 50 samples, means 3 periods @60Hz

DateTime now;
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

#define LED_PIN 13

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

void data_logging(void);
void OLED_task(void);
void button_task(void);
void RTC_task(void);
void PowerManagement_task(void);


/*********    TASk Table (insert Tasks into Table **********************/
static TaskType Tasks[] = {
  { INTERVAL_1000ms, 0, RTC_task },
  { INTERVAL_500ms, 0, OLED_task },
  { INTERVAL_10ms, 0, button_task },
  { INTERVAL_1000ms, 0, data_logging },
  { INTERVAL_500ms, 0, PowerManagement_task },
};

const uint8_t numOfTasks = sizeof(Tasks) / sizeof(*Tasks);

TaskType *getTable(void) {
  return Tasks;
}



unsigned long voltageLastSample;
Rms readRms; // create an instance of Rms.
float VoltRange = 2000.00;


/*
 *   Function Name: Setup 
 * 
 *   Description: Setup function, Initialized LED, Serial port and
 *     task scheduler structures
 */
void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
  pTask = getTable();
  if (NULL == pTask) {
    //Error
    while (1)
      ;
  }
  readRms.begin(VoltRange, RMS_WINDOW, ADC_10BIT, BLR_ON, CNT_SCAN);
  readRms.start(); //start measuring


}


/*
 *   Function Name: Loop 
 * 
 *   Description: Scheduler- Calls tasks at stated intervals
 *     
 */
void loop() {
  for (taskIndex = 0; taskIndex < numOfTasks; taskIndex++) {
  
   

    if (micros() >= voltageLastSample + 1000) /* every 0.2 milli second taking 1 reading */
    {
      readRms.update(analogRead(A1)); // read the ADC.
      voltageLastSample = micros();                /* to reset the time again so that next cycle can start again*/
    }


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