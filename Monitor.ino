/*
 *. Monito.ino
 */

//Function Prototypes
double tempInCelcius(int adcVal);
#define LRECEPTICALTEMP_PIN A4
#define RRECEPTICALTEMP_PIN A5
#define PLUGTEMP_PIN A3
#define VOLTAGE_PIN A1
#define CURRENT_PIN A2
#define AMBIENTTEMP_PIN A0

int acVolt;
int acCurr;

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

void GetValues(void)  {
  acVolt = analogRead(A1);  // read the ADC, channel for Vin
  acCurr = analogRead(A2);  // read the ADC, channel for Iin
  acPower.update(acVolt, acCurr);
}

void sensormonitor_task(void)
{
  acPower.publish();
  gSensors.voltage = acPower.rmsVal1;
  gSensors.current = acPower.rmsVal2;

  gSensors.ambientTemp = tempInCelcius(analogRead(AMBIENTTEMP_PIN));
  gSensors.plugTemp = tempInCelcius(analogRead(PLUGTEMP_PIN)) + gSensors.plugTempCorrection;
  gSensors.LRecepticalTemp = tempInCelcius(analogRead(LRECEPTICALTEMP_PIN)) + gSensors.LRecepticalTempCorrection;
  gSensors.RRecepticalTemp = tempInCelcius(analogRead(RRECEPTICALTEMP_PIN)) + gSensors.RRecepticalTempCorrection;
  //Add Temperature Sampling/ Calculations and Storing

  //Sample the other Sensors
}

double B =3380;        //Parameter of Thermistor
double T0 = 298.15;     //Room Temp in Kelvin  
double R0 = 10000;      //Resistance of Thermistor at Room Temp
double VREF = 3.3;      //VREF of ADC
double ADC_PRECISION = 1023.0; 

double tempInCelcius(int adcVal)
{
  double R;            //Calculated Resistance of Thermistor
  double temperature;  //Calculated Temp in C
  double voltage;      //Calcuated Voltage at ADC Pin
  
  voltage = ( (adcVal * VREF) / ADC_PRECISION ); 
  R = ((VREF * 10000) / voltage) - 10000;

  temperature =(1/((1/T0) + ((1/B)*log(R/R0))) - 273.15)  ;
  return temperature;
}

void readCalibrationData(void) 
{
  Serial.println("Starting readCalibrationData()");
  
  //get processor ID for reading calibration file that we don't want used on wrong processor
  uint8_t uniqueID [16]; //https://github.com/smartmeio/microcontroller-id-library/blob/master/README.md

  MicroID.getUniqueID(uniqueID, 16);

  char id [41];
  MicroID.getUniqueIDString(id);
  Serial.print("Device ID (string): ");
  Serial.println(id);
  //This should be stored in the calibration file at some point and verified  
  String calibrationString="0,0,0,0";
  
  File calibrationFile = SD.open("SenCal.csv", FILE_READ);  
  // if the file is available, read:
  if (calibrationFile) {
    char * csv_str = 0;
    //CSV_Parser cp( csv_str,  true, ',');
    
    
    
    //calibrationString = calibrationFile.readStringUntil('\n');
    
    
  }
  else
  {
    Serial.println("error opening SenCal.csv");
  }
  Serial.println("calibratinString is ");
  Serial.println(calibrationString);
  calibrationFile.close();

  
}
