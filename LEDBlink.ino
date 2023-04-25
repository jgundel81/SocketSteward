/*
 *.   LEDBlink.ino
 *.   Manages blink patterns
 */ 

 //Pins on the GPIO Expander
#define RED_LED_PIN 8
#define AMBER_LED_PIN 9
#define GREEN_LED_PIN 10
#define AMBER_AFGF_INDICATOR_INPUT_PIN 0
#define RED_AFGF_INDICATOR_INPUT_PIN 1
#define GFI_TRIP_RELAY_PIN 12
#define TEST_LOAD_RELAY_PIN 13
#define WHITE_ALARM_LED_PIN 2


//Remember active low so higher number is dimmer
#define LED_DIM_LEVEL 100
//0-255 Value for 0-100%

void initLEDs() {

  aw.pinMode(RED_LED_PIN, AW9523_LED_MODE);
  aw.pinMode(AMBER_LED_PIN, AW9523_LED_MODE);
  aw.pinMode(GREEN_LED_PIN, AW9523_LED_MODE);
  aw.pinMode(GFI_TRIP_RELAY_PIN, OUTPUT);  
  aw.pinMode(TEST_LOAD_RELAY_PIN, OUTPUT);
  aw.pinMode(WHITE_ALARM_LED_PIN, OUTPUT);
  aw.pinMode(AMBER_AFGF_INDICATOR_INPUT_PIN, INPUT);
  aw.pinMode(RED_AFGF_INDICATOR_INPUT_PIN, INPUT);


  aw.analogWrite(RED_LED_PIN, 0);
  aw.analogWrite(AMBER_LED_PIN, 0);
  aw.analogWrite(GREEN_LED_PIN, 0);
}





void blinkpattern_task() {     //SCHEDULED TO RUN EVERY 100 MS
  static bool isInited = false;
  static bool redState = false;
  static bool amberState = false;
  static bool greenState = false;
  static uint32_t redTick = 0;
  static uint32_t amberTick = 0;
  static uint32_t greenTick = 0;
  //ONLY do this once
  if (false == isInited) {
    isInited = true;
    initLEDs();
  }

  //Red LED 
  if(!((error_led_table[gLatestEvent].red.onTime == 0)&&(error_led_table[gLatestEvent].red.offTime == 0)))
  {
    if (true == redState) 
    {
      if (redTick >= error_led_table[gLatestEvent].red.onTime) 
      {
        aw.analogWrite(RED_LED_PIN, 0);
        Serial.println("R=false");
        redTick = 0;
        redState = false;
      }
      else
        aw.analogWrite(RED_LED_PIN, LED_DIM_LEVEL);
        Serial.println("R1=DIMLVL");
    } 
    else 
    {
      if (redTick >= error_led_table[gLatestEvent].red.offTime) 
      {
        aw.analogWrite(RED_LED_PIN, LED_DIM_LEVEL);
        Serial.println("R2=true");
        redTick = 0;
        redState = true;
      }
      else
        aw.analogWrite(RED_LED_PIN, 0);
        Serial.println("R3=0");
    }
  }
  else
  {
    aw.analogWrite(RED_LED_PIN, 0);
    //Serial.println("R4=0");
  }

  //Amber LED 
  if(!((error_led_table[gLatestEvent].amber.onTime == 0)&&(error_led_table[gLatestEvent].amber.offTime == 0)))
  {
    if (true == amberState) 
    {
      if (amberTick >= error_led_table[gLatestEvent].amber.onTime) 
      {
        aw.analogWrite(AMBER_LED_PIN, 0);
        Serial.println("A1=0");
        amberTick = 0;
        amberState = false;
      }
      else
        aw.analogWrite(AMBER_LED_PIN, LED_DIM_LEVEL);
        Serial.println("A2=DIMLVL");
    } 
    else 
    {
      if (amberTick >= error_led_table[gLatestEvent].amber.offTime) 
      {
        aw.analogWrite(AMBER_LED_PIN, LED_DIM_LEVEL);
        Serial.println("A3=DIMLVL");
        amberTick = 0;
        amberState = true;
      }
      else
        aw.analogWrite(AMBER_LED_PIN, 0);
        Serial.println("A4=0");
    }
  }
  else
  {
    aw.analogWrite(AMBER_LED_PIN, 0);
    //Serial.println("A5=0");
  }

    //Green LED 
  if(!((error_led_table[gLatestEvent].green.onTime == 0)&&(error_led_table[gLatestEvent].green.offTime == 0)))
  {
    if (true == greenState) 
    {
      if (greenTick >= error_led_table[gLatestEvent].green.onTime) 
      {
        aw.analogWrite(GREEN_LED_PIN, 0);
        Serial.println("G1=0");
        greenTick = 0;
        greenState = false;
        
      }
      else
        aw.analogWrite(GREEN_LED_PIN, LED_DIM_LEVEL);
        Serial.println("G2=DIMLVL");
    } 
    else 
    {
      if (greenTick >= error_led_table[gLatestEvent].green.offTime) 
      {
        aw.analogWrite(GREEN_LED_PIN, LED_DIM_LEVEL);
        Serial.println("G3=DIMLVL");
        greenTick = 0;
        greenState = true;
        
      }
      else
        aw.analogWrite(GREEN_LED_PIN, 0);
        Serial.println("G4=0");
    }
  }
  else
  {
        aw.analogWrite(GREEN_LED_PIN, 0);
        //Serial.println("G5=0");
  }



  redTick += 100; //100ms Tick
  greenTick += 100; //100ms Tick
  amberTick += 100; //100ms Tick
}