#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);


void initOLED(void) {
  display.begin(0x3C, true);  // Address 0x3C default
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  display.setRotation(1);
  display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.print("Initializing Display");
    display.display(); // actually display all of the above
}

void OLED_task(void) {

  static int count = 0;
  //Only init once
  static bool isInited = false;
  if(false == isInited)
  {
    isInited = true;
    initOLED();
  

    
  }
    // Clear the buffer.
      display.clearDisplay();
        display.display();

  display.setCursor(0, 0);
<<<<<<< Updated upstream
  display.print(now.month(),10);
  display.print("/");
  display.print(now.day(),10);
  display.print("/");
  display.print(now.year(),10);
=======
  display.print("Socket Steward Logger"); //Max characters is 21
>>>>>>> Stashed changes
  display.setCursor(0, 10);
  display.print("(");
  display.print(daysOfTheWeek[now.dayOfTheWeek()]);
  display.print(") ");
  display.print(now.hour(), 10);
  display.print(':');
  display.print(now.minute(), 10);
  display.print(':');
  display.print(now.second(), 10);
  display.display();
}
