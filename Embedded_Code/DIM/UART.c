#include <ADC.h>
#include <msp430g2553.h>
#include <UART.h>

void init_UART(){
    P1SEL = BIT1 + BIT2;      //Sets pin 1.1 to transfer and receive data
    P1SEL2 = BIT1 + BIT2;     //Sets pin 1.2 to transfer and receive data

    UCA0CTL1 |= UCSSEL_2;
    UCA0BR0 = 104;          //Sets baud rate to 9600 bps (standard)
    UCA0BR1 = 0;            //Overflow of UCA0BR0 - not needed unless baud rate needs to be higher
    UCA0MCTL = UCBRS0;      //Modulation UCBRSx=1
    UCA0CTL1 &= ~UCSWRST;   //Initialized the USCI state machine (Universal Serial Communication Interface)
}

void UART_String(char * uart_data){
    unsigned int i = 0;
    while(uart_data[i]){            //While loop to send characters of a string
        while(UCA0STAT & UCBUSY);   //Checks to see if the transmission is currently busy
        UCA0TXBUF = uart_data[i];   //Sends values of the data to the UCA0TXBUF to be sent over USB
        i++;                        //Increment to next character in the string
    }
}

void test_UART( int buf){
    UART_String("buf");
    UART_Char('0' + buf);
    UART_Char(':');
    UART_Char('0' + (adc[buf] / 100));
    UART_Char('0' + (adc[buf] % 100) / 10);
    UART_Char('0' + adc[buf] % 10);
    UART_Char(',');
    UART_String("\n\r");
}

void UART_Char (char uart_data){
    while (UCA0STAT & UCBUSY);
    UCA0TXBUF = uart_data;
}
