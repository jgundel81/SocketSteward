/*
 *  Errors.ino
 */


/* Power Status values from the Definitions tab in https://docs.google.com/spreadsheets/d/12GRyLT-Wnm3DVdJ-egUSnQluc5JlK3vpjTF1b88WlaU/edit#gid=319617444&range=B1
*
* Bitwise testing is in use, please add new state definitions with care. See the Macros that follow.
* Please DO NOT set and clear individual bits. The resulting power status won't make sense. Just use or define a new power status. 
* There is only ONE power status, the latest set by the last EVENT that happened. (presently called an error)
*/

// All connected states have bit position 4 set ie, Ox10 through 0x1F to allow bitwise tests. 
// 
#define CONNECTED_ANY_FLAG          0x10  // Use to test connected state, or to set connected before capacity is known
#define	CONNECTED_W_NORMAL_FLAG	    0x11  // this is the believed FULL 15 amp capacity of the entire circuit
#define	CONNECTED_W_NOTIFYING_FLAG	0x12  // use if user maintenance is requested, perhaps network link down, etc. yet system is fully operational
#define	CONNECTED_W_WATCH_FLAG      0x14  // use if power quality seems below NEC voltage drop goals or other impairments might be at hand. 
#define	CONNECTED_W_WARNING_FLAG      0x18  // the system believes the power capacity is not sufficient for continuous loading, but should be OK with electronic, brief or low power continuous loading such as LED lighting.
// all Disconnected states have bit position 5 set and are exclusive. 
#define	DISCONNECTED_BY_ALARM     	0x20  // Software in this file decided to disconnect power
#define	DISCONNECTED_BY_REQUEST   	0x21  // The user wanted power disconnected - perhaps by remote control or smartphone app
#define	DISCONNECTED_BY_AFGF	      0x22  // the AFGF circuit told us it disconnected power via LED blink codes
#define	DISCONNECTED_PENDING_AFGF   0x23  // voltage suddenly lost yet software still powered up AND AFGF HAS NOT SAID ANYTHING YET
#define	DISCONNECTED_AT_POWER_UP  	0x24  // during initialization the AFGF is believed tripped when powered up
#define	DISCONNECTED_UNKNOWN	      0x25  // this is not fully unexpected yet ... perhaps AFGF blink codes or other functions need time. Should be a temporary state. 
// other states where being connected or not is either pending or uncertain should NOT have a code that sets bit 4 or 5!
#define	INITIALIZING	              0x00
#define	ERROR_IN_SYSTEM_FLAG        0x80 // used if system issues are present, or being connected or disconnected is not certain. 

#define PRESERVE_LIMITATION_FLAGS   0x10
#define OVERRIDE_LIMITATION_FLAGS   0x20

// Bitwise test macros. To clarify how bitwise tests are being used, please add new macros here and avoid in-line tests elsewhere. 
#define ANY_CONNECTED_STATUS(x) (x & (1UL << 4) )  
#define ANY_DISCONNECTED_STATUS(x) (x & (1UL << 5) ) 



//Types of events that occur  and result in a change of gPowerStatus.
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
  appliance_blocked_vdrop,          //Load present at power
  ext_volt_dips_det,          //external volt DIPS detected
  boot_up,
  sdcard_error,
  test_load_error,            //SD Card Error
  unknown_trip,
  sensor_error,
  rtc_battery_error,
  NUM_OF_ERRORS     //This needs to be last 
} error_conditions_t;

//Global Error Variable 
//Used for menu display
error_conditions_t gLatestEvent = boot_up;
error_conditions_t gPreviousEvent = boot_up;  
int gPowerStatus = INITIALIZING;


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
  const char *spokenMsgFile;
} error_messages_t;

u_int32_t event_count[NUM_OF_ERRORS];

// Table of Error Messages  (these are EVENTS, many not being true errors. 
// When they occur, the gPowerStatus is typically changed to a new status. Each occurance generates a new line in event log file)
// Make sure order matches enums above
error_messages_t error_message_table[NUM_OF_ERRORS] = {
/*. Splash Screen Msg            Detailed Message to be displayed on detailed message screen.  */
  {" Power is connected  ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "beep.wav"}, //no_error
  {" Thermal Runaway     ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TR.wav" },//thermal_runaway,
  {" Low Voltage         ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav"},//low_voltage,
  {" Ground Fault        ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "GF.wav"},//ground_fault,
  {" Arc Fault           ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav"},//arc_fault,
  {" trp_no_indication   ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav"},//trp_no_indication
  {" trp_gfci_load_gf    ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav"},//trp_gfci_load_gf
  {" trp_series_arc      ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav"},//trp_series_arc
  {" trp_parallel_arc    ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav"},//trp_parallel_arc
  {" trp_overvoltage     ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav"},//trp_overvoltage
  {" trp_af_slf_tst_fail ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav"},//trp_af_slf_tst_fail
  {" trp_gfci_sf_tst_fail", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav"},//trp_gfci_slf_tst_fail
  {" hot_attch_plug      ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav"},//hot_attch_plug
  {" hot_receptacle      ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav"},//hot_receptacle
  {" no_load_at_pwr_up   ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav"},//no_load_at_pwr_up
  {" full_cap_confirmed  ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav"},//full_cap_confirmed
  {" mrg_cap_confirmed   ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav"},//mrg_cap_confirmed
  {" load_prsnt_at_pwr   ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav"},//load_prsnt_at_pwr
  {" ext_volt_dips_dect  ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav"},//ext_volt_dips_dect
  {" Initializing System ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav"},//boot_up  
  {" SD Card Error       ", "    SD Card Error\r\nMake Sure SD Card\r\nInserted & Reset \r\n", "TBD.wav"},//No SD Card  
  {" Test Load Error     ", "    Test resistor\r\ncurrent too low  \r\nand needs repair \r\n", "TBD.wav"},//No SD Card  
  {" Unknown Trip        ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", "TBD.wav"},//unknown_trip,
  {" NOT FULLY PROTECTED ", "  SENSOR FAILURE \r\n CONTACT SUPPORT \r\n SYSTEM IMPAIRED \r\n", "TBD.wav"},//sensor_error
  {" Clock Batt Warning  ", "Logging timestamp\r\ntime wrong. Check\r\nCR1220 battery.  \r\n", "TBD.wav"},//clock bat
 
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
  {{0,0},{0,0},{0,0}},//boot_up,
  {{0,0},{0,0},{0,0}},//sd card error,      
  {{0,0},{0,0},{0,0}},//unknown_trip,
  {{0,0},{0,0},{0,0}},//sensor_error,
  {{0,0},{0,0},{0,0}},//clock_bat
};



