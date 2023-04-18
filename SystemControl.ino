/*
 *. SystemControl.ino
 *  See Errors.ino for definitions and bitwise macros. 
  */
/*
wire nut resistances milliohms
Solid 3.15
Wago 6.81
Lever splice 20.85
wirenut 6.55
14 ga
*/


#define INCLUDE_TIMESTAMP 0X01
#define INCLUDE_SENSORS 0X02
#define INCLUDE_STATUS 0X04

//Prototypes
void writeEventLog(String messageText);
void disconnectPower(void);

typedef struct
{
  float AVGambientTemp;
  float AVGvoltage;
  float AVGcurrent;
  float AVGimpedance;
  // Add the rest of the sensors to monitor here.
} sensor_trends_t;

sensor_trends_t gSensorTrend;


// This control task is an if/else state machine that assess sensor inputs and declares events and updates gPowerStatus which reflects the resulting machine state.
//
// The UI messaging is presently all in the EVENTS table, but a table is expected to be created for text that describes the current state.

void control_task(void) {
  //writeTrace("head of control_task()", INCLUDE_TIMESTAMP && INCLUDE_SENSORS && INCLUDE_STATUS );
  static uint32_t tick1 = 0;  //used for waiting when a pending AFGF pulse code might arrive.


  //Add some logic here to declare an event and set gPowerStatus.

  //Check for missing temp sensors
  if (gSensors.plugTemp < -20) {
    //WITHOUT SENSORS, system cannot do it's protection and we don't want it to be used.
    gLatestEvent = sensor_error;
    gPowerStatus &= ERROR_IN_SYSTEM_FLAG;
    
    // is a limp mode requested by virtue of datalogging being turned on?
    //allows a user to first turn on datalogging and then reset power and run in a "limp mode" ie, disconnect won't be forced .
    // WARNING: limp mode will be cleared and a disconnect will happen at a reboot clear limp mode so must be repeated every reboot
    if (!dataloggingEnabled) {
      disconnectPower();  // disconnecting power since this unit needs to be repaired
      gPowerStatus = DISCONNECTED_BY_ALARM & ERROR_IN_SYSTEM_FLAG;
    }

    writeEventLog("Warning: sensor(s) not providing valid data. System cannot do it's job. ");
  }

  // Checking RTC battery
  if (now.year() < 2023) {
    gLatestEvent = rtc_battery_error;
    if (gPowerStatus && CONNECTED_ANY_FLAG) {
      gPowerStatus &= CONNECTED_W_NOTIFYING_FLAG;
    } else {
      // if not connected, don't connect, and don't change power status since this didn't cause a disconnect
      // a user AFGF reset will apply power and then will be again warned of the battery problem
    }
  }


  // VOLTAGE ANALYSIS

  // check for line undervolt. THe RMS ac voltage sensing is on the output of the AFGF disconnect relay.
  if (gSensors.voltage < 10) {
    if (gPowerStatus == INITIALIZING) {  // no volts but just booted up. Might have been previously tripped and then power cycled.
      gLatestEvent = trp_no_indication;
      gPowerStatus = DISCONNECTED_AT_POWER_UP;
      writeEventLog("Volts < 10 while initializing. Assumed AFGF circuit was tripped before and needs to be reset. ");
    } else if (gPowerStatus == DISCONNECTED_UNKNOWN) {  // no volts, if still no message from AFGF blinking LEDS but those leds could take 3 seconds to read the longest blink sequence.
      if (0 == (tick1++ % 30)) {
        tick1 = 0;
        gPowerStatus = DISCONNECTED_PENDING_AFGF;
        writeEventLog("vOLTS < 10 (outlet disconnected) for 3 seconds with no AFGF codes. This could be a hardware failure.");
      }
    } else if (ANY_CONNECTED_STATUS(gPowerStatus)) {  //appliance power was connected, but now AC volts are lost? Perhaps this test occurred while AFGF blink are still being interpreted.
      gLatestEvent = unknown_trip;
      gPowerStatus = DISCONNECTED_UNKNOWN;
      writeEventLog("vOLTS < 10 when previously connected. Waiting for AFGF blink codes.");
    } else if (ANY_DISCONNECTED_STATUS(gPowerStatus)) {
      // These are all the other disconnected conditions and volts still say it is disconnected
      if(gSensors.current > 1)
      {
        gLatestEvent = sensor_error;
        gPowerStatus &= ERROR_IN_SYSTEM_FLAG;
        writeEventLog("No load-side voltage detected, yet current > 1 amp. This may be a voltage sensor failure or disconnect");
      }
      
    }
  } else if (gSensors.voltage < 90) {  // This could be a serioous voltage dip, need to add code for voltage dip analysis 
    gLatestEvent = low_voltage;
    gPowerStatus = CONNECTED_WARNING_FLAG;
    writeEventLog("Voltage dipped below 90, suggesting high resistance ");
    

  } else if (gSensors.voltage < 105) {  // this can be a typical inrush situation with compressor loads in the home

    // to distuinghish between 3rd party voltage dips and the attached appliance inrush current dips, they need to be separated.
    // if a dip follows the IR drop the attached appliance, then we focus on circuit impedance see https://docs.google.com/spreadsheets/d/1NpMZh3mdEe2BVCBC3MXAsRShZ2FT8-U_3JEMv86e1w4/edit#gid=793225366&range=O50 (story called VdropAlarm Math)





  } else if (gSensors.voltage < 125) {  // in normal operating range... where
    if(gPowerStatus && CONNECTED_ANY_FLAG){
      // Was not connected, needs to be connected and have impedance calculated
      runImpedanceTest(UPDATE_gAnalysis_impedance);
    }
    
  }

  // debug code to short out all errors
  //    gLatestEvent = no_error;
  //    gPowerStatus = CONNECTED_ANY_FLAG;
}

// END control task



void writeEventLog(String messageText) {
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
  dataString += "  EventID= ";
  dataString += String(gLatestEvent);
  dataString += "  Resulting Power Status= ";
  dataString += String(gPowerStatus);
  if (messageText != "") {
    dataString += " ";
    dataString += messageText;
  }


  Serial.println(dataString);
  File eventfile = SD.open("eventLog.csv", FILE_WRITE);
  if (eventfile) {
    eventfile.println(dataString);
  } else {
    Serial.println("unable open eventLog.csv");
    // ? should this just return here, or attempt to open the datalog file anyway? There is more SD card recovery stuff in SystemLogging.ino
  }
  if (dataloggingEnabled) {
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
    } else {
      Serial.print(datalogFileName);
      Serial.println(" could not be opened");
    }
  }
}


void disconnectPower(void) {
  aw.digitalWrite(GFI_TRIP_RELAY_PIN, HIGH);
  delay(200);  // need to confirm how long it will take to trip a GF circuit
  aw.digitalWrite(GFI_TRIP_RELAY_PIN, LOW);
  // FORCE A NEW RMS UPDATE HERE 
  if (gSensors.voltage > 10 || gSensors.current > 1) {
    writeEventLog("disconnectPower() failed to interrupt power via GF Trip relay. Check relay and trip resistor wiring.");
    writeEventLog("voltage > 10 @ " + String(gSensors.voltage));
    writeEventLog("current > 1 @ " + String(gSensors.current));
    gPowerStatus = ERROR_IN_SYSTEM_FLAG;
  }
}
