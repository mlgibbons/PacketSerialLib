
/************************************************************
 *
 * Test script for the PacketSerial library 
 * which allows one to send and receive data in packets over 
 * streams. If data is not correctly encoded into a packet
 * the library will drop it transparently.
 *
 ************************************************************/

#include "packetSerial.h"

PacketSerial* m_packetSerial = NULL; 

void setup()  {
  // setup serial link
  Serial.begin(57600);      
  
  // now wrap it in the packetising wrapper class
  m_packetSerial = new PacketSerial(&Serial);              
}

int loopCount = 0;

void loop() {
    char buffer[256];
    
    // see if a packet has been received
    bool rc = m_packetSerial->getData(buffer, sizeof(buffer));
        
    // if packet received then send the payload back
    if (rc) {
       m_packetSerial->sendData(buffer);                 
    }
        
    delay(1);
        
    // Periodically send a heartbeat message
    loopCount++;
    if (loopCount>10000) {
      loopCount=0;
      m_packetSerial->sendData("Still alive!");          
    }
}


