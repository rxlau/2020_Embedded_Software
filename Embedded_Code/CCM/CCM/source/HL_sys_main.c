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
#include "stdio.h"
#include "motor.h"

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
#define PWM_PERIOD              100
#define BATTERY_TEMP_FAN_TURNON 0xFFF

//ADC dependent params
#define BRAKE_APPLIED_CUTOFF    1000    //Should we have a high and low parameter and calculate these values based off of that?
#define BSE_CLEAR_CUTOFF        400
#define DEADZONE_LOW            1536
#define DEADZONE_HIGH           2560
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
#define IMDLED 15

//N2HET1 (DEBUGGING PINS)
#define APPSInd 22
#define BSEInd  25
#define FaultInd 27

//====================================

//PWM Channels
//====================================
#define Throttle pwm0
#define Fans pwm4
//====================================


//User Function Declarations
//====================================
void adcConversion(unsigned int *adcArray);
void adcConversionRedundant(unsigned int *adcArray);
int APPSFault(int);
int brakeCheck();
int BSEFault(unsigned int accel1, unsigned int accel2, unsigned int brake);
void fault(int caller);
void motorOutput(unsigned int output);
void pwmSetup();
void startup();
int getThrottleOutput(unsigned int *adcArray);
void ThrottleDecode(unsigned int *outputArray, unsigned int *adcArray);
void getAllCANData(uint8_t *canData);
void sendAllDataOBD(uint8_t *canData);
uint32_t checkPackets(uint8_t *src_packet,uint8_t *dst_packet,uint32_t psize);
//====================================

