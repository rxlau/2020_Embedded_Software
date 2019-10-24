/*Component: Suspension System Array 
 * Portion: Wheel Speed 
 * This code is used to calculate wheel speed.
 */


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

int getwheelspeed(rotation_time) 
{
    wheel_speed_rotation = (rotation_time)/distance_per_rotation; 
    return wheel_speed_rotation;   
  }
void loop() 
{
  /* Start Timer*/
    hall_read = digitalRead(hall_pin) 
    
    /**Magnet is close to sensor  **/
    if(hall_read == LOW)
    {
       sample = sample + 1; 
      /* End Timer*/ 
      /* time of half rotation from timer*/

        temp_time =+temp_time; 
      /*count the sample number*/ 
      if(sample == 2) 
      {
        sample = 0; 
        /*function to obtain wheel speed*/ 
        getwheelspeed(temp_time);
        
        }
         
      
      } 
      
    /*Magnet is away from the sensor */
    else (hall_pin == HIGH) 
    
}
