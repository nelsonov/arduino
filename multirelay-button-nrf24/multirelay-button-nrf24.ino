/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Based on the MySensors RelayActuator Example Sketch credited below:
 * Version 1.0 - Henrik EKblad
 * Contribution by a-lurker and Anticimex,
 * Contribution by Norbert Truchsess <norbert.truchsess@t-online.de>
 * Contribution by Ivo Pullens (ESP8266 support)
 *
 * DESCRIPTION
 * The EthernetGateway sends data received from sensors to the WiFi link.
 * The gateway also accepts input on ethernet interface, which is then sent out to the radio network.
 *
 * VERA CONFIGURATION:
 * Enter "ip-number:port" in the ip-field of the Arduino GW device. This will temporarily override any serial configuration for the Vera plugin.
 * E.g. If you want to use the defualt values in this sketch enter: 192.168.178.66:5003
 *
 * LED purposes:
 * - To use the feature, uncomment any of the MY_DEFAULT_xx_LED_PINs in your sketch, only the LEDs that is defined is used.
 * - RX (green) - blink fast on radio message recieved. In inclusion mode will blink fast only on presentation recieved
 * - TX (yellow) - blink fast on radio message transmitted. In inclusion mode will blink slowly
 * - ERR (red) - fast blink on error during transmission error or recieve crc error
 *
 * See http://www.mysensors.org/build/esp8266_gateway for wiring instructions.
 * nRF24L01+  ESP8266
 * VCC        VCC
 * CE         GPIO4
 * CSN/CS     GPIO15
 * SCK        GPIO14
 * MISO       GPIO12
 * MOSI       GPIO13
 * GND        GND
 *
 * Not all ESP8266 modules have all pins available on their external interface.
 * This code has been tested on an ESP-12 module.
 * The ESP8266 requires a certain pin configuration to download code, and another one to run code:
 * - Connect REST (reset) via 10K pullup resistor to VCC, and via switch to GND ('reset switch')
 * - Connect GPIO15 via 10K pulldown resistor to GND
 * - Connect CH_PD via 10K resistor to VCC
 * - Connect GPIO2 via 10K resistor to VCC
 * - Connect GPIO0 via 10K resistor to VCC, and via switch to GND ('bootload switch')
 *
  * Inclusion mode button:
 * - Connect GPIO5 via switch to GND ('inclusion switch')
 *
 * Hardware SHA204 signing is currently not supported!
 *
 * Make sure to fill in your ssid and WiFi password below for ssid & pass.
 */


// Enable debug prints to serial monitor
#define MY_DEBUG

// Use a bit lower baudrate for serial prints on ESP8266 than default in MyConfig.h
#define MY_BAUD_RATE 9600

// Enables and select radio type (if attached)
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69
//#define MY_RF24_CE_PIN 49
//#define MY_RF24_CS_PIN 53

//#define MY_GATEWAY_ESP8266

//#define MY_ESP8266_SSID "nelnet"
//#define MY_ESP8266_PASSWORD "56seven8"

// Enable UDP communication
//#define MY_USE_UDP

// Set the hostname for the WiFi Client. This is the hostname
// it will pass to the DHCP server if not static
//#define MY_ESP8266_HOSTNAME "sensor-gateway"

// Enable MY_IP_ADDRESS here if you want a static ip address (no DHCP)
//#define MY_IP_ADDRESS 192,168,178,87

// If using static ip you need to define Gateway and Subnet address as well
//#define MY_IP_GATEWAY_ADDRESS 192,168,178,1
//#define MY_IP_SUBNET_ADDRESS 255,255,255,0

// The port to keep open on node server mode
//#define MY_PORT 5003

// How many clients should be able to connect to this gateway (default 1)
//#define MY_GATEWAY_MAX_CLIENTS 2

// Controller ip address. Enables client mode (default is "server" mode).
// Also enable this if MY_USE_UDP is used and you want sensor data sent somewhere.
//#define MY_CONTROLLER_IP_ADDRESS 192, 168, 178, 68

// Enable inclusion mode
//#define MY_INCLUSION_MODE_FEATURE

// Enable Inclusion mode button on gateway
// #define MY_INCLUSION_BUTTON_FEATURE
// Set inclusion mode duration (in seconds)
#define MY_INCLUSION_MODE_DURATION 60
// Digital pin used for inclusion mode button
#define MY_INCLUSION_MODE_BUTTON_PIN  3


