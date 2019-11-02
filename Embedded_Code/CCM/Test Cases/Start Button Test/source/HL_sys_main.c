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
#include "HL_gio.h"
#include "HL_rti.h"
#include "HL_het.h"
#include "HL_adc.h"
#include "HL_sci.h"
#include "HL_sys_core.h"
#include "stdlib.h"
#include "stdio.h"
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
//==================================================
#define BRAKE_APPLIED_CUTOFF 400
//==================================================


//Pin Name Enumerations
//==================================================
//GIO Port A
#define RTDS 23

//GIO Port B
#define BRAKE_ERROR 6
#define STARTUP_SEQ_COMPLETED 7

//N2Het2
#define STARTBUTTON 3
//==================================================

//User Declared Functions
//==================================================
int brakeCheck();
void startup();
//==================================================

//Global Variables
//==================================================
int timeDelayFlag = 0;
int compare2Counter = 0;
//==================================================

/* USER CODE END */

int main(void)
{
/* USER CODE BEGIN (3) */
    //Initializations
    gioInit();
    rtiInit();
    hetInit();
    adcInit();
    sciInit();

    //Calibrate ADCs
    adcCalibration(adcREG1);
    adcCalibration(adcREG2);

    //Set up RTI
    rtiEnableNotification(rtiREG1, rtiNOTIFICATION_COMPARE0);
    _enable_IRQ_interrupt_();

    //Set up GIO inputs
    //gioSetBit(gioPORTA, STARTBUTTON, 0); //start button

    gioSetBit(hetPORT2, RTDS, 0); //RTDS buzzer

    gioSetBit(gioPORTB, BRAKE_ERROR, 0); //brake error light
    gioSetBit(gioPORTB, STARTUP_SEQ_COMPLETED, 0); //light to show start up sequence has been completed

    //Start RTI
    gioDisableNotification(gioPORTA, STARTBUTTON);  //Make sure start button is disabled
    rtiStartCounter(rtiREG1, rtiCOUNTER_BLOCK1);    //Start RTI Counter

    //infinite loop
    while(1);

/* USER CODE END */

    return 0;
}


/* USER CODE BEGIN (4) */
void rtiNotification(rtiBASE_t *rtiREG, uint32 notification)
{
    if(timeDelayFlag == 0) //Time Delay
    {
        compare2Counter ++;
        if(compare2Counter == 20)
        {
            rtiStopCounter(rtiREG1, rtiCOUNTER_BLOCK1);     //Stop Counter 1
            rtiResetCounter(rtiREG1, rtiCOUNTER_BLOCK1);

            timeDelayFlag = 1;                              //Set Time Delay flag to high
            compare2Counter = 0;

            gioEnableNotification(gioPORTA, STARTBUTTON);   //Enable Start Button
            gioSetBit(gioPORTB, STARTUP_SEQ_COMPLETED, 1);  //Start button active light on
        }
    }
    else
        gioSetBit(gioPORTB, STARTUP_SEQ_COMPLETED, 0);      //Start button not active

    if(timeDelayFlag == 1)//RTDS
    {
        compare2Counter ++;
        if(compare2Counter == 6)
        {
            rtiStopCounter(rtiREG1, rtiCOUNTER_BLOCK1);
            rtiResetCounter(rtiREG1, rtiCOUNTER_BLOCK1);

            gioSetBit(hetPORT2, RTDS, 0);   //Disable RTDS
            gioSetBit(gioPORTB, STARTUP_SEQ_COMPLETED, 0);      //Start button not active

            compare2Counter = 0;
        }
    }
}

//gioNotification
//Function that is called every time a gio Interrupt is issued
//Function has different functionality based on which bit causes
//the interrupt
void gioNotification(gioPORT_t *port, uint32 bit)
{
    if(brakeCheck() == 1)
    {
        gioDisableNotification(gioPORTA, STARTBUTTON);
        startup();
    }
    else
        return;
}

//startup
//Function for running all startup functions after the start
//button has been pressed
void startup()
{
    //turn on buzzer for 3 seconds
    gioSetBit(hetPORT2, RTDS, 1);

    //start RTI
    rtiStartCounter(rtiREG1, rtiCOUNTER_BLOCK0);
}

//brakeCheck
//Checks if the brake is applied above BRAKE_APPLIED_CUTOFF
//returns 1 if brake is applied
//returns 0 if brake is not applied
int brakeCheck()
{
    adcData_t adc_data;
    adcData_t *adcptr = &adc_data;
    unsigned int brakeValue;
    adcStartConversion(adcREG1, adcGROUP1);
    while(!adcIsConversionComplete(adcREG1, adcGROUP1));
    adcGetData(adcREG1, adcGROUP1, adcptr);
    brakeValue = adc_data.value;

    if(brakeValue > BRAKE_APPLIED_CUTOFF)
        return 1;
    else
        return 0;
}
/* USER CODE END */
