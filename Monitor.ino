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

File testVector;  

//Structure to hold all Sensor information
//This will most likely need to be modified.
typedef struct
{
  int ambientTemp;
  int LRecepticalTemp;
  int RRecepticalTemp;
  int plugTemp;
  float voltage;
  float current;
  // Add the rest of the sensors to monitor here.
} system_sensors_t;

system_sensors_t gSensors;

bool readLine(File &f, char* line, size_t maxLen) {
  for (size_t n = 0; n < maxLen; n++) {
    int c = f.read();
    if ( c < 0 && n == 0) return false;  // EOF
    if (c < 0 || c == '\n') {
      line[n] = 0;
      return true;
    }
    line[n] = c;
  }
  return false; // line too long
}

bool readVals(int* v1, int* v2, int* v3, int* v4, float* v5, float* v6) {
  char line[200], *ptr, *str;
  if (!readLine(testVector, line, sizeof(line))) {
    return false;  // EOF or too long
  }
  *v1 = strtol(line, &ptr, 10);
  if (ptr == line) return false;  // bad number if equal
  while (*ptr) {
    if (*ptr++ == ',') break;
  }
  *v2 = strtol(ptr, &str, 10);
  while (*ptr) {
    if (*ptr++ == ',') break;
  }
  *v3 = strtol(ptr, &str, 10);
  while (*ptr) {
    if (*ptr++ == ',') break;
  }
  *v4 = strtol(ptr, &str, 10);
  while (*ptr) {
    if (*ptr++ == ',') break;
  }
  *v5 = strtol(ptr, &str, 10);
  while (*ptr) {
    if (*ptr++ == ',') break;
  }
  *v6 = strtol(ptr, &str, 10);
  while (*ptr) {
    if (*ptr++ == ',') break;
  }
  
  return str != ptr;  // true if number found
}

void GetValues(void)  {
    acVolt = analogRead(A1);  // read the ADC, channel for Vin
    acCurr = analogRead(A2);  // read the ADC, channel for Iin
    acPower.update(acVolt, acCurr);
  }


void sensormonitor_task(void)
{
  testVector = SD.open("TEST.CSV", FILE_READ);
  if (testVector)
  {
    Serial.println("Found test vector!");
    int pTemp, lRecTemp, rRecTemp, ambTemp;
    float volt, cur;
    readVals(&pTemp, &lRecTemp, &rRecTemp, &ambTemp, &volt, &cur);
    gSensors.plugTemp = pTemp;
    Serial.println(gSensors.plugTemp);
    gSensors.LRecepticalTemp = lRecTemp;
    Serial.println(gSensors.LRecepticalTemp);
    gSensors.RRecepticalTemp = rRecTemp;
    Serial.println(gSensors.RRecepticalTemp);
    gSensors.ambientTemp = ambTemp;
    Serial.println(gSensors.ambientTemp);
    gSensors.voltage = volt;
    Serial.println(gSensors.voltage);
    gSensors.current = cur;
    Serial.println(gSensors.current);
    testVector.close();
  }
  else
  {
    Serial.println("Failed to find test vector!");
    acPower.publish();
    gSensors.voltage = acPower.rmsVal1;
    gSensors.current = acPower.rmsVal2;
    gSensors.plugTemp = tempInCelcius(analogRead(PLUGTEMP_PIN));
    gSensors.LRecepticalTemp = tempInCelcius(analogRead(LRECEPTICALTEMP_PIN));
    gSensors.RRecepticalTemp = tempInCelcius(analogRead(RRECEPTICALTEMP_PIN));
    gSensors.ambientTemp = tempInCelcius(analogRead(AMBIENTTEMP_PIN));
  }

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
