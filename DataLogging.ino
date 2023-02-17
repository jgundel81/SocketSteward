/*
*     DataLogging.ino
*    File to handle all Datalogging 
*/

//Chip Select pin connected to SDCard
const int chipSelect = 10;

//Flag so the Button Thread can Start/Stop DataLogging
bool dataloggingEnabled = false;

#define VREF 3300       //VREF 3.3V
#define MAX_COUNT 1024  //10 Bit 2^10

/*
*   SD Card Initialization Function 
*
*/
void initSDCard(void) {
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect))
  {
    Serial.println("Card failed, or not present");
  }
  Serial.println("card initialized.");
}


String msg = "";

/*
*   Function to set debug flag
*/
void startLogging() {
  Serial.print("Logging");
  digitalWrite(LED_PIN,HIGH);
  dataloggingEnabled = true;
}


/*
*   Function to clear debug flag
*/
void stopLogging() {
  dataloggingEnabled = false;
  digitalWrite(LED_PIN,LOW);
  Serial.print("Stop Logging");
}




/*
*   Data logging Task
* 
*/
void data_logging(void) {
  static bool isInited = false;
  static uint32_t tick = 0;

  uint32_t analogValues[5] = { 0 };


  //ONLY do this once
  if (false == isInited) {
    isInited = true;
    initSDCard();
  }


  if (false == dataloggingEnabled) {
    return;
  }
  /*
  //Only Log every 10 seconds
  if (0 == (tick++ % 10)) 
  {

    Serial.println("Logging");

    //Read the 5 ADCs
    // This is where you can do the conversion!!
    analogValues[0] = (analogRead(A1) * VREF) / MAX_COUNT ;
    analogValues[1] = (analogRead(A2) * VREF) / MAX_COUNT ;
    analogValues[2] = (analogRead(A3) * VREF) / MAX_COUNT ;
    analogValues[3] = (analogRead(A4) * VREF) / MAX_COUNT ;
    analogValues[4] = (analogRead(A5) * VREF) / MAX_COUNT ;
    

    String dataString = "";
    dataString += String(now.month());
    dataString += "/";
    dataString += String(now.day());
    dataString += "/";
    dataString += String(now.year());
    dataString += " ";
    dataString += String(now.hour());
    dataString += ":";
    dataString += String(now.minute());
    dataString += ":";
    dataString += String(now.second());
    dataString += ",";
    dataString += String(analogValues[0]);
    dataString += ",";
    dataString += String(analogValues[1]);
    dataString += ",";
    dataString += String(analogValues[2]);
    dataString += ",";
    dataString += String(analogValues[3]);
    dataString += ",";
    dataString += String(analogValues[4]);

    File dataFile = SD.open("datalog.txt", FILE_WRITE);
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      // print to the serial port too:
      Serial.println(dataString);
    }
  }
  */
}