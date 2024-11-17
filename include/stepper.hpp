#ifndef STEPPER_H
#define STEPPER_H

#include <Arduino.h>

// Define constants
#define Y_AXIS_TOTAL_STEPS 3000
#define X_AXIS_TOTAL_STEPS 3000
#define Z_AXIS_TOTAL_STEPS 10000

// Define pins for stepper motors
#define X_STEP_PIN 13
#define X_DIR_PIN 12
#define Y_STEP_PIN 14
#define Y_DIR_PIN 27
#define Z_STEP_PIN 26
#define Z_DIR_PIN 25

// Define pins for limit switches (inputs)
#define X_MIN_PIN 15
#define Y_MIN_PIN 4
#define Z_MIN_PIN 5

// Step speed in microseconds
#define STEPPER_SPEED_DELAY_US 1500

// Enum for direction of movement
enum Direction {
  FORWARD = true,
  BACKWARD = false
};
enum Axis {
    X_AXIS,
    Y_AXIS,
    Z_AXIS
};

// Function declarations
void setupStepper();
void startStepper();
bool stepperInit(long* x_steps, long* y_steps, long* z_steps);
bool jogAxis(uint16_t steps, Axis axis, Direction dir);
long calibrateAxis(uint8_t stepPin, uint8_t dirPin, volatile bool* minLimitReached);
long calibrateAxis(Axis axis);
void setTargetPosition(uint16_t position, Axis axis);
void goToPosition(uint16_t position, Axis axis);
void moveXandYinParallel(uint16_t posX, uint16_t posY);

void print_all_variables();

void IRAM_ATTR toggleStepOnTimer();
void IRAM_ATTR xMinLimitSwitchISR();
void IRAM_ATTR yMinLimitSwitchISR();
void IRAM_ATTR zMinLimitSwitchISR();

#endif // STEPPER_H