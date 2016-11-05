
#include "NodeMessagingUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <avr/sleep.h>
#include <RFM12B.h>
#include <MGLib.h>

#define SERIAL_BAUD         57600

MessagingEndpoint* msgEndpoint;

void setup() 
{
  Serial.begin(SERIAL_BAUD);
  //Logger.init(&Serial, INFO);
  
  AbstractMessageTransport* msgTransport = new RFMTransport(123,100,1);
  msgEndpoint = new MessagingEndpoint(msgTransport);
  msgEndpoint->connect("RFM-TEST1");  
}

void loop()
{
  int count = 0;
  
  while (1==1) {
    //Serial.println("sending");
    
    if (count<5000) {
      count++;
    } else {
      count = 0;
      msgEndpoint->sendMsg("MQTT-TEST1","123456789");
    }
    
    msgEndpoint->processEvents();
    
    if (msgEndpoint->hasMsg()==true) {
      NodeMsg* rxNodeMsg = msgEndpoint->getMsg();
      Serial.println(rxNodeMsg->payload);
      delete rxNodeMsg;
    }
    
    //Serial.println("sent");
    //Serial.println("sleeping");
    delay(1);
  }
  
}
