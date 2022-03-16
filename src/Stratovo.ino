#include <PS2X_lib.h>  //for v1.6

/******************************************************************
 * set pins connected to PS2 controller:
 *   - 1e column: original 
 *   - 2e colmun: Stef?
 * replace pin numbers by the ones you use
 ******************************************************************/
#define PS2_DAT        12    
#define PS2_CMD        11
#define PS2_SEL        10
#define PS2_CLK        13

#define STEERING_IN     7
#define STEERING_OUT    6

#define BRAKE_IN        5
#define BRAKE_OUT       4

#define STARTER_ON      52
#define THROTTLE_OUT    48
#define THROTTLE_IN     46

const int THROTTLE_RANGE = 30;
const int BRAKE_RANGE = 15;

int current_throttle = 0;
int current_brake = 0;

/******************************************************************
 * select modes of PS2 controller:
 *   - pressures = analog reading of push-butttons 
 *   - rumble    = motor rumbling
 * uncomment 1 of the lines for each mode selection
 ******************************************************************/
//#define pressures   true
#define pressures   false
//#define rumble      true
#define rumble      false

PS2X ps2x; // create PS2 Controller Class

//right now, the library does NOT support hot pluggable controllers, meaning 
//you must always either restart your Arduino after you connect the controller, 
//or call config_gamepad(pins) again after connecting the controller.

int error = 0;
byte type = 0;
byte vibrate = 0;

int throttle_dir = 0;
bool braking = false;

bool small = false;

void setup(){
 
  Serial.begin(57600);

  pinMode(STARTER_ON, OUTPUT);
  pinMode(STEERING_IN, OUTPUT);
  pinMode(STEERING_OUT, OUTPUT);
  pinMode(BRAKE_IN, OUTPUT);
  pinMode(BRAKE_OUT, OUTPUT);
  pinMode(THROTTLE_IN, OUTPUT);
  pinMode(THROTTLE_OUT, OUTPUT); 
  
  
  delay(300);  //added delay to give wireless ps2 module some time to startup, before configuring it
   
  //CHANGES for v1.6 HERE!!! **************PAY ATTENTION*************
  
  //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
}

void updateThrottle(int dir = 0)  //can be -1, 0, or 1
{
  switch (dir) {
    case -1:
      if (current_throttle < 0) {
        digitalWrite(THROTTLE_IN, LOW);
        digitalWrite(THROTTLE_OUT, LOW);
        digitalWrite(STARTER_ON, LOW);
        return;
      }
      if (!braking) {
        digitalWrite(STARTER_ON, HIGH);
      }
      digitalWrite(THROTTLE_IN, LOW);
      digitalWrite(THROTTLE_OUT, HIGH);
      current_throttle--;
      break;
    case 1:
      digitalWrite(STARTER_ON, HIGH);
      if (current_throttle >= THROTTLE_RANGE) {
        digitalWrite(THROTTLE_IN, LOW);
        digitalWrite(THROTTLE_OUT, LOW);
        return;
      }
      digitalWrite(THROTTLE_IN, HIGH);
      digitalWrite(THROTTLE_OUT, LOW);
      current_throttle++;
      break;
    case 0:
      digitalWrite(THROTTLE_IN, LOW);
      digitalWrite(THROTTLE_OUT, LOW);
  }
}

void updateBrakes(bool press = false)
{
  if (press) {
    if (current_brake >= BRAKE_RANGE) {
      return;
    }
    digitalWrite(BRAKE_IN, HIGH);
    digitalWrite(BRAKE_OUT, LOW);
    current_brake++;
  } else {
    if (current_brake < 0) {
      digitalWrite(BRAKE_IN, LOW);
      digitalWrite(BRAKE_OUT, LOW);
      return;
    }

    digitalWrite(BRAKE_IN, LOW);
    digitalWrite(BRAKE_OUT, HIGH);
    current_brake--;
  }
}

void loop() {
  if (error == 1) {
    return;
  }

  ps2x.read_gamepad(small, vibrate);

  //Steering
  if(ps2x.Button(PSB_PAD_LEFT)){
      digitalWrite(STEERING_IN, HIGH);
      digitalWrite(STEERING_OUT, LOW);
  } else {
    digitalWrite(STEERING_IN, LOW);
  }
  if(ps2x.Button(PSB_PAD_RIGHT)){
    digitalWrite(STEERING_IN, LOW);
    digitalWrite(STEERING_OUT, HIGH);
  } else {
    digitalWrite(STEERING_OUT, LOW);
  }

  //Throttle & Brake
  if (ps2x.Button(PSB_CROSS)) {
    throttle_dir = 1;
  } else if (ps2x.Button(PSB_CIRCLE)) {
    throttle_dir = -1;
  } else {
    throttle_dir = 0;
  }

  if (ps2x.Button(PSB_PAD_DOWN) || ps2x.Button(PSB_R2)) {
    braking = true;
    digitalWrite(STARTER_ON, LOW);
    throttle_dir = -1;
    updateBrakes(true);
  }
  else {
    braking = false;
    updateBrakes(false);
  }

  updateThrottle(throttle_dir);
  Serial.println(current_throttle);

  delay(50);
}