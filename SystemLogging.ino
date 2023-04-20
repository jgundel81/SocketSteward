/*
*     DataLogging.ino
*    File to handle all Datalogging 
*/


#define VREF 3300       //VREF 3.3V
#define MAX_COUNT 1024  //10 Bit 2^10




/*
*   Function to set loging flag
*/
bool startLogging() {
  if(false == gSDCardInited)
  {
    gLatestEvent = sdcard_error;
    return false;
  }
  Serial.println("Start Logging");
  dataloggingEnabled = true;
  firstRun = true;
  return true;
}

bool startCodeTracing() {
  if(false == gSDCardInited)
  {
    gLatestEvent = sdcard_error;
    return false;
  }
  Serial.println("Start Code Tracing");
  codeTracingEnabled = true;
  return true;
}



/*
*   Function to clear debug flag
*/
void stopLogging() {
  dataloggingEnabled = false;
  Serial.println("Stop Logging");
}

#define INCLUDE_TIMESTAMP 0X01
#define INCLUDE_SENSORS 0X02
#define INCLUDE_STATUS 0X04



void writeTrace(String messageText, int flags)
{
  if(!codeTracingEnabled) return;

  String dataString = String(millis());
  dataString += "\t";
  
  if(flags && INCLUDE_TIMESTAMP){
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
  }
    dataString += "\tEventID:";
    dataString += String(gLatestEvent);
    dataString += "\tStatus:";
    dataString += String(gPowerStatus);
  
    dataString += "\t";
    dataString += String(messageText);
  
  
  if(flags && INCLUDE_SENSORS){
    dataString += "\tADC#:";
    dataString += String(countGetValues);
    dataString += "\tTa:";
    dataString += String(gSensors.ambientTemp,1); //print with 1 decimal place
    dataString += "\tTl:";
    dataString += String(gSensors.LRecepticalTemp,1);
    dataString += "\tTr:";
    dataString += String(gSensors.RRecepticalTemp,1);
    dataString += "\tTp:";
    dataString += String(gSensors.plugTemp,1);
    dataString += "\tv:";
    dataString += String(gSensors.voltage,1);
    dataString += "\tI:";
    dataString += String(gSensors.current,1);
    dataString += "\t";
  }


  /*File tracefile = SD.open("trace.csv", FILE_WRITE);  
  if (tracefile)
  {
     tracefile.println(dataString);
  }
  else
  {
     Serial.println("unable open trace.csv");
     // ? should this just return here, or attempt to open the datalog file anyway? There is more SD card recovery stuff in SystemLogging.ino
  }
  */
  Serial.println(dataString);
  if(dataloggingEnabled)
  {
    String datalogFileName = "";
    datalogFileName += String(now.month());
    datalogFileName += String(now.day());
    datalogFileName += String(now.year());
    datalogFileName += ".CSV";
       
    File dataFile = SD.open(datalogFileName, FILE_WRITE);  
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
    }
    else
    {
      Serial.print(datalogFileName);
      Serial.println(" could not be opened");
    }
  }
}



/*
*  DATALOGGING TASK
* 
*/
void data_logging(void) {

  static uint32_t tick = 0;

  uint32_t analogValues[5] = { 0 };


  //ONLY do this once
  if (!gSDCardInited) 
  {
    Serial.println("Calling initSDCard from data_logging()");
    initSDCard();
  }


  if (false == dataloggingEnabled) {
    return;
  }

    //Only Log every 5 seconds
  if (0 == (tick++ % 5)) 
  {

    //Serial.println("Logging");

    
    
     
    String fileName = "";  //forces new filename each day
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
      Serial.println(" could not be opened, trying again");  //has a 2nd attempt ever worked without re-initializing the SD card? If not, delete this and keep only the third
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
        Serial.println("second attempt to open file failed. Now will reinitialize SD card and try again. ");  //this message was once seen in a datalog. Not sure why the SD card needs re-initting 
        void end();
        delay(1000); // don't know if this is needed
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
		}
      }

     //
      //Serial.println("Datalogging has been disabled. Insert SD Card and reboot");
      
      
      
    }
  }
}



