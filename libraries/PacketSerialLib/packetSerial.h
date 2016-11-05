#include "Arduino.h"

class PacketSerial {

  private:
    Stream* m_serial;
    bool m_dataAvailable;
    String m_data;
    
    typedef enum {
      RX_STATE_READING_STX,
      RX_STATE_READING_LEN_DATA,
      RX_STATE_READING_DATA,
      RX_STATE_READING_CRC
    } RXState;
    
    RXState m_receiveState;
    String m_receiveBuffer;        
    
    int m_receiveDataLen;        
    String m_receiveData;        
    int m_receiveCRC;        
  
    void processIncoming();
    
    int calcCRC(const char* data );
    bool checkCRC(const char* data, int crc );

    void resetToStartState();
        
    bool getNextChar(char* cBuffer);

  public:
  
    PacketSerial(Stream* serial, int reserveBufferSize=64);
    
    void sendData(const char* data);
    
    bool getData(char* buffer, int bufferlen);

};
