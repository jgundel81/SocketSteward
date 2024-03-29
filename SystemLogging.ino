/*
*     DataLogging.ino
*    File to handle all Datalogging 
*/

//Chip Select pin connected to SDCard
const int chipSelect = 10;

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
  else
  {
     gSDCardInited = true;
  }
  Serial.println("card initialized.");
}


String msg = "";

/*
*   Function to set debug flag
*/
bool startLogging() {
  if(false == gSDCardInited)
  {
    gCurrentError = sdcard_error;
    return false;
  }
  Serial.println("Start Logging");
  dataloggingEnabled = true;
  return true;
}


/*
*   Function to clear debug flag
*/
void stopLogging() {
  dataloggingEnabled = false;
  Serial.println("Stop Logging");
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

  //Only Log every 10 seconds
  if (0 == (tick++ % 10)) 
  {

    //Serial.println("Logging");

    //Read the 5 ADCs
    // This is where you can do the conversion!!
     
    String fileName = "";
    fileName += String(now.month());
    fileName += "-";
    fileName += String(now.day());
    fileName += "-";
    fileName += String(now.year());
    fileName += ".CSV";
   
    File dataFile = SD.open(fileName, FILE_WRITE);  
    // if the file is available, write to it:
    if (dataFile) 
    {
      String dataString = "";
      dataString += String(now.month());
      dataString += "/";
      dataString += String(now.day());
      dataString += "/";
      dataString += String(now.year());
      dataString += ",";
      dataString += String(now.hour());
      dataString += ":";
      dataString += String(now.minute());
      dataString += ":";
      dataString += String(now.second());
      dataString += ",";
      dataString += String(gSensors.ambientTemp);  //now build the rest of the timestamped entry
      dataString += ",";
      dataString += String(gSensors.LRecepticalTemp);
      dataString += ",";
      dataString += String(gSensors.RRecepticalTemp);
      dataString += ",";
      dataString += String(gSensors.plugTemp);
      dataString += ",";
      dataString += String(gSensors.voltage);
      dataString += " V,";
      dataString += String(gSensors.current);
      dataString += " A";
      dataFile.println(dataString);
      dataFile.close();
      // print to the serial port too:
      Serial.println(dataString);
    }
    else
    {
      Serial.print(fileName);
      Serial.println(" could not be opened");
      dataloggingEnabled = false;
      gCurrentError = sdcard_error;

      Serial.println("Datalogging has been disabled. Insert SD Card and reboot");
    }
  }
}

bool system_log(String msg)
{
  if(false == gSDCardInited)
  {
    return false;
  }

   String fileName = "AlarmLog.txt";
    
   
    File dataFile = SD.open(fileName, FILE_WRITE);  
    // if the file is available, write to it:
    if (dataFile) 
    {
      String dataString = "";
      dataString += String(now.month());
      dataString += "/";
      dataString += String(now.day());
      dataString += "/";
      dataString += String(now.year());
      dataString += ",";
      dataString += String(now.hour());
      dataString += ":";
      dataString += String(now.minute());
      dataString += ":";
      dataString += String(now.second());
      dataString += ":";
      dataString += msg;
      dataFile.println(dataString);
      dataFile.close();
      // print to the serial port too:
      Serial.println(dataString);
      return true;
    }
    Serial.print("Failed to write Log: ");
    Serial.println(msg);
    return false;
}

