/*
 * motor.c
 *
 *  Created on: May 18, 2022
 *      Author: Ryan
 */

/* Include Files */
#include "motor.h"

// Zeros the output of all motors
// Must be called after calling hetInit()
void motorInit() {
    pwmSetDuty(hetRAM1, MOTOR, 0);
}

// sends a PWM signal to the motor controller dictating the output of the motor
// output : desired output of motor in %
void motorSetOutput(uint32 output) {
    pwmSetDuty(hetRAM1, MOTOR, output);
}

// calculates
void motorCalcOutput(uint32 *output, uint32 *driverInput) {
    return;
}
