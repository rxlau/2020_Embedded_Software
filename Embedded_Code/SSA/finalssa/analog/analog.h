#ifndef _ANALOG_
#define _ANALOG_

//Functions:
#include "Energia.h"

void analogSetup();
char* analogData(char *analogarr);
char analogConvert(char readval);
char calcTemp(char aread);


#endif
