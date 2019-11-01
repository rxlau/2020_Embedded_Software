/*Component: Suspension System Array 
 * Portion: Wheel Speed 
 * This code is used to calculate wheel speed.
 */
#include <msp430.h>

int hall_pin = P2.5;
void setup() 
{
  /* Hall Sensor Setup*/
  pinMode(hall_pin, INPUT); 
}

/* function calcs wheel speed */

int Read_Hall() 
{
     /* Reads hall sensor*/
    int read_pin = digitalRead(hall_pin) 
    return read_pin;
    
}
/* Initiates TImer */
void initiate_Timer() 
{
    WDTCL = WDTPW + WDTHOLD; /* stops the dog watch timer */
    TACCR0 = 0; /* stopped the timer */
    TACCTL0 |= CCIE; /* enables the interupt for CCR0 */
    TACTL = TASSEL_2 + ID_0 + MC_2 /* Picks SMCLK, input divider = 1, Mode: Continuous*/
    DCOCTL = CALDCO_8MHz;/* Set the DOC clock to 8 MHz*/
}
int end_timer() 
{
    int temp =0; 
    TACTL = MC_0; /*this will halt the timer */
    temp = TAR; 
    return temp; 
     
}
void loop() 
{
    int hall_read =0;
    int reading = 0; 
    int lil_time = 0;
    int ticks = 0;  
  /* initiate Timer*/
    initiate_Timer();
    while(reading != 0) 
    {
        hall_read = Read_Hall();
        if(hall_read == LOW)
        {
            ticks = end_timer(); //This will end the timer and return the number of ticks 
            reading = 1; /* will then exit this loop to reset the timer*/
          }
       

    }
    roation_1_tick = ; 
    Read_Hall();
 
}
