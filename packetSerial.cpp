
#include "packetSerial.h"

/*
 Packet and field delimiting characters
 */

#define STX  '<'
#define ETX  '>'
#define FSEP ','

#ifdef DEBUG
#define LOG_DEBUG(M) Logger.logDebug(M);
#else
#define LOG_DEBUG(M) 
#endif

/************************************************
 * Returns true if character is numeric
 ************************************************/
bool isNumChar(char aChar)
{
    char numChars[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'}; 

    bool rc = false;
    for (int i=0; i< sizeof(numChars); i++) {
        if (aChar==numChars[i]) rc=true;
    }
   
    return rc;
}             

/*************************************************
 * Constructor
 * Pass in a Stream pointer over which packets
 * will be sent and received
 *************************************************/
PacketSerial::PacketSerial(Stream* serial, int reserveBufferSize)
{
  m_serial = serial;
  
  m_data.reserve(reserveBufferSize);  
  m_receiveBuffer.reserve(reserveBufferSize);
  m_receiveData.reserve(reserveBufferSize);
  
  m_dataAvailable = false;
  resetToStartState();
}

/*************************************************
 * Send data in a packet over the stream
 *************************************************/    
void PacketSerial::sendData(const char* data) 
{
    m_serial->print(STX);
    m_serial->print(strlen(data));
    m_serial->print(FSEP);
    m_serial->print(data);
    m_serial->print(FSEP);
    m_serial->print( calcCRC(data) );
    m_serial->print(ETX);
    m_serial->println();      
}

/*************************************************
 * Get data received in the last packet
 *************************************************/    
bool PacketSerial::getData(char* buffer, int bufferlen)
{
  if (m_dataAvailable==true) {
      // copy received data into output buffer truncating if needed
      memcpy(buffer, m_data.c_str(), min(m_data.length()+1, bufferlen));
      buffer[bufferlen-1]='\0';
      
      // clear received data buffer and flag
      m_dataAvailable = false;      
      m_data = "";
      
      return true;
  } 
  else {
      processIncoming();        
      return false;
  }
}

/**************************************************
 * Reset the receive state machine back into the
   start state i.e. looking for packet header
 **************************************************/
void PacketSerial::resetToStartState()
{
    m_receiveState  = RX_STATE_READING_STX;
    m_receiveBuffer = "";        
    
    m_receiveDataLen = 0;        
    m_receiveData = "";        
    m_receiveCRC = 0;        
}

/**************************************************
 * Process incoming characters on the stream
 **************************************************/
void PacketSerial::processIncoming()
{
    char rxChar;
    bool rc = getNextChar(&rxChar);
    
    if (rc) {

      // *****************************************************
      // Waiting for packet start
      // *****************************************************
      if (m_receiveState==RX_STATE_READING_STX) {
          LOG_DEBUG((String(F("RX_STATE_READING_STX - ")) + rxChar).c_str())
  
          if (rxChar==STX) {
              LOG_DEBUG(String(F("RX_STATE_READING_STX - found STX")).c_str())
              m_receiveBuffer="";
              m_receiveState=RX_STATE_READING_LEN_DATA;              
          } else {
              // not packet start so ignore the char    
              // maybe we started reading mid packet
              LOG_DEBUG((String(F("RX_STATE_READING_STX: Encountered unexpected char in stream - ")) + String(rxChar)).c_str())
          }
      }
      
      // *****************************************************
      // reading data len
      // *****************************************************
      else if (m_receiveState==RX_STATE_READING_LEN_DATA) {
          LOG_DEBUG((String(F("RX_STATE_READING_LEN_DATA - ")) + String(rxChar)).c_str())
          
          if (rxChar!=FSEP) {                            
              if (isNumChar(rxChar)) {
                  m_receiveBuffer += rxChar;
              } else {
                  LOG_DEBUG((String(F("RX_STATE_READING_LEN_DATA: Encountered unexpected char in stream - ")) + String(rxChar)).c_str())
                  resetToStartState();
              }
          }    
          
          else {
              m_receiveDataLen = m_receiveBuffer.toInt();
              m_receiveBuffer  = "";
              m_receiveData = "";              
              m_receiveState = RX_STATE_READING_DATA;
              LOG_DEBUG((String(F("RX_STATE_READING_LEN_DATA: Completed read of data len - ")) + String(m_receiveDataLen)).c_str())
          }
      }
      
      // *****************************************************
      // reading data
      // *****************************************************
      else if (m_receiveState==RX_STATE_READING_DATA) 
      {
          LOG_DEBUG((String(F("RX_STATE_READING_DATA - ")) + String(rxChar)).c_str())
            
          if (m_receiveBuffer.length() < m_receiveDataLen) {
              m_receiveBuffer += rxChar;
          }
           
          else {    
              // should be field seperator after data element
              if (rxChar==FSEP) {
                  m_receiveData = m_receiveBuffer;
                  m_receiveBuffer = "";
                  m_receiveState = RX_STATE_READING_CRC;
                  LOG_DEBUG((String(F("RX_STATE_READING_DATA: Data read complete - ")) + m_receiveData).c_str())
              }
                  
              else {
                  // was expecting field seperator after data block
                  LOG_DEBUG((String(F("RX_STATE_READING_DATA: Encountered unexpected char in stream - ")) + String(rxChar)).c_str())
                  resetToStartState();                       
              }    
          }        
      } 
  
      // *****************************************************
      // reading CRC
      // *****************************************************
      else if (m_receiveState==RX_STATE_READING_CRC) 
      {
          LOG_DEBUG((String(F("RX_STATE_READING_CRC - ")) + String(rxChar)).c_str())
  
          if (rxChar==ETX) {              
              LOG_DEBUG(String(F("RX_STATE_READING_CRC: ETX found")).c_str())

              m_receiveCRC = m_receiveBuffer.toInt();
              m_receiveBuffer = "";

              LOG_DEBUG((String(F("RX_STATE_READING_CRC CRC = ")) + String(m_receiveCRC)).c_str())
              
              if ( checkCRC(m_receiveData.c_str(), m_receiveCRC) == true) {
                  m_data = m_receiveData;
                  m_dataAvailable = true;
                  resetToStartState();
              } else {
                  LOG_DEBUG(String(F("RX_STATE_READING_CRC: CRC check failed")).c_str())
                  resetToStartState();
              }
          }
          else {
            if (isNumChar(rxChar)) {
                m_receiveBuffer += rxChar;                    
            }  
            else {
                LOG_DEBUG((String(F("RX_STATE_READING_CRC: Encountered unexpected char in stream - ")) + String(rxChar)).c_str())
            }
          }
          
      }
      
      else {
          LOG_DEBUG(String(F("***** Received character but unhandled state ********")).c_str())
      }
      
    }
    
}
    
bool PacketSerial::getNextChar(char* cBuffer)
{
  bool rc = false;

  if (m_serial->available() > 0) 
  {
      *cBuffer = m_serial->read();
      rc = true;
  }
  
  return rc;
}

int PacketSerial::calcCRC(const char* data )
{
  return 0;
}

bool PacketSerial::checkCRC(const char* content, int crc ) 
{
    int calcedCRC = calcCRC( content );
    
    if (calcedCRC != crc ) {
        return false;
    } else {
        return true;
    }    
}


