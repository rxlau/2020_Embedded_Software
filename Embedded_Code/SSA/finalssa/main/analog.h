#ifndef _ANALOG_
#define _ANALOG_

#include <msp430.h>
//Functions:

void analogSetup();
char *analogData(char *analogarr[3]);
char analogConvert(char readval);
char calcTemp(char aread);

//read variables

char irread1;
char irread2;
char irread3;

char voltage1, voltage2, voltage3;
char temp1, temp2, temp3;

#endif