//Global Variables ;-;
//====================================
int bseFlag = 0;
int timeDelayFlag = 0;
int compare2Counter = 0;
int faultEnableFlag = 0;
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
    //Why not just disable these notifications until we enter Ready-to-Drive mode?
    rtiEnableNotification(rtiREG1, rtiNOTIFICATION_COMPARE0);   //Enable ADC/Torque Interrupt
    rtiEnableNotification(rtiREG1, rtiNOTIFICATION_COMPARE1);   //Enable Battery Management Interrupt
    rtiEnableNotification(rtiREG1, rtiNOTIFICATION_COMPARE2);   //Enable utility/time delay notification

    //Enable notifications (IRQ and FIQ)
    _enable_interrupt_();


    //Calibrate ADC
    //ADC calibration won't return if inputs are shorted to GND or HIGH
    //adcCalibration(adcREG1);
    //adcCalibration(adcREG2);

    //Setup PWM outputs
    pwmSetup();

    //Setup for time delay
        //After a set time delay we need to set time delay pin to high
        //Leave time delay pin unused after
        //Need to initially have start button disabled
        //Wait for time to pass and call rtinotification compare2 using rticounter 1
    gioDisableNotification(gioPORTA, StartButton);  //Make sure start button is disabled

    rtiStartCounter(rtiREG1, rtiCOUNTER_BLOCK1);    //Start RTI Counter

    faultEnableFlag = 0;


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
    {

        if(faultEnableFlag == 1)        //faults are enabled
        {
            fault(bit);
        }
        else
        {
            return;
        }
        //gioSetBit(hetPORT1, FaultInd, 0);   //This should never fucking get called lmao

    }

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
        unsigned int output;

        //Get ADC Data
        adcConversion(adcArray);


        //TODO: Calibrate adc values once on vehicle (maybe do in adc conversion function)
        int adcDiff = adcArray[0] - adcArray[1];
        if (adcDiff < 0)
        {
            adcDiff *= -1;
        }

        //Check ADC Data
        if(APPSFault(adcDiff) || BSEFault(adcArray[0],adcArray[1],adcArray[2]))
        {
            //Set motor output = 0
            output = 0;
            //Do we want to set throttle/output values in an array and then output once?
        }
        else
        {

            //Run Torque or Regen Vectoring Algorithm
            output = getThrottleOutput(adcArray);

        }

        //Output to motors
        motorOutput(output);


        //Misc output functions (brake light)
        if(adcArray[2] > BRAKE_APPLIED_CUTOFF)
        {
            gioSetBit(hetPORT2, BrakeLight, 1);
        }
        else
        {
            gioSetBit(hetPORT2, BrakeLight, 0);
        }


        //Input from CAN and output to OBD2
        //Converts data to percentages
        uint8_t throttlePercent = output;
        uint8_t brakePercent = (float)((adcArray[2]/4096) * 100);
        uint8_t steeringPercent = (float)((adcArray[3]/4096) * 100);

        uint8_t canData[94];
        //Populate canData buffer with all data from CANBus and throttle,brake,and steering % values
        getAllCANData(canData);
        canData[0] = throttlePercent;
        canData[1] = brakePercent;
        canData[2] = steeringPercent;

        sendAllDataOBD(canData);

    }
    if(notification == rtiNOTIFICATION_COMPARE1) //Battery Management
    {
        //TODO: maybe move these variable elsewhere
        uint8_t bms1_temp[6];
        uint8_t bms2_temp[6];
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
        internalTemp1 = internalTemp1 << 8;
        internalTemp1 = internalTemp1 | bms1_temp[1];

        internalTemp2 = bms2_temp[0];
        internalTemp2 = internalTemp2 << 8;
        internalTemp2 = internalTemp2 | bms2_temp[1];

        //Output to PWM
        //TODO: Add "better" algorithm if necessary, and find "turnon value"
        if((internalTemp1 || internalTemp2) > BATTERY_TEMP_FAN_TURNON)
            pwmSetDuty(hetRAM1,Fans,100);
        else
            pwmSetDuty(hetRAM1, Fans, 0);

    }
    if(notification == rtiNOTIFICATION_COMPARE2 && timeDelayFlag == 0) //Time Delay
    {
        //Time delay is to allow inputs to reach steady state before finishing initialization
        //and allowing the car to startup
        compare2Counter ++;
        if(compare2Counter == 20)                           //10 second delay
        {
            rtiResetCounter(rtiREG1, rtiCOUNTER_BLOCK1);    //Need to reset BEFORE stopping the counter
                                                            //Looks like reset automatically starts the timer
            rtiStopCounter(rtiREG1, rtiCOUNTER_BLOCK1);     //Stop Counter 1

            gioSetBit(hetPORT2, TimeDelay, 1);              //Set Time Delay pin to high

            //Run initial fault check
            faultEnableFlag = 1;                                  //Enable fault detection now that time delay phase is over
            //If any of the fault pins are initially low then we fault
            if(gioGetBit(gioPORTA, BMSFault) == 0)
            {
                gioNotification(gioPORTA, BMSFault);
                //fault(0);
            }

            if(gioGetBit(gioPORTA, IMDFault) == 0)
            {
                gioNotification(gioPORTA, IMDFault);
                //fault(2);
            }

            if(gioGetBit(gioPORTA, BSPDFault) == 0)
            {
                gioNotification(gioPORTA, BSPDFault);
                //fault(1);
            }

            //timeDelayFlag = 1;                              //Set Time Delay flag to high
            compare2Counter = 0;

            gioEnableNotification(gioPORTA, StartButton);   //Enable Start Button

        }
    }
    if(notification == rtiNOTIFICATION_COMPARE2 && timeDelayFlag == 1)//RTDS
    {
        compare2Counter ++;
        if(compare2Counter == 6)
        {
            rtiStopCounter(rtiREG1, rtiCOUNTER_BLOCK1);
            //rtiResetCounter(rtiREG1, rtiCOUNTER_BLOCK1);

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
            //This is getting called once after time delay is finished
            //also ADC is fucking broken lol

            gioToggleBit(hetPORT2, BMSLED);
            gioToggleBit(hetPORT2, IMDLED);
            gioToggleBit(hetPORT2, BSPDLED);
        }

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
    {
        rtiStopCounter(rtiREG1, rtiCOUNTER_BLOCK0);
        gioSetBit(hetPORT1, FaultInd, 1);
    }

    //Zero Throttle and Regen Requests
    pwmSetDuty(hetRAM1,Throttle,0);

    //Set nonessential outputs based caller (DASH LEDs)
    if(caller == 0)
        gioSetBit(hetPORT2, BMSLED, 1);

    if(caller == 1)
        gioSetBit(hetPORT2, BSPDLED, 1);

    if(caller == 2)
        gioSetBit(hetPORT2, IMDLED, 1);

    //gioSetBit(hetPORT1, FaultInd, 1);
    //Wait forever
    while(1)
    {
        if(gioGetBit(gioPORTA, BMSFault) == 0)
            gioSetBit(hetPORT2, BMSLED, 1);

        if(gioGetBit(gioPORTA, BSPDFault) == 0)
            gioSetBit(hetPORT2, BSPDLED, 1);

        if(gioGetBit(gioPORTA, IMDFault) == 0)
            gioSetBit(hetPORT2, IMDLED, 1);
    }
}

