/*
 *. Monitor.ino
 */


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


void sensormonitor_task(void)
{
  readRms.publish();
  gSensors.voltage = readRms.rmsVal;

  //Add Temperature Sampling/ Calculations and Storing

  //Sample the other Sensors
}


#define LPERIOD 2000    // loop period time in us. In this case 2.0ms
//#define RMS_WINDOW 50   // rms window of 50 samples, means 6 periods @60Hz

unsigned long nextLoop;
int acVolt;
int acCurr;
int cnt=0;
float acVoltRange = 2000; // peak-to-peak voltage scaled down to 0-5V is 700V (=700/2*sqrt(2) = 247.5Vrms max).
float acCurrRange = 3.3; // peak-to-peak current scaled down to 0-5V is 5A (=5/2*sqrt(2) = 1.77Arms max).

Power acPower;  // create an instance of Power


void setup2() {  // run once:
  // configure for automatic base-line restoration and continuous scan mode:
	acPower.begin(acVoltRange, acCurrRange, RMS_WINDOW, ADC_10BIT, BLR_ON, CNT_SCAN);
  
  acPower.start(); //start measuring
  
	nextLoop = micros() + LPERIOD; // Set the loop timer variable for the next loop interval.
	}



void loop2() {
	// run repeatedly:
	acVolt = analogRead(A1); // read the ADC, channel for Vin
	acCurr = analogRead(A2); // read the ADC, channel for Iin
	acPower.update(acVolt, acCurr);
	//RmsReading.update(adcVal-512);  // without automatic baseline restoration (BLR_OFF), 
	                                  // substract a fixed DC offset in ADC-units here.
	cnt++;
	if(cnt >= 500) { // publish every sec
		acPower.publish();
		Serial.print(acPower.rmsVal1, 1); // [V]
		Serial.print(", ");
		Serial.print(acPower.rmsVal2, 1); // [A]
		Serial.print(", ");
		Serial.print(acPower.realPwr, 1); // [P]
		Serial.print(", ");
		Serial.println(acPower.energy/3600, 2); // [Wh]
		cnt=0;
	}

	while(nextLoop > micros());  // wait until the end of the time interval
	nextLoop += LPERIOD;  // set next loop time to current time + LOOP_PERIOD
}
// end of Energy.ino




void PowerManagement_task(void) {
  readRms.publish();
  Serial.println("Result:");
  Serial.print(readRms.rmsVal, 2);
  Serial.print(", ");
  Serial.println(readRms.dcBias);
  Serial.println("");
  currentRms.publish();
  Serial.print(currentRms.rmsVal, 2);
  Serial.print(", ");
  Serial.println(currentRms.dcBias);
  Serial.println("");

String dataString = "";
dataString += String(readRms.rmsVal);
dataString += ",";
dataString += String(readRms.dcBias);
dataString += ",";
dataString += String(currentRms.rmsVal);
dataString += ",";
dataString += String(currentRms.dcBias);

  File dataFile = SD.open("datalog.csv", FILE_WRITE);
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
    }
}