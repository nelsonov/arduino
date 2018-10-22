/*
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2018 Sensnology AB
 * Full contributor list: https://github.com/mysensors/MySensors/graphs/contributors
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
 * Version 1.0 - tekka
 *
 * DESCRIPTION
 * Passive node example: This is a passive & independent reporting node
 *
 */
#define SOFTWARE_SERIAL_RX  7
#define SOFTWARE_SERIAL_TX  8
#define SOFTWARE_SERIAL_SPEED 9600

#ifdef SOFTWARE_SERIAL_TX
#include <SoftwareSerial.h>
SoftwareSerial SoftSerial(SOFTWARE_SERIAL_RX, SOFTWARE_SERIAL_TX);
#endif

//Allow operation without active uplink
#define MY_TRANSPORT_WAIT_READY_MS 3000
#define MY_TRANSPORT_UPLINK_CHECK_DISABLED
#define MY_PASSIVE_NODE

// Enable debug prints
//#define MY_SERIALDEVICE SoftSerial
#define MY_DEBUGDEVICE SoftSerial 
#define MY_DEBUG
#define MY_BAUD_RATE 9600


// Passive mode requires static node ID
#define MY_NODE_ID 100

// Enable and select radio type attached
#define MY_RADIO_RF24
//#define MY_RADIO_NRF5_ESB
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95

#include <MySensors.h>


#define CHILD_ID 0   // Id of the sensor child

// Initialize general message
MyMessage msg(CHILD_ID, V_TEMP);

void preHwInit()
{
  #ifdef SOFTWARE_SERIAL_TX
  SoftSerial.begin(SOFTWARE_SERIAL_SPEED);
  #endif  
}
void setup()
{

}

void presentation()
{
	// Send the sketch version information to the gateway and controller
	sendSketchInfo("Passive node", "1.0");

	// Register all sensors to gw (they will be created as child devices)
	present(CHILD_ID, S_TEMP);
}

void loop()
{
  MY_SERIALDEVICE.println("Pong");
	// generate some random data
	send(msg.set(25.0+random(0,30)/10.0,2));
	sleep(2000);
}
