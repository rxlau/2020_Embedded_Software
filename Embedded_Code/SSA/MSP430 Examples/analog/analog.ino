//Functions:
void getTemp();

//pin declerations
int irpin1 = A0; //3 analog IR sensors
int irpin2 = A1;
int irpin3 = A2;
int sustrv = A3; //analog input for suspension travel, not implemented yet

//read variables
int irread1;
int irread2;
int irread3;
int hallread;
float voltage1, voltage2, voltage3;
float temp1, temp2, temp3;

void setup() {

  pinMode(irpin1, INPUT);
  pinMode(irpin2, INPUT);
  pinMode(irpin3, INPUT);
  pinMode(sustrv, INPUT);

  Serial.begin(9600);
}

void loop() {

//Reading IR sensors
  irread1 = analogRead(irpin1);
  voltage1 = (irread1 / 1023) * 5000;
  temp1 = calcTemp(irread1);
  Serial.print("This is the temperature of infrared sensor 1: ");
  Serial.println(temp1);
  irread2 = analogRead(irpin2);
  voltage2 = (irread2 / 1023) * 5000;
  temp2 = calcTemp(irread2);
  Serial.print("This is the temperature of infrared sensor 2: ");
  Serial.println(temp2);
  irread3 = analogRead(irpin3);
  voltage3 = (irread3 / 1023) * 5000;
  temp3 = calcTemp(irread3);
  Serial.print("This is the temperature of infrared sensor 3: ");
  Serial.println(temp3); 

  Serial.flush();
}

float calcTemp(float ADCread)
{
  float temperature;
  float mVolts = (ADCread / 1023) * 5000;
  if(mVolts < 2000)
  {
    temperature = (mVolts - 500) / 10; //standard function below 2V
  }
  else
  {
    float Volts = mVolts / 1000;
    temperature = (41.413793103448 * Volts) - 29.758620689655; //switch to linear regression of data sheet function after 2V
  }
  return temperature; 
}
