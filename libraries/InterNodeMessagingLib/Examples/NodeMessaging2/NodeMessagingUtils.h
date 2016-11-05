#ifndef _NODEMESSAGING_h
#define _NODEMESSAGING_h

#include "MessageFrames.h"
#include <RFM12B.h>

/************************************************************************************
 *  Format of messages
 ************************************************************************************/

typedef struct {
    int    frameType;
    int    frameVersion;
} FrameHeader;  

#define FRAME_TYPE_NODE_VERSION 1

typedef struct {
    FrameHeader frameHeader;
    String  dstNodeId;
    String  srcNodeId;
    boolean reqAck;
    boolean isAck;
    int     msgId;
} NodeMsgHeader;

typedef struct {
    NodeMsgHeader header;
    String payload;
} NodeMsg;

#define NODEMSG_FIELD_SEP ';'

/*******************************************************************************************
 *
 *
 *******************************************************************************************/
class AbstractMessageTransport
{
  public:
    virtual int send(NodeMsg& nodeMsg) {};
    
    int serialiseNodeMsg(NodeMsg& NodeMsg, String& nodeMsgStr);
    
    int addMsgStringField(String& nodeMsgStr, String& fieldValue, boolean withSep=true);
    int addMsgIntField(String& nodeMsgStr, int fieldValue, boolean withSep=true);
    int addMsgBooleanField(String& nodeMsgStr, boolean fieldValue, boolean withSep=true);

    virtual NodeMsg* processEvents() {};

    int parseRadioPayloadIntoFrameHeader(const String& rfmPayload, FrameHeader& frameHeader);
    int parseRadioPayloadIntoNodeMsg(const String& rfmPayload, NodeMsg* nodeMsg);    
    int logNodeMsg(const NodeMsg* nodeMsg);
    
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
    int send(NodeMsg& nodeMsg);
    
    NodeMsg* processEvents();
    
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
    int connect(const char* nodeId, int timeout=5);    

    int sendMsg(const char* destNodeId, const char* payload, boolean reqAck=false, int timeout=5);
    
    boolean hasMsg();
    NodeMsg* getMsg();

    int processEvents();
        
  protected:  
    AbstractMessageTransport* m_msgTransport;
    String m_nodeId;
    int m_nextMsgId;
    NodeMsg* m_rxNodeMsg;
    
  private:
    int buildNodeMsgHeader( NodeMsg& msg, const char* dstNodeId, int msgId, boolean reqAck=false, boolean isAck=false);
};


#endif

