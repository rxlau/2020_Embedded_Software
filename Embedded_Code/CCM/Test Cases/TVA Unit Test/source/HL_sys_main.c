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
#include <stdio.h>
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
/* USER CODE END */

int main(void)
{
/* USER CODE BEGIN (3) */
    //outputArray format
        //  [0]-ThrottleL  [1]-ThrottleR
        //  [2]-RegenL     [3]-RegenR
        unsigned int outputArray[4];
        //adcArray is ordered as follows
        //[0]-Throttle1 [1]-Throttle2
        //[2]-Brake     [3]-Angle
        unsigned int adcArray[4];

        printf("TVA within dead zone, 3000 throttle\n");
        adcArray[0]=3000;    //Throttle1
        adcArray[1]=3000;    //Throttle2
        adcArray[2]=0;       //Brake
        adcArray[3]=2000;    //Angle
        TVA(outputArray, adcArray);
        printf("ThrottleL = %d\n\n", outputArray[0]);
        printf("ThrottleR = %d\n\n", outputArray[1]);

        printf("TVA turning left (angle<1500), 4000 throttle\n");
        adcArray[0]=4000;    //Throttle1
        adcArray[1]=4000;    //Throttle2
        adcArray[3]=1300;    //Angle
        TVA(outputArray, adcArray);
        printf("ThrottleL = %d\n\n", outputArray[0]);
        printf("ThrottleR = %d\n\n", outputArray[1]);

        printf("TVA turning right (angle>2600), 4000 throttle\n");
        adcArray[0]=4000;    //Throttle1
        adcArray[1]=4000;    //Throttle2
        adcArray[3]=3000;    //Angle
        TVA(outputArray, adcArray);
        printf("ThrottleL = %d\n\n", outputArray[0]);
        printf("ThrottleR = %d\n\n", outputArray[1]);

        printf("TVA turning left (angle<1500), 500 throttle\n");
        adcArray[0]=500;    //Throttle1
        adcArray[1]=500;    //Throttle2
        adcArray[3]=100;    //Angle
        TVA(outputArray, adcArray);
        printf("ThrottleL = %d\n\n", outputArray[0]);
        printf("ThrottleR = %d\n\n", outputArray[1]);

        printf("TVA turning right (angle<1500), 500 throttle\n");
        adcArray[0]=500;    //Throttle1
        adcArray[1]=500;    //Throttle2
        adcArray[3]=4000;   //Angle
        TVA(outputArray, adcArray);
        printf("ThrottleL = %d\n\n", outputArray[0]);
        printf("ThrottleR = %d\n\n", outputArray[1]);
/* USER CODE END */

    return 0;
}


/* USER CODE BEGIN (4) */
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
