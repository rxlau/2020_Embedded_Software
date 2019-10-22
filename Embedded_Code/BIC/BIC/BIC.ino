#include <SD.h>
// set this to the hardware serial port you wish to use
#define BMS1 Serial1
#define BMS2 Serial2
#define VOLTAGE_SIZE 8
#define CURRENT_SIZE 8
#define SOC_SIZE 8
#define TEMP_SIZE 11
#define OS_SIZE 8
#define CELL_SIZE 33

File logTinyBMS;
//IntervalTimer fans;
unsigned char read_byte;
unsigned char voltageBMS1[VOLTAGE_SIZE];
unsigned char voltageBMS2[VOLTAGE_SIZE];
unsigned char currentBMS1[CURRENT_SIZE];
unsigned char currentBMS2[CURRENT_SIZE];
unsigned char socBMS1[SOC_SIZE];
unsigned char socBMS2[SOC_SIZE];
unsigned char tempBMS1[TEMP_SIZE];
unsigned char tempBMS2[TEMP_SIZE];
unsigned char onlineBMS1[OS_SIZE];
unsigned char onlineBMS2[OS_SIZE];
unsigned char cellsBMS1[CELL_SIZE];
unsigned char cellsBMS2[CELL_SIZE];

/*void CAN_CritWrite(void){
  Can0.write(msg);
  count++;
  delay(100);
  delay(100);
}*/

void writeVoltage(int bms){
  int sizeA = 4;
  unsigned char v[sizeA] = {0xAA, 0x14, 0x7F, 0x1F};
  if(bms == 1){
    for(int i = 0; i < sizeA; i++){
      BMS1.write(v[i]);
    }
  }
  else if(bms == 2){
    for(int i = 0; i < sizeA; i++){
      //BMS2.write(v[i]);
    }
  }
}
void writeCurrent(int bms){
  int sizeA = 4;
  unsigned char c[sizeA] = {0xAA, 0x15, 0xBE, 0xDF};
  if(bms == 1){
    for(int i = 0; i < sizeA; i++){
      BMS1.write(c[i]);
    }
  }
  else if(bms == 2){
    for(int i = 0; i < sizeA; i++){
      //BMS2.write(c[i]);
    }
  }
}

void writeSOC(int bms){
  int sizeA = 4;
  unsigned char soc[sizeA] = {0xAA, 0x1A, 0xFE, 0xDB};
  if(bms == 1){
    for(int i = 0; i < sizeA; i++){
      BMS1.write(soc[i]);
    }
  }
  else if(bms == 2){
    for(int i = 0; i < sizeA; i++){
      BMS2.write(soc[i]);
    }
  }
}
void writeTemp(int bms){
  int sizeA = 4;
  unsigned char temp[sizeA] = {0xAA, 0x1B, 0x3F, 0x1B};
    if(bms == 1){
    for(int i = 0; i < sizeA; i++){
      BMS1.write(temp[i]);
    }
  }
  else if(bms == 2){
    for(int i = 0; i < sizeA; i++){
      BMS2.write(temp[i]);
    }
  }
}
void writeOnlineStat(int bms){
  int sizeA = 4;
  unsigned char os[sizeA] = {0xAA, 0x18, 0x7F, 0x1A};
  if(bms == 1){
    for(int i = 0; i < sizeA; i++){
      BMS1.write(os[i]);
    }
  }
  else if(bms == 2){
    for(int i = 0; i < sizeA; i++){
      BMS2.write(os[i]);
    }
  }
}
void writeCells(int bms){
  int sizeA = 8;
  unsigned char cells[sizeA] = {0xAA, 0x03, 0x00, 0x00, 0x00, 0x0E, 0xDD, 0xD5};
  if(bms == 1){
    for(int i = 0; i < sizeA; i++){
      BMS1.write(cells[i]);
    }
  }
  else if(bms == 2){
    for(int i = 0; i < sizeA; i++){
      BMS2.write(cells[i]);
    }
  }
}

