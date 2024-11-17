#include <Arduino.h>
#include "stepper.hpp"

hw_timer_t* step_timer = NULL;

volatile bool x_axis_should_be_moving = false;
volatile bool y_axis_should_be_moving = false;
volatile bool z_axis_should_be_moving = false;
volatile Direction x_axis_direction = FORWARD;
volatile Direction y_axis_direction = FORWARD;
volatile Direction z_axis_direction = FORWARD;

volatile bool x_axis_step_status = 0; // 0 = low, 1 = high
volatile bool y_axis_step_status = 0;
volatile bool z_axis_step_status = 0;

volatile uint16_t x_axis_position = 0;
volatile uint16_t y_axis_position = 0;
volatile uint16_t z_axis_position = 0;

volatile uint16_t x_axis_position_target = 0;
volatile uint16_t y_axis_position_target = 0;
volatile uint16_t z_axis_position_target = 0;


volatile bool x_min_limit_switch_triggered = false;
volatile bool y_min_limit_switch_triggered = false;
volatile bool z_min_limit_switch_triggered = false;

// Interrupt service routines for limit switches
void IRAM_ATTR xMinLimitSwitchISR() {
  x_min_limit_switch_triggered = true;
  x_axis_should_be_moving      = false;
  x_axis_position              = 0;
  timerAlarmDisable(step_timer); // Stop timer when limit switch is reached
  // Move stepper back a bit to ensure it is not stuck
  int steps = 5;
  while (steps > 0) {
    digitalWrite(X_STEP_PIN, HIGH);
    delayMicroseconds(STEPPER_SPEED_DELAY_US);
    digitalWrite(X_STEP_PIN, LOW);
    delayMicroseconds(STEPPER_SPEED_DELAY_US);
    steps--;
  }
}

void IRAM_ATTR yMinLimitSwitchISR() {
  y_min_limit_switch_triggered = true;
  y_axis_should_be_moving      = false;
  y_axis_position              = 0;
  timerAlarmDisable(step_timer);
  int steps = 5;
  while (steps > 0) {
    digitalWrite(X_STEP_PIN, HIGH);
    delayMicroseconds(STEPPER_SPEED_DELAY_US);
    digitalWrite(X_STEP_PIN, LOW);
    delayMicroseconds(STEPPER_SPEED_DELAY_US);
    steps--;
  }
}
void IRAM_ATTR zMinLimitSwitchISR() {
  z_min_limit_switch_triggered = true;
  z_axis_should_be_moving      = false;
  z_axis_position              = 0;
  timerAlarmDisable(step_timer);
  int steps = 20;
  while (steps > 0) {
    digitalWrite(Z_STEP_PIN, HIGH);
    delayMicroseconds(STEPPER_SPEED_DELAY_US);
    digitalWrite(Z_STEP_PIN, LOW);
    delayMicroseconds(STEPPER_SPEED_DELAY_US);
    steps--;
  }
}
// void IRAM_ATTR zMinLimitSwitchISR() {
//     z_min_limit_switch_triggered = true;
//     z_axis_should_be_moving = false;
//     z_axis_position = 0;
//     timerAlarmDisable(step_timer);
// }


// ISR for toggling step pin
void IRAM_ATTR toggleStepOnTimer() {
  if (x_axis_position == x_axis_position_target) {
    x_axis_should_be_moving = false;    
    digitalWrite(X_STEP_PIN, LOW);
  }
  if (x_axis_should_be_moving){
    digitalWrite(X_STEP_PIN, x_axis_step_status);
    x_axis_step_status = !x_axis_step_status;
    x_axis_position += x_axis_direction ? 1 : -1;
  }
  
  if (y_axis_position == y_axis_position_target) {
    y_axis_should_be_moving = false;  
    digitalWrite(Y_STEP_PIN, LOW);  
  }
  if (y_axis_should_be_moving){
    digitalWrite(Y_STEP_PIN, y_axis_step_status);
    y_axis_step_status = !y_axis_step_status;
    y_axis_position += y_axis_direction ? 1 : -1;
  }

  if (z_axis_position == z_axis_position_target) {
    z_axis_should_be_moving = false;  
    digitalWrite(Z_STEP_PIN, LOW);  
  }
  if (z_axis_should_be_moving){
    digitalWrite(Z_STEP_PIN, z_axis_step_status);
    z_axis_step_status = !z_axis_step_status;
    z_axis_position += z_axis_direction ? 1 : -1;
  }


}

