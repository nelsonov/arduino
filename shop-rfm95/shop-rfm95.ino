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

///////////////Version info
#ifndef VERSION_FROM_GIT
#define VERSION_FROM_GIT UNKNOWN
#endif

/***************Debug Enable*************************************************/
// Enable debug prints to serial monitor
#define   MY_DEBUG
#define   MY_SPECIAL_DEBUG
#define   MY_DEBUG_VERBOSE_TRANSPORT
//Want DEBUG output to different port, not Gateway port
#define   MY_DEBUGDEVICE Serial  // Redirect debug
//#define   WITHOUT_CONTROLLER     // Temporary kludge during debugging
/***************End Debug Enable*********************************************/

#define   MY_RFM95_FREQUENCY RFM95_915MHZ
//#define   MY_RFM95_MODEM_CONFIGRUATION RFM95_BW125CR45SF128
//#define   MY_RFM95_ATC_MODE_DISABLED
//#define   MY_RFM95_MAX_POWER_LEVEL_DBM 20

//#define   RFM95_IRQ_PIN   6  // library 2.1.1
#define   RFM95_RST_PIN     9  // library 2.1.1
#define   RFM95_SPI_CS      10 // library 2.1.1
#define   MY_RFM95_IRQ_PIN  1  // library 2.2 beta = GPIO 2
#define   MY_RFM95_IRQ_NUM  3  // library 2.2 beta
#define   MY_RFM95_RST_PIN  5  // library 2.2 beta
#define   MY_RFM95_CS_PIN   6
// library 2.2 beta
#define   MY_RADIO_RFM95
//#define   MY_DEBUG_VERBOSE_RFM95


// Enable repeater functionality for this node
//#define MY_REPEATER_FEATURE
#define MY_BAUD_RATE 9600

#define MY_NODE_ID             201
#include <MySensors.h>
#include <Wire.h>
#include "Adafruit_MCP9808.h"
#define LIGHT_ON               1
#define LIGHT_OFF              0
#define GREEN                  9
#define YELLOW                 10
#define WHITE                  11
#define SAMPLE                 20
#define LIGHT_ANALOG           A5  
#define TEMP_SENSOR_ID         2
#define LIGHT_SENSOR_ID        3
#define LIGHT_LED_ID           4
#define TEMP_I2C_ADDR          0x18
#define WAIT_VALUE             3000   // 1000 = 1 second
bool initialTempValueSent    = false;
bool initialLightValueSent   = false;
bool initialLightLEDValueSent= false;
int lastLightLEDStatus       = 0;
float lastTemp               = 0.0;
float lastLightPct           = 0.0;
const long loopInterval      = 300000; // 1000 = 1 second, 300000 = 5min
unsigned long previousMillis = 0;

MyMessage msgTemp(TEMP_SENSOR_ID, V_TEMP);
MyMessage msgLight(LIGHT_SENSOR_ID, V_LIGHT_LEVEL);
MyMessage msgLightLED(LIGHT_LED_ID, V_STATUS);

// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
void setup()
{
  // Make sure the sensor is found, you can also pass in a different i2c
  // address with tempsensor.begin(0x19) for example
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  if (!tempsensor.begin(TEMP_I2C_ADDR)) {
    MY_DEBUGDEVICE.println("Couldn't find MCP9808!");
    while (1);
  }
  digitalWrite(YELLOW, HIGH);
}

void presentation()
{
	// Send the sketch version information to the gateway and Controller
  sendSketchInfo("shop-rfm95", "VERSION_FROM_GIT");
  present(TEMP_SENSOR_ID, S_TEMP);
  present(LIGHT_SENSOR_ID, S_LIGHT_LEVEL);
  present(LIGHT_LED_ID, S_BINARY);
}


void loop()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= loopInterval) {
    previousMillis = currentMillis;

    //Temperature Sensor
    if (!initialTempValueSent) {
      getInitialTempValue();
    } else {
      lastTemp=tempRead();
      send(msgTemp.setSensor(TEMP_SENSOR_ID).set(lastTemp,1));
    }

    // Light LED
    if (!initialLightLEDValueSent) {
      getInitialLightLEDValue();
    } else {
      send(msgLightLED.setSensor(LIGHT_LED_ID).set(lastLightLEDStatus,1));
    }

    //Light Sensor
    if (!initialLightValueSent) {
        getInitialLightValue();
    } else {
      lastLightPct=lightRead();
      send(msgLight.setSensor(LIGHT_SENSOR_ID).set(lastLightPct,1));
    }
  }
}

