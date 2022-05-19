/*
 * motor.h
 *
 *  Created on: May 18, 2022
 *      Author: Ryan
 */

#ifndef INCLUDE_MOTOR_H_
#define INCLUDE_MOTOR_H_

#include "driverInput.h"

#define MOTOR pwm0
#define ADC_MAX 0xFFF // the ADC has 12 bits of resolution. 0xFFF is 12 bits all set to 1

/* Motor interface functions */
void motorInit(); // Initializes motor outputs to 0
void motorSetOutput(unsigned int output); // sets PWM output
void motorCalcOutput(unsigned int *output, unsigned int *adcArray); // calculates duty cycle percentage from ADC inputs

#endif /* INCLUDE_MOTOR_H_ */
