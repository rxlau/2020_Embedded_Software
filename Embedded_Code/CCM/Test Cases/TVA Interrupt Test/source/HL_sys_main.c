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
#include "HL_rti.h"
#include "stdio.h"
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
void TVA(unsigned int *outputArray, unsigned int *adcArray);

#define DEADZONE_LOW 1536
#define DEADZONE_HIGH 2560

int i = 0;
/* USER CODE END */

int main(void)
{
/* USER CODE BEGIN (3) */
    //Initialize RTI
    rtiInit();
    //Set up RTI
    rtiEnableNotification(rtiREG1, rtiNOTIFICATION_COMPARE0);
    _enable_IRQ_interrupt_();
    //start RTI
    rtiStartCounter(rtiREG1, rtiCOUNTER_BLOCK0);
    //infinite loop
    while(1);
/* USER CODE END */

    return 0;
}


/* USER CODE BEGIN (4) */
void rtiNotification(rtiBASE_t *rtiREG, uint32 notification)
{
    //outputArray format
    //  [0]-ThrottleL  [1]-ThrottleR
    //  [2]-RegenL     [3]-RegenR
    unsigned int outputArray[4];
    //adcArray is ordered as follows
    //[0]-Throttle1 [1]-Throttle2
    //[2]-Brake     [3]-Angle
    unsigned int adcArray[4];

    //hardcoded adc values
    if(i == 0)
    {
    adcArray[0]=500;    //Throttle1
    adcArray[1]=500;    //Throttle2
    adcArray[2]=0;       //Brake
    adcArray[3]=4000;    //Angle
    i++;
    }

    printf("TVA: angle at %d, %d throttle\n", adcArray[3], adcArray[0]);
    TVA(outputArray, adcArray);
    printf("ThrottleL = %d\t", outputArray[0]);
    printf("ThrottleR = %d\n\n", outputArray[1]);

    if(adcArray[0] < 4000)
    {
        adcArray[0] = adcArray[0] + 200;
        adcArray[1] = adcArray[0];
    }
    else
    {
        adcArray[0] = 100;
        adcArray[1] = 100;
    }
    if(adcArray[3] < 4000)
        adcArray[3] = adcArray[3] + 450;
    else
        adcArray[3] = 100;
}



//  [0]-ThrottleL  [1]-ThrottleR
//  [2]-RegenL     [3]-RegenR
void TVA(unsigned int *outputArray, unsigned int *adcArray)
{
    //Unpack and process input array
    unsigned int Angle = adcArray[3];
    float TReq;    //Average two throttle inputs
    float lFactor, rFactor, m;

    //Do some math
    m = (0.6/DEADZONE_LOW);     //Factor used in TVA
    TReq = (adcArray[0] + adcArray[1]) / 2;
    TReq = ((float)TReq/4096) * 100;   //Throttle request as a percentage, represented 0-100

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
    outputArray[0] = (float)TReq * lFactor;    //ThrottleL
    //printf("ThrottleL = %d\t", outputArray[0]);
    outputArray[1] = (float)TReq * rFactor;    //ThrottleR
    //printf("ThrottleR = %d\n\n", outputArray[1]);
    outputArray[2] = 0;
    outputArray[3] = 0;
}
/* USER CODE END */
