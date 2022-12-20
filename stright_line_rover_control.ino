
// Parameters
const int drive_distance = 100;   // cm
const int motor_power = 200;      // 0-255
const int motor_offset = 5;       // Diff. when driving straight
const int wheel_d = 65;           // Wheel diameter (mm)
const float wheel_c = PI * wheel_d; // Wheel circumference (mm)
const int counts_per_rev = 80;   // (4 pairs N-S) * (10:1 gearbox) * (1 rising edges) = 80 pulse

// Pins
const int enc_l_pin = 2;          // encoder Motor A 
const int enc_r_pin = 3;          // encoder Motor B

const int pwma_pin = 5;
const int ain1_pin = 9;  //piedini di controllo direzione marcia motore A (complementari sempre se in marcia)
const int ain2_pin = 4;  //piedini di controllo direzione marcia motore A (complementari sempre se in marcia)

const int pwmb_pin = 6;
const int bin1_pin = 7; //piedini di controllo direzione marcia motore B (complementari sempre se in marcia)
const int bin2_pin = 8; //piedini di controllo direzione marcia motore B (complementari sempre se in marcia)

const int stby_pin = 10;

// Globals
volatile unsigned long enc_l = 0;
volatile unsigned long enc_r = 0;

void setup() {

  // Debug
  Serial.begin(9600);

  // Set up pins
  pinMode(enc_l_pin, INPUT_PULLUP);
  pinMode(enc_r_pin, INPUT_PULLUP);
  
  pinMode(pwma_pin, OUTPUT);
  pinMode(ain1_pin, OUTPUT);
  pinMode(ain2_pin, OUTPUT);
  
  pinMode(pwmb_pin, OUTPUT);
  pinMode(bin1_pin, OUTPUT);
  pinMode(bin2_pin, OUTPUT);
  
  pinMode(stby_pin, OUTPUT);

  // Set up interrupts
  attachInterrupt(digitalPinToInterrupt(enc_l_pin), countLeft, RISING); //risoluzione migliore 
  attachInterrupt(digitalPinToInterrupt(enc_r_pin), countRight, RISING); //risoluzione mogliore

  // Drive straight
  delay(1000);
  enableMotors(true);
  driveStraight(drive_distance, motor_power);
}

void loop() {
  // Do nothing
}

void driveStraight(float dist, int power) {

  unsigned long num_ticks_l;
  unsigned long num_ticks_r;

  // Set initial motor power
  int power_l = motor_power;
  int power_r = motor_power;

  // Used to determine which way to turn to adjust
  unsigned long diff_l;
  unsigned long diff_r;

  // Reset encoder counts
  enc_l = 0;
  enc_r = 0;

  // Remember previous encoder counts
  unsigned long enc_l_prev = enc_l;
  unsigned long enc_r_prev = enc_r;

  // Calculate target number of ticks
  float num_rev = (dist * 10) / wheel_c;  // Convert to mm
  unsigned long target_count = num_rev * counts_per_rev;
  
  // Debug
  Serial.print("Driving for ");
  Serial.print(dist);
  Serial.print(" cm (");
  Serial.print(target_count);
  Serial.print(" ticks) at ");
  Serial.print(power);
  Serial.println(" motor power");

  // Drive until one of the encoders reaches desired count
  while ( (enc_l < target_count) && (enc_r < target_count) ) {

    // Sample number of encoder ticks
    num_ticks_l = enc_l;
    num_ticks_r = enc_r;

    // Print out current number of ticks
    Serial.print(num_ticks_l);
    Serial.print("\t");
    Serial.println(num_ticks_r);

    // Drive
    drive(power_l, power_r);

    // Number of ticks counted since last time
    diff_l = num_ticks_l - enc_l_prev;
    diff_r = num_ticks_r - enc_r_prev;

    // Store current tick counter for next time
    enc_l_prev = num_ticks_l;
    enc_r_prev = num_ticks_r;

    // If left is faster, slow it down and speed up right
    if ( diff_l > diff_r ) {
      power_l -= motor_offset;
      power_r += motor_offset;
    }

    // If right is faster, slow it down and speed up left
    if ( diff_l < diff_r ) {
      power_l += motor_offset;
      power_r -= motor_offset;
    }

    // Brief pause to let motors respond
    delay(20);
  }

  // Brake
  brake();
}

void enableMotors(boolean en) {
  if ( en ) {
    digitalWrite(stby_pin, HIGH);
  } else {
    digitalWrite(stby_pin, LOW);
  }
}

void drive(int power_a, int power_b) {

  // Constrain power to between -255 and 255
  power_a = constrain(power_a, -255, 255);
  power_b = constrain(power_b, -255, 255);

  // Left motor direction
  if ( power_a < 0 ) {
    digitalWrite(ain1_pin, LOW);
    digitalWrite(ain2_pin, HIGH);
  } else {
    digitalWrite(ain1_pin, HIGH);
    digitalWrite(ain2_pin, LOW);
  }

  // Right motor direction
  if ( power_b < 0 ) {
    digitalWrite(bin1_pin, LOW);
    digitalWrite(bin2_pin, HIGH);
  } else {
    digitalWrite(bin1_pin, HIGH);
    digitalWrite(bin2_pin, LOW);
  }

  // Set speed
  analogWrite(pwma_pin, abs(power_a));
  analogWrite(pwmb_pin, abs(power_b));
}

void brake() {
  digitalWrite(ain1_pin, LOW);
  digitalWrite(ain2_pin, LOW);
  digitalWrite(bin1_pin, LOW);
  digitalWrite(bin2_pin, LOW);
  analogWrite(pwma_pin, 0);
  analogWrite(pwmb_pin, 0);
}

void countLeft() {
  enc_l++;
}

void countRight() {
  enc_r++;
}
