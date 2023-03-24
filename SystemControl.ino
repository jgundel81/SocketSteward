/*
 *. SystemControl.ino
 */


// This thread is meant to handle actual error processing and setting.
//Use the sensor information and add to this logic.


void control_task(void)
{

 /*
  Based on gSensors structure data make 
  Set gCurrentError on of the following
  no_error,
  thermal_runaway,
  low_voltage,
  arc_fault,
  ground_fault,
  unknown_trip,
  */

  //Add some logic here to set an error.
  gCurrentError = no_error;
  






}