/* Author: Michael Barbosa
 * Started: 11/9/19
 *  Description:
 *    something here
 *    
 */

// header files
#include <SPI.h>

// MACROS
# define CNF_1 0x2A           // control registers for initial setup
# define CNF_2 0x29           // control registers for initial setup
# define CNF_3 0x28           // control registers for initial setup
# define CFG_1 0x00           // control register values for 8MHz
# define CFG_2 0x90           // control register values for 8MHz
# define CFG_3 0x82           // control register values for 8MHz
# define WRITE_INSTRUCTION 0x02
# define MCP2515_CS_LOW 0
# define MCP2515_CS_HIGH 1
# define CAN_MAX_DLEN 8       // number of bytes to send

/* Transmit buffer addresses **********************************************/
// buffer 0
# define MCP_TXB0CTRL 0x30    // transmit buff 0 control reg
# define MCP_TXB0SIDH 0x31    // transmit buff 0 ID reg
# define MCP_TXB0DLC  0x35    // transmit buff 0 Data length reg
# define MCP_TXB0DATA 0x36    // transmit buff 0 Data Byte
// buffer 1
# define MCP_TXB1CTRL 0x40    // transmit buff 1 control reg
# define MCP_TXB1SIDH 0x41    // transmit buff 1 ID reg
# define MCP_TXB1DLC  0x45    // transmit buff 1 Data length reg
# define MCP_TXB1DATA 0x46    // transmit buff 1 Data Byte
// buffer 2
# define MCP_TXB2CTRL 0x50    // transmit buff 2 control reg
# define MCP_TXB2SIDH 0x51    // transmit buff 2 ID reg
# define MCP_TXB2DLC  0x55    // transmit buff 2 Data length reg
# define MCP_TXB2DATA 0x56    // transmit buff 2 Data Byte
/***********************************************************************************/

// pin declarations
const int MCP_csPin = P1_4; // May need to active this depending on SSA data output


// CAN data struct
typedef struct CAN_FRAME {
  uint16_t  can_id;
  unsigned char can_dlen;
  unsigned char data[CAN_MAX_DLEN];   // this is the can data to be sent
} CAN_FRAME;


// Function Declarations
void Mcp_Init();
void CAN_Message();



void McpSetup()
{
  digitalWrite(MCP_csPin, LOW);
  SPI.transfer(WRITE_INSTRUCTION);  // write a byte to a reg
  SPI.transfer(CNF_1);              // set the reg that we are writting to
  SPI.transfer(CFG_1);              // the date we are writing to the reg above

  SPI.transfer(WRITE_INSTRUCTION);  // write a byte to a reg
  SPI.transfer(CNF_2);              // set the reg that we are writting to
  SPI.transfer(CFG_2);              // the date we are writing to the reg above

  SPI.transfer(WRITE_INSTRUCTION);  // write a byte to a reg
  SPI.transfer(CNF_3);              // set the reg that we are writting to
  SPI.transfer(CFG_3);              // the date we are writing to the reg above
  digitalWrite(MCP_csPin, HIGH);
}

/*
void CAN_Message()
{
  SPI.begin();
  SPI.transfer(WRITE_INSTRUCTION);
  SPI.transfer(reg);
  SPI.transfer(canMsg);
  SPI.end();
}
*/

void setup() {
  pinMode(MCP_csPin, OUTPUT);           // set the mcp chipselect as output
  digitalWrite(MCP_csPin, HIGH);        // set it to high so there is no communication. cs is brought low com start
  SPI.setBitOrder(MSBFIRST);            // sets bit order to shift out MSB first
  //SPI.setClockDivider(SPI_CLOCK_DIV2);  // cuts msp clock frequency by 2 from 16Mhz to 8Mhz
  SPI.setDataMode(SPI_MODE0);           // sets the data mode as CPOL=0
  McpSetup();
  
  SPI.begin();                          // initialize SPI 
  
  Serial.begin(9600);                   // begin serial monitor for test
}



void loop() {
  
  AnalogData( *analogptr, wheelSeed );
  Pitch_XAccel_Data( *imuarr );
  Roll_YAccel_Data( *imuarr );
  Yaw_zAccel_Data( *imuarr );
  
}

void AnalogData( char *analogptr, float wheelSpeed )
{
  CAN_FRAME *canMsg;
  
  // get data from sensors
  //char wheelSpeed = analogptr[0];
  char tireTemp_1 = analogptr[0];
  char tireTemp_2 = analogptr[1];
  char tireTemp_3 = analogptr[2];
  //char susTravel  = analogptr[2];   // unused currently

  canMsg -> can_id   = 0x03;        // can ID to be read
  canMsg -> can_dlen = 8;          // length of CAN data to be transmitted
  canMsg -> data[0]  = wheelSpeed; // setting the wheelspeed to the can message
  canMsg -> data[1]  = tireTemp_1;
  canMsg -> data[2]  = tireTemp_2;
  canMsg -> data[3]  = tireTemp_3;
  canMsg -> data[4]  = 0x00;
  canMsg -> data[5]  = 0x00;
  canMsg -> data[6]  = 0x00;
  canMsg -> data[7]  = 0x00;

  // Begin Transmitting message
  SPI.begin();
  digitalWrite(MCP_csPin, LOW);
  SPI.transfer(WRITE_INSTRUCTION);  // set registers to write instruction
  SPI.transfer(MCP_TXB0SIDH);       // select transmit buffer 0 CAN ID reg
  SPI.transfer(canMsg -> can_id);   // send CAN ID
  SPI.transfer(MCP_TXB0DLC);        // select tranmist buffer 0 DATA LENGTH reg
  SPI.transfer(canMsg -> can_dlen); // send CAN DATA LEN

  SPI.transfer(MCP_TXB0DATA);       // select transmit buffer 0 DATA Bytes and send the canMsg
  for ( int i = 0; i < canMsg -> can_dlen; i++ )
  {
    SPI.transfer( canMsg -> data[i] );
  }
  digitalWrite(MCP_csPin, HIGH);
  SPI.end();
}

