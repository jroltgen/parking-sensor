// State machine states
const unsigned char STATE_INACTIVE = 0;
const unsigned char STATE_TRACKING = 1;
const unsigned char STATE_NEARING  = 2;
const unsigned char STATE_PARKED   = 3;

unsigned char state;
unsigned char pending_state;
unsigned short time_state_entered_ms;
const unsigned short INACTIVITY_TIMEOUT_MS = 10 * 1000; // 30 seconds


// The pin used to trigger and echo on the rangefinder
const int TRIGGER_PIN = 4;
const int ECHO_PIN = 5;
const int RED_PIN = 2;
const int GREEN_PIN = 3;



// LED PWM pin, PWM pin mode not necessary
const int LED_PWM_PIN = 5;

void setup() {
  // Configure state
  state = STATE_INACTIVE;
  pending_state = STATE_INACTIVE;
  time_state_entered_ms = millis();

  // Configure red and green pins as output.
  pinMode( RED_PIN, OUTPUT );
  pinMode( GREEN_PIN, OUTPUT );
  
  // Configure trigger pin as output.
  pinMode( TRIGGER_PIN, OUTPUT );

  // Configure the echo pin as input.
  pinMode( ECHO_PIN, INPUT );


  // Lower pins
  digitalWrite( TRIGGER_PIN, LOW );
  digitalWrite( LED_PWM_PIN, LOW );

  // Configure serial port
  const long BAUD_RATE = 9600;
  Serial.begin( BAUD_RATE );
}

void loop() {
  // Trigger the pulse.
  trigger();
  
  check_state( time_reply() );
  display_state();

  // Take readings at about 5 hz
  // Note - part requires minimum 10ms delay between triggers.
  const unsigned long DELAY_MS = 200;
  delay( DELAY_MS );
}

void trigger()
{
  digitalWrite( TRIGGER_PIN, HIGH );
  delayMicroseconds( 10 );
  digitalWrite( TRIGGER_PIN, LOW );
}

float time_reply()
{
  const int MS_PER_INCH = 148; // Defined by the part datasheet 
  const unsigned long TIMEOUT_US = 100000; // 100 ms
  unsigned long duration_ms = pulseIn( ECHO_PIN, HIGH, TIMEOUT_US );

  // Return range in feet
  float range_feet = duration_ms / MS_PER_INCH / 12.0f; // 12 - inches per foot
  Serial.print( duration_ms );
  Serial.print( ", " );
  Serial.println( range_feet );
  return range_feet;
}

//void output_pwm( int duration ) {
//
//  // Lower bound to 1000, upper bound to 10000
//  if( duration < 1000 ) duration = 1000;
//  else if( duration > 10000 ) duration = 10000;
//
//  Serial.print("duration");
//  Serial.println( duration );
//  // Convert to 0-255
//  float normalized = ((float)duration - 1000 ) / 9000;
//  int pwm_value = ( 1 - normalized ) * 255;
//
//  // Invert
//  Serial.print("Analog value: ");
//  Serial.println( pwm_value );
//  analogWrite( LED_PWM_PIN, pwm_value );
//}

void check_state( float distance_ft )
{
const float TRACKING_DISTANCE = 12.0f;
const float NEARING_DISTANCE = 6.0f;
const float PARKED_DISTANCE = 3.5f;

if( distance_ft < 0.01 )
  {
  // Invalid.  Bail.
  return;  
  }
  
if( state == STATE_INACTIVE )
  {
  if( distance_ft > NEARING_DISTANCE && distance_ft < TRACKING_DISTANCE )
    {
    go_to_state( STATE_TRACKING );
    }
  }
else
  {
  if( distance_ft >= TRACKING_DISTANCE && state != STATE_INACTIVE )
    {
    go_to_state( STATE_INACTIVE );
    }
  else if( distance_ft < TRACKING_DISTANCE && distance_ft >= NEARING_DISTANCE && state != STATE_TRACKING )
    {
    go_to_state( STATE_TRACKING );
    }
  else if( distance_ft < NEARING_DISTANCE && distance_ft >= PARKED_DISTANCE && state != STATE_NEARING )
    {
    go_to_state( STATE_NEARING );
    }
  else if( distance_ft < PARKED_DISTANCE && state != STATE_PARKED )
    {
    go_to_state( STATE_PARKED );
    }
  else if( millis() - time_state_entered_ms > INACTIVITY_TIMEOUT_MS )
    {
    go_to_state( STATE_INACTIVE );
    }
  }
   
}

void go_to_state( const unsigned char new_state )
{
  static unsigned short state_cnt;
  static const unsigned short NUM_STATES_FILTER = 2;
  if( new_state != pending_state )
    {
    pending_state = new_state;
    state_cnt = 0;
    }
  state_cnt++;
  Serial.print("Cur: ");
  Serial.print( state );
  Serial.print(", New: ");
  Serial.println( new_state );

  if( state_cnt >= NUM_STATES_FILTER )
    {
    state = new_state;
    pending_state = new_state;
    time_state_entered_ms = millis();
    }
}

void display_state()
{
  switch( state )
  {
    case STATE_INACTIVE:
      digitalWrite( RED_PIN, LOW );
      digitalWrite( GREEN_PIN, LOW );
      break;

    case STATE_TRACKING:
      digitalWrite( RED_PIN, LOW );
      digitalWrite( GREEN_PIN, HIGH );
      break;

    case STATE_NEARING:
      static int current_green_state = HIGH;
      current_green_state = !current_green_state;
      digitalWrite( RED_PIN, LOW );
      digitalWrite( GREEN_PIN, current_green_state );
      break;

    case STATE_PARKED:
      digitalWrite( RED_PIN, HIGH );
      digitalWrite( GREEN_PIN, LOW );
      break;
  }
}

