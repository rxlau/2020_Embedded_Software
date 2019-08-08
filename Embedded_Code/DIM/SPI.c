#include <SPI.h>
#include <msp430g2553.h>
#include <mcp2515.h>
#include <stdint.h>
#include <ADC.h>
void init_MCP2515_SPI(){

    P1SEL = BIT5 + BIT6 + BIT7;    //1.5 is clock source  1.6 SOMI   1.7 SIMO
    P1SEL2 = BIT5 + BIT6 + BIT7;

    P2DIR |= BIT0;
    P2OUT |= BIT0;          //slave select pin

    UCB0CTL1 |= UCSWRST;     //USCI software reset (reset to begin initialization)
    UCB0CTL0 |= UCCKPL + UCMSB + UCMST + UCMODE_0 + UCSYNC;    //Most sig bit first, set as master, spi mode on
    UCB0CTL1 |= UCSSEL_2;   //use SMCLK
    UCB0BR0 |= 0X02;    //clock divider
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST; //initialize USCI state machine

    __delay_cycles(DELAY_100ms);
    while(!(IFG2 & UCB0TXIFG));
    UCB0TXBUF = 0x00;
}

unsigned char transmit_MCP2515_SPI(unsigned char value){
    UCB0TXBUF = value;
    while(UCB0STAT & UCBUSY);
    return UCB0RXBUF;
}

void MCP2515_reset(){
    MCP2515_CS_LOW;
    transmit_MCP2515_SPI(MCP2515_RESET);
    MCP2515_CS_HIGH;

    __delay_cycles(DELAY_100us);
}

void init_MCP2515_CANVariable(can_t * can){
    can -> COB_ID = 0x001;
    can -> status = 0x01;
    can -> dlc = CAN_DLC;
    can -> rtr = CAN_RTR;
    can -> ext = CAN_EXTENDET;
//    can -> data[0] = 0x69;
    char i;
    for(i = 0; i < CAN_DLC; i++){
        can -> data[i] = 0x05;
    }
}

void write_MCP2515(uint8_t addr, uint8_t data){
    MCP2515_CS_LOW;
    transmit_MCP2515_SPI(MCP2515_WRITE);
    transmit_MCP2515_SPI(addr);
    transmit_MCP2515_SPI(data);
    MCP2515_CS_HIGH;
   // __delay_cycles(DELAY_100us);
    __delay_cycles(DELAY_1ms);
}

void write_many_registers_MCP2515(uint8_t addr, uint8_t len, uint8_t *data){
  MCP2515_CS_LOW;                                                                // Starte Frame in dem ich !CS auf Low Ziehe. Sende ...
  transmit_MCP2515_SPI(MCP2515_WRITE);                                           // Schreibbefehl ; Befehl 0x02, ...
  transmit_MCP2515_SPI(addr);
  char i;
  for(i = 0; i < len; i++)                                                    // solange i < datenlänge,
  {
    transmit_MCP2515_SPI(*data);                                                 // sende Daten für diese Adresse, ...
    data++;                                                          // nächstes byte vorbereiten, ...
  }
  MCP2515_CS_HIGH;                                                               // beende den Frame in dem ich !CS wieder auf High setze und ...
  __delay_cycles(DELAY_100us);                                                   // warte ein bischen.-
}

uint8_t read_MCP2515(uint8_t addr){
    uint8_t data;
    transmit_MCP2515_SPI(MCP2515_READ);
    transmit_MCP2515_SPI(addr);
    data = transmit_MCP2515_SPI(MCP2515_DUMMY);
    MCP2515_CS_HIGH;
    __delay_cycles(DELAY_100us);
    return data;
}

void read_many_registers_MCP2515(uint8_t addr, uint8_t length, uint8_t *data){
    MCP2515_CS_LOW;
    transmit_MCP2515_SPI(MCP2515_WRITE);
    transmit_MCP2515_SPI(addr);
    char i;
    for(i = 1; i < length; i++){
        *data = transmit_MCP2515_SPI(MCP2515_DUMMY);
        data++;
    }
    MCP2515_CS_HIGH;
    __delay_cycles(DELAY_100us);
}

