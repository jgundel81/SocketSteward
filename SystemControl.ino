/*
 *. SystemControl.ino
 *  See Errors.ino for definitions and bitwise macros. 
  */



//Prototypes
void logEvent(String messageText);
void disconnectPower(void);


// This control task is a spaghetti code state machine meant to handle actual event  processing and setting.
// The EVENTS were coded without use of POWER STATUS. At this moment in April 13, 2023 I am using the new POWER STATUS to indicate the machine state, 
// and the EVENTS to be what moves this state to another state. 
// The UI messaging is presently all in the EVENTS table, but a table is expected to be created for text that describes the current state. 
void control_task(void)
{

  static uint32_t tick1 = 0; //used when waiting for AFGF pulse code to arrive. 
 
 /*
  Based on gSensors structure data make 
  Set gLatestEvent on of the following
  */
  
  //Add some logic here to declare an event and set gPowerStatus.

  //Check for missing temp sensors
  if(gSensors.plugTemp < -20)
  {
    gLatestEvent = sensor_error;
    gPowerStatus = ERROR_IN_SYSTEM;
    logEvent("one or more RTC temp sensor(s) are not connected");
    disconnectPower();
  }

  
  // VOLTAGE ANALYSIS
  
  // check for line undervolt. THe RMS ac voltage sensing is on the output of the AFGF disconnect relay. 
  if(gSensors.voltage < 10)
  {
    if(gPowerStatus == INITIALIZING){  // no volts but just booted up. Might have been previously tripped and then power cycled. 
    gLatestEvent = trp_no_indication;
    gPowerStatus = DISCONNECTED_AT_POWER_UP;
    logEvent("Volts < 10 while initializing. Assumed AFGF circuit was tripped before and needs to be reset. ");
    }
    else if (gPowerStatus == DISCONNECTED_UNKNOWN){  // no volts, if still no message from AFGF blinking LEDS but those leds could take 3 seconds to read the longest blink sequence.
      if (0 == (tick1++ % 30)) {
        tick1 = 0;
        gPowerStatus = DISCONNECTED_UNEXPECTED;
        logEvent("vOLTS < 10 (outlet disconnected) for 3 seconds with no AFGF codes. This could be a hardware failure.");
      }
    }
    else if(ANY_CONNECTED_STATUS(gPowerStatus))  {  //appliance power was connected, but now AC volts are lost? Perhaps this test occurred while AFGF blink are still being interpreted.
      gLatestEvent = trp_no_indication;
      gPowerStatus = DISCONNECTED_UNKNOWN;
      logEvent("vOLTS < 10 when previously connected. Waiting for AFGF blink codes.");
    }
     else if (ANY_DISCONNECTED_STATUS(gPowerStatus)){
      // These are all the other disconnected conditions 
    }
  }
  else if (gSensors.voltage < 90){   // This could be a voltage dip
      
    
  }
  else if (gSensors.voltage < 105){

  }
 else if (gSensors.voltage < 125){
    
  }



  
}

// END control task



void logEvent(String messageText)
{
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
      dataString += "EventID=";
      dataString += String(gLatestEvent);
      dataString += "new Power Status=";
      dataString += String(gPowerStatus);
      if(messageText != "")
      {
        dataString += " ";
        dataString += messageText;
      }

  
  Serial.println(messageText);
  File eventfile = SD.open("eventLog.csv", FILE_WRITE);  
  if (eventfile)
  {
     eventfile.println(dataString);
  }
  else
  {
     Serial.println("unable open eventLog.csv");
     // ? should this just return here, or attempt to open the datalog file anyway? There is more SD card recovery stuff in SystemLogging.ino
  }
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


void disconnectPower(void)
{
  aw.digitalWrite(GFI_TRIP_RELAY_PIN, HIGH);
  delay(200);  // need to confirm how long it will take to trip a GF circuit
  aw.digitalWrite(GFI_TRIP_RELAY_PIN, LOW);
  delay(200);
  if(gSensors.voltage > 10 || gSensors.current > 1){
    logEvent("disconnectPower() failed to interrupt power via GF Trip relay. Check relay and trip resistor wiring.");
    logEvent("voltage > 10 @ " + String(gSensors.voltage));
    logEvent("current > 1 @ " + String(gSensors.current));
    gPowerStatus = ERROR_IN_SYSTEM;
  }
}
