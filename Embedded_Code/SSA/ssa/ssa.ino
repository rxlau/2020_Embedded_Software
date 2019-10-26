/*
   Written and maintained by: 
   Andrew Kettle and Jada Berenguer
   October 26th, 2019
*/

//headers:

#include "Wire.h"

//Functions:
void getI2CData();
void I2C_init();
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
uint8_t gX0, gX1, gY0, gY1, gZ0, gZ1, aX0, aX1, aY0, aY1, aZ0, aZ1;
//int readbytes = 2;

//registers

#define lsm9ds1_ag 0x6B //device reg, FIND OUT WHAT THIS IS CONCRETELY

#define gyro_control1 0x10 //Control of the gyro reg
#define accel_control4 0x1E //Control of the accel reg4
#define accel_control5 0x1F //Control of the accel reg5
#define mag_pwr 0x16 //the register that powers the mangetometer
//#define strt (0xD4>>1)
#define strtw 0xD6 //send before write?
#define strtr 0xD5

#define gyroX0 0x18 //accel output registerss
#define gyroX1 0x19
#define gyroY0 0x1A
#define gyroY1 0x1B
#define gyroZ0 0x1C
#define gyroZ1 0x1D

#define accelX0 0x28 //accel output registerss
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

  I2C_init(); //initialize I2C transmission

  Serial.begin(9600);
}

void loop() {

//Note: delay is measured in milliseconds  

//Reading IMU
  getI2CData();

//Reading IR sensors

/*
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
*/
//Reading Hall effect

/*
  hallread = DigitalRead(hallpin);
  Serial.print("This is the value of the hall effect: ");
  Serial.println(hallread); //raw digital, should be limited to 1 or 0, make sure pull down is acting accordingly
  delay(50); 
  */
  Serial.flush();
}

void I2C_init()
{
  Wire.begin(); //initialize i2c transmission
  Wire.beginTransmission(lsm9ds1_ag);
  //Wire.write(strtw);
  //init power modes
  Wire.write(gyro_control1); //Control register for gyro
  Wire.write(192); //Powers and set to 952 HZ, stock settings elsewhere
  Wire.write(accel_control4);
  Wire.write(56); //Makes sure accel output is turned on
  Wire.write(accel_control5);
  Wire.write(56); //not working
	
  //Wire.write(mag_pwr); //Mag mode
  Wire.endTransmission();
}

void getI2CData() //possible specify what data we want later instead of just including everything?
{

  Wire.beginTransmission(lsm9ds1_ag);
  
  Wire.write(gyroX0); //getting data from gyro registers
  Wire.write(gyroX1);
  Wire.write(gyroY0);
  Wire.write(gyroY1);
  Wire.write(gyroZ0);
  Wire.write(gyroZ1);
  
  Wire.write(accelX0); //getting data from accelerometer registers
  Wire.write(accelX1);
  Wire.write(accelY0);
  Wire.write(accelY1);
  Wire.write(accelZ0);
  Wire.write(accelZ1);

  Wire.endTransmission(false); //continue transmission until reading is done

  Wire.requestFrom(lsm9ds1_ag, 24); //requesting 12 bytes, or 3 16 bit numbers.
  //readbytes = Wire.available(); //getting real numbers
  
  if(Wire.available()<=24) 
  { 
    gX0 = Wire.read();
    gX1 = Wire.read();
    gY0 = Wire.read();
    gY1 = Wire.read();
    gZ0 = Wire.read();
    gZ1 = Wire.read();

    aX0 = Wire.read();
    aX1 = Wire.read();
    aY0 = Wire.read();
    aY1 = Wire.read();
    aZ0 = Wire.read();
    aZ1 = Wire.read();
  }

  Wire.endTransmission();

  if((gX0 != 255) && (gX1 != 255) && (gY0 != 255) && (gY1 != 255) && (gZ0 != 255) && (gZ1 != 255) && (aX0 != 255) && (aX1 != 255) && (aY0 != 255) && (aY1 != 255) && (aZ0 != 255) && (aZ1 != 255))
  {
  Serial.print("Gyro X0= ");
  Serial.println(gX0);
  Serial.print("Gyro X1= ");
  Serial.println(gX1);
  Serial.print("Gyro Y0= ");
  Serial.println(gY0);
  Serial.print("Gyro Y1= ");
  Serial.println(gY1);
  Serial.print("Gyro Z0= ");
  Serial.println(gZ0);
  Serial.print("Gyro Z1= ");
  Serial.println(gZ1);
  
  Serial.print("Accel X0= ");
  Serial.println(aX0);
  Serial.print("Accel X1= ");
  Serial.println(aX1);
  Serial.print("Accel Y0= ");
  Serial.println(aY0);
  Serial.print("Accel Y1= ");
  Serial.println(aY1);
  Serial.print("Accel Z0= ");
  Serial.println(aZ0);
  Serial.print("Accel Z1= ");
  Serial.println(aZ1);
  }

  
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
