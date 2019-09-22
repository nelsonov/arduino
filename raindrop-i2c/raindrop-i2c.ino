#include <Wire.h>
 
#define I2CADDR 0x08
#define ANALOGPIN 0
const long interval = 3000; //miliseconds between sensor reed

byte raindropOutput = 0;
byte raindropValue = 0;
byte splitbytes[] = {0, 0};
unsigned int regIndex = 0;
unsigned long previousMillis = 0; 

void requestData()
{
  Serial.print("Replying to Master ");
  Serial.println(raindropOutput);
  Wire.write(raindropValue);
  /*
  Serial.println(splitbytes[regIndex]);
  Wire.write(splitbytes[regIndex]);
  regIndex=0;  // always reset register back to 0
  */
}

/*
void receiveData ()
{
  while(Wire.available()) {
    regIndex = Wire.read();
    Serial.print("Got data: ");
    Serial.println(regIndex);
  }
}
*/

void setup()
{
  Wire.begin(I2CADDR);
  Wire.onRequest(requestData);
  //  Wire.onReceive(receiveData);
  Serial.begin(115200);
  Serial.println("Initialized");
}

void readSensor()
{
  unsigned int rawValue = analogRead(ANALOGPIN);
  unsigned int invertedValue = 1024 - rawValue;
  Serial.print(invertedValue);
  raindropValue = invertedValue >> 2;
  Serial.print(" ");
  Serial.println(raindropValue);
  
  /*
  splitbytes[0]=raindropValue & 0xff;
  splitbytes[1]=raindropValue >> 8;
  Serial.print(raindropValue);
  Serial.print(" ");
  Serial.print(splitbytes[0]);
  Serial.print(" ");
  Serial.print(splitbytes[1]);
  Serial.println();
  */
}

void loop()     
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    readSensor();
  }
}
