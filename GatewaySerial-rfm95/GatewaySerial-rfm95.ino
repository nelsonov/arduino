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
* DESCRIPTION
* The ArduinoGateway prints data received from sensors on the serial link.
* The gateway accepts input on seral which will be sent out on radio network.
*
* The GW code is designed for Arduino Nano 328p / 16MHz
*
* Wire connections (OPTIONAL):
* - Inclusion button should be connected between digital pin 3 and GND
* - RX/TX/ERR leds need to be connected between +5V (anode) and digital pin 6/5/4 with resistor 270-330R in a series
*
* LEDs (OPTIONAL):
* - To use the feature, uncomment any of the MY_DEFAULT_xx_LED_PINs
* - RX (green) - blink fast on radio message recieved. In inclusion mode will blink fast only on presentation recieved
* - TX (yellow) - blink fast on radio message transmitted. In inclusion mode will blink slowly
* - ERR (red) - fast blink on error during transmission error or recieve crc error
*
*/

/***************Software Serial**********************************************/
//#define SOFTWARE_SERIAL_RX  7
//#define SOFTWARE_SERIAL_TX  8
//#define SOFTWARE_SERIAL_SPEED 9600

#ifdef SOFTWARE_SERIAL_TX
#include <SoftwareSerial.h>
SoftwareSerial SoftSerial(SOFTWARE_SERIAL_RX, SOFTWARE_SERIAL_TX); // RX, TX
#endif
/***************End Software Serial******************************************/

/***************Debug Enable*************************************************/
// Enable debug prints to serial monitor
//#define   MY_DEBUG
#define   MY_SPECIAL_DEBUG
#define   MY_DEBUG_VERBOSE_TRANSPORT
//Want DEBUG output to different port, not Gateway port
#define   MY_DEBUGDEVICE Serial  // Redirect debug
#define   WITHOUT_CONTROLLER     // Temporary kludge during debugging
/***************End Debug Enable*********************************************/

/***************Radio********************************************************/
// Enable and select radio type attached
//#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69
// Set LOW transmit power level as default, if you have an amplified NRF-module and
// power your radio separately with a good regulator you can turn up PA level.
//#define MY_RF24_PA_LEVEL RF24_PA_LOW

#define   MY_RFM95_FREQUENCY RFM95_915MHZ
//#define   MY_RFM95_MODEM_CONFIGRUATION RFM95_BW125CR45SF128 
//#define   MY_RFM95_ATC_MODE_DISABLED
//#define   MY_RFM95_MAX_POWER_LEVEL_DBM 20

//#define   RFM95_IRQ_PIN     5  // library 2.1.1 = GPIO Pin 9
#define   RFM95_RST_PIN     5  // library 2.1.1
#define   RFM95_SPI_CS      6  // library 2.1.1
#define   MY_RFM95_IRQ_PIN  2  // library 2.2 beta = GPIO 2
#define   MY_RFM95_IRQ_NUM  0  // library 2.2 beta
#define   MY_RFM95_RST_PIN  5  // library 2.2 beta
#define   MY_RFM95_CS_PIN   6  // library 2.2 beta
#define   MY_RADIO_RFM95
#define   MY_DEBUG_VERBOSE_RFM95
/****************End Radio****************************************************/

/****************Gateway******************************************************/
// Enable serial gateway
#define MY_GATEWAY_SERIAL
//Want Gateway <-> Controller communication over Serial1
#define MY_SERIALDEVICE Serial

// There's no need for a blinding-fast speed when so little is being communicated
// If doing OTA's, then this speed may need to be reconsidered
#define MY_BAUD_RATE 9600
/***************End Gateway***************************************************/

/********* Inclusion Mode ****************************************************/
// Enable inclusion mode
//#define MY_INCLUSION_MODE_FEATURE
// Enable Inclusion mode button on gateway
//#define MY_INCLUSION_BUTTON_FEATURE

// Inverses behavior of inclusion button (if using external pullup)
//#define MY_INCLUSION_BUTTON_EXTERNAL_PULLUP

// Set inclusion mode duration (in seconds)
//#define MY_INCLUSION_MODE_DURATION 60-9600
// Digital pin used for inclusion mode button
//#define MY_INCLUSION_MODE_BUTTON_PIN  3

