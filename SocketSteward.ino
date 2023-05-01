


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
#include <Ewma.h>  // uint32_t data filter
//#include <AudioZero.h> compile errors

Adafruit_AW9523 aw;

#define RMS_WINDOW 50   // rms window of 50 samples, means 3 periods @60Hz
#define INCLUDE_TIMESTAMP 0X01   //Used for writeTrace()
#define INCLUDE_SENSORS 0X02
#define INCLUDE_STATUS 0X04

/* Power Status values from the Definitions tab in https://docs.google.com/spreadsheets/d/12GRyLT-Wnm3DVdJ-egUSnQluc5JlK3vpjTF1b88WlaU/edit#gid=319617444&range=B1
*
* Bitwise testing is in use, please add new state definitions with care. See the Macros that follow.
* Please DO NOT set and clear individual bits. The resulting power status won't make sense. Just use or define a new power status. 
* There is only ONE power status, the latest set by the last EVENT that happened. (presently called an error)
*/

// All connected states have bit position 4 set ie, Ox10 through 0x1F to allow bitwise tests. 
// 
#define CONNECTED_ANY_FLAG          0x10  // Use to test connected state, or to set connected before capacity is known
#define	CONNECTED_W_NORMAL_FLAG	    0x11  // this is the believed FULL 15 amp capacity of the entire circuit
#define	CONNECTED_W_NOTIFYING_FLAG	0x12  // use if user maintenance is requested, perhaps network link down, etc. yet system is fully operational
#define	CONNECTED_W_WATCH_FLAG      0x14  // use if power quality seems below NEC voltage drop goals or other impairments might be at hand. 
#define	CONNECTED_W_WARNING_FLAG      0x18  // the system believes the power capacity is not sufficient for continuous loading, but should be OK with electronic, brief or low power continuous loading such as LED lighting.
// all Disconnected states have bit position 5 set and are exclusive. 
#define	DISCONNECTED_BY_ALARM     	0x20  // Software in this file decided to disconnect power
#define	DISCONNECTED_BY_REQUEST   	0x21  // The user wanted power disconnected - perhaps by remote control or smartphone app
#define	DISCONNECTED_BY_AFGF	      0x22  // the AFGF circuit told us it disconnected power via LED blink codes
#define	DISCONNECTED_PENDING_AFGF   0x23  // voltage suddenly lost yet software still powered up AND AFGF HAS NOT SAID ANYTHING YET
#define	DISCONNECTED_AT_POWER_UP  	0x24  // during initialization the AFGF is believed tripped when powered up
#define	DISCONNECTED_UNKNOWN	      0x25  // this is not fully unexpected yet ... perhaps AFGF blink codes or other functions need time. Should be a temporary state. 
// other states where being connected or not is either pending or uncertain should NOT have a code that sets bit 4 or 5!
#define	INITIALIZING	              0x00
#define	ERROR_IN_SYSTEM_FLAG        0x80 // used if system issues are present, or being connected or disconnected is not certain. 

#define PRESERVE_LIMITATION_FLAGS   0x10
#define OVERRIDE_LIMITATION_FLAGS   0x20




//Prototypes
void writeEventLog(String messageText);
void disconnectPower(void);




DateTime now;
DateTime dataLoggingStartedTime;
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

#define LED_PIN 13
#define TEST_LOAD_RELAY_PIN 13
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
#define INTERVAL_50ms 50

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
void GFCI_AFCI_task(void);

void GetValues(void);


/*********    TASK Table (insert Tasks into Table **********************/
static TaskType Tasks[] = {
  { INTERVAL_1000ms, 0, RTC_task },
  { INTERVAL_500ms, 0, display_task },
  { INTERVAL_10ms, 0, button_task },
  { INTERVAL_100ms, 0, control_task },  //if changing control task, please also change static tick1 to maintain 3 second timer
  { INTERVAL_500ms, 0, sensormonitor_task },
  { INTERVAL_100ms, 0, blinkpattern_task}, 
  { INTERVAL_50ms,  0, GFCI_AFCI_task},

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
 *   Function Name: Setup 
 * 
 *   Description: Setup function, Initialized LED, Serial port and
 *     task scheduler structures
 */
void setup() 
{
   pinMode(LED_PIN, OUTPUT);
   Serial.begin(250000);
   delay(1000);
   if(Serial) {
     delay(5000);  //give the slowpoke IDE time to feel connected. the prot will remain false if no USB is connected (I think)
   }
   else Serial.println("Serial is false");

  Serial.println("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");  //will form feed clear the screen? https://stackoverflow.com/questions/10105666/clearing-the-terminal-screen
   Serial.print("This is Socket Steward sy millis() is ");
   Serial.println(millis());


  //Initialize GPIO Expander.
   
   if (!aw.begin(0x58))
   {
    Serial.print(" Check connection.. AW9523 GPIO didn't initialize at millis = ");
    Serial.println(millis());
     }
   else{
     Serial.println("AW9523 GPIO Expander found, thank you");
   }
   
   initSDCard();

   // AudioZero.begin(44100);  //needs wav files formatted Using Audacity: Tracks > Stereo Track to Mono    Project Rate (HZ) > set to 44100    File > Export > Save as type: Other uncompressed files > Options...    Select WAV, Unsigned 8 bit PCM    

  pTask = getTable();
  if (NULL == pTask) {
    //Error
    while (1)
      ;
  }
  aw.pinMode(TEST_LOAD_RELAY_PIN, OUTPUT);
  aw.digitalWrite(TEST_LOAD_RELAY_PIN, LOW);
  
  //writeTrace("RMS0", INCLUDE_SENSORS && INCLUDE_STATUS );
   
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