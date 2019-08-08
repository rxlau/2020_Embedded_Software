#include <ADC.h>
#include <Faults.h>
#include <msp430g2553.h>
#include <PWM.h>
#include <SPI.h>
#include <stdint.h>
#include <UART.h>
#include <mcp2515.h>

/*
 * main.c
 */

can_t can_tx;
can_t can_rx;
int startButton = 0;
int CAN_Data[5];
char ready= 0x00;
char fault = 0x00;
char throttle,throttle2, brake, steering;
int fuckThis = 0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    BCSCTL1 = CALBC1_8MHZ; // Set range
    DCOCTL = CALDCO_8MHZ;
    init_ADC();
//   init_UART();
//   init_PWM();

//    TACCTL0 = CCIE;
//    TACTL = TASSEL_2 + MC_1;
//    CCR0 = 0xFFFF;
//   // __bis_SR_register(LPM0_bits + GIE);

    P1DIR |= BIT0;                                                                 // P1.0 Green LED
    P1OUT |=  BIT4;
    P1REN |= BIT4;
    P1IE |= BIT4;                                                                  // P1.4 Interrupt
    P1IES |= BIT4;                                                                 // P1.4 Interrupt on Hi/lo Edge
    P1IFG &= ~BIT4;

    P2SEL &= ~BIT1;
    P2DIR &= ~BIT1;
    P2IN &= ~BIT1;
    P2REN = BIT1;


    init_MCP2515_SPI();
    MCP2515_init();
    init_MCP2515_CANVariable(&can_tx);
    init_MCP2515_CANVariable(&can_rx);
    __enable_interrupt();

    while(1){
        if(fuckThis ==0){
            read_ADC();
        }
        //Calibrations for inputs
//        if(brakeInput < 977){   //1023 is resting   987 is full press
            brake = 0x00;
//        }test_UART(CAN_Data);
//        else{
//            brake = 255 - 7.08*(brakeInput - 987);
//        }
        if(acc1Input < 543 || acc1Input > 662){   //540 is resting   652 is full press
            throttle = 0x00;
        }
        else{
//            throttle = 2.257*(acc1Input - 540);
            throttle = (acc1Input - 540) << 1;
        }
        if(acc2Input < 495 || acc2Input > 625){ //492 is resting 615 is full press
            throttle2 = 0x00;
        }
        else{
//            throttle2 = 2.07*(acc2Input - 492);
            throttle2 = (acc2Input - 492) << 1;
        }
        if(steeringInput < 190 ||  steeringInput > 845){ //  200 is full left     830 is full right
            steering = 0x80;    //binary value of 128
            throttle = 0x00;
        }
        else{
//            steering =  0.404*(steeringInput - 200);
            steering =  (steeringInput - 200) >> 1;
        }



        if(P2IN & BIT1){
            startButton = 0;
        }
        else{
            startButton = 1;
        }
        //if(brakeInput > 0xA0 && startButton==1){
        if(startButton==1){
            ready = 0xFF;
            while (1) {
                read_ADC();
              if(APPS_Fault(throttle,throttle2)){
                  fault=0xFF;
              }
              else{
                  fault=0x00;
              }
                //Calibrations for inputs
      //        if(brakeInput < 977){   //1023 is resting   987 is full press
              brake = 0x00;
      //        }
      //        else{
      //            brake = 255 - 7.08*(brakeInput - 987);
      //        }
              if(acc1Input < 543 || acc1Input > 662){   //540 is resting   652 is full press
                  throttle = 0x00;
              }
              else{
//                  throttle = 2.257*(acc1Input - 540);
                  throttle = (acc1Input - 540) << 1;
              }
              if(acc2Input < 495 || acc2Input > 625){ //492 is resting 615 is full press
                  throttle2 = 0x00;
              }
              else{
//                  throttle2 = 2.07*(acc2Input - 492);
                  throttle2 = (acc2Input - 492) << 1;
              }
              if(steeringInput < 190 ||  steeringInput > 845){ //  200 is full left     830 is full right
      //            steering = 0x80;    //binary value of 128
                  throttle = 0x00;
              }
              else{
//                  steering =  0.404*(steeringInput - 200);
                  steering =  (steeringInput - 200) >> 1;
              }

                CAN_Data[0] = steering;
                CAN_Data[1] = brake;
                CAN_Data[2] = throttle;
                CAN_Data[3] = fault;
                CAN_Data[4] = ready;

                if(MCP2515_spi_test ()){
                    P1OUT ^= 0;                      // P1.0 Toggle
                }
                fuckThis = 0;

            }
       }
        CAN_Data[0] = steering;
        CAN_Data[1] = brake;
        CAN_Data[2] = throttle;
        CAN_Data[3] = fault;
        CAN_Data[4] = ready;

        if(MCP2515_spi_test ()){
            P1OUT ^= 0;                                 // P1.0 Toggle
        }
        fuckThis = 0;
    }
}
//// Port 1: Interrupt-Service-Routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  fuckThis = 1;
  MCP2515_can_rx0(&can_rx);                                                      // Read information in RX0
  //__delay_cycles(DELAY_10ms);                                                   // Wait 10ms
  int i;
  for(i = 0; i < 5; i++){
      can_tx.data[i] = CAN_Data[i];
  }
  MCP2515_can_tx0(&can_tx);                                                      // Send the received info back
  P1IFG &= ~BIT4;                                                                // P1.4 IFG reset interrupt

}

//
//#pragma vector=TIMER0_A0_VECTOR
//__interrupt void Timer_A (void){
//
//        MCP2515_can_rx0(&can_rx);                                                      // Read information in RX0
//              //__delay_cycles(DELAY_10ms);                                                   // Wait 10ms
//              int i;
//              for(i = 0; i < 5; i++){
//                  can_tx.data[i] = CAN_Data[i];
//              }
//              MCP2515_can_tx0(&can_tx);
//                                                           // Send the received info back
//}





