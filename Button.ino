/*
*    Button.ino
*    File to handle all Button Reading
*/

typedef struct
{
  bool buttonPressed;
  uint8_t button;
} buttonStatus_t;

//Button Structure
buttonStatus_t gButtonStatus;


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
  static bool ButtonsInited = false;
  if(false == ButtonsInited)
  {
    ButtonsInited = true;
    initButtons();
  }

  if(!digitalRead(BUTTON_A))
  {
    do
    {
      //Stay here while pressing Button
    }while(!digitalRead(BUTTON_A));
    gButtonStatus.button = BUTTON_A;
    gButtonStatus.buttonPressed = true;
    Serial.println("ButtonA");


  }
  if(!digitalRead(BUTTON_B))
  {
    do
    {
      //Stay here while pressing Button
    }while(!digitalRead(BUTTON_B));
    gButtonStatus.button = BUTTON_B;
    gButtonStatus.buttonPressed = true;
    Serial.println("ButtonB");
    
  }
  if(!digitalRead(BUTTON_C))
  {
    do
    {
      //Stay here while pressing Button
    }while(!digitalRead(BUTTON_C));
    gButtonStatus.button = BUTTON_C;
    gButtonStatus.buttonPressed = true;
    Serial.println("ButtonC");
    
  }
}