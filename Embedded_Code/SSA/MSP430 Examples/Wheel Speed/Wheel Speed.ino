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

void setup() 
{
  /* Hall Sensor Setup*/
  pinMode(hall_pin, INPUT); 
  /*interrupt setup for the hall sensor to be excecuted in FALLING state*/
  attachInterrupt(digitalPinToInterrupt(hall_pin),pin_ISR,FALLING);
  Serial.begin(9600);
}

/* function calcs wheel speed */
int wheel_speed(int half_rot_time)
{
  /* the tire radius is 18 inches */
  int wheel_diameter = 18; 
  int circumf = 0;
  int half_rotation = 0; 
  /* The circumf is the distance for a full rotation */
  circumf = (3.14159 * wheel_diameter);
  /* half_rotation is distance for a half rotation*/
  half_rotation = circumf/2;
  /* Wheel speed for this half of a rotation*/
  wheel_speed = half_rotation/ half_rot_time;
  return wheel_speed; 
}

/* Interrupt function to calculate the wheel speed */
void pin_ISR()
{
  end_time = micros(); 
 Serial.print("End Time"); /* Testing purpose for engineer to see initial time taken*/
 Serial.println(end_time); /* Testing purpose for engineer to see initial time taken*/
 delay(10); /* Testing purpose for engineer to see initial time taken*/
  time_difference = end_time - initial_time;
  Serial.print("Time Difference"); /* Testing purpose for engineer to see initial time taken*/
  Serial.println(time_difference);/* Testing purpose for engineer to see initial time taken*/ 
  delay(10);/* Testing purpose for engineer to see initial time taken*/
  w_speed = wheel_speed(time_difference);
}

/* Start the timer in the loop */
void loop() 
{
  initial_time = micros();
  Serial.print("Initial Time:");/* Testing purpose for engineer to see initial time taken*/
  Serial.println(initial_time);/* Testing purpose for engineer to see initial time taken*/
  delay(10);/* Testing purpose for engineer to see initial time taken*/
}