/** @file HL_sys_main.c 
*   @brief Application main file
*   @date 11-Dec-2018
*   @version 04.07.01
*
*   This file contains an empty main function,
*   which can be used for the application.
*/

/* 
* Copyright (C) 2009-2018 Texas Instruments Incorporated - www.ti.com  
* 
* 
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*
*    Redistributions of source code must retain the above copyright 
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the 
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/


/* USER CODE BEGIN (0) */
/* USER CODE END */

/* Include Files */

#include "HL_sys_common.h"

/* USER CODE BEGIN (1) */

//User Defined Includes
#include "HL_adc.h"
#include "HL_can.h"
#include "HL_gio.h"
#include "HL_het.h"
#include "HL_rti.h"
#include "HL_sys_core.h"
#include "stdlib.h"

/* USER CODE END */

/** @fn void main(void)
*   @brief Application main function
*   @note This function is empty by default.
*
*   This function is called after startup.
*   The user can use this function to implement the application.
*/

/* USER CODE BEGIN (2) */

//Parameters
//====================================
#define BRAKE_APPLIED_CUTOFF 400
#define PWM_PERIOD 100
#define BATTERY_TEMP_FAN_TURNON 0xFFF
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

//PWM Channels
//====================================
#define AccelL pwm0
#define AccelR pwm1
#define RegenL pwm2
#define RegenR pwm3
#define Fans pwm4
//====================================


//User Function Declarations
//====================================
void adcConversion(unsigned int *adcArray);
int APPSFault(int);
int brakeCheck();
int BSEFault(unsigned int, unsigned int, unsigned int);
void fault(int caller);
void pwmSetup();
void startup();
//====================================

//Global Variables ;-;
//====================================
int bseFlag = 0;
int timeDelayFlag = 0;
//====================================


/* USER CODE END */

int main(void)
{
/* USER CODE BEGIN (3) */

    //Initialization functions
    adcInit();
    canInit();
    gioInit();
    hetInit();
    rtiInit();

    //Set interrupt priorities
        //Don't need this, this is only for simultaneous interrupts

    //Setup RTI
    rtiEnableNotification(rtiREG1, rtiNOTIFICATION_COMPARE0);   //Enable ADC/Torque Interrupt
    rtiEnableNotification(rtiREG1, rtiNOTIFICATION_COMPARE1);   //Enable Battery Management Interrupt
    rtiEnableNotification(rtiREG1, rtiNOTIFICATION_COMPARE2);   //Enable utility/time delay notification

    //Enable notifications (IRQ and FIQ)
    _enable_interrupt_();


    //Calibrate ADC
    adcCalibration(adcREG1);
    adcCalibration(adcREG2);

    //Setup PWM outputs
    pwmSetup();

    //Setup for time delay
        //After a set time delay we need to set time delay pin to high
        //Leave time delay pin unused after
        //Need to initially have start button disabled
        //Wait for time to pass and call ritnotification compare2 using rticounter 1
    gioDisableNotification(gioPORTA, StartButton);  //Make sure start button is disabled

    rtiSetPeriod(rtiREG1, rtiCOMPARE2, 10000);      //Set period to 10000ms => 10 seconds
    rtiStartCounter(rtiREG1, rtiCOUNTER_BLOCK1);    //Start RTI Counter


    //Loop Forever
    while(1);

/* USER CODE END */

    return 0;
}


/* USER CODE BEGIN (4) */

//gioNotification
//Function that is called every time a gio Interrupt is issued
//Function has different functionality based on which bit causes
//the interrupt
//Bits 0, 1, 2 -> BMS, IMD, and BSPD faults respectively
//Bit 3 -> Start button
void gioNotification(gioPORT_t *port, uint32 bit)
{
    if(bit==BMSFault || bit==IMDFault || bit==BSPDFault)  //Fault causing inputs
        fault(bit);

    if(bit==StartButton) //Start Button signal
    {
        //Check Brake
        if(brakeCheck() == 1)
        {
            //Disable Start button
            gioDisableNotification(gioPORTA, StartButton);
            //Run Start Sequence
            startup();
            //Start RTI Counter
            rtiStartCounter(rtiREG1, rtiCOUNTER_BLOCK0);
        }
        else
            return;
    }
}

