/***
 *  This is a simple sketch used to demenstrate and test node-to-node MySensor's communication.
 *  To use this sketch, assemble MySensors nodes - they need nothing more than a radio
 *  1.  Flash each node with the same sketch, open the console and type either 0 or 1 to the respective nodes to set thei ID
 *  2.  You only need to set the node id once, and restart the nodes
 *  3.  To being a ping-pong test, simply type T in the console for one of the nodes.
 *
 *  2015-05-25 Bruce Lacey v1.0
 */

//The Node ID's are hard coded in this sketch, so the option described in #1 above
//of specifying 0 or 1 no longer works.  Each node will need to be flahed with the
//correct #define for eith NODE_YING or NODE_YANG

//Which node is this?
//#define NODE_YING
#define NODE_YANG

// Define two generic nodes with a single child
#define YING 200
#define YANG 201
#define CHILD 1

// Enable debug prints to serial monitor
#define MY_DEBUG
#define MY_BAUD_RATE 9600
//#define   MY_SPECIAL_DEBUG
#define MY_TRANSPORT_WAIT_READY_MS 3000
#define   MY_TRANSPORT_UPLINK_CHECK_DISABLED  //Don't check for uplink
#define   MY_NODE_ID 199  //Fallback NodeId

//We don't care about a parent, but this satisfies various demands
//We hard code a parent ID of 1 and say that it's static
//so that the system won't check for one
#define   MY_PARENT_NODE_ID  1
#define   MY_PARENT_NODE_IS_STATIC

//Override the fallback NodeId
#ifdef NODE_YING
#define MY_NODE_ID YING
#endif
#ifdef NODE_YANG
#define MY_NODE_ID YANG
#endif

// Enable and select radio type attached
#define   MY_RS485
//#define MY_RADIO_RF24
//#define MY_RADIO_NRF24
//#define MY_RADIO_NRF5_ESB
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95

#include <MySensors.h>

//RS485
#ifdef MY_RS485
#ifdef NODE_YING
//#define INPUT_CAPTURE_PIN    10 // receive
//#define OUTPUT_COMPARE_A_PIN     9 // transmit
//#define   MY_RS485_DE_PIN   11
#define INPUT_CAPTURE_PIN    8 // receive
#define OUTPUT_COMPARE_A_PIN     9 // transmit
//#define   MY_RS485_DE_PIN   7
#endif
#ifdef NODE_YANG
#define INPUT_CAPTURE_PIN    8 // receive
#define OUTPUT_COMPARE_A_PIN     9 // transmit
#define   MY_RS485_DE_PIN   7
#endif
#define   MY_RS485_BAUD_RATE   9600
#endif

//RFM24
#ifdef MY_RADIO_RF24
#ifdef NODE_YING
#define   MY_RF24_CE_PIN    5
#define   MY_RF24_CS_PIN    6
//#define   MY_RF24_IRQ_PIN   9
#endif
#ifdef NODE_YANG
//#define   MY_RF24_CE_PIN    4
//#define   MY_RF24_CS_PIN    5
//#define   MY_RF24_IRQ_PIN   6
#endif
#define MY_RF24_DATARATE RF24_1MBPS
#define MY_RF24_SPI_SPEED   (1*1000000ul)

#define   MY_DEBUG_VERBOSE_RF24 
#endif

#ifdef MY_RADIO_RFM95
//RFM95 with various trial settings
#define   MY_RFM95_FREQUENCY RFM95_915MHZ
#define   MY_RFM95_MODEM_CONFIGRUATION RFM95_BW125CR45SF128 
#define   MY_RFM95_ATC_MODE_DISABLED
#define   MY_RFM95_MAX_POWER_LEVEL_DBM 20
#define   MY_DEBUG_VERBOSE_RFM95