void Pitch_XAccel_Data( float *imuarr )
{
  CAN_FRAME *canMsg;
  // get data from sensors
  //float Pitch[]  = //some pitchers data
  //float xAccel[] = // X direction acceleration

  byte *b = (byte *)&imuarr;        // splits imuarr from an arr of 2 floats to an array of 8 bytes

  canMsg -> can_id   = 0x07;        // can ID to be read
  canMsg -> can_dlen = 8;           // length of CAN data to be transmitted
  canMsg -> data[0]  = b[0];        // Need to parse all these indexes. this is only a placeholder
  canMsg -> data[1]  = b[1];        //    and needs to be modified 
  canMsg -> data[2]  = b[2];
  canMsg -> data[3]  = b[3];
  canMsg -> data[4]  = b[4];
  canMsg -> data[5]  = b[5];
  canMsg -> data[6]  = b[6];
  canMsg -> data[7]  = b[7];

  // Begin Transmitting message
  SPI.begin();
  digitalWrite(MCP_csPin, LOW);
  SPI.transfer(WRITE_INSTRUCTION);  // set registers to write instruction
  SPI.transfer(MCP_TXB1SIDH);       // select transmit buffer 1 CAN ID reg
  SPI.transfer(canMsg -> can_id);   // send CAN ID
  SPI.transfer(MCP_TXB1DLC);        // select tranmist buffer 1 DATA LENGTH reg
  SPI.transfer(canMsg -> can_dlen); // send CAN DATA LEN

  SPI.transfer(MCP_TXB1DATA);       // select transmit buffer 1 DATA Bytes and send the canMsg
  for ( int i = 0; i < canMsg -> can_dlen; i++ )
  {
    SPI.transfer( canMsg -> data[i] );
  }
  digitalWrite(MCP_csPin, HIGH);
  SPI.end();
}

void Roll_YAccel_Data( float *imuarr )
{
  CAN_FRAME *canMsg;
  // get data from sensors
  //float roll[] = //some yawning data
  //float yAccel = // the date

  byte *b = (byte *)&imuarr;        // splits imuarr from an arr of 2 floats to an array of 8 bytes

  canMsg -> can_id   = 0x08;        // can ID to be read
  canMsg -> can_dlen = 8;           // length of CAN data to be transmitted
  canMsg -> data[0]  = b[8];        // Need to parse all these indexes. this is only a placeholder
  canMsg -> data[1]  = b[9];        //    and needs to be modified 
  canMsg -> data[2]  = b[10];
  canMsg -> data[3]  = b[11];
  canMsg -> data[4]  = b[12];
  canMsg -> data[5]  = b[13];
  canMsg -> data[6]  = b[14];
  canMsg -> data[7]  = b[15];

  // Begin Transmitting message
  SPI.begin();
  digitalWrite(MCP_csPin, LOW);
  SPI.transfer(WRITE_INSTRUCTION);  // set registers to write instruction
  SPI.transfer(MCP_TXB2SIDH);       // select transmit buffer 2 CAN ID reg
  SPI.transfer(canMsg -> can_id);   // send CAN ID
  SPI.transfer(MCP_TXB2DLC);        // select tranmist buffer 2 DATA LENGTH reg
  SPI.transfer(canMsg -> can_dlen); // send CAN DATA LEN

  SPI.transfer(MCP_TXB2DATA);       // select transmit buffer 2 DATA Bytes and send the canMsg
  for ( int i = 0; i < canMsg -> can_dlen; i++ )
  {
    SPI.transfer( canMsg -> data[i] );
  }
  digitalWrite(MCP_csPin, HIGH);
  SPI.end();
}

void Yaw_zAccel_Data( float *imuarr )
{
  CAN_FRAME *canMsg;
  // get data from sensors
  //float yaw[] = //some yawning data
  //float zAccel = // the date

  byte *b = (byte *)&imuarr;        // splits imuarr from an arr of 2 floats to an array of 8 bytes

  canMsg -> can_id   = 0x09;        // can ID to be read
  canMsg -> can_dlen = 8;           // length of CAN data to be transmitted
  canMsg -> data[0]  = b[16];       // Need to parse all these indexes. this is only a placeholder
  canMsg -> data[1]  = b[17];       //    and needs to be modified 
  canMsg -> data[2]  = b[18];
  canMsg -> data[3]  = b[19];
  canMsg -> data[4]  = b[20];
  canMsg -> data[5]  = b[21];
  canMsg -> data[6]  = b[22];
  canMsg -> data[7]  = b[23];

  // Begin Transmitting message
  SPI.begin();
  digitalWrite(MCP_csPin, LOW);
  SPI.transfer(WRITE_INSTRUCTION);  // set registers to write instruction
  SPI.transfer(MCP_TXB0SIDH);       // select transmit buffer 0 CAN ID reg
  SPI.transfer(canMsg -> can_id);   // send CAN ID
  SPI.transfer(MCP_TXB0DLC);        // select tranmist buffer 0 DATA LENGTH reg
  SPI.transfer(canMsg -> can_dlen); // send CAN DATA LEN

  SPI.transfer(MCP_TXB0DATA);       // select transmit buffer 0 DATA Bytes and send the canMsg
  for ( int i = 0; i < canMsg -> can_dlen; i++ )
  {
    SPI.transfer( canMsg -> data[i] );
  }
  digitalWrite(MCP_csPin, HIGH);
  SPI.end();
}