//rtiNotification
//Function that is called every time an RTI interrupt is issued
//Function has different functionality based on which RTI interrupt
//calls the function.
//rtiNOTIFICATION_COMPARE0 --> ADC translation and torque vectoring
//rtiNOTIFICATION_COMPARE1 --> Battery Fan management
void rtiNotification(rtiBASE_t *rtiREG, uint32 notification)
{
    if(notification == rtiNOTIFICATION_COMPARE0) //Torque Function
    {
        unsigned int adcArray[4];
        //Get ADC Data
        adcConversion(adcArray);

        //TODO: Calibrate adc values once on vehicle (maybe do in adc conversion function)
        int adcDiff = adcArray[0] - adcArray[1];
        if (adcDiff < 0)
            adcDiff *= -1;
        //Check ADC Data
        if(APPSFault(adcDiff) || BSEFault(adcArray[0],adcArray[1],adcArray[2]))
        {
            //Set motor output = 0
            pwmSetDuty(hetRAM1,pwm0,0);
            pwmSetDuty(hetRAM1,pwm1,0);
        }
        else
        {

            //Get CAN Bus Data
            //What data do we want/need right now? all of it?

            //Run Torque or Regen Vectoring Algorithm

            //Output to Motors (update PWM).

        }
        //Misc output functions (brake light)
        if(adcArray[2] > BRAKE_APPLIED_CUTOFF)
            gioSetBit(hetPORT2, BrakeLight, 1);
        else
            gioSetBit(hetPORT2, BrakeLight, 0);

        //Output to CAN bus/OBD2


    }
    if(notification == rtiNOTIFICATION_COMPARE1) //Battery Management
    {
        //TODO: maybe move these variable elsewhere
        static uint8 bms1_temp[6];
        static uint8 bms2_temp[6];
        unsigned int internalTemp1;
        unsigned int internalTemp2;
//        unsigned int ext1Temp1;
//        unsigned int ext2Temp1;
//        unsigned int ext1Temp2;
//        unsigned int ext2Temp2;

        //Get data from CAN Bus
        canGetData(canREG1,0x00,bms1_temp);
        canGetData(canREG1,0x01,bms2_temp);

        //TODO: Internal temp may not be desired data for this calculation
        //Evaluate data and figure out new output
        internalTemp1 = bms1_temp[0];
        internalTemp1 = internalTemp1 << 4;
        internalTemp1 = internalTemp1 | bms1_temp[1];

        internalTemp2 = bms2_temp[0];
        internalTemp2 = internalTemp2 << 4;
        internalTemp2 = internalTemp2 | bms2_temp[1];

        //Output to PWM
        //TODO: Add "better" algorithm if necessary, and find "turnon value"
        if((internalTemp1 || internalTemp2) > BATTERY_TEMP_FAN_TURNON)
            pwmSetDuty(hetRAM1,pwm3,100);


    }
}

//fault
//Fault function, for when there is a fault. Zeros all outputs
//and never returns
//Needs specific behaviors based on the fault that caused it
//Caller Value      Fault Type:
//  0                   BMS
//  1                   BSPD
//  2                   IMD
//  3                   Redundant ADC fail
void fault(int caller)
{
    //Disable Notifications if necessary
    if(caller == 3)
        rtiStopCounter(rtiREG1, rtiCOUNTER_BLOCK0);

    //Zero Throttle and Regen Requests
    pwmSetDuty(hetRAM1,pwm0,0);
    pwmSetDuty(hetRAM1,pwm1,0);
    pwmSetDuty(hetRAM1,pwm2,0);
    pwmSetDuty(hetRAM1,pwm4,0);

    //Set nonessential outputs based caller (DASH LEDs)
    if(caller == 0)
        gioSetBit(hetPORT2, BMSLED, 1);
    if(caller == 1)
        gioSetBit(hetPORT2, BSPDLED, 1);
    if(caller == 2)
        gioSetBit(hetPORT2, IMDLED, 1);
    //Wait forever
    while(1);
}

//startup
//Function for running all startup functions after the start
//button has been pressed
void startup()
{
    //Turn on RTDS for 1-3 seconds
    rtiSetPeriod(rtiREG1, rtiCOMPARE2, 3000);       //Set Period to 3 Seconds
    rtiStartCounter(rtiREG1, rtiCOUNTER_BLOCK1);    //Start RTI Counter
    gioSetBit(hetPORT2, RTDS, 1);

    //Blink dash LEDs
    //I'll do this later - LOOK INTO RIT COMPARE TICKS

}

//brakeCheck
//Checks if the brake is applied above BRAKE_APPLIED_CUTOFF
//returns 1 if brake is applied
//returns 0 if brake is not applied
int brakeCheck()
{
    unsigned int adcArray[4];
    unsigned int brakeValue;
    adcConversion(adcArray);
    brakeValue = adcArray[2];

    if(brakeValue > BRAKE_APPLIED_CUTOFF)
        return 1;
    else
        return 0;
}