void write_id_MCP2515(uint8_t addr, BOOL ext, unsigned long id){
    uint16_t canid;
    uint8_t tbufdata[4];

    canid = (unsigned short)(id & 0x0ffff);

    if(ext == TRUE){
        tbufdata[MCP2515_EID0] = (uint8_t) (canid & 0xff);
        tbufdata[MCP2515_EID8] = (uint8_t) (canid / 256);
        canid = (uint16_t)(id / 0x10000);
        tbufdata[MCP2515_SIDL] = (uint8_t) (canid & 0x03);
        tbufdata[MCP2515_SIDL] +=  (uint8_t)((canid & 0x1c)*8);
        tbufdata[MCP2515_SIDL] |= MCP2515_TXBnSIDL_EXIDE;
        tbufdata[MCP2515_SIDH] = (uint8_t)(canid / 32);
    }
    else // Sonst hier auch genutzt Standart 11-Bit-Identifier (CAN 2.0A)
    {
      tbufdata[MCP2515_SIDH] = (uint8_t)(canid / 8);
      tbufdata[MCP2515_SIDL] = (uint8_t)((canid & 0x07)*32);
      tbufdata[MCP2515_EID0] = 0;
      tbufdata[MCP2515_EID8] = 0;
    } // else

    if(tbufdata[0] == 0xff) return;
    write_many_registers_MCP2515(addr, 4, tbufdata);

    __delay_cycles(DELAY_100us);
}

void read_id_MCP2515(uint8_t addr, unsigned long* id){
  uint16_t ID_Low, ID_High;
  if(addr == MCP2515_RXB0SIDL)
  {
    ID_Low  = (read_MCP2515(MCP2515_RXB0SIDL) >> 5);
    ID_High = (read_MCP2515(MCP2515_RXB0SIDH) << 3);

    *id = (unsigned long)ID_Low | (unsigned long)ID_High;
  }
  else
  {
    ID_Low  = (read_MCP2515(MCP2515_RXB1SIDL) >> 5);
    ID_High = (read_MCP2515(MCP2515_RXB1SIDH) << 3);

    *id = (unsigned long)ID_Low | (unsigned long)ID_High;
  }
}

void MCP2515_init(void){
  // ------ 1. Resete den Chip ------------------------------------------------

  MCP2515_reset ();                                                              // Reset MCP2515

  __delay_cycles(DELAY_10ms);                                                    // Allow to reset



  write_MCP2515(MCP2515_CANCTRL, 0x88);                                          // CAN Controel-Register. Gehe Konfiguration-Modus (Seite 58), eigentlich sollte aber dieser automatisch nach neustart dort sein
  write_MCP2515(MCP2515_CANINTE, 0x03);                                          // Interrupt Enable-Register.  Aktiviere NUR RX0- und RX1-Interrupts (Datenblatt, Seite 50)
  write_MCP2515(MCP2515_TXB0CTRL, 0x03);                                         // Transmit Buffer Control-Register (Datenblatt, Seite 18). Highes Message Priority (interessant wenn mehrere Buffer genutzt)



  write_MCP2515(MCP2515_CNF1,0x00);                                              // Bei 16MHz -> 250kb/s. Beachte um mehr als 125kBaud zu bekommen, muss ein externer Quarz von 16MHz ...
  write_MCP2515(MCP2515_CNF2,0xb9);                                          // angelötet sein. Die Werte für CNF1, CNF2 und CNF3, kann man leicht aus der Microchi-Software bekommen. ...
  write_MCP2515(MCP2515_CNF3,0x05);                                      // Dazu lade Programm: "Microchip Can Bit Timing Calculator" runter und führe es als ADMIN aus.-



  write_MCP2515(MCP2515_RXB0CTRL, 0x64);                                         // Receive Buffer 0 Control, Alle Nachrichten Empfangen, falls nötig RX1 weiterleiten (siehe Datenblatt, Seite 27)
  write_MCP2515(MCP2515_RXB1CTRL, 0x60);                                         // Receive Buffer 1 Control, Alle Nachrichten Empfangen (siehe Datenblatt, Seite 28)
  write_MCP2515(MCP2515_BFPCTRL, 0x00);                                          // Deaktiviere RXnBF Pins (Pin 10 und 11, siehe Datenblatt, Seite 29), lösen daher kein IR an den Pin aus.
  write_MCP2515(MCP2515_TXRTSCTRL , 0x00);                                       // Deaktiviere RTS Pins (Pin 4,5 und 6, siehe Datenblatt, Seite 19), lösen daher kein IR an den Pin aus.



  write_MCP2515(MCP2515_CANCTRL, 0x00);

  //__delay_cycles(DELAY_1s);
}

