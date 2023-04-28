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

#define LEVER_SPLICE_MILLIOHMS 21          //https://drive.google.com/file/d/1aKQNRFilk3AV42mCHJ-DNqQHri_0caNR/view?usp=sharing
#define GA14_CABLE_MILLIOHMS_PER_FOOT 4.8  // 4.8  https://drive.google.com/file/d/1aKQNRFilk3AV42mCHJ-DNqQHri_0caNR/view?usp=sharing
                                           // at 30 ft cable + 8 receptacles (16 lever nuts) an exemplar circuit impedance would be 30 * 4.8 + 16 * 21 = 480 millohms
                                           // But assuming that unit is at the end of the line means it won't catch a too high impedance at the beginning at 144 milliohms
                                           // a future user input could define if the outlet is "close" OR "far" from the fuse box.
#define WATCH_IMPEDANCE_LEVEL 0.4          // 5% Vdrop DROP at 15 amps is R = 6 / 15 = 400 milliohms
#define WARNING_IMPEDANCE_LEVEL 0.64       // 8% Vdrop is 640 milliohms
#define UNACCAPTABLE_IMPEDANCE_LEVEL 0.8   // 10% Vdrop DROP at 15 amps is R = 12 / 15 = 800 milliohms

#define WATCH_IMPEDANCE_LEVEL      0.4             // 5% Vdrop DROP at 15 amps is R = 6 / 15 = 400 milliohms
#define WARNING_IMPEDANCE_LEVEL    0.64             // 8% Vdrop is 640 milliohms
#define UNACCEPTABLE_IMPEDANCE_LEVEL  0.8 




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
  static uint32_t tick1 = 0;    //used for waiting when a pending AFGF pulse code might arrive.
  static uint32_t dipTick = 0;  // measuring dips


  //Add some logic here to declare an event and set gPowerStatus.

  //Check for missing temp sensors
  if (gSensors.plugTemp < -20) {
    //WITHOUT SENSORS, system cannot do it's protection and we don't want it to be used.
    gLatestEvent = sensor_error;
    gPowerStatus &= ERROR_IN_SYSTEM_FLAG;
    processEvent(gLatestEvent);

    // is a limp mode requested by virtue of datalogging being turned on?
    //allows a user to first turn on datalogging and then reset power and run in a "limp mode" ie, disconnect won't be forced .
    // WARNING: limp mode will be cleared and a disconnect will happen at a reboot clear limp mode so must be repeated every reboot
    if (!dataloggingEnabled) {
      disconnectPower();  // disconnecting power since this unit needs to be repaired
      gPowerStatus = DISCONNECTED_BY_ALARM & ERROR_IN_SYSTEM_FLAG;
    }

    //writeEventLog("Warning: sensor(s) not providing valid data. System cannot do it's job. ");
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

  // check for NO VOLTAGE (thus AFGF is tripped and attached appliance power is disconnected)
  if (gSensors.voltage < 10) {

    if (gPowerStatus == INITIALIZING) {  // no volts but just booted up. Might have been previously tripped and then power cycled.
      gLatestEvent = trp_no_indication;
      gPowerStatus = DISCONNECTED_AT_POWER_UP;
    } else if (gPowerStatus == DISCONNECTED_UNKNOWN) {  // no volts, if still no message from AFGF blinking LEDS but those leds could take 3 seconds to read the longest blink sequence.
      if (0 == (tick1++ % 30)) {
        tick1 = 0;
        gPowerStatus = DISCONNECTED_PENDING_AFGF;
      }
    } else if (ANY_CONNECTED_STATUS(gPowerStatus)) {  //appliance power was connected, but now AC volts are lost? Perhaps this test occurred while AFGF blink are still being interpreted.
      gLatestEvent = unknown_trip;
      gPowerStatus = DISCONNECTED_UNKNOWN;
    } else if (ANY_DISCONNECTED_STATUS(gPowerStatus)) {
      // These are all the other disconnected conditions and volts still say it is disconnected
      if (gSensors.current > 1) {
        gLatestEvent = sensor_error;
        gPowerStatus &= ERROR_IN_SYSTEM_FLAG;
      }
    }



  } else if (gSensors.voltage < 90) {  // This could be a serious voltage dip, need to add code for voltage dip analysis
    gLatestEvent = low_voltage;
    gPowerStatus = CONNECTED_W_WARNING_FLAG;


  } else if (gSensors.voltage < 105) {  // this can be a typical inrush situation with compressor loads in the home

    // to distuinghish between 3rd party voltage dips and the attached appliance inrush current dips, they need to be separated.
    // if a dip follows the IR drop the attached appliance, then we focus on circuit impedance see https://docs.google.com/spreadsheets/d/1NpMZh3mdEe2BVCBC3MXAsRShZ2FT8-U_3JEMv86e1w4/edit#gid=793225366&range=O50 (story called VdropAlarm Math)





  } else if (gSensors.voltage < 125) {  // in normal operating range... where
    if (gPowerStatus && CONNECTED_ANY_FLAG) {
      // this is the normal state with appropriate voltages


      // runImpedanceTest(UPDATE_gAnalysis_impedance);  put this in if there was a disconnect event and this is freshly connected but not at boot time??
    }
  }

  // automatically discover a change in event and process it.
  if (gLatestEvent != gPreviousEvent) {
    gPreviousEvent = gLatestEvent;
    ++event_count[gLatestEvent];
    processEvent(gLatestEvent);
  }
  // debug code to short out all errors
  //    gLatestEvent = no_error;
  //    gPowerStatus = CONNECTED_ANY_FLAG;
}

// END control task



void disconnectPower(void) {
  aw.digitalWrite(GFI_TRIP_RELAY_PIN, HIGH);
  Serial.print("GFI Trip relay on");
  delay(200);  // need to confirm how long it will take to trip a GF circuit
   Serial.print("GFI Trip relay off");
  aw.digitalWrite(GFI_TRIP_RELAY_PIN, LOW);
  
  acPower.publish();  //is this shorting out any acquisition already running?  an immediate restart the RMS sequence like we need? https://github.com/MartinStokroos/TrueRMS/blob/master/src/TrueRMS.cpp
  delay(50);          //shouldn't we use the acquire flag from impedance instead?
  acPower.publish();
  gSensors.voltage = acPower.rmsVal1;
  gSensors.current = acPower.rmsVal2;  // no load
  if (gSensors.voltage > 10 || gSensors.current > 1) {
    writeEventLog("disconnectPower() failed to interrupt power via GF Trip relay. Check relay and trip resistor wiring.");
    writeEventLog("voltage > 10 @ " + String(gSensors.voltage));
    writeEventLog("current > 1 @ " + String(gSensors.current));
    gPowerStatus = ERROR_IN_SYSTEM_FLAG;
  }
}