//adcConversion
//Runs redundant ADC on all 4 analog inputs, checks for greater than 10% difference
//between redundant ADC inputs and returns the averaged ADC values in an output array
//
//Output array is ordered as follows
//[0]-Throttle1 [1]-Throttle2
//[2]-Brake     [3]-Angle
void adcConversion(unsigned int *adcArray)
{
    adcData_t adc1Array[4], adc2Array[4];
    unsigned int brake[2], throttle1[2], throttle2[2], angle[2];
    static unsigned int outputArray[4];
    unsigned int tempValue1, tempValue2;
    int num1, num2, i, diff;

    //Start ADC Conversion
    adcStartConversion(adcREG1, adcGROUP1);
    adcStartConversion(adcREG2, adcGROUP1);

    //Wait for ADC to complete
    while(!adcIsConversionComplete(adcREG1, adcGROUP1) ||
            !adcIsConversionComplete(adcREG2, adcGROUP1) );

    //Get ADC Data
    num1 = adcGetData(adcREG1, 1U, adc1Array);
    num2 = adcGetData(adcREG2, 1U, adc2Array);

    for(i = 0; i<num1; i++)
    {
        tempValue1 = adc1Array[i].value;
        tempValue2 = adc2Array[i].value;

        switch(i)
        {
        case 0:
            brake[0] = tempValue1;
            throttle2[1] = tempValue2;
            break;
        case 1:
            angle[0] = tempValue1;
            brake[1] = tempValue2;
            break;
        case 2:
            throttle1[0] = tempValue1;
            angle[1] = tempValue2;
            break;
        case 3:
            throttle2[0] = tempValue1;
            throttle1[1] = tempValue2;
            break;
        }
    }

    //Check and average throttle 1
    diff = throttle1[0] - throttle1[1];
    if(diff < 0)
        diff = diff * -1;
    if(diff > 410)
        fault(3);
    adcArray[0] = (throttle1[0]+throttle1[1])/2;

    //Check and average throttle 2
    diff = throttle2[0] - throttle2[1];
    if(diff < 0)
        diff = diff * -1;
    if(diff > 410)
        fault(3);
    adcArray[1] = (throttle2[0]+throttle2[1])/2;

    //Check and average brakes
    diff = brake[0] - brake[1];
    if(diff < 0)
        diff = diff * -1;
    if(diff > 410)
        fault(3);
    adcArray[2] = (brake[0]+brake[1])/2;

    //Check and average steering angle
    diff = angle[0] - angle[1];
    if(diff < 0)
        diff = diff * -1;
    if(diff > 410)
        fault(3);
    adcArray[3] = (angle[0]+angle[1])/2;

    //Return array of averaged ADC values
    //return outputArray;
}

//APPSFault
//Used for checking the difference between two throttle inputs
//Returns 1 if greater than 10% diff => fault
//Returns 0 if no fault
int APPSFault(int adcDiff)
{
    if (adcDiff > 410)
        return 1;
    else
        return 0;
}

//TODO: can be done without global variable?
int BSEFault(unsigned int accel1, unsigned int accel2, unsigned int brake)
{
    if((bseFlag) && (accel1 > 205 || accel2 > 205))//205 is 5% of 4096
    {
        return 1;
    }
    else if((accel1 > 1024 || accel2 > 1024) && (brake > BRAKE_APPLIED_CUTOFF))//1024 is 25% of 4096
    {
        bseFlag = 1;
        return 1;
    }
    else if((bseFlag) && (accel1 < 205 && accel2 < 205))//205 is 5% of 4096
    {
        bseFlag = 0;
        return 0;
    }
}

//pwmSetup
//Used for initial setup of pwm signals
//Only exists for the sake of making code look cleaner
void pwmSetup()
{
    pwmSetDuty(hetRAM1, AccelL, 0);
    pwmSetPeriod(hetRAM1, AccelL, PWM_PERIOD);

    pwmSetDuty(hetRAM1, AccelR, 0);
    pwmSetPeriod(hetRAM1, AccelR, PWM_PERIOD);

    pwmSetDuty(hetRAM1, RegenL, 0);
    pwmSetPeriod(hetRAM1, RegenL, PWM_PERIOD);

    pwmSetDuty(hetRAM1, RegenR, 0);
    pwmSetPeriod(hetRAM1, RegenR, PWM_PERIOD);

    pwmSetDuty(hetRAM1, Fans, 0);
    pwmSetPeriod(hetRAM1, Fans, PWM_PERIOD);
}
/* USER CODE END */
