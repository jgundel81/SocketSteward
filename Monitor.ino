/*
 *. Monitor.ino
 */

//Function Prototypes
double tempInCelcius(int adcVal);
#define LRECEPTICALTEMP_PIN A4
#define RRECEPTICALTEMP_PIN A5
#define PLUGTEMP_PIN A3
#define VOLTAGE_PIN A1
#define CURRENT_PIN A2
#define AMBIENTTEMP_PIN A0

#define CURRENT_TOO_HIGH_FOR_TEST_LOAD 10
#define MAX_APPLIANCE_AMPS_ALLOWED 10

#define UPDATE_gAnalysis_impedance true

int countGetValues = 0; // count ADC iterations

void disconnectPower(void);

#include <Ewma.h>  // uint32_t data filter
#define WATCH_IMPEDANCE_LEVEL      0.4             // 5% Vdrop DROP at 15 amps is R = 6 / 15 = 400 milliohms
#define WARNING_IMPEDANCE_LEVEL    0.64             // 8% Vdrop is 640 milliohms
#define UNACCEPTABLE_IMPEDANCE_LEVEL  0.8 
//Structure to hold all Sensor information
//This will most likely need to be modified.
typedef struct
{
  float ambientTemp;
  float LRecepticalTemp;
  float LRecepticalTempCorrection;
  float RRecepticalTemp;
  float RRecepticalTempCorrection;
  float plugTemp;
  float plugTempCorrection;
  float voltage;
  float current;
  // Add the rest of the sensors to monitor here.
} system_sensors_t;

system_sensors_t gSensors;

typedef struct
{
  float ambientTemp;
  float LRecepticalTemp;
  float LRecepticalTempCorrection;
  float RRecepticalTemp;
  float RRecepticalTempCorrection;
  float plugTemp;
  float plugTempCorrection;
  float MaxVoltage;
  float MaxCurrent;
  float impedance;
  float minImpedance;
  // Add the rest of the sensors to monitor here.
} sensor_analysis_t;

sensor_analysis_t gAnalysis;



void GetValues(void) { 
  acVoltADC = analogRead(A1);  // read the ADC, channel for Vin
  acCurrADC = analogRead(A2);  // read the ADC, channel for Iin
  acPower.update(acVoltADC, acCurrADC);
  
  //++countGetValues;
}



Ewma AmbientTempFilter(0.01);  // 0.1 = Less smoothing / 0.01 more smoothing
Ewma LTempFilter(0.01);
Ewma RTempFilter(0.01);
Ewma PlugTempFilter(0.01);

Ewma VoltFilter(0.1);   
Ewma CurFilter(0.1);


void sensormonitor_task(void) {
  //writeTrace("mon", INCLUDE_SENSORS && INCLUDE_STATUS );
  acPower.publish();  //calculates the RMS values from the series stored in GetValues()
  
  
  
  gSensors.voltage = acPower.rmsVal1;
  gSensors.current = acPower.rmsVal2;

  /*
  if (gAnalysis.impedance != 0 && millis() < 70)  // change the millis() comparison to a higher number if you want this to run 
  {
    Serial.println("this is probably obsolete now");
    float val = runImpedanceTest(UPDATE_gAnalysis_impedance);
    Serial.print("Impedance returned was ");
    Serial.println(val);
  }
  */

  // TC.restartTimer(2000); // 2 msec

  TC.stopTimer(); 
  gSensors.plugTemp = PlugTempFilter.filter(tempInCelcius(analogRead(PLUGTEMP_PIN)));
  gSensors.LRecepticalTemp = LTempFilter.filter(tempInCelcius(analogRead(LRECEPTICALTEMP_PIN)));
  gSensors.RRecepticalTemp = RTempFilter.filter(tempInCelcius(analogRead(RRECEPTICALTEMP_PIN)));
  gSensors.ambientTemp = AmbientTempFilter.filter(tempInCelcius(analogRead(AMBIENTTEMP_PIN)));
  TC.restartTimer(2000);  // 2 msec

  //writeTrace("ADC", INCLUDE_SENSORS && INCLUDE_STATUS );
  
}

double B = 3380;     //Parameter of Thermistor
double T0 = 298.15;  //Room Temp in Kelvin
double R0 = 10000;   //Resistance of Thermistor at Room Temp
double VREF = 3.3;   //VREF of ADC
double ADC_PRECISION = 1023.0;

double tempInCelcius(int adcVal) {
  double R;            //Calculated Resistance of Thermistor
  double temperature;  //Calculated Temp in C
  double voltage;      //Calcuated Voltage at ADC Pin

  voltage = ((adcVal * VREF) / ADC_PRECISION);
  R = ((VREF * 10000) / voltage) - 10000;

  temperature = (1 / ((1 / T0) + ((1 / B) * log(R / R0))) - 273.15);
  return temperature;
}

float runImpedanceTest(bool update_gAnalysis) {
  float I_underLoad, V_underLoad, R_underLoad;
  writeTrace("R1", INCLUDE_SENSORS && INCLUDE_STATUS );
  
  aw.digitalWrite(RELAY_PIN_IO_EXPANDER, LOW);
  
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
    aw.digitalWrite(RELAY_PIN_IO_EXPANDER, HIGH);  //turn on the load
    delay(50);
    acPower.publish();
    V_underLoad = acPower.rmsVal1;                // voltage under load
    I_underLoad = acPower.rmsVal2;                // current under load
    aw.digitalWrite(RELAY_PIN_IO_EXPANDER, LOW);  //turn load back off

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
