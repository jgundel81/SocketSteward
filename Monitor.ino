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



#include <Ewma.h>  // uint32_t data filter

Ewma AmbientTempFilter(0.1);  // Less smoothing - faster to detect changes, but more prone to noise
Ewma LRTempFilter2(0.1);


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

  acVoltADC = analogRead(A1);            // read the ADC, channel for Vin WARNING: If accessing outside of ISR, declare as volitile
  acCurrADC = analogRead(A2);            // read the ADC, channel for Iin
  acPower.update(acVoltADC, acCurrADC);  //adds to the RMS array
}


void sensormonitor_task(void) {
  //writeTrace("head of sensormonitor_task()", INCLUDE_TIMESTAMP && INCLUDE_SENSORS && INCLUDE_STATUS );
  acPower.publish();  //calculates the RMS values from the
  gSensors.voltage = acPower.rmsVal1;
  gSensors.current = acPower.rmsVal2;

  TC.stopTimer();
  gSensors.plugTemp = tempInCelcius(analogRead(PLUGTEMP_PIN));
  gSensors.LRecepticalTemp = tempInCelcius(analogRead(LRECEPTICALTEMP_PIN));
  gSensors.RRecepticalTemp = tempInCelcius(analogRead(RRECEPTICALTEMP_PIN));
  gSensors.ambientTemp = tempInCelcius(analogRead(AMBIENTTEMP_PIN));
  TC.restartTimer(2000);  // 2 msec

  //Sample the other Sensors
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
  
  aw.digitalWrite(RELAY_PIN_IO_EXPANDER, LOW);
  
  acPower.publish();  //is this shorting out any acquisition already running?  an immediate restart the RMS sequence like we need?
  delay(50);          //shouldn't we use the acquire flag from impedance instead?
  acPower.publish();
  gSensors.voltage = acPower.rmsVal1;
  gSensors.current = acPower.rmsVal2;  // no load voltage
  
  
  if (gSensors.current > MAX_APPLIANCE_AMPS_ALLOWED) {
    gLatestEvent = appliance_blocked_vdrop;
    Serial.println("in runImpedanceTest() The measured appliance load was too high to add another 12 amp load. Will use appliance load for impedance");
    gPowerStatus = CONNECTED_W_NOTIFYING_FLAG;
    if(update_gAnalysis == UPDATE_gAnalysis_impedance) 
    {
      gAnalysis.impedance = gSensors.voltage / gSensors.current;
    }
    return gSensors.voltage / gSensors.current;
  
  }
  if (gSensors.voltage < 100) 
  {
    gLatestEvent = low_voltage;
    // gPowerStatus = // will let alarm task decide what the status is.
    Serial.println("Voltage to low, aborting runImpedanceTest()");
    return false;
  } 
  if(true)
  {
    aw.digitalWrite(RELAY_PIN_IO_EXPANDER, HIGH);  //turn on the load
    delay(50);
    acPower.publish();
    V_underLoad = acPower.rmsVal1;                // voltage under load
    I_underLoad = acPower.rmsVal2;                // current under load
    aw.digitalWrite(RELAY_PIN_IO_EXPANDER, LOW);  //turn load back off

    Serial.println("Voltage before:");
    Serial.println(gSensors.voltage);
    Serial.println("Voltage under load:");
    Serial.println(V_underLoad);

    Serial.println("Current before:");
    Serial.println(gSensors.current);
    Serial.print("Current Through Resistor:");
    Serial.println(I_underLoad);

    Serial.println("impedance before:");
    Serial.println(gSensors.voltage / gSensors.current);

    R_underLoad = V_underLoad / I_underLoad;
    Serial.println("Impedance under load:");
    Serial.println(R_underLoad);
    if(update_gAnalysis == UPDATE_gAnalysis_impedance) 
    {
      gAnalysis.impedance = V_underLoad / I_underLoad;
    }
    return R_underLoad;
  }
}
