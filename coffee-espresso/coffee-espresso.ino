/*
 Original script from
 
 https://forum.mysensors.org/topic/4847/multi-button-relay-sketch/34

 https://forum.mysensors.org/user/korttoma
*/
// Enable debug prints to serial monitor
#define SN "Coffee Espresso"
#define SV "0.1"
#define MY_NODE_ID 10

#define MY_DEBUG
#define MY_BAUD_RATE 9600

 /***************nRF24***********************/
// Default Pinout
//  GND  <-->   GND     Black
// 3.3V  <-->   3.3V    Red
//    9  <-->   CE      Orange
//   10  <-->   CSN/CS  Yellow
//   11  <-->   MOSI    Blue
//   12  <-->   MISO    Violet
//   13  <-->   SCK     Green
//    2  <-->   IRQ     Gray  (Optional)
#define   MY_RADIO_RF24
#define   MY_RADIO_NRF24
#define   MY_RF24_CE_PIN  9   // Default
#define   MY_RF24_CS_PIN  10  // Default
//#define   MY_RF24_IRQ_PIN     // OPTIONAL
//#define   MY_RF24_POWER_PIN   // OPTIONAL
//#define   MY_RF24_CHANNEL     // OPTIONAL
//#define   MY_RF24_DATARATE    // OPTIONAL
#define   MY_RF24_SPI_SPEED   (2*1000000ul)
//#define   MY_DEBUG_VERBOSE_RF24
/**********END nRF24*************************/


#include <MySensors.h>
#include <SPI.h>
#include <Bounce2.h>
#define RELAY_ON 0                      // switch around for ACTIVE LOW / ACTIVE HIGH relay
#define RELAY_OFF 1
//

#define noRelays 2                    //2-4
const int relayPin[] = {8, 7};       //  switch around pins to your desire
const int buttonPin[] = {5, 4};   //  switch around pins to your desire
bool initialValueSent = false;
class Relay             // relay class, store all relevant data (equivalent to struct)
{
  public:
    int buttonPin;                   // physical pin number of button
    int relayPin;             // physical pin number of relay
    boolean relayState;               // relay status (also stored in EEPROM)
};

Relay Relays[noRelays];
Bounce debouncer[noRelays];
MyMessage msg[noRelays];

/*
  void before() {
  for (int sensor=1, pin=RELAY_1; sensor<=NUMBER_OF_RELAYS;sensor++, pin++) {
    // Then set relay pins in output mode
    pinMode(pin, OUTPUT);
    // Set relay to last known state (using eeprom storage)
    digitalWrite(pin, loadState(sensor)?RELAY_ON:RELAY_OFF);
  }
    }*/

void setup() {
  wait(100);
  // Initialize Relays with corresponding buttons
  for (int i = 0; i < noRelays; i++) {
    Relays[i].buttonPin = buttonPin[i];              // assign physical pins
    Relays[i].relayPin = relayPin[i];
    msg[i].sensor = i;                                   // initialize messages
    msg[i].type = V_STATUS;
    pinMode(Relays[i].buttonPin, INPUT_PULLUP);
    wait(100);
    pinMode(Relays[i].relayPin, OUTPUT);
    Relays[i].relayState = loadState(i);                               // retrieve last values from EEPROM
    digitalWrite(Relays[i].relayPin, Relays[i].relayState ? RELAY_ON : RELAY_OFF); // and set relays accordingly
    //send(msg[i].set(Relays[i].relayState ? true : false));                 // move to controller_presentation()
    wait(50);
    debouncer[i] = Bounce();                        // initialize debouncer
    debouncer[i].attach(buttonPin[i]);
    debouncer[i].interval(30);
    wait(50);
  }
}

void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo(SN, SV);

  wait(100);

  for (int i = 0; i < noRelays; i++)
    present(i, S_BINARY);      // present sensor to gateway

  wait(100);
}

void loop()
{
  if (!initialValueSent) {
    controller_presentation();
  }
  for (byte i = 0; i < noRelays; i++) {
    if (debouncer[i].update()) {
      
      int value = debouncer[i].read();
      
      if ( value == LOW) {
        Serial.print("Button Pressed: ");
        Serial.println(i);
        Relays[i].relayState = !Relays[i].relayState;
        digitalWrite(Relays[i].relayPin, Relays[i].relayState ? RELAY_ON : RELAY_OFF);
        send(msg[i].set(Relays[i].relayState ? true : false));
        // save sensor state in EEPROM (location == sensor number)
        saveState( i, Relays[i].relayState );

      }

    }
  }
  //wait(20);
}

void receive(const MyMessage &message) {
  if (message.type == V_STATUS) {
    if (message.sensor < noRelays) {          // check if message is valid for relays..... previous line  [[[ if (message.sensor <=noRelays){ ]]]
      Serial.print("RCVD MSG change relay state: ");
      Serial.print(message.sensor);
      Serial.print("  ");
      Serial.println(message.getBool());
      Relays[message.sensor].relayState = message.getBool();
      digitalWrite(Relays[message.sensor].relayPin, Relays[message.sensor].relayState ? RELAY_ON : RELAY_OFF); // and set relays accordingly
      send(msg[message.sensor].set(Relays[message.sensor].relayState ? true : false));
      saveState( message.sensor, Relays[message.sensor].relayState ); // save sensor state in EEPROM (location == sensor number)
    }
  }
  wait(20);
}

void controller_presentation()
{
  wait(100);
  for (int i = 0; i < noRelays; i++)
  {
    send(msg[i].set(Relays[i].relayState ? true : false));
  }
  wait(100);
  initialValueSent = true;
}

