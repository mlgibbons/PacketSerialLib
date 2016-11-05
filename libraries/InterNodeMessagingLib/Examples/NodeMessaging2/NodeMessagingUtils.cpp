#include "NodeMessagingUtils.h"
#include <stdio.h>
#include <MGLib.h>

// *****************************************************************************************
MessagingEndpoint::MessagingEndpoint(AbstractMessageTransport* msgTransport)
{
  m_msgTransport = msgTransport;
  m_nextMsgId = 1;
  m_rxNodeMsg = NULL;  
}
   
// *****************************************************************************************
int MessagingEndpoint::connect(const char* nodeId, int timeout) 
{
  m_nodeId = String(nodeId);  
}

// *****************************************************************************************
int MessagingEndpoint::sendMsg(const char* dstNodeId, const char* payload, boolean reqAck, int timeout) 
{        
    NodeMsg msg;
    buildNodeMsgHeader(msg, dstNodeId, m_nextMsgId++, reqAck, false);
    msg.payload = payload;
    m_msgTransport->send(msg);
    
    //:TODO:wait for ack if requested    
}

// *****************************************************************************************
int MessagingEndpoint::buildNodeMsgHeader( NodeMsg& msg, const char* dstNodeId, int msgId, boolean reqAck, boolean isAck)
{
    msg.header.frameHeader.frameType        = FRAME_TYPE_NODE;
    msg.header.frameHeader.frameVersion     = FRAME_TYPE_NODE_VERSION;
    
    msg.header.dstNodeId   = dstNodeId;
    msg.header.srcNodeId   = m_nodeId;
    msg.header.reqAck      = reqAck;
    msg.header.isAck       = isAck;    
    msg.header.msgId       = msgId;
}

// *****************************************************************************************
int MessagingEndpoint::processEvents() 
{
  NodeMsg* nodeMsg = m_msgTransport->processEvents();

  if (nodeMsg!=NULL) {
    m_rxNodeMsg = nodeMsg;
    
    //send back ack if requested 
    if (nodeMsg->header.reqAck==true) {
      NodeMsg ackMsg;
      buildNodeMsgHeader(ackMsg, nodeMsg->header.srcNodeId.c_str(), nodeMsg->header.msgId, false, true);
      m_msgTransport->send(ackMsg);
    }
    
    if (nodeMsg->header.isAck==true) {
       // we should never get here as if an ack is requested then the send method
       // will wait until the ack is received or timeout. So, to get here means that the
       // send method timed out. Just log and dump as the hihg level layer will retry 
       Serial.println((String(F("Unexpected ACK received for msgId:["))+nodeMsg->header.msgId+F("] from nodeId:[")+nodeMsg->header.srcNodeId+F("]")).c_str());
    }    
  }  
}

// *****************************************************************************************
boolean MessagingEndpoint::hasMsg()
{
  return m_rxNodeMsg==NULL ? false: true;
}

// *****************************************************************************************
NodeMsg* MessagingEndpoint::getMsg() 
{
  NodeMsg* rc = m_rxNodeMsg;
  m_rxNodeMsg = NULL;
  return rc;
}  


// *****************************************************************************************
int AbstractMessageTransport::addMsgStringField(String& nodeMsgStr, String& fieldValue, boolean withSep)
{
    nodeMsgStr += fieldValue;
    if (withSep==true) nodeMsgStr += NODEMSG_FIELD_SEP;
}

// *****************************************************************************************
int AbstractMessageTransport::addMsgIntField(String& nodeMsgStr, int fieldValue, boolean withSep)
{
    nodeMsgStr += fieldValue;
    if (withSep==true) nodeMsgStr += NODEMSG_FIELD_SEP;
}

// *****************************************************************************************
int AbstractMessageTransport::addMsgBooleanField(String& nodeMsgStr, boolean fieldValue, boolean withSep)
{
    nodeMsgStr += fieldValue;
    if (withSep==true) nodeMsgStr += NODEMSG_FIELD_SEP;
}


// *****************************************************************************************
int AbstractMessageTransport::serialiseNodeMsg(NodeMsg& nodeMsg, String& nodeMsgStr)
{
    nodeMsgStr = "";
    addMsgIntField(nodeMsgStr, nodeMsg.header.frameHeader.frameType);              
    addMsgIntField(nodeMsgStr, nodeMsg.header.frameHeader.frameVersion);           
    addMsgStringField(nodeMsgStr, nodeMsg.header.dstNodeId);            
    addMsgStringField(nodeMsgStr, nodeMsg.header.srcNodeId);               
    addMsgBooleanField(nodeMsgStr, nodeMsg.header.reqAck);            
    addMsgBooleanField(nodeMsgStr, nodeMsg.header.isAck);                
    
    if (nodeMsg.header.isAck==true) {
      addMsgIntField(nodeMsgStr, nodeMsg.header.msgId, false);        
    } else {
      addMsgIntField(nodeMsgStr, nodeMsg.header.msgId);  
      addMsgStringField(nodeMsgStr, nodeMsg.payload, false);            
    }
}

// *****************************************************************************************
RFMTransport::RFMTransport(int rfmNodeId, int rfmNetworkId, int rfmGatewayId)
{
    m_rfmNodeId    = rfmNodeId;
    m_rfmNetworkId = rfmNetworkId;
    m_rfmGatewayId = rfmGatewayId;
            
    m_radio = new RFM12B();        
    m_radio->Initialize(m_rfmNodeId, RF12_868MHZ, m_rfmNetworkId);
    m_radio->Encrypt(NULL);

	delay(100);  // allow a small delay for the radio to init otherwise the first packet sent can be lost
}
   
