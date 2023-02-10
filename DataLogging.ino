/*
*     DataLogging.ino
*    File to handle all Datalogging 
*/

//Chip Select pin connected to SDCard
const int chipSelect = 10;

//Flag so the Button Thread can Start/Stop DataLogging
bool dataloggingEnabled = false;

static bool firstRun = true;

#define VREF 3300       //VREF 3.3V
#define MAX_COUNT 1024  //10 Bit 2^10
#define R2 10000        //Thermistor resistance
#define R0 10000        //Some other resistance
#define B 3380          //Beta value
#define T0 298.15       //Thermistor temp parameter

/*
*   SD Card Initialization Function 
*
*/
void initSDCard(void) {
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect))
  {
    Serial.println("Card failed, or not present");
  }
  Serial.println("card initialized.");
}


String msg = "";

/*
*   Function to set debug flag
*/
void startLogging() {
  Serial.print("Logging");
  digitalWrite(LED_PIN,HIGH);
  dataloggingEnabled = true;
}


/*
*   Function to clear debug flag
*/
void stopLogging() {
  dataloggingEnabled = false;
  Serial.print("Stop Logging");
  digitalWrite(LED_PIN,LOW);
}




/*
*   Data logging Task
* 
*/
void data_logging(void) {
  static bool isInited = false;
  static uint32_t tick = 0;

  uint32_t analogValues[5] = { 0 };
  uint32_t temps[5] = { 0 };
  uint32_t resistance[5] = { 0 };


  //ONLY do this once
  if (false == isInited) {
    isInited = true;
    initSDCard();
  }


  if (false == dataloggingEnabled) {
    return;
  }

  //Only Log every 10 seconds
  if (0 == (tick++ % 5)) 
  {

    Serial.println("Logging");

    //Read the 5 ADCs
    // This is where you can do the conversion!!
    analogValues[0] = (analogRead(A1) * VREF) / MAX_COUNT ; //voltages
    analogValues[1] = (analogRead(A2) * VREF) / MAX_COUNT ;
    analogValues[2] = (analogRead(A3) * VREF) / MAX_COUNT ;
    analogValues[3] = (analogRead(A4) * VREF) / MAX_COUNT ;
    analogValues[4] = (analogRead(A5) * VREF) / MAX_COUNT ;
    resistance[0] = ((VREF * R2) / analogValues[0]) - R2;
    resistance[1] = ((VREF * R2) / analogValues[1]) - R2;
    resistance[2] = ((VREF * R2) / analogValues[2]) - R2;
    resistance[3] = ((VREF * R2) / analogValues[3]) - R2;
    resistance[4] = ((VREF * R2) / analogValues[4]) - R2;
    temps[0] = (1 / ((1 / T0) + (1 / B) * log(resistance[0] / R0))) - 273.15;
    temps[1] = (1 / ((1 / T0) + (1 / B) * log(resistance[1] / R0))) - 273.15;
    temps[2] = (1 / ((1 / T0) + (1 / B) * log(resistance[2] / R0))) - 273.15;
    temps[3] = (1 / ((1 / T0) + (1 / B) * log(resistance[3] / R0))) - 273.15;
    temps[4] = (1 / ((1 / T0) + (1 / B) * log(resistance[4] / R0))) - 273.15;

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
    dataString += String(analogValues[0]);
    dataString += ",";
    dataString += String(analogValues[1]);
    dataString += ",";
    dataString += String(analogValues[2]);
    dataString += ",";
    dataString += String(analogValues[3]);
    dataString += ",";
    dataString += String(analogValues[4]);
    dataString += ",";
    dataString += String(temps[0]);
    dataString += ",";
    dataString += String(temps[1]);
    dataString += ",";
    dataString += String(temps[2]);
    dataString += ",";
    dataString += String(temps[3]);
    dataString += ",";
    dataString += String(temps[4]);

    String firstLine = "";
    firstLine += "Time,A0 Voltage,A1 Voltage,A2 Voltage,A3 Voltage,A4 Voltage,A0 Temp, A1 Temp, A2 Temp, A3 Temp, A4 Temp";

    File dataFile = SD.open("datalog.csv", FILE_WRITE);
    // if the file is available, write to it:
    if (dataFile) {
      if (firstRun == true){
        dataFile.println(firstLine);
        Serial.println(firstLine); //Print what each parameter is
        firstRun = false;
      }
      dataFile.println(dataString);
      dataFile.close();
      // print to the serial port too:
      Serial.println(dataString);
    }
  }
}