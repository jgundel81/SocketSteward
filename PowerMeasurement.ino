


int adcVal;
int cnt = 0;
// input scaling circuit in front of the ADC.





void PowerManagement_task(void) {
  readRms.publish();
  Serial.print(readRms.rmsVal, 2);
  Serial.print(", ");
  Serial.println(readRms.dcBias);
}