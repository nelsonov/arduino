#include <AltSoftSerial.h>


String str;
int txcount;
AltSoftSerial altSerial;

#define RS485_DE_PIN 7

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  altSerial.begin(9600);
#ifdef RS485_DE_PIN
  pinMode(RS485_DE_PIN, OUTPUT);
  digitalWrite(RS485_DE_PIN, LOW);
#endif
  txcount=0;
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("TX Echo Request: ");
  Serial.println(txcount);
#ifdef RS485_DE_PIN
  digitalWrite(RS485_DE_PIN, HIGH);
  delay(10);
#endif    
  altSerial.print("Echo Request\t");
  altSerial.println(txcount);
#ifdef RS485_DE_PIN
  altSerial.flush();
  delay(10);
  digitalWrite(RS485_DE_PIN, LOW);
#endif  
  delay(1000);
  if(altSerial.available() > 0)
  {
    str = altSerial.readString();
    Serial.print("RX ");
    Serial.println(str);
  } else {
    Serial.print("TO:              ");
    Serial.println(txcount);
  }
  txcount++;
}