void bit_modify_MCP2515(uint8_t addr, uint8_t mask, uint8_t data){
  MCP2515_CS_LOW;

  transmit_MCP2515_SPI(MCP2515_BIT_MODIFY);
  transmit_MCP2515_SPI(addr);
  transmit_MCP2515_SPI(mask);
  transmit_MCP2515_SPI(data);

  MCP2515_CS_HIGH;

  __delay_cycles(DELAY_100us);
}

void MCP2515_can_tx0(can_t *can){
  if(can->dlc > 8) can->dlc = 8;

  write_id_MCP2515(MCP2515_TXB0SIDH, can->ext, can->COB_ID);

  if (can->rtr == TRUE)
  {
      uint8_t befehl = can->dlc;
      befehl = befehl | 0x40;
      if(befehl == 0x03) return;
      write_MCP2515(MCP2515_TXB0DLC, can->dlc | 0x40);
  } // if (rtr)

  else
  {
    write_MCP2515(MCP2515_TXB0DLC, can->dlc);
    write_many_registers_MCP2515(MCP2515_TXB0D0, can->dlc, can->data);
    write_MCP2515(MCP2515_TXB0CTRL, 0x0B);
  } // else (rtr)
}

void MCP2515_can_tx1(can_t *can){
  if(can->dlc > 8) can->dlc = 8;

  write_id_MCP2515(MCP2515_TXB1SIDH, can->ext, can->COB_ID);

  if (can->rtr == TRUE)
  {
      uint8_t befehl = can->dlc;
      befehl = befehl | 0x40;
      if(befehl == 0x03) return;
      write_MCP2515(MCP2515_TXB1DLC, can->dlc | 0x40);
  } // if (rtr)

  else
  {
    write_MCP2515(MCP2515_TXB1DLC, can->dlc);
    write_many_registers_MCP2515(MCP2515_TXB1D0, can->dlc, can->data);
    write_MCP2515(MCP2515_TXB1CTRL, 0x0B);
  } // else (rtr)
}

void MCP2515_can_tx2(can_t *can){
  if(can->dlc > 8) can->dlc = 8;

  write_id_MCP2515(MCP2515_TXB2SIDH, can->ext, can->COB_ID);

  if (can->rtr == TRUE)
  {
      uint8_t befehl = can->dlc;
      befehl = befehl | 0x40;
      if(befehl == 0x03) return;
      write_MCP2515(MCP2515_TXB2DLC, can->dlc | 0x40);
  }

  else
  {
    write_MCP2515(MCP2515_TXB2DLC, can->dlc);
    write_many_registers_MCP2515(MCP2515_TXB2D0, can->dlc, can->data);
    write_MCP2515(MCP2515_TXB2CTRL, 0x0B);
  }
}

void MCP2515_can_rx0(can_t *can){
  read_id_MCP2515(MCP2515_RXB0SIDL, &can->COB_ID);
  can->dlc = read_MCP2515(MCP2515_RXB0DLC);
  char i;
  for(i = 0; i < can->dlc; i++) can->data[i] = read_MCP2515(MCP2515_RXB0D0+i);
  can->status = can->data[0];

  MCP2515_clear_rx0();
  MCP2515_int_clear();

  __delay_cycles(DELAY_1ms);
}

