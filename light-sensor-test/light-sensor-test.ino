#define SAMPLE 20                    //this is how many SAMPLEs the device takes per reading
#define ANALOG_PIN A0

void setup() {
  Serial.begin(115200);
}

void loop(void)
{
  int16_t adc0 = 0;
  long sum = 0;
  int16_t reportval = 0;
  int i;
  for (i=0; i < SAMPLE; i++) {
    adc0 = analogRead(ANALOG_PIN);
    sum += adc0;
  }
  reportval = 1024 - (sum / SAMPLE) ;
  Serial.println(reportval);
  delay(1000);
}

