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
  Serial.println(" initSDCard() called");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect))
  {
    Serial.println("Card failed, or not present");
    //SDInited=false;
  }
  else
  {
    Serial.println("Card initialized.");
  }
  
}


String msg = "";

/*
*   Function to set debug flag
*/
void startLogging() {
  Serial.println("Start Logging");
  dataloggingEnabled = true;
  firstRun = true;
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
  static bool SDInited = false;
  static uint32_t tick = 0;

  uint32_t analogValues[5] = { 0 };


  //ONLY do this once
  if (false == SDInited) 
  {
    SDInited = true;
    Serial.println("Calling initSDCard from data_logging() and setting SDInited to true");
    initSDCard();
  }


  if (false == dataloggingEnabled) {
    return;
  }

    //Only Log every 5 seconds
  if (0 == (tick++ % 5)) 
  {

    //Serial.println("Logging");

    //Read the 5 ADCs
    // This is where you can do the conversion!!
     
    String fileName = "";
    fileName += String(now.month());
    fileName += String(now.day());
    fileName += String(now.year());
    fileName += ".CSV";
    
    String dataString = "";
      
    File dataFile = SD.open(fileName, FILE_WRITE);  
    // if the file is available, write to it:
    if (dataFile) {
      if(firstRun == true)
      {
          String firstLine = "Date,Time,Tambient,TleftBlade,TrightBlade,Tplug,Vrms,Irms";
          dataFile.println(firstLine);
          Serial.println(firstLine);
          firstRun = false;
      }
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
      dataString += ",";
      dataString += String(gSensors.current);
      dataFile.println(dataString);
      dataFile.close();
      // print to the serial port too:
      Serial.println(dataString);
    }
    else
    {
      Serial.print(fileName);
      Serial.println(" could not be opened, trying again");
      File dataFile = SD.open(fileName, FILE_WRITE);  
      if (dataFile) {
        dataFile.println("2nd try to open and write was beeded");
        dataFile.println(dataString);
        dataFile.close();
        // print to the serial port too:
        Serial.println(dataString);
      }
      else
      {
        Serial.println("second attempt to open file failed. Now will reinitialize SD card and try again. ");
        void end();
        delay(1000);
        initSDCard();
        File dataFile = SD.open(fileName, FILE_WRITE);  
        if (dataFile) {
          dataFile.println("3rd try to open and write worked after void and reinit");
          Serial.println("3rd try to open and write worked after void and reinit");
          dataFile.println(dataString);
          dataFile.close();
          // print to the serial port too:
          Serial.println(dataString);
        }
        else
        {
         Serial.println("unable to open even after reinitializing the SD card. Must stop datalogging ");
         void end();
         dataloggingEnabled = false;
         SDInited = false;
        }
      }

     //
      //Serial.println("Datalogging has been disabled. Insert SD Card and reboot");
      
      
      
    }

    
    
  }
}
