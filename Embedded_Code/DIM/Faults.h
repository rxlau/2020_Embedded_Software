#include <stdint.h>

//returns 1 if fault, 0 if no fault. (checks acc pedal transfer functions)
int APPS_Fault(int,int);

//returns 1 if BSE fault, 0 if no fault (checks that acc is not depressed when brake is depressed >10%)
int BSE_Fault(int,int,int);
