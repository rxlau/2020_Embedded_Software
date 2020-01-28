#include <stdbool.h>
#include <FlexCAN.h>

//unsigned char lucas[] = [1, 2, 3, 4, 5, 6, 7, 8];
static CAN_message_t msg;

void CANWrite(void){
  /*Can0.write(msg);
  delay(100);
  delay(100); */
  msg.buf[0]++;
  Can0.write(msg);
  msg.buf[0]++;
  Can0.write(msg);
  msg.buf[0]++;
  Can0.write(msg);
  msg.buf[0]++;
  Can0.write(msg);
  msg.buf[0]++;
  Can0.write(msg);  
  delay(20);
}

void SetupCAN(void)
{
  Can0.begin(500000);
  delay(1000);
  
  msg.id = 0x7;
  msg.len = 8;
  
  msg.buf[0] = 0;
  msg.buf[1] = 1;
  msg.buf[2] = 2;
  msg.buf[3] = 3;
  msg.buf[4] = 4;
  msg.buf[5] = 5;
  msg.buf[6] = 6;
  msg.buf[7] = 7;
  
}

void setup() {
  SetupCAN();
}

void loop() {
  CANWrite();
}