//My nodes are different hardware with different pinouts
#ifdef NODE_YING
#define   RFM95_IRQ_PIN     9  // library 2.1.1 = GPIO Pin 9
#define   RFM95_RST_PIN     5  // library 2.1.1
#define   RFM95_SPI_CS      6  // library 2.1.1
//#define   MY_RFM95_IRQ_PIN  9  // library 2.2 beta = GPIO 9
//#define   MY_RFM95_RST_PIN  5  // library 2.2 beta
#define   MY_RFM95_CS_PIN   6  // library 2.2 beta
#endif
#ifdef NODE_YANG
//#define   RFM95_IRQ_PIN     6  // library 2.1.1
//#define   RFM95_RST_PIN     9  // library 2.1.1
#define   RFM95_SPI_CS     10  // library 2.1.1
//#define   MY_RFM95_IRQ_PIN  6  // library 2.2 beta = GPIO Pin 6
//#define   MY_RFM95_RST_PIN  9  // library 2.2 beta
#define   MY_RFM95_CS_PIN  10  // library 2.2 beta
#endif
#endif

#define MY_DEBUG_VERBOSE_TRANSPORT
#include "MYSLog.h"
#define VSN "v1.0"

MyMessage mPing(CHILD, V_VAR1);   //Ping message
MyMessage mPong(CHILD, V_VAR2);   //Pong message

void setup()
{
#ifdef NODE_YING
  setNodeId(YING);
#endif
#ifdef NODE_YANG
  setNodeId(YANG);
#endif
}

//////////////////////////////////////////////
//No changes have been made below this point
//////////////////////////////////////////////

void presentation()
{
	present(CHILD, S_CUSTOM);  //

	sendSketchInfo( nodeTypeAsCharRepresentation( getNodeId() ), VSN );
	LOG(F("\n%sReady.\n"), nodeTypeAsCharRepresentation(getNodeId()));
}

void loop()
{

	// Interactive command and control
	// Entering a number from 0 or 1 will write the node 200 (YING) or 201 (YANG) to EEPROM
	// Entering T on either node will initiatve a ping-pong test.
	if (Serial.available()) {
		byte inChar = Serial.read();
		uint8_t node = getNodeId();

		// Manual Test Mode
		if (inChar == 'T' || inChar == 't') {
			LOG(F("T received - starting test from %i...\n"), node);
			MyMessage msg = mPong;
			msg.sender = (node == YING ? YANG : YING);
			sendPingOrPongResponse( msg );
		} else if (inChar == '0' or inChar == '1') {
			byte nodeID = 200 + (inChar - '0');
			setNodeId(nodeID);
		} else {
			LOG("Invalid input\n");
		}
	}
}

void receive(const MyMessage &message)
{

	LOG(F("Received %s from %s\n"), msgTypeAsCharRepresentation((mysensor_data)message.type),
	    nodeTypeAsCharRepresentation(message.sender));

	delay(250);
	sendPingOrPongResponse( message );
}

void sendPingOrPongResponse( MyMessage msg )
{

	MyMessage response = (msg.type == V_VAR1 ? mPong : mPing);

	LOG(F("Sending %s to %s\n"), msgTypeAsCharRepresentation( (mysensor_data)response.type ),
	    nodeTypeAsCharRepresentation(msg.sender));

	// Set payload to current time in millis to ensure each message is unique
	response.set( (uint32_t)millis() );
	response.setDestination(msg.sender);
	send(response);
}

void setNodeId(byte nodeID)
{
	LOG(F("Setting node id to: %i.\n***Please restart the node for changes to take effect.\n"), nodeID);
	hwWriteConfig(EEPROM_NODE_ID_ADDRESS, (byte)nodeID);
}

const char * msgTypeAsCharRepresentation( mysensor_data mType )
{
	return mType == V_VAR1 ? "Ping" : "Pong";
}

const char * nodeTypeAsCharRepresentation( uint8_t node )
{
	return node == YING ? "Ying Node" : "Yang Node";
}