void receive(const MyMessage &message)
{
  if (message.isAck()) {
     Serial.println("This is an ack from gateway");
  } else if (message.type == V_STATUS && message.sender == 0) {
    switch (message.sensor) {
      case LIGHT_LED_ID:
        int state = message.getBool();
        digitalWrite(WHITE, state ? LIGHT_ON : LIGHT_OFF);
        break;
    } // switch
  } else {
    switch (message.sensor) {
      case TEMP_SENSOR_ID:
        if (!initialTempValueSent) {
           MY_DEBUGDEVICE.println("Receiving initial Temperature value from controller");
           initialTempValueSent = true;
      }
      case LIGHT_SENSOR_ID:
        if (!initialLightValueSent) {
           MY_DEBUGDEVICE.println("Receiving initial Light Sensor value from controller");
           initialLightValueSent = true;
      }
      case LIGHT_LED_ID:
        if (!initialLightLEDValueSent) {
           MY_DEBUGDEVICE.println("Receiving initial Light LED status from controller");
           initialLightLEDValueSent = true;
      } // case
    } // switch
  } // else
} // function

float lightRead (void)
{
  int16_t adc0 = 0;
  long sum = 0;
  float samplelevel = 0.0;
  int i;
  float percentval = 0.0;
  for (i=0; i < SAMPLE; i++) {
    adc0 = analogRead(LIGHT_ANALOG);
    sum += adc0;
  }
  samplelevel = 1024 - (sum / SAMPLE);
  MY_DEBUGDEVICE.print("Light Sensor: ");
  MY_DEBUGDEVICE.println(samplelevel);
  percentval = (samplelevel / 1024.0) * 100.0;
  MY_DEBUGDEVICE.print("Light Sensor: ");
  MY_DEBUGDEVICE.print(percentval);
  MY_DEBUGDEVICE.println("%");
  return percentval;
}

float tempRead (void)
{
  //MY_DEBUGDEVICE.println("wake up MCP9808.... "); // wake up MSP9808 - power consumption ~200 mikro Ampere
  //tempsensor.wake();   // wake up, ready to read!

  // Read and print out the temperature, then convert to *F
  float c = tempsensor.readTempC();
  float f = c * 9.0 / 5.0 + 32;
  MY_DEBUGDEVICE.print("Temp: "); MY_DEBUGDEVICE.print(c); MY_DEBUGDEVICE.print("*C\t"); 
  MY_DEBUGDEVICE.print(f); MY_DEBUGDEVICE.println("*F");

  //MY_DEBUGDEVICE.println("Shutdown MCP9808.... ");
  //tempsensor.shutdown(); // shutdown MSP9808 - power consumption ~0.1 mikro Ampere
  //digitalWrite(YELLOW, LOW);
  return f;
}

void getInitialTempValue (void)
{
  MY_DEBUGDEVICE.println("Sending initial temperature value");
  send(msgTemp.setSensor(TEMP_SENSOR_ID).set(lastTemp,1));
  MY_DEBUGDEVICE.println("Requesting initial temperatue value from controller");
  request(TEMP_SENSOR_ID, V_TEMP);
  wait(WAIT_VALUE, C_SET, V_TEMP);

  //We assume (not always correctly) that we have communicated with the controller by now
  digitalWrite(GREEN, HIGH);
}
void getInitialLightValue(void)
{
  MY_DEBUGDEVICE.println("Sending initial light level  value");
  send(msgLight.setSensor(LIGHT_SENSOR_ID).set(lastLightPct,1));
  MY_DEBUGDEVICE.println("Requesting initial light level value from controller");
  request(LIGHT_SENSOR_ID, V_LIGHT_LEVEL);
  wait(WAIT_VALUE, C_SET, V_LIGHT_LEVEL);
}
void getInitialLightLEDValue(void)
{
  MY_DEBUGDEVICE.println("Sending initial light led status");
  send(msgLightLED.setSensor(LIGHT_LED_ID).set(lastLightLEDStatus,1));
  MY_DEBUGDEVICE.println("Requesting initial light led status from controller");
  request(LIGHT_LED_ID, V_STATUS);
  wait(WAIT_VALUE, C_SET, V_STATUS);
}

