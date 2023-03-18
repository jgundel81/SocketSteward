/*
 *. Monito.ino
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
