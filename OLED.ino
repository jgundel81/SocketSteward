/*
*     OLED.ino
*    File to handle all OLED 
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);



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
void OLED_task(void) {

  static int count = 0;
  //Only init once
  static bool isInited = false;
  if(false == isInited)
  {
    isInited = true;
    initOLED();
  }
    
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Socket Steward Logger"); //Max characters is 21
  display.setCursor(0, 10);
  display.print("A-Start"); 
  display.setCursor(0, 25);
  display.print("B-Stop");
  display.setCursor(0, 45);
  if(dataloggingEnabled)
  {
    display.print("Logging");
  }
  else
  {
    display.print("Not Logging");
  }
  display.display();

}