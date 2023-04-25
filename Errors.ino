/*
 *  Errors.ino
 */







//Blink Pattern
typedef struct
{
  uint16_t onTime;
  uint16_t offTime;
} ledBlink_t;

//LED alarm Structure 
//to hold all LED Patterns 
typedef struct
{
  ledBlink_t red;
  ledBlink_t amber;
  ledBlink_t green;
} ledErrorBlinkPattern_t;

//Structure for Error Messages
typedef struct
{
  const char *dashboardMsg;
  const char *detailedErrorMsg;
  const char *soundFile;
  const char *extendedErrorMsg;
} error_messages_t;

u_int32_t event_count[NUM_OF_ERRORS];

// Table of Error Messages  (these are EVENTS, many not being true errors. 
// When they occur, the gPowerStatus is typically changed to a new status. Each occurance generates a new line in event log file)
// Make sure order matches enums above
error_messages_t error_message_table[NUM_OF_ERRORS] = {
/*. Splash Screen Msg            Detailed Message to be displayed on detailed message screen.  Sound        Message to log*/
  {" Power is connected  ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "beep.wav",""}, //no_error
  {" Thermal Runaway     ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TR.wav",""},//thermal_runaway,
  {" Low Voltage         ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav","Voltage dipped below 90, suggesting high resistance."},//low_voltage,
  {" Ground Fault        ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "GF.wav",""},//ground_fault,
  {" Arc Fault           ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav",""},//arc_fault,
  {" trp_no_indication   ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav","Volts < 10 while initializing. Assumed AFGF circuit was tripped before and needs to be reset."},//trp_no_indication
  {" trp_gfci_load_gf    ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav",""},//trp_gfci_load_gf
  {" trp_series_arc      ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav",""},//trp_series_arc
  {" trp_parallel_arc    ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav",""},//trp_parallel_arc
  {" trp_overvoltage     ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav",""},//trp_overvoltage
  {" trp_af_slf_tst_fail ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav",""},//trp_af_slf_tst_fail
  {" trp_gfci_sf_tst_fail", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav",""},//trp_gfci_slf_tst_fail
  {" hot_attch_plug      ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav",""},//hot_attch_plug
  {" hot_receptacle      ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav",""},//hot_receptacle
  {" no_load_at_pwr_up   ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav",""},//no_load_at_pwr_up
  {" full_cap_confirmed  ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav",""},//full_cap_confirmed
  {" mrg_cap_confirmed   ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav",""},//mrg_cap_confirmed
  {" load_prsnt_at_pwr   ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav",""},//load_prsnt_at_pwr
  {" ext_volt_dips_dect  ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav",""},//ext_volt_dips_dect
  {" Initializing System ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav",""},//boot_up  
  {" SD Card Error       ", "    SD Card Error\r\nMake Sure SD Card\r\nInserted & Reset \r\n", "TBD.wav",""},//No SD Card  
  {" Test Load Error     ", "    Test resistor\r\ncurrent too low  \r\nand needs repair \r\n", "TBD.wav",""},//No SD Card  
  {" Unknown Trip        ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav","Volts < 10 when previously connected. Waiting for AFGF blink codes."},//unknown_trip,
  {" Sensor Error        ", "  SENSOR FAILURE \r\n CONTACT SUPPORT \r\n SYSTEM IMPAIRED \r\n", "TBD.wav","No load-side voltage detected, yet current > 1 amp. This may be a voltage sensor failure or disconnect."},//sensor_error
  {" Clock Batt Warning  ", "Logging timestamp\r\ntime wrong. Check\r\nCR1220 battery.  \r\n", "TBD.wav",""},//clock bat
 
};

ledErrorBlinkPattern_t error_led_table[NUM_OF_ERRORS] = {
  {{0,0},{0,0},{0,0}}, //no_error
  {{0,0},{0,0},{0,0}},//thermal_runaway,
  {{0,0},{0,0},{0,0}},//low_voltage,
  {{0,0},{0,0},{0,0}},//ground_fault,
  {{0,0},{0,0},{0,0}},//arc_fault,
  {{0,0},{0,0},{0,0}},//trp_no_indication,
  {{0,0},{0,0},{0,0}},//trp_gfci_load_gf,
  {{0,0},{0,0},{0,0}},//trp_series_arc,
  {{0,0},{0,0},{0,0}},//trp_parallel_arc,
  {{0,0},{0,0},{0,0}},//trp_overvoltage,
  {{0,0},{0,0},{0,0}},//trp_af_slf_tst_fail,
  {{0,0},{0,0},{0,0}},//trp_gfci_slf_tst_fail,
  {{0,0},{0,0},{0,0}},//hot_attch_plug,
  {{0,0},{0,0},{0,0}},//hot_receptacle,
  {{0,0},{0,0},{0,0}},//no_load_at_pwr_up,
  {{0,0},{0,0},{0,0}},//full_cap_confirmed,
  {{0,0},{0,0},{0,0}},//mrg_cap_confirmed,
  {{0,0},{0,0},{0,0}},//load_prsnt_at_pwr,
  {{0,0},{0,0},{0,0}},//ext_volt_dips_det,
  {{0,0},{0,0},{0,0}},//boot_up,
  {{0,0},{0,0},{0,0}},//sd card error,      
  {{0,0},{0,0},{0,0}},//unknown_trip,
  {{0,0},{0,0},{0,0}},//sensor_error,
  {{0,0},{0,0},{0,0}},//clock_bat
};



