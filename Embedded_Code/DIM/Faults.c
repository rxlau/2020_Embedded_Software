#include <Faults.h>
#include <stdint.h>
//#include "main.h"
int BSE_flag = 0;
//returns 1 if fault, 0 if no fault. (checks acc pedal transfer functions)
int APPS_Fault(int acc1, int acc2){
    //25 is 10% of 255 which is the max value for ADC inputs
    //if apps flag has been already triggered but fault is still occurring, do nothing
    if((abs(acc1-acc2) > 25)) {
        return 1;
    }
    //if there is no fault occurring, disable APPS flag if it is triggered and continue execution.
    else {
        return 0;
    }
}
/*
//returns 1 if BSE fault, 0 if no fault (checks that brake is not depressed when acc is depressed >25%)
int BSE_Fault(int brakeAngle, int acc1, int acc2){
    acc2 = 1023 - (acc2*4.5);
    if((BSE_flag) && (acc1 > 51 || acc2 > 51)) { //51 is 5% of 1023
        return 1;
    }
    if((acc1 > 256 || acc2 > 256) && brakeAngle > 542){ //256 is 25% of 1023      542 is 53% of 5V where 2.67V is "hard press" of brakes
        BSE_flag = 1;
        return 1;
    }
    else if ((BSE_flag) && (acc1 < 51 && acc2 < 51)){ //51 is 5% of 1023  if flag is already set, flag will clear when APPS values are less than 5%
        BSE_flag = 0;
        return 0;
    }
    return 0;

}*/
