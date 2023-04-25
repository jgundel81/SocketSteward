












float runImpedanceTest(bool update_gAnalysis) {
  float I_underLoad, V_underLoad, R_underLoad;
  writeTrace("R1", INCLUDE_SENSORS && INCLUDE_STATUS );
  
  aw.digitalWrite(TEST_LOAD_RELAY_PIN, LOW);
  
  acPower.publish();  //is this shorting out any acquisition already running?  an immediate restart the RMS sequence like we need? https://github.com/MartinStokroos/TrueRMS/blob/master/src/TrueRMS.cpp
  delay(50);          //shouldn't we use the acquire flag from impedance instead?
  acPower.publish();
  gSensors.voltage = acPower.rmsVal1;
  gSensors.current = acPower.rmsVal2;  // no load voltage
  writeTrace("R2", INCLUDE_SENSORS && INCLUDE_STATUS );
  
  
  if (gSensors.current > MAX_APPLIANCE_AMPS_ALLOWED) {
    gLatestEvent = appliance_blocked_vdrop;
    Serial.println("in runImpedanceTest() The measured appliance load was too high to add another 12 amp load. Will use appliance load for impedance");
    gPowerStatus = CONNECTED_W_NOTIFYING_FLAG;
    if(update_gAnalysis == UPDATE_gAnalysis_impedance) 
    {
      gAnalysis.impedance = gSensors.voltage / gSensors.current;
    }
    writeTrace("R3", INCLUDE_SENSORS && INCLUDE_STATUS );
  
    return gSensors.voltage / gSensors.current;
  
  }
  if (gSensors.voltage < 100) 
  {
    writeTrace("R4", INCLUDE_SENSORS && INCLUDE_STATUS );
  
    gLatestEvent = low_voltage;
    // gPowerStatus = // will let alarm task decide what the status is.
    Serial.println("Voltage to low, aborting runImpedanceTest()");
    return false;
  } 
  if(true)  // not yet aborted
  {
    Serial.println("Test Load turning ON at mSec: ");
    Serial.println(millis());
    aw.digitalWrite(TEST_LOAD_RELAY_PIN, HIGH);  //turn on the load
        delay(50);
    acPower.publish();
    V_underLoad = acPower.rmsVal1;                // voltage under load
    I_underLoad = acPower.rmsVal2;                // current under load
    aw.digitalWrite(TEST_LOAD_RELAY_PIN, LOW);  //turn load back off
    Serial.print("Test Load off at mSec: ");
    Serial.println(millis());

    Serial.print(" V before:");
    Serial.print(gSensors.voltage,1);
    Serial.print(" Vload:");
    Serial.print(V_underLoad,1);

    Serial.print(" Current:");
    Serial.print(gSensors.current,1);
    Serial.print(" Current Through Resistor:");
    Serial.print(I_underLoad,1);

    Serial.print(" Impedance before:");
    Serial.print(gSensors.voltage / gSensors.current,1);

    R_underLoad = V_underLoad / I_underLoad;
    Serial.print("  Impedance after:");
    Serial.println(R_underLoad,1);

    if(I_underLoad < MAX_APPLIANCE_AMPS_ALLOWED){
      // dummy load AND appliance load all too low for some reason.
      gLatestEvent = test_load_error;
      gPowerStatus |= ERROR_IN_SYSTEM_FLAG; 
      Serial.println("Current during impedance test was below expected test current. ");
      return false;
    } 

    if(update_gAnalysis == UPDATE_gAnalysis_impedance) 
    {
      if(R_underLoad > 50){
        Serial.println("Calculated resistance makes no sense because we should have caught it already");
      }
      else{
        gAnalysis.impedance = R_underLoad;
        if(R_underLoad < WATCH_IMPEDANCE_LEVEL ){
          gPowerStatus = CONNECTED_W_NORMAL_FLAG; // this overwrites any other flags. 
          gLatestEvent = full_cap_confirmed;
        }
        else if(R_underLoad < WARNING_IMPEDANCE_LEVEL){
          gPowerStatus = CONNECTED_W_WATCH_FLAG; // this overwrites any other flags. 
          gLatestEvent = mrg_cap_confirmed;

        }
        else if(R_underLoad < UNACCEPTABLE_IMPEDANCE_LEVEL){
          // NEED SOME TREND MONITORING HERE TO SEE IF IMPEDANCE IS CLIMBING
          gPowerStatus = CONNECTED_W_WARNING_FLAG; // this overwrites any other flags. 
          gLatestEvent = mrg_cap_confirmed;
        }
        else {
          disconnectPower();
          gPowerStatus = CONNECTED_W_WARNING_FLAG; // this overwrites any other flags. 
          gLatestEvent = mrg_cap_confirmed;
          

        }
      }
      
    }
    return R_underLoad;
  }
}