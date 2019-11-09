/*
 * CCM.c
 *
 *  Created on: Nov 2, 2019
 *      Author: Joshua Miller
 */


#include "CCM.h"
#include "HL_sys_common.h"
#include "HL_adc.h"
#include "HL_can.h"
#include "HL_gio.h"
#include "HL_het.h"
#include "HL_rti.h"
#include "HL_sys_core.h"
#include "stdlib.h"


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
        return 1;
    else
        return 0;
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

//BSEFault
//Used for checking if throttle and brake are pressed at the same time (indicating a short circuit)
//Return 1 if throttle is > 25% and brake is pressed, until brake is released and throttle is < 5%
//Return 0 if no fault
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
    //NEED A DEFAULT RETURN SIGNAL
    return;
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
    pwmSetDuty(hetRAM1,ThrottleL,0);
    pwmSetDuty(hetRAM1,ThrottleR,0);
    pwmSetDuty(hetRAM1,RegenL,0);
    pwmSetDuty(hetRAM1,RegenR,0);

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

//Motor Output
//Handles setting PWM values for motor output
void motorOutput(unsigned int *outputArray)
{
    pwmSetDuty(hetRAM1, ThrottleL, outputArray[0]);
    pwmSetDuty(hetRAM1, ThrottleR, outputArray[1]);
    pwmSetDuty(hetRAM1, RegenL, outputArray[2]);
    pwmSetDuty(hetRAM1, RegenR, outputArray[3]);
}

//pwmSetup
//Used for initial setup of pwm signals
//Only exists for the sake of making code look cleaner
void pwmSetup()
{
    pwmSetDuty(hetRAM1, ThrottleL, 0);

    pwmSetDuty(hetRAM1, ThrottleR, 0);

    pwmSetDuty(hetRAM1, RegenL, 0);

    pwmSetDuty(hetRAM1, RegenR, 0);

    pwmSetDuty(hetRAM1, Fans, 0);
}

//startup
//Function for running all startup functions after the start
//button has been pressed
void startup()
{
    //Turn on RTDS for 1-3 seconds
    gioSetBit(hetPORT2, RTDS, 1);
    rtiStartCounter(rtiREG1, rtiCOUNTER_BLOCK1);    //Start RTI Counter

    //Blink dash LEDs
    //This happens on the RTI interrupt
}

//TVA
//Output array format
//  [0]-ThrottleL  [1]-ThrottleR
//  [2]-RegenL     [3]-RegenR
void TVA(unsigned int *outputArray, unsigned int *adcArray)
{
    //Unpack and process input array
    unsigned int Angle = adcArray[3];
    float TReq = (adcArray[0] + adcArray[1]) / 2;    //Average two throttle inputs
    float lFactor, rFactor, m;

    //Do some math
    m = (0.6/DEADZONE_LOW);     //Factor used in TVA
    TReq = (TReq/4096) * 100;   //Throttle request as a percentage, represented 0-100

    //Run the Torque Vectoring Algorithm
    if(Angle <= DEADZONE_HIGH && Angle >= DEADZONE_LOW) //If Angle is within the deadzone
    {
        lFactor = 1;
        rFactor = 1;
    }
    else if(Angle < DEADZONE_LOW || Angle > DEADZONE_HIGH)//If Angle is outside of deadzone
    {
        if(Angle < DEADZONE_LOW)    //Turning Left
        {
            lFactor = (m * Angle) + 0.4;
        }
        else
        {
            lFactor = 1;
        }

        if(Angle > DEADZONE_HIGH)   //Turning Right
        {
            rFactor = ((-m) * Angle) + (0.4 - (4096 * -m));
        }
        else
        {
            rFactor = 1;
        }

    }
    //Populate output array
    outputArray[0] = TReq * lFactor;    //ThrottleL
    outputArray[1] = TReq * rFactor;    //ThrottleR
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
