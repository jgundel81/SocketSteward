/*
*    Button.ino
*    File to handle all Button Reading
*/


//Functions to set Logging from Button Presses
void startLogging();
void stopLogging();

#define BUTTON_A  9
#define BUTTON_B  6
#define BUTTON_C  5

void initButtons(void)
{
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  Serial.begin(9600);
}

/*
*   Button task
*   Gets called every 100ms
*/ 
void button_task(void)
{
  static bool isInited = false;
  if(false == isInited)
  {
    isInited = true;
    initButtons();
  }

  if(!digitalRead(BUTTON_A))
  {
    do
    {
      //Stay here while pressing Button
    }while(!digitalRead(BUTTON_A));

    startLogging();
    Serial.println("Button A Pressed!");
  }
  if(!digitalRead(BUTTON_B))
  {
    do
    {
      //Stay here while pressing Button
    }while(!digitalRead(BUTTON_B));

    stopLogging();
    Serial.println("Button B Pressed!");
  }
  if(!digitalRead(BUTTON_C))
  {
    Serial.println("Button C Pressed!");
  }
}