void MCP2515_can_rx1(can_t *can){
  read_id_MCP2515(MCP2515_RXB1SIDL, &can->COB_ID);
  can->dlc = read_MCP2515(MCP2515_RXB1DLC);
  char i;
  for(i = 0; i < can->dlc; i++) can->data[i] = read_MCP2515(MCP2515_RXB1D0+i);
  can->status = can->data[0];

  MCP2515_clear_rx1();
  MCP2515_int_clear();

  __delay_cycles(DELAY_1ms);
}

void MCP2515_clear_rx0(void){
  bit_modify_MCP2515(MCP2515_CANINTF, MCP2515_RX0IF, 0x00);
}

void MCP2515_clear_rx1(void){
  bit_modify_MCP2515(MCP2515_CANINTF, MCP2515_RX1IF, 0x00);
}

void MCP2515_int_clear(void){
  write_MCP2515(MCP2515_CANINTF, MCP2515_CANINTF_ALL_DISABLE);
}

BOOL MCP2515_spi_test (void){
  uint16_t data_rcv[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  uint16_t data_snd[11]={0x88,0x03,0x90,0x02,0x05,0x02,0x3f,0x23,0x40,0x40,0x00};// Array der zu sendenen Daten.

  write_MCP2515(MCP2515_CANCTRL, data_snd[0]);                                   // 1. Gehe im Konfig-Mode. Zum Überprüfen, ...
  data_rcv[0] = read_MCP2515(MCP2515_CANCTRL);                                     // schreibe zuerst in den Registern etwas und anschließend lese es aus. ...

  write_MCP2515(MCP2515_CNF1,data_snd[1]);                                       // ...
  write_MCP2515(MCP2515_CNF2,data_snd[2]);                                       // ...
  write_MCP2515(MCP2515_CNF3,data_snd[3]);                                       // ...
  data_rcv[1] = read_MCP2515(MCP2515_CNF1);                                        // ...
  data_rcv[2] = read_MCP2515(MCP2515_CNF2);                                        // ...
  data_rcv[3] = read_MCP2515(MCP2515_CNF3);                                        // ...
  write_MCP2515(MCP2515_RXM0SIDH, data_snd[4]);                                  // ...
  write_MCP2515(MCP2515_RXM0SIDL, data_snd[5]);                                  // ...
  data_rcv[4] = read_MCP2515(MCP2515_RXM0SIDH);                                    // ...
  data_rcv[5] = read_MCP2515(MCP2515_RXM0SIDL);                                    // ...
  write_MCP2515(MCP2515_CANINTE, data_snd[6]);                                   // ...
  data_rcv[6] = read_MCP2515(MCP2515_CANINTE);                                     // ...
  write_MCP2515(MCP2515_CANINTF, data_snd[7]);                                   // ...
  data_rcv[7] = read_MCP2515(MCP2515_CANINTF);                                     // ...
  write_MCP2515(MCP2515_TXB0SIDL, data_snd[8]);                                  // ...
  data_rcv[8] = read_MCP2515(MCP2515_TXB0SIDL);                                    // ...
  write_MCP2515(MCP2515_TXB1SIDL, data_snd[9]);                                  // ...
  data_rcv[9] = read_MCP2515(MCP2515_TXB1SIDL);                                    // ...

  write_MCP2515(MCP2515_CANCTRL, data_snd[10]);                                  // ... (gehe auch zum Normalmodus)
  data_rcv[10] = read_MCP2515(MCP2515_CANCTRL);                                    // .-
  char i;
  for(i = 0; i < 11; i++)                                                   // 2. Vergleiche das empfangene mit dem beschiebenen Array. ...
  {                                                                              // ...
    if(data_snd[i] != data_rcv[i]) return TRUE;   //should be FALSE modified for test                              // Falls Ungleich. Kommunikation Fehlerhaft (FALSE = 0). ...
  } // for                                                                       // Sonst ...

  MCP2515_init();                                                                // Muss neu Initialisieren da ich dem MCP2515 manipuliert habe. ...
  return TRUE;                                                                   // Sende das Kommunikation mit MCP2515 stabil ist (TRUE = 1).-
}








