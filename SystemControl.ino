/*
 *. SystemControl.ino
 */


// This thread is meant to handle actual error processing and setting.
//Use the sensor information and add to this logic.

#define MAX_15A_VDROP_PERCENT 5                                       // percent allowed voltage drop
int maxImpedanceMilliOhms = (120 * MAX_15A_VDROP_PERCENT * 10) / 15;  //this is the desired max impedance in milliohms. It will someday be field configurable, so override the #define
float circuitImpedance = 0;
float vNoLoad = 0;
bool vNoLoadRunOnce = true;

void control_task(void) {

  /*
  no_error,
  thermal_runaway,
  low_voltage,
  ground_fault,
  arc_fault,
  trp_no_indication,
  trp_gfci_load_gf,
  trp_series_arc,
  trp_parallel_arc,
  trp_overvoltage,
  trp_af_slf_tst_fail,
  trp_gfci_slf_tst_fail,
  hot_attch_plug,
  hot_receptacle,
  no_load_at_pwr_up,
  full_cap_confirmed,
  mrg_cap_confirmed,
  load_prsnt_at_pwr,
  ext_volt_dips_det,
  exc_volt_drop_load,
  null,
  unknown_trip
  */

  //Add some logic here to set an error.
  gCurrentError = no_error;

  if (low_voltage < 25) {
    gCurrentError = low_voltage;
  } else {
    gCurrentError = no_error;
  }

  if (gSensors.plugTemp > 27) {
    gCurrentError = thermal_runaway;
  } else {
    gCurrentError = no_error;
  }

  if (vNoLoadRunOnce == true) {
    if (gSensors.current < 1) {
      vNoLoad = gSensors.voltage;
      vNoLoadRunOnce = false;
    }
  }

  if (gSensors.current > 0) {
    circuitImpedance = (vNoLoad - gSensors.voltage) / gSensors.current;  //This is the present voltage drop from no-load voltage to the "under load" voltage, ie the voltage resulting from the current flow in this moment.
  }
  if (circuitImpedance > maxImpedanceMilliOhms) {
    gCurrentError = exc_volt_drop_load;
  } else {
    gCurrentError = no_error;
  }
}