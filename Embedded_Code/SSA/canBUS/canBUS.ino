/* Author: Michael Barbosa
 * Started: 11/9/19
 *  Description:
 *    something here
 *    
 */

// header files
#include <SPI.h>

// MACROS
# define CAN_MAX_DLEN 8


// CAN data struct
typedef struct {
  int16_t  can_id;
  unsigned char can_dlen;
  unsigned char data[CAN_MAX_DLEN];   // this is the can data to be sent
} CAN_FRAME;



// pin declarations
//const int csPin = P1_4; // May need to active this depending on SSA data output

// Function Declarations


void setup() {
  SPI.setBitOrder(MSBFIRST);            // sets bit order to shift out MSB first
  SPI.setClockDivider(SPI_CLOCK_DIV2);  // cuts msp clock frequency by 2 from 16Mhz to 8Mhz
  SPI.setDataMode(SPI_MODE0);           // sets the data mode as CPOL=0
  
  SPI.begin();                          // initialize SPI 
  
  Serial.begin(9600);   // begin serial monitor for test
}


void loop() {
  
  
}

CAN_FRAME *WheelData()
{
  CAN_FRAME *canMsg;
  // get data from sensors
  char wheelSpeed = //some wheel speed value;
  char tireTemp   = //some tire temp value;
  char susTravel  = //some susboi traveling;

  canMsg.can_id   = 0x03        // can ID to be read
  canMsg.can_dlen = 8;          // length of CAN data to be transmitted
  canMsg.data[0]  = wheelSpeed; // setting the wheelspeed to the can message
  canMsg.data[1]  = tireTemp;
  canMsg.data[2]  = susTravel;
  canMsg.data[3]  = 0x00;
  canMsg.data[4]  = 0x00;
  canMsg.data[5]  = 0x00;
  canMsg.data[6]  = 0x00;
  canMsg.data[7]  = 0x00;

  return canMsg;
}

CAN_FRAME *Pitch_XAccel_Data()
{
  CAN_FRAME *canMsg;
  // get data from sensors
  float Pitch[]  = //some pitchers data
  float xAccel[] = // X direction acceleration

  canMsg.can_id   = 0x07        // can ID to be read
  canMsg.can_dlen = 8;          // length of CAN data to be transmitted
  canMsg.data[0]  = pitch[0];   // Need to parse all these indexes. this is only a placeholder
  canMsg.data[1]  = pitch[1];   //    and needs to be modified 
  canMsg.data[2]  = pitch[2];
  canMsg.data[3]  = pitch[3];
  canMsg.data[4]  = xAccel[0];
  canMsg.data[5]  = xAccel[1];
  canMsg.data[6]  = xAccel[2];
  canMsg.data[7]  = xAccel[3];

  return canMsg;
}

CAN_FRAME *Roll_YAccel_Data()
{
  CAN_FRAME *canMsg;
  // get data from sensors
  float roll[] = //some yawning data
  float yAccel = // the date

  canMsg.can_id   = 0x08        // can ID to be read
  canMsg.can_dlen = 8;          // length of CAN data to be transmitted
  canMsg.data[0]  = roll[0];    // Need to parse all these indexes. this is only a placeholder
  canMsg.data[1]  = roll[1];    //    and needs to be modified 
  canMsg.data[2]  = roll[2];
  canMsg.data[3]  = roll[3];
  canMsg.data[4]  = yAccel[0];
  canMsg.data[5]  = yAccel[1];
  canMsg.data[6]  = yAccel[2];
  canMsg.data[7]  = yAccel[3];

  return canMsg;
}

CAN_FRAME *Yaw_zAccel_Data()
{
  CAN_FRAME *canMsg;
  // get data from sensors
  float yaw[] = //some yawning data
  float zAccel = // the date

  canMsg.can_id   = 0x09        // can ID to be read
  canMsg.can_dlen = 8;          // length of CAN data to be transmitted
  canMsg.data[0]  = yaw[0];     // Need to parse all these indexes. this is only a placeholder
  canMsg.data[1]  = yaw[1];     //    and needs to be modified 
  canMsg.data[2]  = yaw[2];
  canMsg.data[3]  = yaw[3];
  canMsg.data[4]  = zAccel[0];
  canMsg.data[5]  = zAccel[1];
  canMsg.data[6]  = zAccel[2];
  canMsg.data[7]  = zAccel[3];

  return canMsg;
}
