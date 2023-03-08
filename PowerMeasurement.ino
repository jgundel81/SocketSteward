


int adcVal;
int cnt = 0;
// input scaling circuit in front of the ADC.





void PowerManagement_task(void) {
  readRms.publish();
  Serial.print(readRms.rmsVal, 2);
  Serial.print(", ");
  Serial.println(readRms.dcBias);
  currentRms.publish();
  Serial.print(currentRms.rmsVal, 2);
  Serial.print(", ");
  Serial.println(currentRms.dcBias);

String dataString = "";
dataString += String(readRms.rmsVal);
dataString += ",";
dataString += String(readRms.dcBias);
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