//startup
//Function for running all startup functions after the start
//button has been pressed
void startup()
{
    //Turn on RTDS for 1-3 seconds
    gioSetBit(hetPORT2, RTDS, 1);
    rtiStartCounter(rtiREG1, rtiCOUNTER_BLOCK1);    //Start RTI Counter

    timeDelayFlag = 1;
    compare2Counter = 0;

    //Blink dash LEDs
    //This happens on the RTI interrupt
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
    adcData_t adc1Array[4];
    unsigned int tempValue1;
    int num1, i;

    //Start ADC Conversion
    adcStartConversion(adcREG1, adcGROUP1);

    //Wait for ADC to complete
    while(!adcIsConversionComplete(adcREG1, adcGROUP1));

    //Get ADC Data
    num1 = adcGetData(adcREG1, 1U, adc1Array);

    for(i = 0; i<num1; i++)
    {
        tempValue1 = adc1Array[i].value;

        switch(i)
        {
        case 0:
            adcArray[2] = tempValue1;   //Brake
            break;
        case 1:
            adcArray[3] = tempValue1;   //Angle
            break;
        case 2:
            adcArray[0] = tempValue1;   //Throttle1
            break;
        case 3:
            adcArray[1] = tempValue1;   //Throttle2
            break;
        }
    }

}

void adcConversionRedundant(unsigned int *adcArray)
{
    adcData_t adc1Array[4], adc2Array[4];
    unsigned int brake[2], throttle1[2], throttle2[2], angle[2];
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

    if(num1 != num2)
        fault(3);

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
    {
        gioSetBit(hetPORT1, APPSInd, 1);
        return 1;
    }
    else
    {
        gioSetBit(hetPORT1, APPSInd, 0);
        return 0;
    }
}

//BSEFault
//Used for checking if throttle and brake are pressed at the same time (indicating a short circuit)
//Return 1 if throttle is > 25% and brake is pressed, until brake is released and throttle is < 5%
//Return 0 if no fault
int BSEFault(unsigned int accel1, unsigned int accel2, unsigned int brake)
{
    if((bseFlag) && ((accel1 > BSE_CLEAR_CUTOFF) || (accel2 > BSE_CLEAR_CUTOFF)))//205 is 5% of 4096
    {
        gioSetBit(hetPORT1, BSEInd, 1);
        return 1;
    }
    else if((accel1 > 1024 || accel2 > 1024) && (brake > BRAKE_APPLIED_CUTOFF))//1024 is 25% of 4096
    {
        bseFlag = 1;
        gioSetBit(hetPORT1, BSEInd, 1);
        return 1;
    }
    else if((bseFlag) && ((accel1 < BSE_CLEAR_CUTOFF) && (accel2 < BSE_CLEAR_CUTOFF)))//205 is 5% of 4096
    {
        bseFlag = 0;
        gioSetBit(hetPORT1, BSEInd, 0);
        return 0;
    }
    //NEED A DEFAULT RETURN SIGNAL
    return 0;
}

//pwmSetup
//Used for initial setup of pwm signals
//Only exists for the sake of making code look cleaner
void pwmSetup()
{
    pwmSetDuty(hetRAM1, Throttle, 0);

    pwmSetDuty(hetRAM1, Fans, 0);
}



//Motor Output
//Handles setting PWM values for motor output
void motorOutput(unsigned int output)
{
    pwmSetDuty(hetRAM1, Throttle, output);

    //pwmSetDuty(hetRAM1, Throttle, 75);
    //pwmSetDuty(hetRAM1, ThrottleR, 25);


    //pwmSetDuty(hetRAM1, RegenL, outputArray[2]);
    //pwmSetDuty(hetRAM1, RegenR, outputArray[3]);
}

