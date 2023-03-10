
/*

int adcVal;
int cnt = 0;
// input scaling circuit in front of the ADC.


 *
 * File: Energy.ino
 * Purpose: TrueRMS library example project
 * Version: 1.0.1
 * Modified: 18-05-2020
 * Date: 10-10-2019
 * URL: https://github.com/MartinStokroos/TrueRMS
 * License: MIT License
 *
 *
 * This is an example of a complete AC-power meter application, measuring energy, apparent power, real power, power factor,
 * rms voltage and current. Use external electronic circuitry to scale down the AC-voltage or current to be 
 * compliant with the ADC input voltage range of 0-5V. To measure line voltages, use a differential amplifier+level 
 * shifter circuit with resistive voltage dividers at the input. Use for example a current sensor with a Hall sensor
 * based on the zero flux method. ALWAYS USE AN ISOLATION TRANSFORMER FOR SAFETY!
 * 
 * The RMS_WINDOW defines the number of samples used to calculate the RMS-value. The length of the RMS_WINDOW must 
 * be specified as a whole number and must fit at least one cycle of the base frequency of the input signal.
 * If RMS_WINDOW + sample-rate does not match with the fundamental frequency of the input signal(s), slow fluctuations 
 * in the rms values and power readings will occure.
 *
*/

#include <TrueRMS.h>

#define LPERIOD 2000    // loop period time in us. In this case 2.0ms
//#define RMS_WINDOW 50   // rms window of 50 samples, means 6 periods @60Hz

unsigned long nextLoop;
int acVolt;
int acCurr;
int cnt=0;
float acVoltRange = 2000; // peak-to-peak voltage scaled down to 0-5V is 700V (=700/2*sqrt(2) = 247.5Vrms max).
float acCurrRange = 3.3; // peak-to-peak current scaled down to 0-5V is 5A (=5/2*sqrt(2) = 1.77Arms max).

Power acPower;  // create an instance of Power


void setup() {  // run once:
	Serial.begin(115200);
  // configure for automatic base-line restoration and continuous scan mode:
	acPower.begin(acVoltRange, acCurrRange, RMS_WINDOW, ADC_10BIT, BLR_ON, CNT_SCAN);
  
  acPower.start(); //start measuring
  
	nextLoop = micros() + LPERIOD; // Set the loop timer variable for the next loop interval.
	}



void loop() {
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



/*
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
*/