// iBoardBot project
#include <stdint.h>

#define SERVER_HOST "ibb.jjrobots.com"
#define SERVER_URL "http://ibb.jjrobots.com/ibbsvr/ibb.php"
 
#define MAX_PACKET_SIZE 768

// This depends on the pulley teeth and microstepping. For 20 teeth GT2 and 1/16 => 80  200*16 = 3200/(20*2mm) = 80
#define X_AXIS_STEPS_PER_UNIT 80    
// This depends on the pulley teeth and microstepping. For 42 teeth GT2 and 1/16 => 38  200*16 = 3200/(42*2mm) = 38
#define Y_AXIS_STEPS_PER_UNIT 38 
#define X_Y_STEP_FACTOR (X_AXIS_STEPS_PER_UNIT/Y_AXIS_STEPS_PER_UNIT)

// THIS VALUES DEPENDS ON THE MOTORS, PULLEYS AND ROBOT CONSTRUCTION
#define MAX_ACCEL_X 180    //max 250      // Maximun motor acceleration in (steps/seg2)/1000
#define MAX_ACCEL_Y 110    //max 250

#define MAX_SPEED_X 30000   // Maximun speed in steps/seg (max 32000)
#define MAX_SPEED_Y 15000
#define SPEED_PAINT_X 4000   //4200 // Paint speed in steps/seg
#define SPEED_PAINT_Y 1905   //1905//2100 // Paint speed in steps/seg
#define SPEED_ERASER_X 30000 // Erase speed in steps/seg
#define SPEED_ERASER_Y 15000  // Erase speed in steps/seg

// Define speeds in steps/sec
#define MAX_SPEED_X_STEPS MAX_SPEED_X*X_AXIS_STEPS_PER_UNIT
#define MAX_SPEED_Y_STEPS MAX_SPEED_Y*Y_AXIS_STEPS_PER_UNIT

// Servo definitions
#define SERVO_NEUTRO 1450  // Servo neutral position
#define SERVO_MIN_PULSEWIDTH 900
#define SERVO_MAX_PULSEWIDTH 2100
// Servo values por NoPaint, Paint and Erase servo positions...
#define SERVO1_PAINT 1800
#define SERVO1_ERASER 1080
#define SERVO2_LIFT 1290 //1300
#define SERVO2_PAINT 1660 //1650

#define SERVO_SPEED 3   //4 // Servo move speed (higher is quicker), recommended:2

// UNCOMMENT THIS LINES TO INVERT MOTORS (by default 1)
#define INVERT_X_AXIS 1
#define INVERT_Y_AXIS 1

// Offset position in mm with respect to initial robot position (0,0)
#define ROBOT_OFFSET_X 0
#define ROBOT_OFFSET_Y 0

// Robot 
#define ROBOT_MIN_X 0
#define ROBOT_MIN_Y 0
#define ROBOT_MAX_X 358
#define ROBOT_MAX_Y 123

// Eraser offset in mm
#define ERASER_OFFSET_X 0    
#define ERASER_OFFSET_Y 0

// Offset for Paint in mm
#define PAINT_OFFSET_X 0
#define PAINT_OFFSET_Y 6

// Initial robot position in mm
// The robot must be set at this position at start time (steps initialization)
#define ROBOT_INITIAL_POSITION_X 0
#define ROBOT_INITIAL_POSITION_Y 0

#define POSITION_TOLERANCE_X 30 //24 //36
#define POSITION_TOLERANCE_Y 14 //12 //18

// Utils
#define CLR(x,y) (x&=(~(1<<y)))
#define SET(x,y) (x|=(1<<y))

#define RAD2GRAD 57.2957795
#define GRAD2RAD 0.01745329251994329576923690768489

#define MINIMUN_SPEED 25000 // Fo timer counter 

#define WIFICONFIG_MAXLEN 30

// Variable definitions
// Wifi status
bool Wconnected;
uint8_t Network_errors;

// Global variables for message reading
long message_timer_init;
uint8_t message_readed;
bool message_chunked;
int message_size;
uint8_t message_status;
int message_index;
int message_index_buffer;
char mc0,mc1,mc2,mc3,mc4,mc5,mc6,mc7;  // Last 5 chars from message for parsing...
bool show_command;
long draw_init_time;

// Log and Timer variables
long loop_counter;
long timeout_counter;
long wait_counter;
int dt;
long timer_old;
long timer_value;
int debug_counter;
int stopped_counter=0;
int delay_counter=0;
int finish=0;
uint8_t erase_mode=0;
bool poll_again = false;
bool home_position = true;
bool timeout_recover=false;

// kinematic variables
// position, speed and acceleration are in step units
volatile int16_t position_x;  // This variables are modified inside the Timer interrupts
volatile int16_t position_y;

int16_t speed_x;
int16_t speed_y;
int16_t max_speed_x;
int16_t max_speed_y;

int8_t dir_x;     //(dir=1 positive, dir=-1 negative)
int8_t dir_y;  
int16_t target_position_x;
int16_t target_position_y;
int16_t target_speed_x;
int16_t target_speed_y;
int16_t acceleration_x = MAX_ACCEL_X;
int16_t acceleration_y = MAX_ACCEL_Y;

int16_t pos_stop_x;
int16_t pos_stop_y;

uint16_t com_pos_x;
uint16_t com_pos_y;
uint16_t com_speed_x;
uint16_t com_speed_y;
int16_t last_move_x;
int16_t last_move_y;

long servo_counter;
int16_t servo_pos1;
int16_t servo_pos2;
bool servo1_ready=false;
bool servo2_ready=false;

int16_t commands_index;
int16_t commands_lines;

//EEPROM object to store Wifi configuration
struct WifiConfigS{
  uint8_t status;
  char ssid[WIFICONFIG_MAXLEN];
  char pass[WIFICONFIG_MAXLEN];
  char proxy[WIFICONFIG_MAXLEN];
  unsigned int port;
};

WifiConfigS WifiConfig;

// Some util functions...
int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

int16_t myAbs(int16_t param)
{
  if (param<0)
    return -param;
  else
    return param;
}

long myAbsLong(long param)
{
  if (param<0)
    return -param;
  else
    return param;
}

int sign(int val)
{
  if (val<0)
    return(-1);
  else
    return(1);
}