//void pwmPulse(){
//  //digitalWrite(2, HIGH);
//  Serial.print("\nHIGH\n");
//  delayMicroseconds(1.5);
//  //digitalWrite(2, LOW);
//  Serial.print("LOW\n");
//  delayMicroseconds(1.5);
//}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); 
  BMS1.begin(115200);
  BMS2.begin(115200);
  pinMode(2, OUTPUT);
  analogWriteFrequency(2, 3);
  SD.begin();
  
  //fans.begin(pwmPulse, 1500000);

//  // initialize Timer1
//  noInterrupts(); // disable all interrupts
//  TCCR1A = 0;
//  TCCR1B = 0;
//  TCNT1 = 0;
//  
//  OCR1A = 31250; // compare match register 16MHz/256/2Hz
//  TCCR1B |= (1 &lt;&lt; WGM12); // CTC mode
//  TCCR1B |= (1 &lt;&lt; CS12); // 256 prescaler
//  TIMSK1 |= (1 &lt;&lt; OCIE1A); // enable timer compare interrupt
  //interrupts(); // enable all interrupts

  //Can0.begin(125000); 
  printf("Setup finished");
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  /*
   * Voltage- 0xAA, 0x14, 0x7F, 0x1F 
   * Current- 0xAA, 0x15, 0xBE, 0xDF
   * Cells 1-14- 0xAA, 0x03, 0x00, 0x00, 0x00, 0x0E, 0xDD, 0xD5   
   * State of Charge- 0xAA, 0x1A, 0xFE, 0xDB
   * //change internal temp to device temperatures (read all 3)
   * //1.5 sec time toggle a gpio pin 3Hz 50%duty
   * //Internal temperature- 0xAA, 0x09, 0x02, 0x30, 0x00, 0x8A, 0x44 (48 in hex?)
   * All Temps- 0xAA, 0x1B, 0x3F, 0x1B
   * Online Status- 0xAA, 0x18, 0x7F, 0x1A
   * 
   * Write data to SD card (log info would send over CAN)
   * 
  */
  byte read_byte;
  //interrupts();
  logTinyBMS = SD.open("LOG.txt", FILE_WRITE);
/*VOLTAGE*/
  //Write VOLTAGE 1
  writeVoltage(1);
  //Read VOLTAGE 1
  for(int i = 0; i < VOLTAGE_SIZE; i++){
    read_byte = BMS1.read();
    if(i == 1 && read_byte == 0x00){
      Serial.print("Error reading BMS 1 Voltage\n");
      logTinyBMS.print("Error reading BMS 1 Voltage\n");
      break;
    }
    Serial.print(read_byte, DEC);
    logTinyBMS.print(read_byte, DEC);
    Serial.print(" ");
    logTinyBMS.print(" ");
    voltageBMS1[i] = read_byte;
  }
  Serial.print("\n");
  logTinyBMS.print("\n");
  //print overall voltage value
  float volt1 = voltageBMS1[2] + (voltageBMS1[3]<<8) + (voltageBMS1[4]<<16) + (voltageBMS1[5]<<24);
  Serial.print("---->OVERALL VOLTAGE BMS 1: ");
  Serial.print(volt1);
  Serial.print("\n");
  logTinyBMS.print("---->OVERALL VOLTAGE BMS 1: ");
  logTinyBMS.print(volt1);
  logTinyBMS.print("\n");
  
  //Write VOLTAGE 2
  writeVoltage(2);
  //Read VOLTAGE 2
  for(int i = 0; i < VOLTAGE_SIZE; i++){
    read_byte = BMS2.read();
    if(i == 1 && read_byte == 0x00){
      Serial.print("Error reading BMS 2 Voltage\n");
      logTinyBMS.print("Error reading BMS 2 Voltage\n");
      break;
    }
    Serial.print(read_byte, DEC);
    logTinyBMS.print(read_byte, DEC);
    Serial.print(" ");
    logTinyBMS.print(" ");
    voltageBMS2[i] = read_byte;
  }
  Serial.print("\n");
  logTinyBMS.print("\n");
  float volt2 = voltageBMS2[2] + (voltageBMS2[3]<<8) + (voltageBMS2[4]<<16) + (voltageBMS2[5]<<24);
  Serial.print("---->OVERALL VOLTAGE BMS 2: ");
  Serial.print(volt2);
  Serial.print("\n");
  logTinyBMS.print("---->OVERALL VOLTAGE BMS 2: ");
  logTinyBMS.print(volt2);
  logTinyBMS.print("\n");