// Initializes stepper motor control (e.g., pins and timer)
void setupStepper() {
  // Configure stepper motor pins as outputs
  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(Y_STEP_PIN, OUTPUT);
  pinMode(Y_DIR_PIN, OUTPUT);
  pinMode(Z_STEP_PIN, OUTPUT);
  pinMode(Z_DIR_PIN, OUTPUT);

    // Internal pullup seemed a bit weak. Currently using external pullup resistors.
  pinMode(X_MIN_PIN, INPUT);
  pinMode(Y_MIN_PIN, INPUT);
  pinMode(Z_MIN_PIN, INPUT);

  // Attach interrupts to limit switch pins
  attachInterrupt(digitalPinToInterrupt(X_MIN_PIN), xMinLimitSwitchISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(Y_MIN_PIN), yMinLimitSwitchISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(Z_MIN_PIN), zMinLimitSwitchISR, FALLING);

  // Initialize the timer for step control (timer 0)
  step_timer = timerBegin(0, 80, true); // Timer 0, prescaler 80 (1 microsecond per tick)
  timerAttachInterrupt(step_timer, &toggleStepOnTimer, true); // Attach the ISR to the timer
}

void setAxisDirections(){
  digitalWrite(X_DIR_PIN, x_axis_direction ? HIGH : LOW);
  digitalWrite(Y_DIR_PIN, y_axis_direction ? HIGH : LOW);
  digitalWrite(Z_DIR_PIN, z_axis_direction ? HIGH : LOW);
}

/**
 * @brief Moves the stepper motor a given number of steps in a given direction
 * It will block, waiting for the stepper to reach its target position.
 * 
 * @param steps 
 * @param axis 
 * @param dir 
 * @return true 
 * @return false returns false if the limit switch is triggered
 */
bool jogAxis(uint16_t steps, Axis axis, Direction dir) {
  // Set the direction and target position
  switch (axis) {
    case X_AXIS:
      x_axis_direction = dir;
      x_axis_position_target = x_axis_position + (dir ? steps : -steps);
      if (x_axis_position_target > X_AXIS_TOTAL_STEPS) {
        x_axis_position_target = X_AXIS_TOTAL_STEPS;
      }
      x_axis_should_be_moving = true;
      break;
    case Y_AXIS:
      y_axis_direction = dir;
      y_axis_position_target = y_axis_position + (dir ? steps : -steps);
      if (y_axis_position_target > Y_AXIS_TOTAL_STEPS) {
        y_axis_position_target = Y_AXIS_TOTAL_STEPS;
      }
      y_axis_should_be_moving = true;
      break;
    case Z_AXIS:
      z_axis_direction = dir;
      z_axis_position_target = z_axis_position + (dir ? steps : -steps);
      // if (z_axis_position_target > Z_AXIS_TOTAL_STEPS) {
      //   z_axis_position_target = Z_AXIS_TOTAL_STEPS;
      // }
      z_axis_should_be_moving = true;
      break;
  }
  setAxisDirections();
  timerAlarmEnable(step_timer);
  // Wait until the given axis stops moving
  switch (axis) {
    case X_AXIS:
      while (x_axis_should_be_moving) { NOP(); }
      break;
    case Y_AXIS:
      while (y_axis_should_be_moving) { NOP(); }
      break;
    case Z_AXIS:
      while (z_axis_should_be_moving) { NOP(); }
      break;
  }
  return !(x_min_limit_switch_triggered || y_min_limit_switch_triggered || z_min_limit_switch_triggered);

  
}

void goHome(){
  // Move z axis first so we dont crash
  // z_axis_direction = BACKWARD;
  // z_axis_should_be_moving = true;
  // while(!z_min_limit_switch_triggered){
  //   NOP();
  // }
  // z_axis_should_be_moving = false;


  x_axis_direction = BACKWARD;
  // y_axis_direction = BACKWARD;
  x_axis_should_be_moving = true;
  // y_axis_should_be_moving = true;

  while(!x_min_limit_switch_triggered || !y_min_limit_switch_triggered){
    if(x_min_limit_switch_triggered) x_axis_should_be_moving = false;
    if(y_min_limit_switch_triggered) y_axis_should_be_moving = false;
  }
}

void setTargetPosition(uint16_t position, Axis axis){
  switch(axis){
    case X_AXIS:
      x_axis_position_target = position;
      x_axis_direction = (x_axis_position_target > x_axis_position ? FORWARD : BACKWARD);
      x_axis_should_be_moving = true;
      break;
    case Y_AXIS:
      y_axis_position_target = position;
      y_axis_direction = (y_axis_position_target > y_axis_position ? FORWARD : BACKWARD);
      y_axis_should_be_moving = true;
      break;
    case Z_AXIS:
      z_axis_position_target = position;
      z_axis_direction = (z_axis_position_target > z_axis_position ? FORWARD : BACKWARD);
      z_axis_should_be_moving = true;
      break;
  }
  setAxisDirections();
  timerAlarmEnable(step_timer);
}

/**
 * @brief Blocking function that moves the stepper to a given position
 * 
 * @param position 
 * @param axis 
 */
