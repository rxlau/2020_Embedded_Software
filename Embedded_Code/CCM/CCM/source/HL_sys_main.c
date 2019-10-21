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
#include "HL_rti.c"
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
#define IMDLED 22
//====================================


//User Function Declarations
//====================================
unsigned int *adcCoversion();
int brakeCheck();
void fault(int caller);
void startup();
//====================================

//Global Variables ;-;
//====================================
hetSIGNAL_t ThrottleL, ThrottleR;
hetSIGNAL_t RegenL, RegenR;
hetSIGNAL_t BatteryFans;
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
        /*PENDING EXTERNAL TESTING*/
        //vimChannelMap();

    //Setup RTI
    rtiEnableNotification(rtiREG1, rtiNOTIFICATION_COMPARE0);   //Enable ADC/Torque Interrupt
    rtiEnableNotification(rtiREG1, rtiNOTIFICATION_COMPARE1);   //Enable Battery Management Interrupt

    //Enable notifications (IRQ and FIQ)
    _enable_interrupt_();


    //Calibrate ADC
    adcCalibration(adcREG1);
    adcCalibration(adcREG2);

    //Setup PWM outputs
    ThrottleL.duty = 0;             //NEED TO TEST IF I CAN MODIFY THESE WITHOUT RECALLING SETSIGNAL
    ThrottleL.period = PWM_PERIOD;

    ThrottleR.duty = 0;
    ThrottleR.period = PWM_PERIOD;

    RegenL.duty = 0;
    RegenL.period = PWM_PERIOD;

    RegenR.duty = 0;
    RegenR.period = PWM_PERIOD;

    BatteryFans.duty = 0;
    BatteryFans.period = PWM_PERIOD;

    //Start RTI Counter
    //rtiStartCounter(rtiREG1, rtiCOUNTER_BLOCK0);

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
        adcArray = adcConversion();

        //Check ADC Data


        //Get CAN Bus Data


        //Run Torque or Regen Vectoring Algorithm


        //Output to Motors (update PWM).


        //Misc output functions (brake light)
        if(adcArray[2] > BRAKE_APPLIED_CUTOFF)
            gioSetBit(hetPORT2, BrakeLight, 1);
        else
            gioSetBit(hetPORT2, BrakeLight, 0);

        //Output to CAN bus/OBD2


    }
    if(notification == rtiNOTIFICATION_COMPARE1) //Battery Management
    {
        //Get data from CAN Bus

        //Evaluate data and figure out new output

        //Output to PWM

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
        rtiStopCounter(rtiREG1, ritCOUNTER_BLOCK0);

    //Zero Throttle and Regen Requests

    //Set nonessential outputs based caller (DASH LEDs)

    //Wait forever
    while(1);
}

//startup
//Function for running all startup functions after the start
//button has been pressed
void startup()
{

}

//brakeCheck
//Checks if the brake is applied above BRAKE_APPLIED_CUTOFF
//returns 1 if brake is applied
//returns 0 if brake is not applied
int brakeCheck()
{
    unsigned int adcArray[4];
    unsigned int brakeValue;
    adcArray = adcConversion();
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
unsigned int *adcCoversion()
{
    adcData_t adc1Array[4], adc2Array[4];
    unsigned int brake[2], throttle1[2], throttle2[2], angle[2];
    unsigned int outputArray[4];
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
    outputArray[0] = (throttle1[0]+throttle1[1])/2;

    //Check and average throttle 2
    diff = throttle2[0] - throttle2[1];
    if(diff < 0)
        diff = diff * -1;
    if(diff > 410)
        fault(3);
    outputArray[1] = (throttle2[0]+throttle2[1])/2;

    //Check and average brakes
    diff = brake[0] - brake[1];
    if(diff < 0)
        diff = diff * -1;
    if(diff > 410)
        fault(3);
    outputArray[2] = (brake[0]+brake[1])/2;

    //Check and average steering angle
    diff = throttle1[0] - throttle1[1];
    if(diff < 0)
        diff = diff * -1;
    if(diff > 410)
        fault(3);
    outputArray[3] = (throttle1[0]+throttle1[1])/2;

    //Return array of averaged ADC values
    return outputArray;     //WILL THIS ARRAY GET YEETED ON RETURN
                            //DO WE NEED TO MALLOC -- Probably
                            //or we can make array static, just have to make sure its
                            //not used by two functions at the same time
}
/* USER CODE END */
