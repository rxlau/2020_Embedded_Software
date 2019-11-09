/** @file HL_sys_main.c 
*   @brief Application main file
*   @date 08-Feb-2017
*   @version 04.06.01
*
*   This file contains an empty main function,
*   which can be used for the application.
*/

/* 
* Copyright (C) 2009-2016 Texas Instruments Incorporated - www.ti.com  
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
#include "CCM.h"

/* USER CODE END */

/** @fn void main(void)
*   @brief Application main function
*   @note This function is empty by default.
*
*   This function is called after startup.
*   The user can use this function to implement the application.
*/

/* USER CODE BEGIN (2) */
//Global Variables ;-;
//====================================
int bseFlag = 0;
int timeDelayFlag = 0;
int compare2Counter = 0;
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
        //Wait for time to pass and call rtinotification compare2 using rticounter 1
    gioDisableNotification(gioPORTA, StartButton);  //Make sure start button is disabled
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
        unsigned int outputArray[4];
        //Output array format
        //  [0]-ThrottleL  [1]-ThrottleR
        //  [2]-RegenL     [3]-RegenR

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
            outputArray[0] = 0;
            outputArray[1] = 0;

            //Should we also zero Regen?
            outputArray[2] = 0;
            outputArray[3] = 0;
            //Do we want to set throttle/output values in an array and then output once?
        }
        else
        {
            //Run Torque or Regen Vectoring Algorithm
            TVA(outputArray, adcArray);
        }

        //Output to motors
        motorOutput(outputArray);

        //Misc output functions (brake light)
        if(adcArray[2] > BRAKE_APPLIED_CUTOFF)
            gioSetBit(hetPORT2, BrakeLight, 1);
        else
            gioSetBit(hetPORT2, BrakeLight, 0);

        //Input from CAN and output to OBD2
        //Converts data to percentages
        uint8_t throttleLPercent = outputArray[0];
        uint8_t throttleRPercent = outputArray[1];
        uint8_t regenLPercent = outputArray[2];
        uint8_t regenRPercent = outputArray[3];
        uint8_t brakePercent = adcArray[2]/4096;
        uint8_t steeringPercent = adcArray[3]/4096;

        uint8_t canData[94];
        //Populate canData buffer with all data from CANBus and throttle,brake,and steering % values
        getAllCANData(canData);
        canData[0] = (((adcArray[0] + adcArray[1])/2) * 100);
        canData[1] = brakePercent;
        canData[2] = steeringPercent;

        sendAllDataOBD(canData);

    }
    if(notification == rtiNOTIFICATION_COMPARE1) //Battery Management
    {
        //TODO: maybe move these variable elsewhere
        static uint8_t bms1_temp[6];
        static uint8_t bms2_temp[6];
        unsigned int internalTemp1;
        unsigned int internalTemp2;
//        unsigned int ext1Temp1;
//        unsigned int ext2Temp1;
//        unsigned int ext1Temp2;
//        unsigned int ext2Temp2;

        //Get data from CAN Bus
        canGetData(canREG1,canMESSAGE_BOX1,bms1_temp);
        canGetData(canREG1,canMESSAGE_BOX2,bms2_temp);

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
    if(notification == rtiNOTIFICATION_COMPARE2 && timeDelayFlag == 0) //Time Delay
    {
        compare2Counter ++;
        if(compare2Counter == 20)
        {
            rtiStopCounter(rtiREG1, rtiCOUNTER_BLOCK1);     //Stop Counter 1
            rtiResetCounter(rtiREG1, rtiCOUNTER_BLOCK1);

            gioSetBit(hetPORT2, TimeDelay, 1);              //Set Time Delay pin to high

            timeDelayFlag = 1;                              //Set Time Delay flag to high
            compare2Counter = 0;

            gioEnableNotification(gioPORTA, StartButton);   //Enable Start Button
        }
    }
    if(notification == rtiNOTIFICATION_COMPARE2 &&timeDelayFlag == 1)//RTDS
    {
        compare2Counter ++;
        if(compare2Counter == 6)
        {
            rtiStopCounter(rtiREG1, rtiCOUNTER_BLOCK1);
            rtiResetCounter(rtiREG1, rtiCOUNTER_BLOCK1);

            gioSetBit(hetPORT2, RTDS, 0);   //Disable RTDS

            compare2Counter = 0;
            //Need to set all DASH LEDS low
            gioSetBit(hetPORT2, BMSLED, 0);
            gioSetBit(hetPORT2, IMDLED, 0);
            gioSetBit(hetPORT2, BSPDLED, 0);

            //Start RTI Counter 0
            rtiStartCounter(rtiREG1, rtiCOUNTER_BLOCK0);
        }
        else
        {
            gioToggleBit(hetPORT2, BMSLED);
            gioToggleBit(hetPORT2, IMDLED);
            gioToggleBit(hetPORT2, BSPDLED);
        }

    }

}

//All functions below need to be declared for CAN to function, but
//are only used when using interrupts with CAN
void canMessageNotification(canBASE_t *node, uint32_t messageBox){
    return;
}

void canErrorNotification(canBASE_t *node, uint32_t messageBox){
    return;
}

void esmGroup1Notification(unsigned channel){
    return;
}

void esmGroup2Notification(unsigned channel)
{
    return;
}
/* USER CODE END */
