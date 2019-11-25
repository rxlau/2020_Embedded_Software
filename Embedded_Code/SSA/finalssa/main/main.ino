#include <wheelspeed.h>
#include <analog.h>
#include <imu.h>

/* main function for calling all of the various SSA functions */

//include all header files

void setup() //initializes different sensors
{
	I2C_init();	
	analogSetup();
	wheelspeedSetup();
}

void loop()
{
	//gets data from sensors
	char analogarr[3] = {0}; 
	float imuarr[6] = {0};
	float wheelspeed = 0; 

	char *analogptr = &analogarr[0];
	float *imuptr = &imuarr[0];
	float *wheelspeedptr;
	
	//pack this data into CAN
	analogptr = analogData(analogptr); 
	imuptr = getI2CData(imuptr);
	wheelspeedptr = getwheelspeedData();

}
