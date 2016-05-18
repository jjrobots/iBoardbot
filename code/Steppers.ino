// iBoardbot project

// STEPPERS MOTOR CONTROL
// SPEED, ACCELERATION AND POSITION CONTROL using Arduino 16 bit Timer interrupts

// STEPPER MOTOR PINS (NEW)
// X MOTOR: MOTOR2 output - TIMER1
//     X-STEP: 12    (PD6)
//     X-DIR:  5    (PC6)
//     X-ENABLE: 4  (P)
// Y MOTOR: MOTOR1 output - TIMER3
//     Y-STEP: 7    (PE6)
//     Y-DIR:  8    (PB4)
//     Y-ENABLE: 4 (P)

// We control the speed of the motors with interrupts (Timer1 and Timer3) tested up to 32Khz.
// The position of the motor is controlled at 1Khz (in the main loop)

// TIMER 1 : STEPPER MOTOR SPEED CONTROL X-AXIS
ISR(TIMER1_COMPA_vect)
{
  if (dir_x == 0)
    return;

  SET(PORTD, 6); // STEP X-AXIS
  position_x += dir_x;
  __asm__ __volatile__ (
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop");
  CLR(PORTD, 6);
}

// TIMER 3 : STEPPER MOTOR SPEED CONTROL Y-AXIS  (2 Motors, left and right)
ISR(TIMER3_COMPA_vect)
{
  if (dir_y == 0)
    return;

  SET(PORTE, 6); // STEP Y-AXIS (Y-left)
  position_y += dir_y;
  __asm__ __volatile__ (
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop");
  CLR(PORTE, 6);
}

// We use a ramp for acceleration and deceleration
// To calculate the point we should start to decelerate we use this formula:
// stop_position = actual_posicion + (actual_speed*actual_speed)/(2*max_deceleration)
// Input parameters:
//    target_position_x
//    target_speed_x
//    max_acceleration_x

void positionControl(int dt)
{
  //int16_t pos_stop;
  int32_t temp;

  //SET(PORTF,3); // for external timing debug
  // dt = delta time in microseconds...
  //if ((dt<400)||(dt>1500)){
    //Serial.print("dt:");
    //Serial.println(dt);
    //}
  dt = constrain(dt, 500, 2000); // Limit dt (it should be around 1000 most times)
  
  // X AXIS
  temp = (long)speed_x * speed_x;
  temp = temp / (2000 * (long)acceleration_x);
  pos_stop_x = position_x + sign(speed_x) * temp;
  if (target_position_x > position_x){ // Positive move
    if (pos_stop_x >= target_position_x){  // Start decelerating?
      //Serial.println("Aqui");
      setMotorXSpeed(0, dt);         // The deceleration ramp is done inside the setSpeed function
      }
    else{
      //Serial.println("Aqui2");
      setMotorXSpeed(target_speed_x, dt);   // The aceleration ramp is done inside the setSpeed function
      }
  }
  else{   // Negative move
    if (pos_stop_x <= target_position_x)  // Start decelerating?
      setMotorXSpeed(0, dt);
    else
      setMotorXSpeed(-target_speed_x, dt);
  }

  // Y AXIS
  temp = (long)speed_y * speed_y;
  temp = temp / (2000 * (long)acceleration_y);
  pos_stop_y = position_y + sign(speed_y) * temp;
  if (target_position_y > position_y) // Positive move
  {
    if (pos_stop_y >= target_position_y)  // Start decelerating?
      setMotorYSpeed(0, dt);         // The deceleration ramp is done inside the setSpeed function
    else
      setMotorYSpeed(target_speed_y, dt);   // The aceleration ramp is done inside the setSpeed function
  }
  else   // Negative move
  {
    if (pos_stop_y <= target_position_y)  // Start decelerating?
      setMotorYSpeed(0, dt);
    else
      setMotorYSpeed(-target_speed_y, dt);
  }
  //CLR(PORTF,3); // for external timing debug
}

// Speed could be positive or negative
void setMotorXSpeed(int16_t tspeed, int16_t dt)
{
  long timer_period;
  int16_t accel;

  // Limit max speed
  //if (tspeed > MAX_SPEED_X)
  //  tspeed = MAX_SPEED_X;
  //else if (tspeed < -MAX_SPEED_X)
  //  tspeed = -MAX_SPEED_X;
 
  // We limit acceleration => speed ramp
  accel = ((long)acceleration_x * dt) / 1000; // We divide by 1000 because dt are in microseconds
  if (((long)tspeed - speed_x) > accel) // We use long here to avoid overflow on the operation
    speed_x += accel;
  else if (((long)speed_x - tspeed) > accel)
    speed_x -= accel;
  else
    speed_x = tspeed;

  // Check if we need to change the direction pins
  if ((speed_x == 0) && (dir_x != 0))
    dir_x = 0;
  else if ((speed_x > 0) && (dir_x != 1)){
#ifdef INVERT_X_AXIS
    CLR(PORTC, 6);  // X-DIR
#else
    SET(PORTC, 6);
#endif
    dir_x = 1;
  }
  else if ((speed_x < 0) && (dir_x != -1)){
#ifdef INVERT_X_AXIS
    SET(PORTC, 6);
#else
    CLR(PORTC, 6);
#endif
    dir_x = -1;
  }

  if (speed_x == 0)
    timer_period = MINIMUN_SPEED;
  else if (speed_x > 0)
    timer_period = 2000000 / speed_x; // 2Mhz timer
  else
    timer_period = 2000000 / -speed_x;

  if (timer_period > MINIMUN_SPEED)   // Check for minimun speed (maximun period without overflow)
    timer_period = MINIMUN_SPEED;

  OCR1A = timer_period;
  // Check  if we need to reset the timer...
  if (TCNT1 > OCR1A)
    TCNT1 = timer_period-4;
}

// Speed could be positive or negative
void setMotorYSpeed(int16_t tspeed, int16_t dt)
{
  long timer_period;
  int16_t accel;

  // Limit max speed
  //if (tspeed > MAX_SPEED_Y)
  //  tspeed = MAX_SPEED_Y;
  //else if (tspeed < -MAX_SPEED_Y)
  //  tspeed = -MAX_SPEED_Y;
  //Serial.println(tspeed);

  // We limit acceleration => speed ramp
  accel = ((long)acceleration_y * dt) / 1000;
  if (((long)tspeed - speed_y) > accel)
    speed_y += accel;
  else if (((long)speed_y - tspeed) > accel)
    speed_y -= accel;
  else
    speed_y = tspeed;

  // Check if we need to change the direction pins
  if ((speed_y == 0) && (dir_y != 0))
    dir_y = 0;
  else if ((speed_y > 0) && (dir_y != 1)) {
#ifdef INVERT_Y_AXIS // Y-DIR
    CLR(PORTB, 4);
#else
    SET(PORTB, 4);
#endif
    dir_y = 1;
  }
  else if ((speed_y < 0) && (dir_y != -1)){
#ifdef INVERT_Y_AXIS  // Y-DIR 
    SET(PORTB, 4);
#else
    CLR(PORTB, 4);
#endif
    dir_y = -1;
  }

  if (speed_y == 0)
    timer_period = MINIMUN_SPEED;
  else if (speed_y > 0)
    timer_period = 2000000 / speed_y;   // 2Mhz timer ( 2000000 / 2*speed)
  else
    timer_period = 2000000 / -speed_y;

  if (timer_period > MINIMUN_SPEED)   // Check for minimun speed (maximun period without overflow)
    timer_period = MINIMUN_SPEED;

  OCR3A = timer_period;
  // Check  if we need to reset the timer...
  if (TCNT3 > OCR3A)
    TCNT3 = timer_period-4;
  //if (TCNT3 > OCR3A)
  //  TCNT3 = 0;
}

// Set Robot position in 1/10 mm.
// This function check for valid robot positions values
void setPosition_mm10(int target_x_mm10_new, int target_y_mm10_new)
{
  int target_x_mm10;
  int target_y_mm10;
  
  target_x_mm10 = target_x_mm10_new + ROBOT_OFFSET_X*10;
  target_y_mm10 = target_y_mm10_new + ROBOT_OFFSET_Y*10;
  target_x_mm10 = constrain(target_x_mm10, ROBOT_MIN_X*10, ROBOT_MAX_X*10);
  target_y_mm10 = constrain(target_y_mm10, ROBOT_MIN_Y*10, ROBOT_MAX_Y*10);
  target_position_x = (float)target_x_mm10 * (X_AXIS_STEPS_PER_UNIT / 10.0);
  target_position_y = (float)target_y_mm10 * (Y_AXIS_STEPS_PER_UNIT / 10.0);
  adjustSpeed();
}

// Adjust robot speed on each axis to achieve straight line movements... 
// for simplicity we don´t take into account accelerations, only speed
void adjustSpeed()
{
  float diff_x;
  float diff_y; 
  
  // Speed adjust to draw straight lines
  diff_x = abs((float)target_position_x - (float)position_x);
  diff_y = abs((float)target_position_y - (float)position_y) * X_Y_STEP_FACTOR;
  if (diff_x >= diff_y) { // Wich axis will be slower?
    // X axis is the main axis
    target_speed_x = max_speed_x;
    //acceleration_x = MAX_ACCEL_X;
    target_speed_y = (float)max_speed_y * diff_y / diff_x;
    //acceleration_y = (float)MAX_ACCEL_Y * diff_y / diff_x;
    //if (acceleration_y<60)
    //  acceleration_y=60;
    }
  else {
    target_speed_y = max_speed_y;
    //acceleration_y = MAX_ACCEL_Y;
    target_speed_x = (float)max_speed_x * diff_x / diff_y;
    //acceleration_x = (float)MAX_ACCEL_X * diff_x / diff_y;
    //if (acceleration_x<120)
    //  acceleration_x=120;
    }
}
/*
// Original
void adjustSpeed()
{
  float diff_x;
  float diff_y;
  
  // Speed adjust to draw straight lines
  diff_x = myAbs(target_position_x - position_x);
  diff_y = myAbs(target_position_y - position_y) * X_Y_STEP_FACTOR;
  if (diff_x > diff_y) { // Wich axis will be slower?
    target_speed_x = max_speed_x;
    target_speed_y = (float)max_speed_y * diff_y / diff_x;
    }
  else {
    target_speed_x = (float)max_speed_x * diff_x / diff_y;
    target_speed_y = max_speed_y;
    }
}
*/

// Set speed in mm/sec
void setSpeed(int target_sx, int target_sy)
{
  target_sx = constrain(target_sx * X_AXIS_STEPS_PER_UNIT, 0, MAX_SPEED_X);
  target_sy = constrain(target_sy * Y_AXIS_STEPS_PER_UNIT, 0, MAX_SPEED_Y);
  target_speed_x = target_sx;
  target_speed_y = target_sy;
}

// Set speed in steps/sec
void setSpeedS(int target_sx, int target_sy)
{
  target_sx = constrain(target_sx, 0, MAX_SPEED_X);
  target_sy = constrain(target_sy, 0, MAX_SPEED_Y);
  target_speed_x = target_sx;
  target_speed_y = target_sy;
}


