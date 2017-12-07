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
 * Version 1.0 - Henrik Ekblad
 *
 * DESCRIPTION
 * Example sketch showing how to control physical relays.
 * This example will remember relay state after power failure.
 * http://www.mysensors.org/build/relay
 */

/***************Debug Enable*************************************************/
// Enable debug prints to serial monitor
#define   MY_DEBUG
#define   MY_SPECIAL_DEBUG
#define   MY_DEBUG_VERBOSE_TRANSPORT
//Want DEBUG output to different port, not Gateway port
#define   MY_DEBUGDEVICE Serial  // Redirect debug
#define   WITHOUT_CONTROLLER     // Temporary kludge during debugging
/***************End Debug Enable*********************************************/

// Enable and select radio type attached
//#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

#define   MY_RFM95_FREQUENCY RFM95_915MHZ
//#define   MY_RFM95_MODEM_CONFIGRUATION RFM95_BW125CR45SF128
//#define   MY_RFM95_ATC_MODE_DISABLED
//#define   MY_RFM95_MAX_POWER_LEVEL_DBM 20

//#define   RFM95_IRQ_PIN     6  // library 2.1.1
#define   RFM95_RST_PIN     9  // library 2.1.1
#define   RFM95_SPI_CS     10  // library 2.1.1
#define   MY_RFM95_IRQ_PIN  2  // library 2.2 beta = GPIO 2
#define   MY_RFM95_IRQ_NUM  0  // library 2.2 beta
#define   MY_RFM95_RST_PIN  5  // library 2.2 beta
#define   MY_RFM95_CS_PIN  6  // library 2.2 beta
#define   MY_RADIO_RFM95
#define   MY_DEBUG_VERBOSE_RFM95


// Enable repeater functionality for this node
//#define MY_REPEATER_FEATURE
#define MY_BAUD_RATE 9600
#define MY_DEBUG_VERBOSE_TRANSPORT

#include <MySensors.h>
#include <Wire.h>
#include "Adafruit_MCP9808.h"

#define TEMP_SENSOR_ID 2
bool initialValueSent = false;
int lasttemp = 0;

MyMessage msg(TEMP_SENSOR_ID, V_TEMP);

// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
void setup()
{
  // Make sure the sensor is found, you can also pass in a different i2c
  // address with tempsensor.begin(0x19) for example
  if (!tempsensor.begin(0x18)) {
    MY_DEBUGDEVICE.println("Couldn't find MCP9808!");
    while (1);
  }
}

void presentation()
{
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("TempSensor-rfm95", "0.2");
  present(TEMP_SENSOR_ID, S_TEMP);
}


void loop()
{
  if (!initialValueSent) {
    MY_DEBUGDEVICE.println("Sending initial value");
    send(msg.set(lasttemp));
    MY_DEBUGDEVICE.println("Requesting initial value from controller");
    request(TEMP_SENSOR_ID, V_TEMP);
    wait(2000, C_SET, V_TEMP);
  }
  //MY_DEBUGDEVICE.println("wake up MCP9808.... "); // wake up MSP9808 - power consumption ~200 mikro Ampere
  //tempsensor.wake();   // wake up, ready to read!

  // Read and print out the temperature, then convert to *F
  float c = tempsensor.readTempC();
  float f = c * 9.0 / 5.0 + 32;
  //MY_DEBUGDEVICE.print("Temp: "); MY_DEBUGDEVICE.print(c); MY_DEBUGDEVICE.print("*C\t"); 
  //MY_DEBUGDEVICE.print(f); MY_DEBUGDEVICE.println("*F");

  send(msg.setSensor(TEMP_SENSOR_ID).set(f,1));
  lasttemp = f;
  
  //MY_DEBUGDEVICE.println("Shutdown MCP9808.... ");
  //tempsensor.shutdown(); // shutdown MSP9808 - power consumption ~0.1 mikro Ampere
  
  delay(300000); // 1000 = 1 second
}

void receive(const MyMessage &message)
{
  if (message.type == V_TEMP) {
	    if (!initialValueSent) {
       MY_DEBUGDEVICE.println("Receiving initial value from controller");
       initialValueSent = true;
	  }
  }
}

