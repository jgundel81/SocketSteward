/*
 *. SystemControl.ino
 */


// This thread is meant to handle actual error processing and setting.
//Use the sensor information and add to this logic.

error_conditions_t previousError = no_error;
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

//   if(low_voltage < 25){

//     gCurrentError = low_voltage;
    
//   }
//   else 
// {  
//   gCurrentError = no_error;

// }

    //only log error when it happens once
    if(previousError != gCurrentError)
    {
      if(true == system_log(error_message_table[gCurrentError].dashboardMsg))
      {
         previousError = gCurrentError;
      }
    }




}