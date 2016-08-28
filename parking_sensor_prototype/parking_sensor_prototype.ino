// State machine states
const unsigned char STATE_INACTIVE = 0;
const unsigned char STATE_TRACKING = 1;
const unsigned char STATE_NEARING  = 2;
const unsigned char STATE_PARKED   = 3;

unsigned char state;
unsigned short time_state_entered_ms;
const unsigned short INACTIVITY_TIMEOUT_MS = 30 * 1000; // 30 seconds


// The pin used to trigger and echo on the rangefinder
const int TRIGGER_PIN = 4;
const int ECHO_PIN = 5;
const int RED_PIN = 2;
const int GREEN_PIN = 3;

// Defined by the part datasheet 
const int MS_PER_INCH = 148;

// LED PWM pin, PWM pin mode not necessary
const int LED_PWM_PIN = 5;

void setup() {
  // Configure state
  state = STATE_INACTIVE;
  time_state_entered_ms = millis();
  
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

  // Time the reply.
  const unsigned long TIMEOUT_MS = 40000; // 40 mS
  unsigned long duration_ms = pulseIn( ECHO_PIN, HIGH, TIMEOUT_MS );


  
  // Output to console
  float range_feet = duration_ms / MS_PER_INCH / 12.0f; // 12 - inches per foot
  Serial.println( range_feet );
  
  check_state( range_feet );
  handle_state();

  // Convert to PWM - range bound to 1000 to 10,000
  //output_pwm( duration );

  // Take readings at about 10 hz
  // Note - part requires minimum 10ms delay between triggers.
  const unsigned long DELAY_MS = 250;
  delay( DELAY_MS );
}

void trigger() {
  digitalWrite( TRIGGER_PIN, HIGH );
  delayMicroseconds( 10 );
  digitalWrite( TRIGGER_PIN, LOW );
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
const float TRACKING_DISTANCE = 10.0f;
const float NEARING_DISTANCE = 5.0f;
const float PARKED_DISTANCE = 3.5f;
  
  switch( state )
  {
    case STATE_INACTIVE:
      if( distance_ft > NEARING_DISTANCE && distance_ft < TRACKING_DISTANCE )
        {
        go_to_state( STATE_TRACKING );
        }
      break;

    case STATE_TRACKING:
      if( distance_ft > TRACKING_DISTANCE )
        {
        go_to_state( STATE_INACTIVE );
        }
      else if( distance_ft < NEARING_DISTANCE )
        {
        go_to_state( STATE_NEARING );
        }
      else
        {
        check_inactive();
        }
      break;

    case STATE_NEARING:
      if( distance_ft > NEARING_DISTANCE )
        {
        go_to_state( STATE_TRACKING );
        }
      else if( distance_ft < PARKED_DISTANCE )
        {
        go_to_state( STATE_PARKED );
        }
      else
        {
        check_inactive();
        }
      break;

    case STATE_PARKED:
      if( distance_ft > PARKED_DISTANCE )
        {
        go_to_state( STATE_NEARING );
        }
      else
        {
        check_inactive();
        }
      break;
  }
}

void check_inactive()
{
  if( millis() - time_state_entered_ms > INACTIVITY_TIMEOUT_MS )
    {
    go_to_state( STATE_INACTIVE );
    }
}

void go_to_state( const unsigned char new_state )
{
  state = new_state;
  time_state_entered_ms = millis();
}

void handle_state()
{
  switch( state )
  {
    case STATE_INACTIVE:
      digitalWrite( RED_PIN, LOW );
      digitalWrite( GREEN_PIN, LOW );
      break;

    case STATE_TRACKING:
      // TODO flashing
      digitalWrite( RED_PIN, LOW );
      digitalWrite( GREEN_PIN, HIGH );
      break;

    case STATE_NEARING:
      // TODO flashing
      digitalWrite( RED_PIN, HIGH );
      digitalWrite( GREEN_PIN, HIGH );
      break;

    case STATE_PARKED:
      digitalWrite( RED_PIN, HIGH );
      digitalWrite( GREEN_PIN, LOW );
      break;
  }
}