void goToPosition(uint16_t position, Axis axis){
  setTargetPosition(position, axis);
  Serial.println("Going to position");
  switch (axis){
    case X_AXIS:
      while(x_axis_should_be_moving){NOP();}
      break;
    case Y_AXIS:
      while(y_axis_should_be_moving){NOP();}
      break;
    case Z_AXIS:
      while(z_axis_should_be_moving){NOP();}
      break;
  }
}
void moveXandYinParallel(uint16_t posX, uint16_t posY){
  // Calculate the number of steps to take
  uint16_t x_steps = abs(posX - x_axis_position);
  uint16_t y_steps = abs(posY - y_axis_position);
  // Set the direction
  x_axis_direction = posX > x_axis_position ? FORWARD : BACKWARD;
  y_axis_direction = posY > y_axis_position ? FORWARD : BACKWARD;
  // Set the target position
  x_axis_position_target = posX;
  y_axis_position_target = posY;
  // Start moving
  x_axis_should_be_moving = true;
  y_axis_should_be_moving = true;
  // Set the direction
  setAxisDirections();
  // Start the timer
  timerAlarmEnable(step_timer);
  // Wait until the target position is reached
  while (x_axis_should_be_moving || y_axis_should_be_moving) { NOP(); }
}


// Manually moves the stepper and counts the steps until the limit switch is reached
long calibrateAxis(uint8_t stepPin, uint8_t dirPin, volatile bool* minLimitReached) {
  // Set the direction to move towards the limit switch
  digitalWrite(dirPin, BACKWARD ? HIGH : LOW);
  
  long steps = 0;
  while (!*minLimitReached) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(STEPPER_SPEED_DELAY_US);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(STEPPER_SPEED_DELAY_US);
    steps++;
  }
  Serial.print("Min limit reached");
  // Move the stepper back a bit to ensure it is not stuck
  int steps_back = 20;
  digitalWrite(dirPin, FORWARD ? HIGH : LOW);
  while (steps_back > 0) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(STEPPER_SPEED_DELAY_US);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(STEPPER_SPEED_DELAY_US);
    steps_back--;
  }
  // Stop the stepper
  digitalWrite(stepPin, LOW);
  *minLimitReached = false;
  return steps;
}
long calibrateAxis(Axis axis){
  switch(axis){
    case X_AXIS:
      return calibrateAxis(X_STEP_PIN, X_DIR_PIN, &x_min_limit_switch_triggered);
    case Y_AXIS:
      return calibrateAxis(Y_STEP_PIN, Y_DIR_PIN, &y_min_limit_switch_triggered);
    case Z_AXIS:
      return calibrateAxis(Z_STEP_PIN, Z_DIR_PIN, &z_min_limit_switch_triggered);
    default:
      return 0;
  }
}

void startStepper(){
  timerAlarmWrite(step_timer, STEPPER_SPEED_DELAY_US, true);
  timerAlarmEnable(step_timer);
}

/**
 * @brief Initialize the stepper motor control
 * It is going to move the steppers to the home position. 
 * (There is no encoder, so that is the only way to know the position)
 * 
 * @return true if everything is ok
 * @return false if something went wrong
 */
bool stepperInit(long* x_steps, long* y_steps, long* z_steps) {
    setupStepper();

    *x_steps = calibrateAxis(X_STEP_PIN, X_DIR_PIN, &x_min_limit_switch_triggered);
    // *y_steps = calibrateAxis(Y_STEP_PIN, Y_DIR_PIN, &y_min_limit_switch_triggered);
    *y_steps = 0;
    *z_steps = 0;

    startStepper();
    // calibrateAxis(Y_STEP_PIN, Y_DIR_PIN, &y_min_limit_switch_triggered);
    // calibrateAxis(Z_STEP_PIN, Z_DIR_PIN, &z_min_limit_reached);

    // Move the steppers to the home position
    //goHome();

    return true;
}


void print_all_variables(){
  Serial.println("X-axis variables:");
  Serial.print("Position: ");
  Serial.println(x_axis_position);
  Serial.print("Target position: ");
  Serial.println(x_axis_position_target);
  Serial.print("Should be moving: ");
  Serial.println(x_axis_should_be_moving);
  Serial.print("Direction: ");
  Serial.println(x_axis_direction);
  Serial.println("Y-axis variables:");
  Serial.print("Position: ");
  Serial.println(y_axis_position);
  Serial.print("Target position: ");
  Serial.println(y_axis_position_target);
  Serial.print("Should be moving: ");
  Serial.println(y_axis_should_be_moving);
  Serial.print("Direction: ");
  Serial.println(y_axis_direction);
  Serial.println("Z-axis variables:");
  Serial.print("Position: ");
  Serial.println(z_axis_position);
  Serial.print("Target position: ");
  Serial.println(z_axis_position_target);
  Serial.print("Should be moving: ");
  Serial.println(z_axis_should_be_moving);
  Serial.print("Direction: ");
  Serial.println(z_axis_direction);
}