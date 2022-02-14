/*
 * CCM.h
 *
 *  Created on: Nov 2, 2019
 *      Author: Joshua Miller
 *
 *  Header File that contains enumerations and function declarations
 */

#ifndef INCLUDE_CCM_H_
#define INCLUDE_CCM_H_

#include "HL_sys_common.h"

//====================================
//Parameters
//====================================
#define BRAKE_APPLIED_CUTOFF 400
#define PWM_PERIOD 100
#define BATTERY_TEMP_FAN_TURNON 0xFFF

#define DEADZONE_LOW 1536
#define DEADZONE_HIGH 2560
//====================================

//====================================
//Pin enumerations
//====================================
//GIOA
#define BMSFault 0
#define BSPDFault 1
#define IMDFault 2
#define StartButton 3
#define TVToggle 4
#define RegenToggle 5
#define TractionToggle 6

//N2HET2
#define RTDS 23
#define BrakeLight 11
#define TimeDelay 10
#define BMSLED 9
#define BSPDLED 14
#define IMDLED 22
//====================================

//====================================
//PWM Channels
//====================================
#define Throttle pwm0
//====================================

//====================================
//User Function Declarations
//====================================
void adcConversion(unsigned int *adcArray);
int APPSFault(int);
int brakeCheck();
int BSEFault(unsigned int accel1, unsigned int accel2, unsigned int brake);
void fault(int caller);
void motorOutput(unsigned int *outputArray);
void pwmSetup();
void startup();
void TVA(unsigned int *outputArray, unsigned int *adcArray);
void getAllCANData(uint8_t *canData);
void sendAllDataOBD(uint8_t *canData);
uint32_t checkPackets(uint8_t *src_packet,uint8_t *dst_packet,uint32_t psize);
//====================================

//====================================
//Global Variables
//====================================
extern int bseFlag;
extern int timeDelayFlag;
extern int compare2Counter;
//====================================

#endif /* INCLUDE_CCM_H_ */
