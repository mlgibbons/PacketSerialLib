#ifndef _NODEMESSAGING_h
#define _NODEMESSAGING_h

#include <RFM12B.h>

/************************************************************************************
 *  Format of frames and messages
 *
 *  See /docs/protocols/InterNodeMessaging.txt for full details
 *
 ************************************************************************************/

#define NODE_MSG_FIELD_SEP     ';'
#define NODE_MSG_FIELD_SEP_STR ";"

 // ----------------------------------------------------------------------
 // UnreliableProtocol
 // ----------------------------------------------------------------------

#define UNRELIABLE_PROTOCOL_MSG_MAX_PAYLOAD_SIZE 32
#define UNRELIABLE_PROTOCOL_MSG_VERSION 1

typedef struct {
	char   payload[UNRELIABLE_PROTOCOL_MSG_MAX_PAYLOAD_SIZE];
} UnreliableProtocolMsg;

// ----------------------------------------------------------------------
// ReliableProtocol
// ----------------------------------------------------------------------

#define RELIABLE_PROTOCOL_MSG_MAX_PAYLOAD_SIZE 32
#define RELIABLE_PROTOCOL_MSG_VERSION 1

typedef struct {
	int     msgId;
	boolean reqAck;
	boolean isAck;
	char    payload[RELIABLE_PROTOCOL_MSG_MAX_PAYLOAD_SIZE];
} ReliableProtocolMsg;

// ----------------------------------------------------------------------
// InterNodeMsg
// ----------------------------------------------------------------------

enum MsgProtocol {
	UnreliableProtocol = 1,
	ReliableProtocol = 2
};

union InterNodeMsgPayload {    
	UnreliableProtocolMsg unreliableProtocolMsg;
	//ReliableProtocolMsg   reliableProtocolMsg;
};

#define INTER_NODE_MSG_MAX_NODE_ID_SIZE 4
#define INTER_NODE_MSG_VERSION 1

typedef struct _InterNodeMsg {
	char dstNodeId[INTER_NODE_MSG_MAX_NODE_ID_SIZE];
	char srcNodeId[INTER_NODE_MSG_MAX_NODE_ID_SIZE];
	MsgProtocol protocol;
	union InterNodeMsgPayload payload;
} InterNodeMsg;

// ----------------------------------------------------------------------
// Frame
// ----------------------------------------------------------------------

enum FrameType {
	InterNodeMsgFrame = 1
};

union MsgFramePayload {
	InterNodeMsg interNodeMsg;
};

typedef struct {
    FrameType frameType;
    union MsgFramePayload payload;
} MsgFrame;  

// Utility functions

int addMsgStringField(char* buffer, const int bufferSize, const char* fieldValue, const boolean withSep = true);
int addMsgIntField(char* buffer, const int bufferSize, const int fieldValue, const boolean withSep = true);
int addMsgBooleanField(char* buffer, const int bufferSize, const boolean fieldValue, const boolean withSep = true);

/*******************************************************************************************
 *
 *
 *******************************************************************************************/
class AbstractMessageTransport
{
  public:
	  virtual int bind(const char* nodeId) {};
	  
	  virtual int send(const InterNodeMsg* nodeMsg) {};
    
	  virtual InterNodeMsg* processEvents() {};

  protected:

    int serialiseInterNodeMsg(const InterNodeMsg* nodeMsg, char* buffer, int bufferSize);    

    int deserialiseInterNodeMsg(const char* buffer, InterNodeMsg** nodeMsg); 
    
  private:
  
};

/*******************************************************************************************
 *
 *
 *******************************************************************************************/
class RFMTransport: public AbstractMessageTransport
{
  public:
  
    RFMTransport(int rfmNodeId, int rfmNetworkId, int rfmGatewayId=1);

    int send(const InterNodeMsg* nodeMsg);
    
    InterNodeMsg* processEvents();
    
  protected:
  
    RFM12B* m_radio;
    int     m_rfmNetworkId;
    int     m_rfmNodeId;
    int     m_rfmGatewayId;
    
    int mapNodeIdToRFMNodeId(const char *nodeId, int& rfmNodeId);     
};

/*******************************************************************************************
 *
 *
 *******************************************************************************************/
class MessagingEndpoint 
{
  public:
    MessagingEndpoint(AbstractMessageTransport* m_msgTransport);

    int bind(const char* nodeId);    

    int sendMsg(const char* destNodeId, const char* payload);
    
    boolean hasMsg();
    int getMsg(InterNodeMsg** msg);

    int processEvents();
        
  protected:  
    AbstractMessageTransport* m_msgTransport;
    char m_nodeId[INTER_NODE_MSG_MAX_NODE_ID_SIZE];

    InterNodeMsg* m_rxMsg;

  private:

    //int parseProtocolMsg(const String& payload, ProtocolMsg& protocolMsg);
    //void parseUnreliableProtocolMsg(const String& payload, UnreliableProtocolMsg& upm);
    //void parseReliableProtocolMsg(const String& payload, ReliableProtocolMsg& rpm);

};


#endif

