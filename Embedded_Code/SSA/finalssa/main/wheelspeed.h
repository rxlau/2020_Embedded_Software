#ifndef _WHEELSPEED_
#define _WHEELSPEED_

#include "Wire.h"
#include "msp430.h"

void wheelspeedSetup();
float wheelspeedCalc(float time_difference_micro); //delete print statements after testing
float *getwheelspeedData();

int Read_Hall();
int initiateTimer();
int end_Timer();
int wheel_speed();
int hall_pin = 13;
float initial_time = 0;
float end_time = 0;
float time_difference = 0;

int w_speed=0;
int KV = 0;

#endif