/*CURRENT*/
  //Write CURRENT 1
  writeCurrent(1);
  //Read CURRENT 1
  for(int i = 0; i < CURRENT_SIZE; i++){
    read_byte = BMS1.read();
    if(i == 1 && read_byte == 0x00){
      Serial.print("Error reading BMS 1 Current\n");
      logTinyBMS.print("Error reading BMS 1 Current\n");
      break;
    }
    Serial.print(read_byte, DEC);
    logTinyBMS.print(read_byte, DEC);
    Serial.print(" ");
    logTinyBMS.print(" ");
    currentBMS1[i] = read_byte;
  }
  Serial.print("\n");
  logTinyBMS.print("\n");
  float curr1 = currentBMS1[2] + (currentBMS1[3]<<8) + (currentBMS1[4]<<16) + (currentBMS1[5]<<24);
  Serial.print("---->OVERALL CURRENT BMS 1: ");
  Serial.print(curr1);
  Serial.print("\n");
  logTinyBMS.print("---->OVERALL CURRENT BMS 1: ");
  logTinyBMS.print(curr1);
  logTinyBMS.print("\n");
  
  //Write CURRENT 2
  writeCurrent(2);
  //Read CURRENT 2
  for(int i = 0; i < CURRENT_SIZE; i++){
    read_byte = BMS2.read();
    if(i == 1 && read_byte == 0x00){
      Serial.print("Error reading BMS 2 Current\n");
      logTinyBMS.print("Error reading BMS 2 Current\n");
      break;
    }
    Serial.print(read_byte, DEC);
    logTinyBMS.print(read_byte, DEC);
    Serial.print(" ");
    logTinyBMS.print(" ");
    currentBMS2[i] = read_byte;
  }
  Serial.print("\n");
  logTinyBMS.print("\n");
  float curr2 = currentBMS2[2] + (currentBMS2[3]<<8) + (currentBMS2[4]<<16) + (currentBMS2[5]<<24);
  Serial.print("---->OVERALL CURRENT BMS 2: ");
  Serial.print(curr2);
  Serial.print("\n");
  logTinyBMS.print("---->OVERALL CURRENT BMS 2: ");
  logTinyBMS.print(curr2);
  logTinyBMS.print("\n");


/*STATE OF CHARGE*/
 //Write SOC 1
  writeSOC(1);
  //Read SOC 1
  for(int i = 0; i < SOC_SIZE; i++){
    read_byte = BMS1.read();
    Serial.print(read_byte, DEC);
    logTinyBMS.print(read_byte, DEC);
    Serial.print(" ");
    logTinyBMS.print(" ");
    socBMS1[i] = read_byte;
  }
  Serial.print("\n");
  logTinyBMS.print("\n");
  
  //Write SOC 2
  writeSOC(2);
  //Read SOC 2
  for(int i = 0; i < SOC_SIZE; i++){
    read_byte = BMS2.read();
    Serial.print(read_byte, DEC);
    logTinyBMS.print(read_byte, DEC);
    Serial.print(" ");
    logTinyBMS.print(" ");
    socBMS2[i] = read_byte;
  }
  Serial.print("\n");
  logTinyBMS.print("\n");


