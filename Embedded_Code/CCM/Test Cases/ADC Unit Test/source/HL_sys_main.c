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

    //Set up GIO as an error light
    gioSetBit(gioPORTB, 6, 0);
    gioSetBit(gioPORTB, 7, 0);

    //Start RTI
    rtiStartCounter(rtiREG1, rtiCOUNTER_BLOCK0);

    //Infinite Loop
    while(1);

/* USER CODE END */

    return 0;
}


/* USER CODE BEGIN (4) */
void rtiNotification(rtiBASE_t *rtiREG, uint32 notification) {
    //Variable Declarations
    adcData_t adc1_array[4], adc2_array[4];
    unsigned int adc1Value, adc2Value, tempValue, charNum;
    unsigned char message[8];
    int adcDiff, num1, num2, i, j, fault;

    //Run ADC
    adcStartConversion(adcREG1, adcGROUP1);
    adcStartConversion(adcREG2, adcGROUP1);
    while(!adcIsConversionComplete(adcREG1, adcGROUP1) || !adcIsConversionComplete(adcREG2, adcGROUP1) );
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

    //Compare ADC values
    for(i = 0; i < num1; i++) {

        if(i+1 > 3)
            j = 0;
        else
            j = i+1;

        adc1Value = (unsigned int)adc1_array[i].value;
        adc2Value = (unsigned int)adc2_array[j].value;

        adcDiff = adc1Value - adc2Value;
        if(adcDiff < 0)
            adcDiff *= -1;

        printf("Value_1[%d]: %d\n", i, adc1Value);
        printf("Value_2[%d]: %d\n", j, adc2Value);

        switch(i) {
        case 0:
            printf("Throttle_1 difference: %d\n", adcDiff);

            charNum = ltoa(adc1Value, (char*)message);
            sciSend(sciREG1, 15, (unsigned char*)"ADC1 Value[2]: ");
            sciSend(sciREG1,charNum, message);
            sciSend(sciREG1, 2, (unsigned char *)"\r\n");

            charNum = ltoa(adc2Value, (char*)message);
            sciSend(sciREG1, 15, (unsigned char*)"ADC2 Value[3]: ");
            sciSend(sciREG1,charNum, message);
            sciSend(sciREG1, 2, (unsigned char *)"\r\n");

            charNum = ltoa(adcDiff, (char*)message);
            sciSend(sciREG1, 23, (unsigned char*)"Throttle_1 Difference: ");
            sciSend(sciREG1,charNum, message);
            sciSend(sciREG1, 2, (unsigned char *)"\r\n");

            if(adcDiff >= 410) {
                //greater than 10% disagreement between the adc modules
                printf("ERROR_THROTTLE_1\n");
                sciSend(sciREG1, 18, (unsigned char*)"ERROR_THROTTLE_1\r\n");
                fault = 1;
                //exit(0);
            }
        break;

        case 1:
            printf("Throttle_2 difference: %d\n", adcDiff);

            charNum = ltoa(adc1Value, (char*)message);
            sciSend(sciREG1, 15, (unsigned char*)"ADC1 Value[3]: ");
            sciSend(sciREG1,charNum, message);
            sciSend(sciREG1, 2, (unsigned char *)"\r\n");

            charNum = ltoa(adc2Value, (char*)message);
            sciSend(sciREG1, 15, (unsigned char*)"ADC2 Value[0]: ");
            sciSend(sciREG1,charNum, message);
            sciSend(sciREG1, 2, (unsigned char *)"\r\n");

            charNum = ltoa(adcDiff, (char*)message);
            sciSend(sciREG1, 23, (unsigned char*)"Throttle_2 Difference: ");
            sciSend(sciREG1,charNum, message);
            sciSend(sciREG1, 2, (unsigned char *)"\r\n");

            if(adcDiff >= 410) {
                //greater than 10% disagreement between the adc modules
                printf("ERROR_THROTTLE_2\n");
                sciSend(sciREG1, 17, (unsigned char*)"ERROR_THROTTLE_2\n");
                fault = 1;
                //exit(0);
            }
            break;

        case 2:
            printf("Steering difference: %d\n", adcDiff);

            charNum = ltoa(adc1Value, (char*)message);
            sciSend(sciREG1, 15, (unsigned char*)"ADC1 Value[0]: ");
            sciSend(sciREG1,charNum, message);
            sciSend(sciREG1, 2, (unsigned char *)"\r\n");

            charNum = ltoa(adc2Value, (char*)message);
            sciSend(sciREG1, 15, (unsigned char*)"ADC2 Value[1]: ");
            sciSend(sciREG1,charNum, message);
            sciSend(sciREG1, 2, (unsigned char *)"\r\n");

            charNum = ltoa(adcDiff, (char*)message);
            sciSend(sciREG1, 21, (unsigned char*)"Steering Difference: ");
            sciSend(sciREG1,charNum, message);
            sciSend(sciREG1, 2, (unsigned char *)"\r\n");

            if(adcDiff >= 410) {
                //greater than 10% disagreement between the adc modules
                printf("ERROR_STEERING\n");
                sciSend(sciREG1, 16, (unsigned char*)"ERROR_STEERING\n");
                fault = 1;
                //exit(0);
            }
            break;

        case 3:
            printf("Brake difference: %d\n", adcDiff);

            charNum = ltoa(adc1Value, (char*)message);
            sciSend(sciREG1, 15, (unsigned char*)"ADC1 Value[1]: ");
            sciSend(sciREG1,charNum, message);
            sciSend(sciREG1, 2, (unsigned char *)"\r\n");

            charNum = ltoa(adc2Value, (char*)message);
            sciSend(sciREG1, 15, (unsigned char*)"ADC2 Value[2]: ");
            sciSend(sciREG1,charNum, message);
            sciSend(sciREG1, 2, (unsigned char *)"\r\n");

            charNum = ltoa(adcDiff, (char*)message);
            sciSend(sciREG1, 18, (unsigned char*)"Brake Difference: ");
            sciSend(sciREG1,charNum, message);
            sciSend(sciREG1, 2, (unsigned char *)"\r\n");

            if(adcDiff >= 410) {
                //greater than 10% disagreement between the adc modules
                printf("ERROR_BRAKES\n");
                sciSend(sciREG1, 13, (unsigned char*)"ERROR_BRAKES\n");
                fault = 1;
                //exit(0);
            }
            break;
        }
    }
    if(fault == 1)
        exit(0);
}
/* USER CODE END */