// *****************************************************************************************
int RFMTransport::send(NodeMsg& nodeMsg) 
{        
    String nodeMsgStr;
    serialiseNodeMsg(nodeMsg, nodeMsgStr);

    int dstRFMNodeId;
    mapNodeIdToRFMNodeId(nodeMsg.header.dstNodeId.c_str(), dstRFMNodeId);
      
    m_radio->Send(dstRFMNodeId, nodeMsgStr.c_str(), nodeMsgStr.length());
}

// *****************************************************************************************
int RFMTransport::mapNodeIdToRFMNodeId(const char* nodeId, int& rfmNodeId)
{
  // for now all messages go through the gateway
  // we could add an ARP type mapping protocol in here or a 
  // /etc/hosts type mapping file but for the moment just send
  // to the gateway
  
  rfmNodeId = m_rfmGatewayId;
}

// *****************************************************************************************
NodeMsg* RFMTransport::processEvents()
{
  NodeMsg* rxNodeMsg = NULL;  

  if (m_radio->ReceiveComplete())
  {
      if (m_radio->CRCPass())
      {
          Serial.println("Got radio msg");
          
          if (m_radio->ACKRequested()) {
              m_radio->SendACK();
          }
          
          int src = m_radio->GetSender();
          int dst = m_radio->GetDestination();
          
          // convert payload to string
          uint8_t* rfmPayloadBuffer = (uint8_t *) &(m_radio->Data[0]);
          int rfmPayloadLen = *(m_radio->DataLen);

          String rfmPayload;
          rfmPayload.reserve(rfmPayloadLen);
          for (int x=0; x < rfmPayloadLen; x++)
          {
              rfmPayload += (char) rfmPayloadBuffer[x];
          }

          Serial.println((String(F("Payload:["))+rfmPayload+F("]")).c_str());
          
          // parse the frame header
          //FrameHeader frameHeader;
          //parseRadioPayloadIntoFrameHeader(rfmPayload.c_str(), frameHeader);
      
          NodeMsg nodeMsg;
          parseRadioPayloadIntoNodeMsg(rfmPayload, &nodeMsg);
          
          logNodeMsg(&nodeMsg);
      }
  }

  return rxNodeMsg;  
}

// *****************************************************************************************
int AbstractMessageTransport::parseRadioPayloadIntoFrameHeader(const String& rfmPayload, FrameHeader& frameHeader)
{
    frameHeader.frameType    = 0;
    frameHeader.frameVersion = 0;

    String rfmPayloadText(rfmPayload);
    
    frameHeader.frameType = getField(&rfmPayloadText, NODEMSG_FIELD_SEP).toInt();
    frameHeader.frameVersion = getField(&rfmPayloadText, NODEMSG_FIELD_SEP).toInt();
}


// *****************************************************************************************
int AbstractMessageTransport::parseRadioPayloadIntoNodeMsg(const String& rfmPayload, NodeMsg* nodeMsg)
{
    nodeMsg->header.frameHeader.frameType     = 0;
    nodeMsg->header.frameHeader.frameVersion  = 0;
    nodeMsg->header.dstNodeId                 = "";
    nodeMsg->header.srcNodeId                 = "";
    nodeMsg->header.reqAck                    = false;
    nodeMsg->header.reqAck                    = false;
    nodeMsg->header.msgId                     = 0;
    nodeMsg->payload                          = "";

    String rfmPayloadText(rfmPayload);
    
    nodeMsg->header.frameHeader.frameType    = getField(&rfmPayloadText, NODEMSG_FIELD_SEP).toInt();
    nodeMsg->header.frameHeader.frameVersion = getField(&rfmPayloadText, NODEMSG_FIELD_SEP).toInt();
    nodeMsg->header.dstNodeId                = getField(&rfmPayloadText, NODEMSG_FIELD_SEP);
    nodeMsg->header.srcNodeId                = getField(&rfmPayloadText, NODEMSG_FIELD_SEP);
    nodeMsg->header.reqAck                   = getField(&rfmPayloadText, NODEMSG_FIELD_SEP).toInt();
    nodeMsg->header.isAck                    = getField(&rfmPayloadText, NODEMSG_FIELD_SEP).toInt();    
    nodeMsg->header.msgId                    = getField(&rfmPayloadText, NODEMSG_FIELD_SEP).toInt();

    if (nodeMsg->header.isAck==false) {
      nodeMsg->payload = rfmPayloadText.substring(0,rfmPayloadText.length());
    }
}

// *****************************************************************************************
int AbstractMessageTransport::logNodeMsg(const NodeMsg* nodeMsg)
{
    int rc = 0;
    
    Serial.println("NodeMsg");
    Serial.println((String(F("  frameType:    ["))+nodeMsg->header.frameHeader.frameType+"]").c_str());
    Serial.println((String(F("  frameVersion: ["))+nodeMsg->header.frameHeader.frameVersion+"]").c_str());
    Serial.println((String(F("  dstNodeId:    ["))+nodeMsg->header.dstNodeId+"]").c_str());
    Serial.println((String(F("  srcNodeId:    ["))+nodeMsg->header.srcNodeId+"]").c_str());
    Serial.println((String(F("  msgId:        ["))+nodeMsg->header.msgId+"]").c_str());
    Serial.println((String(F("  reqAck:       ["))+nodeMsg->header.reqAck+"]").c_str());
    Serial.println((String(F("  isAck:        ["))+nodeMsg->header.isAck+"]").c_str());
    Serial.println((String(F("  payload:      ["))+nodeMsg->payload+"]").c_str());
    
    return rc;
}


