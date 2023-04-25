/*
 *. Monitor.ino THIS ONLY COLLECTS SENSOR DATA. Processing is done in SystemControl.ino
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

int countGetValues = 0;  // count ADC iterations




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
  bool gfciTrip;
  uint8_t afciBlinks;
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



void GFCI_AFCI_task(void) {
  static bool PrevRedState = false;
  static bool PrevAmberState = false;
  static long long lastAmberTick = millis();  // JOE, why long long instead of Unsigned long? And since this wraps in 50 days,
                                              // should we do something to make sure we don't have a counter overrun issue?
                                              // if lastAmberTick was a 24 bit
                                              // Can we stuff millis() with 2^24 - 240000 to force a wrap in the first 4 minutes of code execution?
                                              // (just to make this an immediate visible bug catcher?)
  static unsigned long lastRedTick = millis();
  static uint8_t amberCounter = 0;
  static uint8_t redCounter = 0;


  if (HIGH == aw.digitalRead(RED_AFGF_INDICATOR_INPUT_PIN)) {
    aw.analogWrite(RED_LED_PIN, LED_DIM_LEVEL);  //Red on, turn on LED
    if (false == PrevRedState) {
      PrevRedState = true;

      redCounter++;
      lastRedTick = millis();
    }
  } else {
    PrevRedState = false;
    aw.analogWrite(RED_LED_PIN, 0);  // red not on set LED off and Set previous state false
  }





  if (redCounter > 5) {
    Serial.print("More than 5 blinks:");
    Serial.println(redCounter);
    redCounter = 0;
  }


  // AMBER
  if (HIGH == aw.digitalRead(AMBER_AFGF_INDICATOR_INPUT_PIN)) {
    aw.analogWrite(AMBER_LED_PIN, LED_DIM_LEVEL);  //amber on, turn on LED
    if (false == PrevAmberState) {
      PrevAmberState = true;

      amberCounter++;
      lastAmberTick = millis();
    }
  } else {
    PrevAmberState = false;
    aw.analogWrite(AMBER_LED_PIN, 0);  // amber not on set LED off and Set previous state false
  }

  if (millis() > (lastAmberTick + 700))  //this belongs in SystemLogging but the millis count is important to address the solid-on condition. Fix so it is rollover safe http://www.gammon.com.au/millis
  {

    switch (amberCounter) {
      case 0:
        break;
      case 1:  //solid on but this happens at 700ms is that ok?
       
          gLatestEvent = trp_gfci_load_gf;
          gPowerStatus = DISCONNECTED_BY_AFGF;
          Serial.println("GFCI Trip (amber on > 700 mS sec");
        
        // being on less than 2 seconds happens at boot, and when human is pressing reset button, so it is ignored
        break;

      case 2:
        gLatestEvent = trp_series_arc;
        gPowerStatus = DISCONNECTED_BY_AFGF;
        break;

      case 3:
        gLatestEvent = trp_parallel_arc;
        gPowerStatus = DISCONNECTED_BY_AFGF;
        break;
        break;
      case 4:
        gLatestEvent = trp_overvoltage;
        gPowerStatus = DISCONNECTED_BY_AFGF;
        break;
      case 5:
        gLatestEvent = trp_af_slf_tst_fail;
        gPowerStatus = DISCONNECTED_BY_AFGF;
        break;
      default:
        Serial.println("more than 5 blinks from AFGF Amber detected");
        break;
    }



    if (amberCounter > 0) {
      Serial.print("Amber blinks:");
      Serial.println(amberCounter);
      gSensors.afciBlinks = amberCounter;
      amberCounter = 0;
    }
  }
}
