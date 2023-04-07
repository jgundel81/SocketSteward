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

// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 ambientTemp = Adafruit_MCP9808();

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

void GetValues(void)  {
  acVolt = analogRead(A1);  // read the ADC, channel for Vin
  acCurr = analogRead(A2);  // read the ADC, channel for Iin
  acPower.update(acVolt, acCurr);
}

void check_Sensor(){
   Serial.begin(9600);

   if (!ambientTemp.begin(0x18)) {
    Serial.println("Couldn't find MCP9808! Check your connections and verify the address is correct.");
   }
     else
  {
     check_Sensor = true;
  }
  Serial.println("Found MCP9808!");

}

void sensormonitor_task(void)
{
  acPower.publish();
  gSensors.voltage = acPower.rmsVal1;
  gSensors.current = acPower.rmsVal2;

  //gSensors.ambientTemp = tempInCelcius(analogRead(AMBIENTTEMP_PIN));
  gSensors.plugTemp = tempInCelcius(analogRead(PLUGTEMP_PIN));
  gSensors.LRecepticalTemp = tempInCelcius(analogRead(LRECEPTICALTEMP_PIN));
  gSensors.RRecepticalTemp = tempInCelcius(analogRead(RRECEPTICALTEMP_PIN));
  
  ambientTemp.wake();   // wake up, ready to read!
  ambientTemp.getSensor();

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
