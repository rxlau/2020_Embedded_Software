#include <wheelspeed.h>
#include <analog.h>
#include <imu.h>

/* main function for calling all of the various SSA functions */

//include all header files
char analogarr[3]; 
float imuarr[6];
float wheelspeed = 0; 
char *analogptr = &analogarr[0];
float *imuptr = &imuarr[0];

void setup() //initializes different sensors
{
	I2C_init();	
	analogSetup();
	wheelspeedSetup();
	Serial.begin(9600);
}

void loop()
{	
	//pack this data into CAN
	analogptr = analogData(analogptr); 
	imuptr = getI2CData(imuptr);
	wheelspeed = getwheelspeedData();

  //testing
  
  for(int i = 0; i < 6; i++)
  {    
    Serial.println(*(imuptr+i)); //print statement is printing backwards questionmarks
  }
  for(int i = 0; i < 3; i++)
  {    
    Serial.println(*(analogptr+i)); //print statement is printing backwards questionmarks
  }
  Serial.println(wheelspeed);
  Serial.flush();

}
