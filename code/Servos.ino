// iBoardbot project

// Init servo on T4 timer. Output OC4B (Leonardo Pin10) servo1, OC4D (Leonardo Pin6) servo2, optional I2C output for servo2
// Servo2 compatible with OC4A (Leonardo Pin13)
// We configure the Timer4 for 11 bits PWM (enhacend precision) and 16.3ms period (OK for most servos)
// Resolution: 8us per step (this is OK for servos, around 175 steps for typical servo)

void initServo()
{
  int temp;
   
  // Initialize Timer4 as Fast PWM
  TCCR4A = (1<<PWM4A)|(1<<PWM4B);
  TCCR4B = 0;
  TCCR4C = (1<<PWM4D);
  TCCR4D = 0;
  TCCR4E = (1<<ENHC4); // Enhaced -> 11 bits

  // Reset servos to neutral position
  temp = 1500 >> 3;
  TC4H = temp >> 8;
  OCR4A = temp & 0xff;
  TC4H = temp >> 8;
  OCR4B = temp & 0xff;
 
  // Reset timer
  TC4H = 0;
  TCNT4 = 0;

  // Set TOP to 1023 (10 bit timer)
  TC4H = 3;
  OCR4C = 0xFF;

  // OC4A = PC7 (Pin13)  OC4B = PB6 (Pin10)   OC4D = PD7 (Pin6)
  // Set pins as outputs
  //DDRB |= (1 << 6);  // OC4B = PB6 (Pin10 on Leonardo board)
  //DDRC |= (1 << 7);  // OC4A = PC7 (Pin13 on Leonardo board)
  //DDRD |= (1 << 7);  // OC4D = PD7 (Pin6 on Leonardo board)

  //Enable OC4B output. OC4B is set when TCNT4=0 and clear when counter reach OCR4B
  //Enable servo2 on OC4A y OC4D
  TCCR4A |= (1 << COM4B1);
  TCCR4A |= (1 << COM4A1);
  TCCR4C |= (1 << COM4D1);
  // set prescaler to 256 and enable timer    16Mhz/256/1024 = 61Hz (16.3ms)
  TCCR4B = (1 << CS43) | (1 << CS40);
  // Enable Timer4 overflow interrupt and OCR4 interrupt
  TIMSK4 = (1 << TOIE4) | (1 << OCIE4A);
  
  servo1_ready=true;
  servo2_ready=true;
}

ISR(TIMER4_OVF_vect)
{
  SET(PORTD,1); // I2C servo2 output
}

ISR(TIMER4_COMPA_vect)
{
  CLR(PORTD,1);
}

void disableServo1()
{
  while (TCNT4<0xFF);   // Wait for sync...
  CLR(TCCR4A,COM4B1);
  servo1_ready=false;
}

void disableServo2()
{
  while (TCNT4<0xFF);   // Wait for sync...
  TIMSK4 = 0;
  CLR(TCCR4A,COM4A1);
  CLR(TCCR4C,COM4D1);
  servo2_ready=false;
}

void enableServo1()
{
  while (TCNT4<0xFF);   // Wait for sync...
  SET(TCCR4A,COM4B1);
  servo1_ready=true;
}

void enableServo2()
{
  while (TCNT4<0xFF);   // Wait for sync...
  TIMSK4 = (1 << TOIE4) | (1 << OCIE4A);
  SET(TCCR4A,COM4A1);
  SET(TCCR4C,COM4D1);
  servo2_ready=true;
}

/*
void stopServo()
{
  while (TCNT4<0xFF);   // Wait for sync...
  CLR(PORTD,1);
  TCCR4A = 0;
  TCCR4B = 0;
  TCCR4C = 0;
  TCCR4D = 0;
  TCCR4E = 0;
  TIMSK4 = 0;
  // Reset timer
  TC4H = 0;
  TCNT4 = 0;
  servo1_ready=false;
  servo2_ready=false;
}
*/

// move servo1 on OC4B (pin10)
void moveServo1(int pwm)
{
  pwm = constrain(pwm, SERVO_MIN_PULSEWIDTH, SERVO_MAX_PULSEWIDTH) >> 3; // Check max values and Resolution: 8us
  // 11 bits => 3 MSB bits on TC4H, LSB bits on OCR4B
  TC4H = pwm >> 8;
  OCR4B = pwm & 0xFF;
}

// move servo2 on OC4A (pin13)
void moveServo2(int pwm)
{
  pwm = constrain(pwm, SERVO_MIN_PULSEWIDTH, SERVO_MAX_PULSEWIDTH) >> 3; // Check max values and Resolution: 8us
  // 11 bits => 3 MSB bits on TC4H, LSB bits on OCR4A
  TC4H = pwm >> 8;
  OCR4A = pwm & 0xFF;
  OCR4D = pwm & 0xFF;  // New 2.1 boards servo2 output
}

