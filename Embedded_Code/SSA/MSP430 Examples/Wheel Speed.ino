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

void setup() 
{
  /* Hall Sensor Setup*/
  pinMode(hall_pin, INPUT); 
}

/* function calcs wheel speed */

int Read_Hall() 
{
     /* Reads hall sensor*/
    int read_pin = digitalRead(hall_pin);
    return read_pin;
    
}
/* Initiates TImer */
void initiate_Timer() 
{
    WDTCTL = WDTPW + WDTHOLD; /* stops the dog watch timer */
    TACCR0 = 0; /* stopped the timer */
    TACCTL0 |= CCIE; /* enables the interupt for CCR0 */
    TACTL = TASSEL_2 + ID_3 + MC_2; /* Picks SMCLK, input divider = 8, Mode: Continuous*/ 
    DCOCTL = CALDCO_8MHZ;/* Set the DOC clock to 8 MHz*/
}
int end_timer() 
{
    int temp =0; 
    TACTL = MC_0; /*this will halt the timer */
    temp = TAR; /* this is the number of ticks but I think this is in hex have to convert*/
    return temp; 
     
}
int wheel_speed(int half_rot_time)
{
  /* the tire radius is 18 inches */
  int wheel_diameter = 18; 
  int circumf = 0; 
  int wheel_speed = 0;
  int half_rotation = 0; 
  /* The circumf is the distance for a full rotation */
  circumf = (3.14159 * wheel_diameter);
  /* half_rotation is distance for a half rotation*/
  half_rotation = circumf/2;
  /* Wheel speed for this half of a rotation*/
  wheel_speed = half_rotation/ half_rot_time;
  return wheel_speed; 
  }
  
void loop() 
{
    int hall_read =0;
    int reading = 0; 
    int _speed_ = 0;
    int lil_time = 0;
    int ticks = 0;  
  /* initiate Timer*/
    initiate_Timer();
    while(reading = 0) 
    {
        hall_read = Read_Hall();
        if(hall_read == LOW)
        {
            ticks = end_timer(); /*This will end the timer and return the number of ticks, each tick is 1 micro second*/
            _speed_ = wheel_speed(ticks); /* will give the speed of this rotation*/
            reading = 1; /* will then exit this loop to reset the timer*/
            Serial.print(_speed_,DEC);
          }
       

    }
}