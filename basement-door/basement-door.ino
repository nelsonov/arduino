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
 * Version 1.0 - Henrik EKblad
 * Contribution by a-lurker and Anticimex,
 * Contribution by Norbert Truchsess <norbert.truchsess@t-online.de>
 * Contribution by Ivo Pullens (ESP8266 support)
 *
 /***************nRF24***********************/
// Default Pinout
//  GND  <-->   GND
// 3.3V  <-->   3.3V
//    9  <-->   CE
//   10  <-->   CSN/CS
//   11  <-->   MOSI
//   12  <-->   MISO
//   13  <-->   SCK
//    2  <-->   IRQ
#define   MY_RADIO_RF24
#define   MY_RADIO_NRF24
//#define   MY_RF24_CE_PIN  5
//#define   MY_RF24_CS_PIN  6
//#define   MY_RF24_IRQ_PIN     // OPTIONAL
//#define   MY_RF24_POWER_PIN   // OPTIONAL
//#define   MY_RF24_CHANNEL     // OPTIONAL
//#define   MY_RF24_DATARATE    // OPTIONAL
#define   MY_RF24_SPI_SPEED   (2*1000000ul)
#define   MY_DEBUG_VERBOSE_RF24
/**********END nRF24*************************/

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

#define RELAY_PIN  7  // Arduino Digital I/O pin number for relay 
#define BUTTON_PIN  8  // Arduino Digital I/O pin number for button 
#define CHILD_ID 1   // Id of the sensor child
#define RELAY_ON 1
#define RELAY_OFF 0
#define WRITE_RELAY_ON 1
#define WRITE_RELAY_OFF 0

Bounce debouncer = Bounce(); 
int oldValue=0;
bool state;
bool initialValueSent = false;

MyMessage msg(CHILD_ID,V_LIGHT);

void setup()
{
  // Setup the button
  pinMode(BUTTON_PIN,INPUT_PULLUP);

  digitalWrite(BUTTON_PIN,HIGH);

  // After setting up the button, setup debouncer
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(5);

  // Make sure relays are off when starting up
  digitalWrite(RELAY_PIN, WRITE_RELAY_OFF);
  // Then set relay pins in output mode
  pinMode(RELAY_PIN, OUTPUT);  

  // Set relay to last known state (using eeprom storage) 
  state = loadState(CHILD_ID);
  digitalWrite(RELAY_PIN, state?WRITE_RELAY_ON:WRITE_RELAY_OFF);
}

void presentation()
{
	// Present locally attached sensors here
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Relay & Button", "0.1");

  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID, S_BINARY);
}


void loop()
{
	// Send locally attached sensors data here
  if (!initialValueSent) {
    Serial.println("Sending initial value");
    send(msg.set(state?RELAY_ON:RELAY_OFF));
    Serial.println("Requesting initial value from controller");
    request(CHILD_ID, V_STATUS);
    wait(2000, C_SET, V_STATUS);
  }

  debouncer.update();
  // Get the update value
  int value = debouncer.rose();
  if (value != oldValue  && value==0) {
      Serial.print("Button Press.  Old state: ");
      Serial.print(state);
      state = !state;
      writestate(state);
      //digitalWrite(RELAY_PIN, state?WRITE_RELAY_ON:WRITE_RELAY_OFF);
  }
  oldValue = value;
}

void receive(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.isAck()) {
     Serial.println("This is an ack from gateway");
  }
  if (message.type == V_STATUS) {
    if (!initialValueSent) {
      Serial.println("Receiving initial value from controller");
      initialValueSent = true;
     }
     state = message.getBool();
     Serial.print("Incoming change for sensor:");
     Serial.print(message.sensor);
     writestate(state);
  }
}
void writestate(bool newstate) {
     digitalWrite(RELAY_PIN, newstate?WRITE_RELAY_ON:WRITE_RELAY_OFF);
     // Store state in eeprom
     saveState(CHILD_ID, newstate);
     send(msg.set(newstate?RELAY_ON:RELAY_OFF));
     // Write some debug info
     Serial.print(", New status: ");
     Serial.println(newstate);
}