/*ALL TEMPERATURES*/
  //Write TEMP 1
  writeTemp(1);
  //Read TEMP 1
  for(int i = 0; i < TEMP_SIZE; i++){
    read_byte = BMS1.read();
    Serial.print(read_byte, DEC);
    logTinyBMS.print(read_byte, DEC);
    Serial.print(" ");
    logTinyBMS.print(" ");
    tempBMS1[i] = read_byte;
  }
  Serial.print("\n");
  logTinyBMS.print("\n");

  
  //Write TEMP 2
  writeTemp(2);
  //Read TEMP 2
  for(int i = 0; i < TEMP_SIZE; i++){
    read_byte = BMS2.read();
    Serial.print(read_byte, DEC);
    logTinyBMS.print(read_byte, DEC);
    Serial.print(" ");
    logTinyBMS.print(" ");
    tempBMS2[i] = read_byte;
  }
  Serial.print("\n");
  logTinyBMS.print("\n");
  

/*ONLINE STATUS*/
  //Write ONLINE STAT 1
  writeOnlineStat(1);
  //Read ONLINE STAT 1
  for(int i = 0; i < OS_SIZE; i++){
    read_byte = BMS1.read();
    if(i == 1 && read_byte == 0x00){
      Serial.print("Error reading BMS 1 Online Status\n");
      logTinyBMS.print("Error reading BMS 1 Online Status\n");
      break;
    }
    Serial.print(read_byte, DEC);
    logTinyBMS.print(read_byte, DEC);
    Serial.print(" ");
    logTinyBMS.print(" ");
    onlineBMS1[i] = read_byte;
  }
  Serial.print("\n");
  logTinyBMS.print("\n");
  Serial.print("---->ONLINE STATUS BMS 1: ");
  Serial.print(onlineBMS1[2]);
  Serial.print("\n");
  logTinyBMS.print("---->ONLINE STATUS BMS 1: ");
  logTinyBMS.print(onlineBMS1[2]);
  logTinyBMS.print("\n");

  //Write ONLINE STAT 2
  writeOnlineStat(2);
  //Read ONLINE STAT 2
  for(int i = 0; i < OS_SIZE; i++){
    read_byte = BMS2.read();
    if(i == 1 && read_byte == 0x00){
      Serial.print("Error reading BMS 2 Online Status\n");
      logTinyBMS.print("Error reading BMS 2 Online Status\n");
      break;
    }
    Serial.print(read_byte, DEC);
    logTinyBMS.print(read_byte, DEC);
    Serial.print(" ");
    logTinyBMS.print(" ");
    onlineBMS2[i] = read_byte;
  }
  Serial.print("\n");
  logTinyBMS.print("\n");
  Serial.print("---->ONLINE STATUS BMS 2: ");
  Serial.print(onlineBMS2[2]);
  Serial.print("\n");
  logTinyBMS.print("---->ONLINE STATUS BMS 2: ");
  logTinyBMS.print(onlineBMS2[2]);
  logTinyBMS.print("\n");



/*CELLS 1-14*/
  //Write CELLS 1
  writeCells(1);
  //Read CELLS 1
  for(int i = 0; i < CELL_SIZE; i++){
    read_byte = BMS1.read();
    Serial.print(read_byte, DEC);
    logTinyBMS.print(read_byte, DEC);
    Serial.print(" ");
    logTinyBMS.print(" ");
    cellsBMS1[i] = read_byte;
  }
  Serial.print("\n");
  logTinyBMS.print("\n");
  
  
  //Write CELLS 2
  writeCells(2);
  //Read CELLS 2
  for(int i = 0; i < CELL_SIZE; i++){
    read_byte = BMS2.read();
    Serial.print(read_byte, DEC);
    logTinyBMS.print(read_byte, DEC);
    Serial.print(" ");
    logTinyBMS.print(" ");
    cellsBMS2[i] = read_byte;
  }
  Serial.print("\n");
  logTinyBMS.print("\n");
  logTinyBMS.close();

  analogWrite(2, 128);
}
