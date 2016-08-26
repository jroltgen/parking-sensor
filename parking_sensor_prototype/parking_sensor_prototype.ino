// The pin used to trigger and echo on the rangefinder
const int TRIGGER_PIN = 2;
const int ECHO_PIN = 3;

// LED PWM pin, PWM pin mode not necessary
const int LED_PWM_PIN = 5;

void setup() {
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
  unsigned long duration = pulseIn( ECHO_PIN, HIGH, TIMEOUT_MS );

  // Output to console
  Serial.println( duration );

  // Convert to PWM - range bound to 1000 to 10,000
  output_pwm( duration );

  // Take readings at about 10 hz
  // Note - part requires minimum 10ms delay between triggers.
  const unsigned long DELAY_MS = 100;
  delay( DELAY_MS );
}

void trigger() {
  digitalWrite( TRIGGER_PIN, HIGH );
  delayMicroseconds( 10 );
  digitalWrite( TRIGGER_PIN, LOW );
}

void output_pwm( int duration ) {

  // Lower bound to 1000, upper bound to 10000
  if( duration < 1000 ) duration = 1000;
  else if( duration > 10000 ) duration = 10000;

  Serial.print("duration");
  Serial.println( duration );
  // Convert to 0-255
  float normalized = ((float)duration - 1000 ) / 9000;
  int pwm_value = ( 1 - normalized ) * 255;

  // Invert
  Serial.print("Analog value: ");
  Serial.println( pwm_value );
  analogWrite( LED_PWM_PIN, pwm_value );
}

