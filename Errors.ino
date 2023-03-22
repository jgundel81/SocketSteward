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
  {" Unknown Trip        ", "    Line 1       \r\n    Line 2       \r\n    Line 3       \r\n", },//unknown_trip,
};

ledErrorBlinkPattern_t error_led_table[NUM_OF_ERRORS] = {
  {{0,0},{0,0},{0,0}}, //no_error
  {{1000,200},{3000,500},{200,800}},//thermal_runaway,
  {{0,0},{0,0},{0,0}},//low_voltage,
  {{0,0},{0,0},{0,0}},//ground_fault,
  {{0,0},{0,0},{0,0}},//arc_fault,
  {{0,0},{0,0},{0,0}},//unknown_trip,
};



