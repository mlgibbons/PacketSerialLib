
/************************************************************
 *
 * Test script for the PacketSerial library 
 * which allows one to send and receive data in packets over 
 * streams.
 ************************************************************/

#include "packetSerial.h"

PacketSerial* m_packetSerial = NULL; 

void setup()  {
  // setup serial link
  Serial.begin(57600);      
  
  // now wrap it in the packetising wrapper class
  m_packetSerial = new PacketSerial(&Serial);              
  
  m_packetSerial->sendData("Hello from Arduino");
  m_packetSerial->sendData("I'll reflect any packets you send back to you");
}

int loopCount = 0;

void loop() {
    String rxData;
    
    bool rc = m_packetSerial->getData(&rxData);
        
    if (rc) {
       m_packetSerial->sendData(rxData);                
    }
        
    delay(1);
        
    // Periodically send a heartbeat message
    loopCount++;
    if (loopCount>10000) {
      loopCount=0;
      m_packetSerial->sendData("Still alive!");          
    }
}


