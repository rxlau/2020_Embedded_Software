
int AcceleratorL;  //Left motor duty cycle
int AcceleratorR;  //Right motor duty cycle

__interrupt void Timer_A0_CC0(void);
__interrupt void Timer_A1_CC1(void);

void init_PWM();

void setThrottleValue(int , int );