// Set blinking period
// #define MY_DEFAULT_LED_BLINK_PERIOD 300

// Flash leds on rx/tx/err
// Led pins used if blinking feature is enabled above
//#define MY_DEFAULT_ERR_LED_PIN 16  // Error led pin
//#define MY_DEFAULT_RX_LED_PIN  16  // Receive led pin
//#define MY_DEFAULT_TX_LED_PIN  16  // the PCB, on board LED

//#if defined(MY_USE_UDP)
//#include <WiFiUdp.h>
//#endif

//#include <ESP8266WiFi.h>

#include <MySensors.h>
#include <Bounce2.h>

//First Relay
#define RELAY_1_PIN  7  // Arduino Digital I/O pin number for relay 
#define CHILD_1_ID 1   // Id of the sensor child

//Second Relay
#define RELAY_2_PIN 8
#define CHILD_2_ID 2

#define NUM_RELAYS 2
//Button
#define BUTTON_PIN  6  // Arduino Digital I/O pin number for button 

#define RELAY_ON 1
#define RELAY_OFF 0
#define WRITE_RELAY_ON 0
#define WRITE_RELAY_OFF 1

Bounce debouncer_1 = Bounce(); 
int oldValue=0;
bool initialValueSent = false;

typedef struct {
  bool state;
  int childId;
  int relay_pin;
} relay_record;

relay_record relays[NUM_RELAYS];
MyMessage msg(CHILD_1_ID,V_LIGHT);

void setup()
{
  // Setup the button
  pinMode(BUTTON_PIN,INPUT);
  // Activate internal pull-up
  digitalWrite(BUTTON_PIN,HIGH);

  // After setting up the button, setup debouncer_1
  debouncer_1.attach(BUTTON_PIN);
  debouncer_1.interval(5);

  // Make sure relays are off when starting up
  writestate(RELAY_1_PIN, WRITE_RELAY_OFF);
  writestate(RELAY_2_PIN, WRITE_RELAY_OFF);
  // Then set relay pins in output mode
  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);

  // Set relays to last known state (using eeprom storage) 
  state_1 = loadstate(CHILD_1_ID);
  writestate(RELAY_1_PIN, state_1?WRITE_RELAY_ON:WRITE_RELAY_OFF);
  state_2 = loadstate(CHILD_2_ID);
  writestate(RELAY_2_PIN, state_2?WRITE_RELAY_ON:WRITE_RELAY_OFF);
}

void presentation()
{
	// Present locally attached sensors here
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Relay & Button", "0.1");

  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_1_ID, S_BINARY);
  present(CHILD_2_ID, S_BINARY);
}


void loop()
{
	// Send locally attached sensors data here
  if (!initialValueSent) {
    Serial.println("Sending initial value");
    send(msg.set(state_1?RELAY_ON:RELAY_OFF));
    send(msg.set(state_2?RELAY_ON:RELAY_OFF));
    Serial.println("Requesting initial value from controller");
    request(CHILD_1_ID, V_STATUS);
    request(CHILD_2_ID, V_STATUS)
    wait(2000, C_SET, V_STATUS);
  }

  debouncer_1.update();
  // Get the update value
  int value = debouncer_1.rose();
  if (value != oldValue  && value==0) {
      Serial.print("Button Press.  Old state: ");
      Serial.print(state_1);
      Serial.print(" ");
      Serial.print(state_2);
      state_1 = !state_1;
      state_2 = !state_2;
      Serial.print(" ");
      Serial.print(state_2);
      writestate(state_1);
      writestate(state_2);
  }
  oldValue = value;
}

void receive(const MyMessage &message) {
  int child;
  int in_state;
  // We only expect one type of message from controller. But we better check anyway.
  if (message.isAck()) {
     Serial.println("This is an ack from gateway");
  }
  if (message.type == V_STATUS) {
    if (!initialValueSent) {
      Serial.println("Receiving initial value from controller");
      initialValueSent = true;
     }
     child=message.childId();
     in_state = message.getBool();
     Serial.print("Incoming change for sensor:");
     Serial.print(message.sensor);
     writestate(child, in_state);
  }
}
void writestate(int child, bool newstate) {
     digitalWrite(child, newstate?WRITE_RELAY_ON:WRITE_RELAY_OFF);
     // Store state_1 in eeprom
     savestate(child, newstate);
     send(msg.set(newstate?RELAY_ON:RELAY_OFF));
     // Write some debug info
     Serial.print(", New status: ");
     Serial.println(newstate);
}

