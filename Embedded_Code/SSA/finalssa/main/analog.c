#include "analog.h"

void analogSetup() {

  //pin declerations  
  int irpin1 = A0; //3 analog IR sensors
  int irpin2 = A1;
  int irpin3 = A2;
  int sustrv = A3; //analog input for suspension travel, not implemented yet

  pinMode(irpin1, INPUT);
  pinMode(irpin2, INPUT);
  pinMode(irpin3, INPUT);
  pinMode(sustrv, INPUT);
}

char *analogData(char *analogarr) { //Reading IR sensors

  //sensor1
  irread1 = analogRead(irpin1);
  voltage1 = analogConvert(irread1);
  temp1 = calcTemp(irread1);
  //sensor2
  irread2 = analogRead(irpin2);
  voltage2 = analogConvert(irread2);
  temp2 = calcTemp(irread2);
  //sensor3
  irread3 = analogRead(irpin3);
  voltage3 = analogConvert(irread3);
  temp3 = calcTemp(irread3);

  analogarr[0] = temp1; //contigous places in memory
  analogarr[1] = temp2;
  analogarr[2] = temp3;
	
  return &analogarr;
}

char analogConvert(char readval)
{
	return ((readval/255) * 5000);
}

char calcTemp(char aread)
{
  char temperature;
  char mVolts = analogConvert(aread);

  if(mVolts < 2000)
  {
    temperature = (mVolts - 500) / 10; //standard function below 2V
  }
  else
  {
    char Volts = mVolts / 1000;
    temperature = (41.413793103448 * Volts) - 29.758620689655; //switch to linear regression of data sheet function after 2V
  }
  return temperature; 
}
