/*
   Written and maintained by: 
   Andrew Kettle and Jada Berenguer
   September 27th, 2019
*/

//headers:

#include "Wire.h"

//Functions:
void getI2CData();
void getTemp();

//reassign pins according to pinout
int irpin1 = A0; //3 analog ir sensors
int irpin2 = A1;
int irpin3 = A2;
int sustrv = A3; //analog input for suspension travel, not implemented yet
//int hallpin = P2.5; //Standard gpio for hall effect

//read variables
int irread1;
int irread2;
int irread3;
int hallread;
float voltage1, voltage2, voltage3;
float temp1, temp2, temp3;
uint8_t X0, X1, Y0, Y1, Z0, Z1;
int readbytes = 2;

//registers
#define accel_pwr 0x20 // accel power only
#define gyroaccel_pwr 0x10 //the register that powers on both the gyro and accelerometer
#define mag_pwr 0x16 //the register that powers the mangetometer
//#define strt 0xD5
//#define strt (0xD4>>1)
#define who_am_I 0x0F 
#define lsm9ds1_ag 0x6B //device reg
#define controlreg5 0x1F // controls output
#define strtw 0xD6 //send before write?
#define strtr 0xD5
#define accelX0 0x28 //output registers
#define accelX1 0x29
#define accelY0 0x2A
#define accelY1 0x2B
#define accelZ0 0x2C
#define accelZ1 0x2D

void setup() {

  pinMode(irpin1, INPUT);
  pinMode(irpin2, INPUT);
  pinMode(irpin3, INPUT);
  pinMode(sustrv, INPUT);
  //pinMode(hallpin, INPUT);
  Wire.begin(); //initialize i2c transmission
  Wire.beginTransmission(lsm9ds1_ag);
  //Wire.write(strtw);
  //init power modes
  Wire.write(accel_pwr); //Accel mode
  Wire.write(80); //Powers 
  Wire.write(controlreg5);
  Wire.write(56); //makes sure output is turned on
  //Wire.write(mag_pwr); //Mag mode
  Wire.endTransmission();
  Serial.begin(9600);
}

void loop() {

//Note: delay is measured in milliseconds  

//Reading IMU
  getI2CData();

//Reading IR sensors

  irread1 = analogRead(irpin1);
  voltage1 = (irread1 / 1023) * 5000;
  temp1 = calcTemp(irread1);
  Serial.print("This is the temperature of infrared sensor 1: ");
  Serial.println(temp1);
  //delay(50);
  irread2 = analogRead(irpin2);
  voltage2 = (irread2 / 1023) * 5000;
  temp2 = calcTemp(irread2);
  Serial.print("This is the temperature of infrared sensor 2: ");
  Serial.println(temp2);
  //delay(50);
  irread3 = analogRead(irpin3);
  voltage3 = (irread3 / 1023) * 5000;
  temp3 = calcTemp(irread3);
  Serial.print("This is the temperature of infrared sensor 3: ");
  Serial.println(temp3); 
  //delay(50);

//Reading Hall effect

  hallread = DigitalRead(hallpin);
  Serial.print("This is the value of the hall effect: ");
  Serial.println(hallread); //raw digital, should be limited to 1 or 0, make sure pull down is acting accordingly
  delay(50); 
  

  delay(200);
  Serial.flush();
}

void getI2CData() //possible specify what data we want later instead of just including everything?
{

  Wire.beginTransmission(lsm9ds1_ag);
  
  Wire.write(accelX0); //getting data from accelerometer registers
  Wire.write(accelX1);
  Wire.write(accelY0);
  Wire.write(accelY1);
  Wire.write(accelZ0);
  Wire.write(accelZ1);
  
  Wire.endTransmission(false);

  Wire.requestFrom(lsm9ds1_ag, 12); //requesting 12 bytes, or 3 16 bit numbers.
  //readbytes = Wire.available(); //getting real numbers
  
  if(Wire.available()<=12) 
  { 
    X0 = Wire.read();
    X1 = Wire.read();
    Y0 = Wire.read();
    Y1 = Wire.read();
    Z0 = Wire.read();
    Z1 = Wire.read();
  }

  Wire.endTransmission();
  
  Serial.print("X0= ");
  Serial.println(X0);
  Serial.print("X1= ");
  Serial.println(X1);
  Serial.print("Y0= ");
  Serial.println(Y0);
  Serial.print("Y1= ");
  Serial.println(Y1);
  Serial.print("Z0= ");
  Serial.println(Z0);
  Serial.print("Z1= ");
  Serial.println(Z1);
  
}

float calcTemp(float ADCread)
{
  float temperature;
  float mVolts = (ADCread / 1023) * 5000;
  if(mVolts < 2000)
  {
    temperature = (mVolts - 500) / 10; //standard function below 2V
  }
  else
  {
    float Volts = mVolts / 1000;
    temperature = (41.413793103448 * Volts) - 29.758620689655; //switch to linear regression of data sheet function after 2V
  }
  return temperature; 
}
