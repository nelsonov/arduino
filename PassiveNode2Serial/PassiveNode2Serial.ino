/* PassiveNode2Serial
 * nelsonov (lnelson@nelnet.org) Oct. 2018
 * 
 * Hacked version of the example sketch PassiveNode
 * 
 * 
 *******************************
 * Description
 * This works on a bare arduino with no devices connected
 * 
 * This sketch redirects debug output to SoftSerial and
 * prints the word "pong" on the physical Serial port.  This is
 * the same port that is used to upload the sketch.
 * 
 * This sketch expects an NRF24.  If none is attached (which would
 * be on purpose), then copious debug output will be generated on the
 * SoftSerial port.
 * 
 *******************************
 * Based on PassiveNode example by tekka:
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
