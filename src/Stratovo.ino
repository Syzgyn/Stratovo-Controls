#include <PS2X_lib.h> //for v1.6
#include <Servo.h>

/******************************************************************
 * set pins connected to PS2 controller:
 *   - 1e column: original
 *   - 2e colmun: Stef?
 * replace pin numbers by the ones you use
 ******************************************************************/
#define PS2_DAT 12
#define PS2_CMD 11
#define PS2_SEL 10
#define PS2_CLK 13

#define STEERING_IN 7
#define STEERING_OUT 6

#define BRAKE_IN 5
#define BRAKE_OUT 4

#define STARTER_ON 52
#define SHIFTER_SERVO 46
#define THROTTLE_SERVO 48

const int THROTTLE_RANGE = 30;
const int BRAKE_RANGE = 15;

const int THROTTLE_STOPPED = 100;
const int THROTTLE_MAX = 20;

const int SHIFTER_FWD = 165;
const int SHIFTER_REV = 90;
const int SHIFTER_WAIT = 1000;

int current_throttle = 0;
int current_brake = 0;
int current_shifter = 0;

/******************************************************************
 * select modes of PS2 controller:
 *   - pressures = analog reading of push-butttons
 *   - rumble    = motor rumbling
 * uncomment 1 of the lines for each mode selection
 ******************************************************************/
// #define pressures   true
#define pressures false
// #define rumble      true
#define rumble false

PS2X ps2x; // create PS2 Controller Class

// right now, the library does NOT support hot pluggable controllers, meaning
// you must always either restart your Arduino after you connect the controller,
// or call config_gamepad(pins) again after connecting the controller.

int error = 0;
byte type = 0;
byte vibrate = 0;

int throttle_dir = 0;
bool braking = false;
bool shifting = false;

bool small = false;

Servo throttle_servo;
Servo shifter_servo;

void setup()
{

  Serial.begin(57600);

  pinMode(STARTER_ON, OUTPUT);
  pinMode(STEERING_IN, OUTPUT);
  pinMode(STEERING_OUT, OUTPUT);
  pinMode(BRAKE_IN, OUTPUT);
  pinMode(BRAKE_OUT, OUTPUT);

  throttle_servo.write(THROTTLE_STOPPED);
  throttle_servo.attach(THROTTLE_SERVO);

  shifter_servo.write(SHIFTER_FWD); // Write before attaching so it goes here on startup
  shifter_servo.attach(SHIFTER_SERVO);

  delay(300); // added delay to give wireless ps2 module some time to startup, before configuring it

  // CHANGES for v1.6 HERE!!! **************PAY ATTENTION*************

  // setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
}

/**
 * Updates the throttle servo based on the controller input
 *
 * IMPORTANT: The throttle servo decreases to go faster, and increases to slow down
 * @param int dir - the direction the throttle should move
 */
void updateThrottle(int dir = 0) // can be -1, 0, or 1 for slow down, no change, speed up
{
  current_throttle = throttle_servo.read() + (dir * -1);
  int bound_throttle = constrain(current_throttle, THROTTLE_MAX, THROTTLE_STOPPED);
  throttle_servo.write(bound_throttle);

  if (bound_throttle < THROTTLE_STOPPED && !braking)
  {
    digitalWrite(STARTER_ON, HIGH);
  }
  else
  {
    digitalWrite(STARTER_ON, LOW);
  }
}

void updateBrakes(bool press = false)
{
  // Serial.println(current_brake);
  if (press)
  {
    if (current_brake >= BRAKE_RANGE)
    {
      return;
    }
    digitalWrite(BRAKE_IN, HIGH);
    digitalWrite(BRAKE_OUT, LOW);
    current_brake++;
  }
  else
  {
    if (current_brake < 0)
    {
      digitalWrite(BRAKE_IN, LOW);
      digitalWrite(BRAKE_OUT, LOW);
      return;
    }

    digitalWrite(BRAKE_IN, LOW);
    digitalWrite(BRAKE_OUT, HIGH);
    current_brake--;
  }
}

void updateShifter()
{
  current_throttle = throttle_servo.read();
  if (current_throttle < THROTTLE_STOPPED)
  {
    return;
  }
  current_shifter = shifter_servo.read();
  if (current_shifter > 160)
  {
    // Set to reverse
    shifter_servo.write(SHIFTER_REV);
  }
  else
  {
    shifter_servo.write(SHIFTER_FWD);
  }

  // Long delay to make sure it gets where it needs to go.
  delay(SHIFTER_WAIT);
}

void loop()
{
  if (error == 1)
  {
    return;
  }

  ps2x.read_gamepad(small, vibrate);

  // Steering
  if (ps2x.Button(PSB_PAD_LEFT))
  {
    digitalWrite(STEERING_IN, HIGH);
    digitalWrite(STEERING_OUT, LOW);
  }
  else
  {
    digitalWrite(STEERING_IN, LOW);
  }
  if (ps2x.Button(PSB_PAD_RIGHT))
  {
    digitalWrite(STEERING_IN, LOW);
    digitalWrite(STEERING_OUT, HIGH);
  }
  else
  {
    digitalWrite(STEERING_OUT, LOW);
  }

  // Throttle & Brake
  if (ps2x.Button(PSB_CROSS))
  {
    throttle_dir = 1;
  }
  else if (ps2x.Button(PSB_CIRCLE))
  {
    throttle_dir = -1;
  }
  else
  {
    throttle_dir = 0;
  }

  if (ps2x.Button(PSB_PAD_DOWN) || ps2x.Button(PSB_R2))
  {
    braking = true;
    digitalWrite(STARTER_ON, LOW);
    throttle_dir = -1;
    updateBrakes(true);
  }
  else
  {
    braking = false;
    updateBrakes(false);
  }

  updateThrottle(throttle_dir);

  if (ps2x.Button(PSB_SELECT))
  {
    updateShifter();
  }

  delay(50);
}