// Set blinking period
//#define MY_DEFAULT_LED_BLINK_PERIOD 300

// Inverses the behavior of leds
//#define MY_WITH_LEDS_BLINKING_INVERSE
/************ End Inclusion Mode***********************************************/

/************ Traffic LED Flash***********************************************/
// Flash leds on rx/tx/err
// Uncomment to override default HW configurations
//#define MY_DEFAULT_ERR_LED_PIN 4  // Error led pin
//#define MY_DEFAULT_RX_LED_PIN  6  // Receive led pin
//#define MY_DEFAULT_TX_LED_PIN  13  // the PCB, on board LED
/************* End Traffic LED Flash*******************************************/

#ifdef WITHOUT_CONTROLLER
#define   MY_TRANSPORT_WAIT_READY_MS 3000
#define   MY_TRANSPORT_UPLINK_CHECK_DISABLED  //Don't check for uplink
#define   MY_NODE_ID 199  //Fallback NodeId
#endif

#include <MySensors.h>
#define LED_ONBOARD 13
#define RELAY_1  11  // Arduino Digital I/O pin number for first relay (second on pin+1 etc)
#define NUMBER_OF_RELAYS 1 // Total number of attached relays
#define RELAY_ON 1  // GPIO value to write to turn on attached relay
#define RELAY_OFF 0 // GPIO value to write to turn off attached relay

#define CHILD_ID 1
bool state = false;
bool initialValueSent = false;

MyMessage msg(CHILD_ID, V_STATUS);



void setup()
{
	// Setup locally attached sensors
  //
  //Turn On on-board LED as power indicator
  pinMode(LED_ONBOARD, OUTPUT);
//  MY_SERIALDEVICE.begin(MY_BAUD_RATE);
#ifdef SOFTWARE_SERIAL_TX
  SoftSerial.begin(SOFTWARE_SERIAL_SPEED);
#endif
  MY_DEBUGDEVICE.print("Setup complete");
}

#undef WITHOUT_CONTROLLER
void presentation()
{
  sendSketchInfo("SerialGW+Relay", "0.2");
#ifndef WITHOUT_CONTROLLER
	// Present locally attached sensors
  for (int sensor=1, pin=RELAY_1; sensor<=NUMBER_OF_RELAYS; sensor++, pin++) {
    MY_DEBUGDEVICE.print("Presenting: ");
    MY_DEBUGDEVICE.print(sensor);
    MY_DEBUGDEVICE.print("=");
    MY_DEBUGDEVICE.println(S_BINARY);
    present(sensor, S_BINARY);
    // Then set relay pins in output mode
    pinMode(pin, OUTPUT);
    // Set relay to last known state (using eeprom storage)
    digitalWrite(pin, loadState(sensor)?RELAY_ON:RELAY_OFF);
  }
#endif  
}

void loop()
{
  if (!initialValueSent) {
    MY_DEBUGDEVICE.println("Sending initial value");
    send(msg.set(state?RELAY_ON:RELAY_OFF));
    MY_DEBUGDEVICE.println("Requesting initial value from controller");
    request(CHILD_ID, V_STATUS);
    wait(2000, C_SET, V_STATUS);
  }
  
  //Turn On on-board LED as power indicator
  digitalWrite(LED_ONBOARD, HIGH);

}

void receive(const MyMessage &message)
{
  MY_DEBUGDEVICE.println("Message received");
  // We only expect one type of message from controller. But we better check anyway.
    if (message.isAck()) {
      MY_DEBUGDEVICE.println("This is an ack from gateway");
  }
  if (message.type==V_STATUS) {
    if (!initialValueSent) {
      MY_DEBUGDEVICE.println("Receiving initial value from controller");
      initialValueSent = true;
     }
    // Change relay state
    digitalWrite(message.sensor-1+RELAY_1, message.getBool()?RELAY_ON:RELAY_OFF);
    // Store state in eeprom
    saveState(message.sensor, message.getBool());
    // Write some debug info
    MY_DEBUGDEVICE.print("Incoming change for sensor:");
    MY_DEBUGDEVICE.print(message.sensor);
    MY_DEBUGDEVICE.print(", New status: ");
    MY_DEBUGDEVICE.println(message.getBool());
  }
}

