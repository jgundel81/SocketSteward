/*
* MonLED.ino file to monitor
* AFGF receptacle LEDS and to set alarm states
*/


Adafruit_AW9523 aw;

#define AFGF_REDLED  0
#define AFGF_YELLOWLED  1

int numOfBlinks;
int redLedCnt = 0;
int yellowLedCnt = 0;

// 
void initLedMonitor(void)
{
  if (! aw.begin(0x58)) {
    Serial.println("AW9523 not found? Check wiring!");
  }
   aw.pinMode(AFGF_REDLED,INPUT);
   aw.pinMode(AFGF_YELLOWLED,INPUT);
}
 



/*
*   LedMonitor task
*  
*/ 
void LedMonitorTask(void)
{
  static bool isInited = false;
  if(false == isInited)
  {
    isInited = true;
   initLedMonitor();
  }
 if( aw.digitalRead(AFGF_REDLED))
 {

   Serial.println("Red Led");
 }

  if(aw.digitalRead(AFGF_YELLOWLED))
  {
    Serial.println("Yellow Led");
  }
}