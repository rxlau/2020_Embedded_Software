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
#include "HL_adc.h"
#include "HL_gio.h"
#include "HL_rti.h"
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
/* USER CODE END */

int main(void)
{
/* USER CODE BEGIN (3) */
    //Initializations
    adcInit();
    gioInit();
    rtiInit();
    sciInit();

    //Calibrate ADC Modules
    adcCalibration(adcREG1);
    adcCalibration(adcREG2);

    //Set up RTI
    rtiEnableNotification(rtiREG1, rtiNOTIFICATION_COMPARE0);
    _enable_IRQ_interrupt_();

    //Set up GIO inputs
    gioSetBit(gioPORTB, 6, 0); //throttle1 error light
    gioSetBit(gioPORTB, 7, 0); //throttle2 error light

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
    //Variable Declarations
    adcData_t adc1_array[3], adc2_array[3];
    unsigned int tempValue, charNum;
    unsigned char message[8];
    int adcDiff, num1, num2, i;

    //Run ADC
    adcStartConversion(adcREG1, adcGROUP1);
    adcStartConversion(adcREG2, adcGROUP2);
    while(!adcIsConversionComplete(adcREG1, adcGROUP1) || !adcIsConversionComplete(adcREG2, adcGROUP1));
    num1 = adcGetData(adcREG1, 1U, adc1_array);
    num2 = adcGetData(adcREG2, 1U, adc2_array);

    for(i = 0; i < num1; i++) {
        tempValue = adc1_array[i].value;

        //printf("adc1_array:\nValue[%d] = %d\n", i, tempValue);

        charNum = ltoa(tempValue, (char*)message);
        sciSend(sciREG1, 7, (unsigned char*)"Value: ");
        sciSend(sciREG1, charNum, message);
        sciSend(sciREG1, 2, (unsigned char *)"\r\n");
    }
    sciSend(sciREG1, 2, (unsigned char *)"\r\n");


    for(i = 0; i < num2; i++) {
        tempValue = adc2_array[i].value;

        printf("adc2_array:\nValue[%d] = %d\n", i, tempValue);

        charNum = ltoa(tempValue, (char*)message);
        sciSend(sciREG1, 7, (unsigned char*)"Value: ");
        sciSend(sciREG1, charNum, message);
        sciSend(sciREG1, 2, (unsigned char *)"\r\n");
    }
    sciSend(sciREG1, 2, (unsigned char *)"\r\n");

    //Compare ADC Values
    for(i=0; i<num1; i++) {
        switch(i) {
        case(0):
                adcDiff = adc1_array[0].value - adc2_array[1].value;
                if(adcDiff < 0)
                    adcDiff *= -1;
                printf("brake_diff = %d\n", adcDiff);
                charNum = ltoa(adcDiff, (char*)message);
                sciSend(sciREG1, 13, (unsigned char*)"brake_diff = ");
                sciSend(sciREG1, charNum, message);
                sciSend(sciREG1, 2, (unsigned char *)"\r\n");

                if(adcDiff > 410)
                {
                    printf("ERROR_BRAKE\n");
                    sciSend(sciREG1, 18, (unsigned char*)"ERROR_BRAKE\r\n");
                }


                break;

        case(1):
                adcDiff = adc1_array[1].value - adc2_array[2].value;
                if(adcDiff < 0)
                    adcDiff *= -1;
                printf("throttle1_diff = %d\n", adcDiff);
                charNum = ltoa(adcDiff, (char*)message);
                sciSend(sciREG1, 17, (unsigned char*)"throttle1_diff = ");
                sciSend(sciREG1, charNum, message);
                sciSend(sciREG1, 2, (unsigned char *)"\r\n");

                if(adcDiff > 410)
                {
                    gioSetBit(gioPORTB, 6, 1);
                    printf("ERROR_THROTTLE_1\n");
                    sciSend(sciREG1, 18, (unsigned char*)"ERROR_THROTTLE_1\r\n");
                }
                else
                    gioSetBit(gioPORTB, 6, 0);

                break;
        case(2):
                adcDiff = adc1_array[2].value - adc2_array[0].value;
                if(adcDiff < 0)
                    adcDiff *= -1;
                printf("throttle2_diff = %d\n", adcDiff);
                charNum = ltoa(adcDiff, (char*)message);
                sciSend(sciREG1, 17, (unsigned char*)"throttle2_diff = ");
                sciSend(sciREG1, charNum, message);
                sciSend(sciREG1, 2, (unsigned char *)"\r\n");

                if(adcDiff > 410)
                {
                    gioSetBit(gioPORTB, 7, 1);
                    printf("ERROR_THROTTLE_2\n");
                    sciSend(sciREG1, 18, (unsigned char*)"ERROR_THROTTLE_2\r\n");
                }
                else
                    gioSetBit(gioPORTB, 7, 0);

                break;
        }

    }

}
/* USER CODE END */
