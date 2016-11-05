
#include "InterNodeMessagingLib.h"
#include <stdio.h>
#include <stdlib.h>
#include <avr/sleep.h>
#include <RFM12B.h>
#include <MGLib.h>
#include <EEPROM.h>

#define SERIAL_BAUD 57600

MessagingEndpoint* msgEndpoint;

#define NODE_ID "AAA"

void setup() 
{
  Serial.begin(SERIAL_BAUD);
  Logger.init(&Serial, INFO);
  Logger.logInfo("Setup start");
  
  // Create concrete transport implementation - here we use RFM12B
  // Note that as well as the RFM network address for the node we also pass
  // in the address of the RFM router through which all node to node access
  // is mediated when using the RFM12B transport
  AbstractMessageTransport* msgTransport = new RFMTransport(123,201,1);

  // Create messaging endpoint for use by the app and pas in the transport to use
  msgEndpoint = new MessagingEndpoint(msgTransport);
  
  // Bind the endpoint for this node to an address
  msgEndpoint->bind(NODE_ID);
  
  Logger.logInfo("Setup end");  
}

void loop()
{
  int count = 0;
  
  while (1==1) {
    
    if (count<5000) // send a message every five seconds 
    {
      count++;
    } 
    else {
      logMemInfo();

	  char * dstNodeId = NODE_ID;  // send to myself
      char * message = "1234567890";

	  Logger.logInfo2(F("Loop: Sending [%s] message to [%s]"), message, dstNodeId);
	  count = 0;

      msgEndpoint->sendMsg(dstNodeId, message);

	  Logger.logInfo("Loop:Sent message");
    }
    
    msgEndpoint->processEvents();
    
    if (msgEndpoint->hasMsg()==true) 
    {
	  Logger.logInfo("Loop:Received msg");
	  
	  String srcNodeId, dstNodeId, payload;

	  InterNodeMsg* msg;
      int rc = msgEndpoint->getMsg(&msg);

      Logger.logInfo2(F("Loop: srcNodeId [%s]"), msg->srcNodeId);
      Logger.logInfo2(F("Loop: protocol  [%d]"), msg->protocol);

      if (msg->protocol == UnreliableProtocol)
      {
          Logger.logInfo2(F("Loop: payload [%s]"), msg->payload.unreliableProtocolMsg.payload);
      }

	  delete msg;

      logMemInfo();
    }


    delay(1);
  }
  
}
