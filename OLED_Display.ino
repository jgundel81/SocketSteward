/*
*     OLED.ino
*    File to handle all OLED 
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
//#include <vector>
//#include <CSV_Parser.h> // Install CSV Parser Library (present in Arduino IDE library manager)
using namespace std;

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
void displayOptions(error_conditions_t error);

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
  static bool OLEDInited = false;
  if(false == OLEDInited)
  {
    OLEDInited = true;
    initOLED();
  }


  switch(gDisplayState)
  {
    case dashboard:  // labeled HOME
      displayDashboard(gLatestEvent);
      if(true == gButtonStatus.buttonPressed)
      {
        gButtonStatus.buttonPressed = false;
        if(BUTTON_A == gButtonStatus.button)
        {
         
        }
        else if(BUTTON_B == gButtonStatus.button)
        {
            gDisplayState = options;
        } 
        else if(BUTTON_C == gButtonStatus.button)
        {

            gDisplayState = details;
        } 
      }
      break;
    case options:  
      displayOptions(gLatestEvent);
      if(gButtonStatus.buttonPressed)
      {
        gButtonStatus.buttonPressed = false;
        if(BUTTON_A == gButtonStatus.button)
        {
          gDisplayState = dashboard;
        }
        else if(BUTTON_B == gButtonStatus.button)
        {
          if(true == dataloggingEnabled)
            stopLogging();
          else
          {
            dataLoggingStartedTime = now;
            startLogging();

           // readCalibrationData();
          }
        } 
        else if(BUTTON_C == gButtonStatus.button)
        {
          if(dataloggingEnabled)
            gDisplayState = details;
          else
            gDisplayState = dashboard;
        } 
      }
      break;
    case details:
       displayDetails(gLatestEvent);
      if(gButtonStatus.buttonPressed)
      {
        gButtonStatus.buttonPressed = false;
        if(BUTTON_A == gButtonStatus.button)
        {
        gDisplayState = dashboard;  // secret path to home  page
        }
        else if(BUTTON_B == gButtonStatus.button)
        {
         
        } 
        else if(BUTTON_C == gButtonStatus.button)
        {
          gDisplayState = options;
        } 
      }
      break;
    
  }
}

void displayOptions(error_conditions_t error)
{

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Socket Steward J4.12");  // use top row for future "more options button "
  display.println("                     ");
  display.setCursor(0, 15);
    if(dataloggingEnabled){
      display.setCursor(0, 15);
      display.println("    datalogging: ON");
      display.setCursor(0, 30);
      display.println("< stop datalogger");
      display.setCursor(0, 50);
      display.println("< show live data    ");
    } 
    else{
      display.println("    datalogging: OFF");
      display.setCursor(0, 30);
      display.println("< start datalogger");
      display.setCursor(0, 50);
      display.println("< home              ");
   }
    
  display.display();
  
}

void displayDashboard(error_conditions_t error)
{

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Socket Steward J4.12");
  display.setCursor(0, 15);
  display.println(error_message_table[error].dashboardMsg);
  display.setCursor(0, 30);
  display.println("< datalogger controls");
  display.setCursor(0, 50);
  display.println("< show live data");
  
  display.display();
  
}

void displayDetails(error_conditions_t error)
{

  display.clearDisplay();
  display.setCursor(0, 0);
   display.println("Socket Steward  J4.12");
   if(!dataloggingEnabled && no_error != error)
  {
  display.println("                     ");
  display.println(error_message_table[error].detailedErrorMsg);
  display.println("                     ");
  display.println("< home               ");
  display.println("                     ");
  }
  else
  {
    display.clearDisplay();
    display.setCursor(0, 0); // overwrite banner with timestamp
    
    String dataString = "";
    dataString += String(now.month());
    dataString += "/";
    dataString += String(now.day());
     dataString += " ";
    dataString += String(now.hour());
    dataString += ":";
    dataString += String(now.minute());
    dataString += ":";
    dataString += String(now.second());
    if(dataloggingEnabled)
      dataString += " LOGGING";
    display.println(dataString);
   
    display.setCursor(0, 15);
    display.print("Ta ");
    display.print(gSensors.ambientTemp);
    display.print(" Tplug ");
    display.print(gSensors.plugTemp);
    
   
    display.setCursor(0, 26);
    display.print("TsL ");
    display.print(gSensors.LRecepticalTemp);
    display.print(" TsR ");
    display.print(gSensors.RRecepticalTemp);
    
    
    display.setCursor(0, 37);
   
    
    display.print("v=");
    display.print(gSensors.voltage);
    display.print(" a=");
    display.println(gSensors.current);

    
    display.setCursor(0, 50);
    dataString = "< cntrl ";
    if(dataloggingEnabled)
    {
      dataString += "t0=";
      dataString += String(dataLoggingStartedTime.hour());
      dataString += ":";
      dataString += String(dataLoggingStartedTime.minute());
      dataString += ":";
      dataString += String(dataLoggingStartedTime.second());
    }
    display.println(dataString);
    
  }
  
  display.display();
  
}




