/*
*     OLED.ino
*    File to handle all OLED 
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);


typedef enum
{
  dashboard,
  options,
  details,
} displayState_t;

displayState_t gDisplayState = dashboard;

void displayDashboard(error_conditions_t error);
void displayDetails(error_conditions_t error);

/*
*   OLED init function called once
*/
void initOLED(void) {
  display.begin(0x3C, true);  // Address 0x3C default
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  //Configure the Display
  display.setRotation(1);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.print("Initializing Display");
  display.display(); // actually display all of the above
}

/*
* OLED Task Updates the OLED Display
*/
void display_task(void) {

  static int count = 0;
  //Only init once
  static bool isInited = false;
  if(false == isInited)
  {
    isInited = true;
    initOLED();
  }


  switch(gDisplayState)
  {
    case dashboard:
      displayDashboard(gCurrentError);
      if(true == gButtonStatus.buttonPressed)
      {
        gButtonStatus.buttonPressed = false;
        if(BUTTON_A == gButtonStatus.button)
        {

        }
        else if(BUTTON_B == gButtonStatus.button)
        {

        } 
        else if(BUTTON_C == gButtonStatus.button)
        {

            gDisplayState = details;
        } 
      }
      break;
    case options:
      if(gButtonStatus.buttonPressed)
      {
        gButtonStatus.buttonPressed = false;
        if(BUTTON_A == gButtonStatus.button)
        {

        }
        else if(BUTTON_B == gButtonStatus.button)
        {

        } 
        else if(BUTTON_C == gButtonStatus.button)
        {

        } 
      }
      break;
    case details:
       displayDetails(gCurrentError);
      if(gButtonStatus.buttonPressed)
      {
        gButtonStatus.buttonPressed = false;
        if(BUTTON_A == gButtonStatus.button)
        {

        }
        else if(BUTTON_B == gButtonStatus.button)
        {

        } 
        else if(BUTTON_C == gButtonStatus.button)
        {
          gDisplayState = dashboard;
        } 
      }
      break;
    
  }
}

void displayDashboard(error_conditions_t error)
{

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(" Socket Steward 0.01 ");
  display.println("                     ");
  display.println("                     ");
  display.println(error_message_table[error].dashboardMsg);
  display.println("                     ");
  display.println("< Options            ");
  display.println("                     ");
  display.println("< Details            ");
  display.println("                     ");
  
  display.display();
  
}

void displayDetails(error_conditions_t error)
{

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(" Socket Steward 0.01 ");
  display.println("                     ");
  display.println(error_message_table[error].detailedErrorMsg);
  display.println("                     ");
  display.println("< exit               ");
  display.println("                     ");
  
  display.display();
  
}




