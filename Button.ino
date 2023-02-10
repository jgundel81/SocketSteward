
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
<<<<<<< Updated upstream
    Serial.println("Button A Pressed!");
  }
  if(!digitalRead(BUTTON_B))
  {
    Serial.println("Button B Pressed!");
=======
    do
    {
      //Stay here while pressing Button
    }while(!digitalRead(BUTTON_A));

    startLogging();
  }
  if(!digitalRead(BUTTON_B))
  {
    do
    {
      //Stay here while pressing Button
    }while(!digitalRead(BUTTON_B));

    stopLogging();
>>>>>>> Stashed changes
  }
  if(!digitalRead(BUTTON_C))
  {
    Serial.println("Button C Pressed!");
  }
}