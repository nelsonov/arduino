#include <AltSoftSerial.h>


String str;
int rxcount;

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
  rxcount=-1;
}

void loop() {
  // put your main code here, to run repeatedly:
  if(altSerial.available() > 0)
  {
      str = altSerial.readStringUntil('\t');
      Serial.print(str);
      rxcount = altSerial.parseInt();
      str = altSerial.readStringUntil('\n');
      Serial.print(" ");
      Serial.println(rxcount);
#ifdef RS485_DE_PIN
      digitalWrite(RS485_DE_PIN, HIGH);
      delay(10);
#endif    
      altSerial.print("Echo Reply:   ");
      altSerial.print(rxcount);
#ifdef RS485_DE_PIN
      altSerial.flush();
      delay(10);
      digitalWrite(RS485_DE_PIN, LOW);
#endif    
  }

}
