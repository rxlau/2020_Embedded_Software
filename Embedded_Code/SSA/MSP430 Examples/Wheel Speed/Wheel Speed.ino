/*Component: Suspension System Array
 * Portion: Wheel Speed
 * This code is used to calculate wheel speed.
 */
#include "Wire.h"
#include "msp430.h"

int Read_Hall();
int initiateTimer();
int end_Timer();
int wheel_speed();
int hall_pin = 13;
unsigned long initial_time = 0;
unsigned long end_time = 0;
unsigned long time_difference = 0;

int w_speed=0;
int KV = 0;


void setup()
{
  /* Hall Sensor Setup*/
  pinMode(hall_pin, INPUT);
  /*interrupt setup for the hall sensor to be excecuted in FALLING state*/
 // attachInterrupt(digitalPinToInterrupt(hall_pin),pin_ISR,FALLING);
  Serial.begin(9600);
  Serial.print("end of setup");
}

/* function calcs wheel speed */
unsigned long Wheel_speed(int half_rot_time)
{
  /* the tire radius is 18 inches */
  int wheel_diameter = 18;
  int circumf = 0;
  int half_rotation = 0;
  int wheel_speed_f=0;
  /* The circumf is the distance for a full rotation */
  circumf = (3.14159 * wheel_diameter);
  /* half_rotation is distance for a half rotation*/
  half_rotation = circumf/2;
  /* Wheel speed for this half of a rotation*/
  wheel_speed_f = half_rotation/ half_rot_time;
  return wheel_speed_f;
}

/* Interrupt function to calculate the wheel speed */


/* Start the timer in the loop */
void loop()
{
    int wheel_speed_final =0;
    if(KV == 0)
    {
        initial_time = micros();
        Serial.println("Initial time");
        Serial.print(initial_time, DEC);
        delay(10);
        KV = 1;// variable set to one to not take the initial time again
    }
    if( digitalRead(hall_pin)== LOW)
    {
      end_time = micros();
      Serial.println("End time");
      Serial.print(end_time, DEC);
      delay(10);
      time_difference = initial_time - end_time;
      wheel_speed_final= Wheel_speed(time_difference);
      Serial.println("wheel speed");
      Serial.print(wheel_speed_final, DEC);
      KV = 0;// resetting variable to 0 to take the initial time 
    }
   
}