int getThrottleOutput(unsigned int *adcArray)
{
    //Unpack and process input array
    int TReq = (adcArray[0] + adcArray[1]) / 2;    //Average two throttle inputs
    TReq = ((float)TReq/4096) * 100;
    //Set throttle output
    return TReq;
}


//Does nothing other than map the throttle request to a duty cycle to send to the motors
//Used for debugging since TVA broke
void ThrottleDecode(unsigned int *outputArray, unsigned int *adcArray)
{
    unsigned int temp1;
    temp1 = (adcArray[0] + adcArray[1])/2;
    temp1 = ((float)temp1/4096) * 100;
    outputArray[0] = temp1;
    outputArray[1] = temp1;
    outputArray[2] = 0;
    outputArray[3] = 0;
}

void getAllCANData(uint8_t *canData){
    canGetData(canREG1, canMESSAGE_BOX1,canData+6);
    canGetData(canREG1, canMESSAGE_BOX2,canData+12);
    canGetData(canREG1, canMESSAGE_BOX3,canData+18);
    canGetData(canREG1, canMESSAGE_BOX4,canData+21);
    canGetData(canREG1, canMESSAGE_BOX5,canData+24);
    canGetData(canREG1, canMESSAGE_BOX6,canData+27);
    canGetData(canREG1, canMESSAGE_BOX7,canData+30);
    canGetData(canREG1, canMESSAGE_BOX8,canData+36);
    canGetData(canREG1, canMESSAGE_BOX9,canData+42);
    canGetData(canREG1, canMESSAGE_BOX10,canData+48);
    canGetData(canREG1, canMESSAGE_BOX11,canData+54);
    canGetData(canREG1, canMESSAGE_BOX12,canData+60);
    canGetData(canREG1, canMESSAGE_BOX13,canData+66);
    canGetData(canREG1, canMESSAGE_BOX14,canData+72);
    canGetData(canREG1, canMESSAGE_BOX15,canData+78);
    canGetData(canREG1, canMESSAGE_BOX16,canData+82);
    canGetData(canREG1, canMESSAGE_BOX17,canData+86);
    canGetData(canREG1, canMESSAGE_BOX18,canData+90);
}

void sendAllDataOBD(uint8_t *Data){
    canTransmit(canREG2, canMESSAGE_BOX1,Data);
    canTransmit(canREG2, canMESSAGE_BOX2,Data+6);
    canTransmit(canREG2, canMESSAGE_BOX3,Data+12);
    canTransmit(canREG2, canMESSAGE_BOX4,Data+18);
    canTransmit(canREG2, canMESSAGE_BOX5,Data+21);
    canTransmit(canREG2, canMESSAGE_BOX6,Data+24);
    canTransmit(canREG2, canMESSAGE_BOX7,Data+27);
    canTransmit(canREG2, canMESSAGE_BOX8,Data+30);
    canTransmit(canREG2, canMESSAGE_BOX9,Data+36);
    canTransmit(canREG2, canMESSAGE_BOX10,Data+42);
    canTransmit(canREG2, canMESSAGE_BOX11,Data+48);
    canTransmit(canREG2, canMESSAGE_BOX12,Data+54);
    canTransmit(canREG2, canMESSAGE_BOX13,Data+60);
    canTransmit(canREG2, canMESSAGE_BOX14,Data+66);
    canTransmit(canREG2, canMESSAGE_BOX15,Data+72);
    canTransmit(canREG2, canMESSAGE_BOX16,Data+78);
    canTransmit(canREG2, canMESSAGE_BOX17,Data+82);
    canTransmit(canREG2, canMESSAGE_BOX18,Data+86);
    canTransmit(canREG2, canMESSAGE_BOX19,Data+90);
}

//Function to verify that packets sent and received are intended values
uint32_t checkPackets(uint8_t *src_packet,uint8_t *dst_packet,uint32_t psize){
    uint32_t err=0;
    uint32_t cnt=psize;

    while(cnt--){
        if((*src_packet++) != (*dst_packet++)){
            err++;
        }
    }
    return(err);
}

void adcNotification(adcBASE_t *adc, uint32 group)
{
    return;
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
