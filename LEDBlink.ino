/*
 *.   LEDBlink.ino
 *.   Manages blink patterns
 */ 

 //Pins on the GPIO Expander
#define RED_LED_PIN 8
#define AMBER_LED_PIN 9
#define GREEN_LED_PIN 10



void initLEDs() {
  aw.pinMode(RED_LED_PIN, OUTPUT);
  aw.pinMode(AMBER_LED_PIN, OUTPUT);
  aw.pinMode(GREEN_LED_PIN, OUTPUT);

  //Active Low
  aw.digitalWrite(RED_LED_PIN, HIGH);
  aw.digitalWrite(AMBER_LED_PIN, HIGH);
  aw.digitalWrite(GREEN_LED_PIN, HIGH);
}





void blinkpattern_task() {
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
  if(!((error_led_table[gCurrentError].red.onTime == 0)&&(error_led_table[gCurrentError].red.offTime == 0)))
  {
    if (true == redState) 
    {
      if (redTick >= error_led_table[gCurrentError].red.onTime) 
      {
        aw.digitalWrite(RED_LED_PIN, HIGH);
        redTick = 0;
        redState = false;
      }
      else
        aw.digitalWrite(RED_LED_PIN, LOW);
    } 
    else 
    {
      if (redTick >= error_led_table[gCurrentError].red.offTime) 
      {
        aw.digitalWrite(RED_LED_PIN, LOW);
        redTick = 0;
        redState = true;
      }
      else
        aw.digitalWrite(RED_LED_PIN, HIGH);
    }
  }

  //Amber LED 
  if(!((error_led_table[gCurrentError].amber.onTime == 0)&&(error_led_table[gCurrentError].amber.offTime == 0)))
  {
    if (true == amberState) 
    {
      if (amberTick >= error_led_table[gCurrentError].amber.onTime) 
      {
        aw.digitalWrite(AMBER_LED_PIN, HIGH);
        amberTick = 0;
        amberState = false;
      }
      else
        aw.digitalWrite(AMBER_LED_PIN, LOW);
    } 
    else 
    {
      if (amberTick >= error_led_table[gCurrentError].amber.offTime) 
      {
        aw.digitalWrite(AMBER_LED_PIN, LOW);
        amberTick = 0;
        amberState = true;
      }
      else
        aw.digitalWrite(AMBER_LED_PIN, HIGH);
    }
  }

    //Green LED 
  if(!((error_led_table[gCurrentError].green.onTime == 0)&&(error_led_table[gCurrentError].green.offTime == 0)))
  {
    if (true == greenState) 
    {
      if (greenTick >= error_led_table[gCurrentError].green.onTime) 
      {
        aw.digitalWrite(GREEN_LED_PIN, HIGH);
        greenTick = 0;
        greenState = false;
        
      }
      else
        aw.digitalWrite(GREEN_LED_PIN, LOW);
    } 
    else 
    {
      if (greenTick >= error_led_table[gCurrentError].green.offTime) 
      {
        aw.digitalWrite(GREEN_LED_PIN, LOW);
        greenTick = 0;
        greenState = true;
        
      }
      else
        aw.digitalWrite(GREEN_LED_PIN, HIGH);
    }
  }



  redTick += 100; //100ms Tick
  greenTick += 100; //100ms Tick
  amberTick += 100; //100ms Tick
}