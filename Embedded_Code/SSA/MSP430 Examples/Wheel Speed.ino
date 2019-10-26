/*Component: Suspension System Array 
 * Portion: Wheel Speed 
 * This code is used to calculate wheel speed.
 */
#include <msp430.h>

int hall_pin = P2.5;
int lapse_time = 0; 
int wheel_diameter= 0; 
int sample = 0; 
int temp_time = 0; 
int distance_per_rotation = 0; /* Value is set by circumference of the wheel*/

void setup() 
{
  /* Hall Sensor Setup*/
  pinMode(hall_pin, INPUT); 
}

/* function calcs wheel speed */
int getwheelspeed(rotation_time) 
{
    wheel_speed_rotation = (rotation_time)/distance_per_rotation; 
    return wheel_speed_rotation;   
}
int Read_Hall() 
{
     /* Reads hall sensor*/
    int read_pin = digitalRead(hall_pin) 
    return read_pin;
    
}
void loop() 
{
    int hall_read =0;
    int rotation_1_tick = 0;
    int rotation_2_tick = 0; 
    TACCR0 = 0;
  /* Start Timer*/
    WDTCL = WDTPW + WDTHOLD; /* stops the dog watch timer */
    TACCR0 = 0; /* stopped the timer */
    TACCTL0 |= CCIE; /* enables the interupt for CCR0 */
    TACTL = TASSEL_2 + ID_0 + MC_2 /* Picks SMCLK, input divider = 1, Mode: Continuous*/
    DCOCTL = CALDCO_8MHz;/* Set the DOC clock to 8 MHz*/
    _enable_interrupt(); 
    
    hall_read = Read_Hall();
    /* stop timer here  */ 
    /* Capture the value */ 
 
    roation_1_tick = ; 
    Read_Hall();
    
     
    /*Magnet is away from the sensor */
    else (hall_pin == HIGH) 
    
}