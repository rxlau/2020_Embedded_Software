#include "analog.h"
#include "Energia.h"
  
//pin declerations  
int irpin1 = A0;
int irpin2 = A1;
int irpin3 = A2;
//int sustrv = A3; //analog input for suspension travel, not implemented yet

char irread1;
char irread2;
char irread3;

char voltage1, voltage2, voltage3;
char temp1, temp2, temp3;

void analogSetup() {
  pinMode(irpin1, INPUT);
  pinMode(irpin2, INPUT);
  pinMode(irpin3, INPUT);
}

char* analogData(char *analogarray) { //Reading IR sensors

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

  *(analogarray + 0) = temp1; //contigous places in memory
  *(analogarray + 1) = temp2;
  *(analogarray + 2) = temp3;
	
  return analogarray;
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
