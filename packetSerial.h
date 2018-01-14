#include "Arduino.h"

/***********************************************************************
 *
 * Library for sending data packets over a serial link  
 * 
 * Data sent through sendData() is encapsulated in a packet.
 *
 * Poll for new packet by calling getData() which returns the packet payload
 *
 * Packets which are incorrectly constructed  or fail the CRC 
 * check will be dropped without client notification and resyncing started.
 *
 ***********************************************************************/

 /******************************************
  *
  * Packet comms library 
  * Allows arbitary data to be sent and received over serial link
  * FSM implementation provides robust and predictable behaviour
  * and automatic resyncing to packet start
  *
  ******************************************/
 
class PacketSerial {

  private:
    Stream* m_serial;
    
    bool m_dataAvailable;
    String m_data;          // Payload of last packet received
    
    typedef enum {
      RX_STATE_READING_STX,
      RX_STATE_READING_LEN_DATA,
      RX_STATE_READING_DATA,
      RX_STATE_READING_CRC
    } RXState;
    
    RXState m_receiveState;  // FSM state
    String m_receiveBuffer;  // Temp buffer for extracting current packet field      
    
    int m_receiveDataLen;    // Size of packet payload    
    String m_receiveData;    // Packet payload contents    
    
    void processIncoming();
    
    virtual int calcCRC(const char* data );
    virtual bool checkCRC(const char* data, const int crc );

    void resetToStartState();
        
    bool getNextChar(char* cBuffer);

  public:
  
    // Constructor
    // A note on reserveBufferSize
    //    Strings are used for the working buffers for data 
    //    To reduce/avoid memory issues such as fragmentation the buffers
    //    are reserved in the constructor although they may grow beyond these values
    //    Choose reserveBufferSize as appropriate for your application
    PacketSerial(Stream* serial, const int reserveBufferSize=64);
    
    // Sends the data in a packet
    // data: null terminated string
    void sendData(const char* data);
    
    // Returns true if a packet has been received and the payload copied into the buffer
    // Places the data in  buffer and terminates it with null
    // Truncates packet payload if the size of the  payload >= bufferlen
    bool getData(char* buffer, const int bufferlen);

};
