/*
 *  Errors.ino
 */

//Types of errors
//This needs to be in the same order
//as the tables below
typedef enum
{
  no_error,
  thermal_runaway,
  low_voltage,
  ground_fault,
  arc_fault,
  trp_no_indication,          //Trippped no indication
  trp_gfci_load_gf,           //Tripped GFCI load GF
  trp_series_arc,             //Tripped series ARC
  trp_parallel_arc,           //Tripped parallel ARC
  trp_overvoltage,            //Tripped overvoltage
  trp_af_slf_tst_fail,        //Tripped AF self test failure
  trp_gfci_slf_tst_fail,      //Tripped GFCI self test failure
  hot_attch_plug,             //Hot attachment plug
  hot_receptacle,             //Hot receptacle
  no_load_at_pwr_up,          //No load at power up
  full_cap_confirmed,         //Full capacity confirmed
  mrg_cap_confirmed,          //Marginal capacity confirmed
  load_prsnt_at_pwr,          //Load present at power
  ext_volt_dips_det,          //external volt DIPS detected
  sdcard_error,               //SD Card Error
  unknown_trip,
  NUM_OF_ERRORS     //This needs to be last 
} error_conditions_t;

//Global Error Variable 
//Used for menu display
error_conditions_t gCurrentError;


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
} error_messages_t;


//Table of Error Messages
//Make sure order matches enums above
error_messages_t error_message_table[NUM_OF_ERRORS] = {
/*. Splash Screen Msg            Detailed Message to be displayed on detailed message screen.  */
  {" Power is connected  ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", }, //no_error
  {" Thermal Runaway     ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", },//thermal_runaway,
  {" Low Voltage         ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", },//low_voltage,
  {" Ground Fault        ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", },//ground_fault,
  {" Arc Fault           ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", },//arc_fault,
  {" trp_no_indication   ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", },//trp_no_indication
  {" trp_gfci_load_gf    ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", },//trp_gfci_load_gf
  {" trp_series_arc      ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", },//trp_series_arc
  {" trp_parallel_arc    ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", },//trp_parallel_arc
  {" trp_overvoltage     ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", },//trp_overvoltage
  {" trp_af_slf_tst_fail ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", },//trp_af_slf_tst_fail
  {" trp_gfci_sf_tst_fail", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", },//trp_gfci_slf_tst_fail
  {" hot_attch_plug      ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", },//hot_attch_plug
  {" hot_receptacle      ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", },//hot_receptacle
  {" no_load_at_pwr_up   ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", },//no_load_at_pwr_up
  {" full_cap_confirmed  ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", },//full_cap_confirmed
  {" mrg_cap_confirmed   ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", },//mrg_cap_confirmed
  {" load_prsnt_at_pwr   ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", },//load_prsnt_at_pwr
  {" ext_volt_dips_dect  ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", },//ext_volt_dips_dect
  {" SD Card Error       ", "    SD Card Error\r\nMake Sure SD Card\r\nInserted & Reset \r\n", },//No SD Card  
  {" Unknown Trip        ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", },//unknown_trip,
};

ledErrorBlinkPattern_t error_led_table[NUM_OF_ERRORS] = {
  {{0,0},{0,0},{0,0}}, //no_error
  {{1000,200},{3000,500},{200,800}},//thermal_runaway,
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
  {{0,0},{0,0},{0,0}},//sd card error,      
  {{0,0},{0,0},{0,0}},//unknown_trip,
};



