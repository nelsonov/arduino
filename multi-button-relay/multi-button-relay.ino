/*
 Original script from
 
 https://forum.mysensors.org/topic/4847/multi-button-relay-sketch/34

 https://forum.mysensors.org/user/korttoma
*/
// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24

#define SN "RelayButtonArray"
#define SV "1.0"

#include <MySensors.h>
#include <SPI.h>
#include <Bounce2.h>
#define RELAY_ON 0                      // switch around for ACTIVE LOW / ACTIVE HIGH relay
#define RELAY_OFF 1
//

#define noRelays 4                     //2-4
const int relayPin[] = {14, 15, 16, 17};       //  switch around pins to your desire
const int buttonPin[] = {6, 7, 4, 5};   //  switch around pins to your desire

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
    msg[i].type = V_LIGHT;
    pinMode(Relays[i].buttonPin, INPUT_PULLUP);
    wait(100);
    pinMode(Relays[i].relayPin, OUTPUT);
    Relays[i].relayState = loadState(i);                               // retrieve last values from EEPROM
    digitalWrite(Relays[i].relayPin, Relays[i].relayState ? RELAY_ON : RELAY_OFF); // and set relays accordingly
    send(msg[i].set(Relays[i].relayState ? true : false));                 // make controller aware of last status
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
    present(i, S_LIGHT);                               // present sensor to gateway

  wait(100);
}

void loop()
{
  for (byte i = 0; i < noRelays; i++) {
    if (debouncer[i].update()) {
      
      int value = debouncer[i].read();
      
      if ( value == LOW) {
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
  if (message.type == V_LIGHT) {
    if (message.sensor < noRelays) {          // check if message is valid for relays..... previous line  [[[ if (message.sensor <=noRelays){ ]]]
      Relays[message.sensor].relayState = message.getBool();
      digitalWrite(Relays[message.sensor].relayPin, Relays[message.sensor].relayState ? RELAY_ON : RELAY_OFF); // and set relays accordingly
      saveState( message.sensor, Relays[message.sensor].relayState ); // save sensor state in EEPROM (location == sensor number)
    }
  }
  wait(20);
}
