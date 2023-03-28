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

const int chipSelect = 10;

void sensormonitor_task(void) {
  Serial.println(" ");

  //Add Temperature Sampling/ Calculations and Storing

  //Sample the other Sensors
}

void initSDCard(void) {
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect))
  {
    Serial.println("Card failed, or not present");
  }
  Serial.println("card initialized.");
}

int acVolt;
int acCurr;

void GetValues(void)  {
  acVolt = analogRead(A1);  // read the ADC, channel for Vin
  acCurr = analogRead(A2);  // read the ADC, channel for Iin
  acPower.update(acVolt, acCurr);
}


void PowerManagement_task(void) {
  // run repeatedly:

  static bool isInited = false;

  if (false == isInited) {
    isInited = true;
    initSDCard();
  }


  //RmsReading.update(adcVal-512);  // without automatic baseline restoration (BLR_OFF),
  // substract a fixed DC offset in ADC-units here.
  acPower.publish();
  Serial.print(acPower.rmsVal1, 1);  // [V]
  Serial.print(", ");
  Serial.print(acPower.rmsVal2, 1);  // [A]
  Serial.print(", ");
  Serial.print(acPower.realPwr, 1);  // [P]
  Serial.print(", ");
  Serial.println(acPower.energy / 3600, 2);  // [Wh]

  String dataString = "";
  dataString += String(now.month());
  dataString += "/";
  dataString += String(now.day());
  dataString += "/";
  dataString += String(now.year());
  dataString += " ";
  dataString += String(now.hour());
  dataString += ":";
  dataString += String(now.minute());
  dataString += ":";
  dataString += String(now.second());
  dataString += ",";
  dataString += String(acPower.rmsVal1);
  dataString += ",";
  dataString += String(acPower.rmsVal2);
  dataString += ",";
  dataString += String(acPower.realPwr);
  dataString += ",";
  dataString += String(acPower.energy / 3600);

  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
  }
}
