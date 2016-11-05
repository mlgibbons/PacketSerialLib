#include "InterNodeMessagingLib.h"
#include <stdio.h>
#include <stdlib.h>
#include <MGLib.h>


// *****************************************************************************************
int addMsgStringField(char* buffer, const int bufferSize, const char* fieldValue, const boolean withSep)
{
    int _len;
	
	_len = strlcat(buffer, fieldValue, bufferSize);
	if (_len >= bufferSize) return -1;

	if (withSep == true) {
		_len = strlcat(buffer, NODE_MSG_FIELD_SEP_STR, bufferSize);
	}

	return 0;
}

// *****************************************************************************************
int addMsgIntField(char* buffer, const int bufferSize, const int fieldValue, const boolean withSep)
{
    char valueBuffer[16];
    itoa(fieldValue, valueBuffer, 10);
	return addMsgStringField(buffer, bufferSize, valueBuffer, withSep);
}

// *****************************************************************************************
int addMsgBooleanField(char* buffer, const int bufferSize, const boolean fieldValue, const boolean withSep)
{
	if (fieldValue==true) {
		return addMsgStringField(buffer, bufferSize, "1", withSep);
	} else {
		return addMsgStringField(buffer, bufferSize, "0", withSep);
	}
}

// *****************************************************************************************
MessagingEndpoint::MessagingEndpoint(AbstractMessageTransport* msgTransport)
{
  LOG_FUNCTION("MessagingEndpoint::MessagingEndpoint")

  m_msgTransport = msgTransport;
  m_rxMsg = NULL;  
}
   
// *****************************************************************************************
int MessagingEndpoint::bind(const char* nodeId) 
{
    LOG_FUNCTION("MessagingEndpoint::bind")

    int _len;
    _len = strlcpy(m_nodeId, nodeId, sizeof(m_nodeId));
    
    return (_len >= sizeof(m_nodeId)) ? -1 : 0;
}

// *****************************************************************************************
int MessagingEndpoint::sendMsg(const char* dstNodeId, const char* payload) 
{        
    LOG_FUNCTION("MessagingEndpoint::sendMsg")

	int _len;

	// Protocol content
	UnreliableProtocolMsg upm;
	_len = strlcpy(upm.payload, payload, sizeof(upm.payload));
	if (_len>=sizeof(upm.payload)) return -1;

	// INM payload
	InterNodeMsgPayload inmPayload = { .unreliableProtocolMsg = upm };

    // INM
	InterNodeMsg msg = {"", "", UnreliableProtocol, inmPayload}; 

	_len = strlcpy(msg.dstNodeId, dstNodeId, sizeof(msg.dstNodeId));
	if (_len >= sizeof(msg.dstNodeId)) return -1;

	_len = strlcpy(msg.srcNodeId, m_nodeId, sizeof(msg.srcNodeId));
	if (_len >= sizeof(msg.srcNodeId)) return -1;
	
    return m_msgTransport->send(&msg);
}

// *****************************************************************************************
boolean MessagingEndpoint::hasMsg()
{
    LOG_FUNCTION("MessagingEndpoint::hasMsg")
    return m_rxMsg==NULL ? false : true;
}

// *****************************************************************************************
int MessagingEndpoint::getMsg(InterNodeMsg** msg)
{
    LOG_FUNCTION("MessagingEndpoint::getMsg")

    if (m_rxMsg!=NULL) {
		*msg = m_rxMsg;
		m_rxMsg = NULL;
		return 0;
    }
    else {
        return -1;
    }
}

// *****************************************************************************************
int MessagingEndpoint::processEvents() 
{
  LOG_FUNCTION("MessagingEndpoint::processEvents")

  InterNodeMsg* nodeMsg = m_msgTransport->processEvents();

  if (nodeMsg != NULL) {
      m_rxMsg = nodeMsg;
  }
}

// *****************************************************************************************
/*int MessagingEndpoint::parseProtocolMsg(const String& payload, ProtocolMsg& protocolMsg)
{
    LOG_FUNCTION("MessagingEndpoint::parseProtocolMsg")

    //protocolMsg.protocol = 0;
    //strcpy(protocolMsg.payload,"");
    
    //String payloadText(payload);

    //protocolMsg.protocol = getField(&payloadText, NODEMSG_FIELD_SEP).toInt();
    //protocolMsg.payload = payloadText;
}
*/
// *****************************************************************************************
/*void MessagingEndpoint::parseUnreliableProtocolMsg(const String& payload, UnreliableProtocolMsg& upm)
{
    LOG_FUNCTION("MessagingEndpoint::parseUnreliableProtocolMsg")

    ///upm.version = 0;
    //upm.payload = "";

    //String payloadText(payload);

    //upm.version = getField(&payloadText, NODEMSG_FIELD_SEP).toInt();
    //upm.payload = payloadText;
}
*/
// *****************************************************************************************
/*void MessagingEndpoint::parseReliableProtocolMsg(const String& payload, ReliableProtocolMsg& rpm)
{
    //:TODO
}
*/

// *****************************************************************************************
int AbstractMessageTransport::serialiseInterNodeMsg(const InterNodeMsg* nodeMsg, char* buffer, int bufferSize)
{
    LOG_FUNCTION("AbstractMessageTransport::serialiseInterNodeMsgToFrame")

    int rc = 0;
	
	addMsgIntField(buffer, bufferSize, INTER_NODE_MSG_VERSION);
	if (rc != 0) return rc;
	
	addMsgStringField(buffer, bufferSize, nodeMsg->dstNodeId);
	if (rc != 0) return rc;

	addMsgStringField(buffer, bufferSize, nodeMsg->srcNodeId);
	if (rc != 0) return rc;

	addMsgIntField(buffer, bufferSize, nodeMsg->protocol);
	if (rc != 0) return rc;

	if (nodeMsg->protocol == UnreliableProtocol) 
	{
		addMsgIntField(buffer, bufferSize, UNRELIABLE_PROTOCOL_MSG_VERSION);
		if (rc != 0) return rc;

		addMsgStringField(buffer, bufferSize, nodeMsg->payload.unreliableProtocolMsg.payload, false);
		if (rc != 0) return rc;
	}

	return rc;
}

// *****************************************************************************************
RFMTransport::RFMTransport(int rfmNodeId, int rfmNetworkId, int rfmGatewayId)
{
    LOG_FUNCTION("RFMTransport::RFMTransport")
    
    m_rfmNodeId    = rfmNodeId;
    m_rfmNetworkId = rfmNetworkId;
    m_rfmGatewayId = rfmGatewayId;
            
    m_radio = new RFM12B();        
    m_radio->Initialize(m_rfmNodeId, RF12_868MHZ, m_rfmNetworkId);
    m_radio->Encrypt(NULL);
    
    delay(100);  // allow a small delay for the radio to init otherwise the first packet sent can be lost
}
   
// *****************************************************************************************
int RFMTransport::send(const InterNodeMsg* nodeMsg) 
{        
    LOG_FUNCTION("RFMTransport::send")

    int rc;

    // Serialise the message and send out in a frame
    char buffer[64];
    strcpy(buffer, "");

    // Frame header
    rc = addMsgIntField(buffer, sizeof(buffer), InterNodeMsgFrame);
    if (rc != 0) return rc;

    // Inter Node message
    rc = serialiseInterNodeMsg(nodeMsg, buffer, sizeof(buffer));
    if (rc != 0) return rc;

    Logger.logInfo2(F("RFMTransport::send - Sending [%s]"), buffer);

    // Map logical node id to a physical RFM node id
    int dstRFMNodeId;
    mapNodeIdToRFMNodeId(nodeMsg->dstNodeId, dstRFMNodeId);

    // Send radio msg
    m_radio->Send(dstRFMNodeId, buffer, strlen(buffer));

    return 0;
}

// *****************************************************************************************
int RFMTransport::mapNodeIdToRFMNodeId(const char* nodeId, int& rfmNodeId)
{
  LOG_FUNCTION("RFMTransport::mapNodeIdToRFMNodeId")

  // for now all messages go through the gateway
  // we could add an ARP type mapping protocol in here or a 
  // /etc/hosts type mapping file but for the moment just send
  // to the gateway
  
  rfmNodeId = m_rfmGatewayId;
}

// *****************************************************************************************
InterNodeMsg* RFMTransport::processEvents()
{
  LOG_FUNCTION("RFMTransport::processEvents")
  
  InterNodeMsg* rxNodeMsg = NULL;

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

          Logger.logInfo2(F("RFMTransport::processEvents - received msg [%s]"), rfmPayload.c_str());
          
          // Parse the frame header
          // For char is the frame type
          char frameTypeStr[2] = {'\0','\0'};
          frameTypeStr[0] = rfmPayload[0];
          int frameType = atoi(frameTypeStr);

          if (frameType==InterNodeMsgFrame) 
          {
              Logger.logInfo2(F("RFMTransport::processEvents - frame type is InterNodeMsg"));

              if (deserialiseInterNodeMsg(&rfmPayload[2], &rxNodeMsg) != 0) {
                  Logger.logInfo2(F("RFMTransport::processEvents - failed deserialisation of InterNodeMsg"));
              }

          } else {
              Logger.logInfo2(F("RFMTransport::processEvents - received frame with unknown frame type [%d]"), frameType);
          }
            
      }
  }

  return rxNodeMsg;  
}

// *****************************************************************************************
/*int AbstractMessageTransport::parseRadioPayloadIntoFrame(const String& rfmPayload, MsgFrame& frame)
{
    LOG_FUNCTION("AbstractMessageTransport::parseRadioPayloadIntoFrame")

    //frame.frameType = 0;
    //frame.payload = "";

    //String rfmPayloadText(rfmPayload);
    
    //frame.frameType = getField(&rfmPayloadText, NODEMSG_FIELD_SEP).toInt();
    //frame.payload = rfmPayloadText;
}

*/
// *****************************************************************************************
int AbstractMessageTransport::deserialiseInterNodeMsg(const char* buffer, InterNodeMsg** nodeMsg)
{
    LOG_FUNCTION("AbstractMessageTransport::deserialiseInterNodeMsg")
    
    int version;
    char dstNodeId[INTER_NODE_MSG_MAX_NODE_ID_SIZE];
    char srcNodeId[INTER_NODE_MSG_MAX_NODE_ID_SIZE];
    MsgProtocol protocol;

    // Copy into temp buffer as strtok modifies the string
    char _buffer[64];
    int _len;
    
    _len = strlcpy(_buffer, buffer, sizeof(_buffer));
    if (_len >= sizeof(_buffer)) {
        Logger.logInfo2(F("AbstractMessageTransport::deserialiseInterNodeMsg - failed to copy all of serialised InterNodeMsg into temp buffer"));
        return -1;
    }

    char *token;

    // INM version
    token = strtok(_buffer, NODE_MSG_FIELD_SEP_STR);
    if (token == NULL) return -1;
    version = atoi(token);
    if (version != INTER_NODE_MSG_VERSION) {
        Logger.logInfo2(F("AbstractMessageTransport::deserialiseInterNodeMsg - received frame with unhandled InterNodeMsgVersion - got [%d]; expected [%d]"), version, INTER_NODE_MSG_VERSION);
        return -1;
    }

    // dstNodeId
    token = strtok(NULL, NODE_MSG_FIELD_SEP_STR);
    if (token == NULL) return -1;
    _len = strlcpy(dstNodeId, token, sizeof(dstNodeId));
    if (_len >= sizeof(dstNodeId)) {
        return -1;
    }

    // srcNodeId
    token = strtok(NULL, NODE_MSG_FIELD_SEP_STR);
    if (token == NULL) return -1;
    _len = strlcpy(srcNodeId, token, sizeof(srcNodeId));
    if (_len >= sizeof(srcNodeId)) {
        return -1;
    }

    // protocol
    token = strtok(NULL, NODE_MSG_FIELD_SEP_STR);
    if (token == NULL) return -1;
    protocol = (MsgProtocol) atoi(token);

    if (protocol == UnreliableProtocol) {

        // version
        int upmVersion;
        token = strtok(NULL, NODE_MSG_FIELD_SEP_STR);
        if (token == NULL) return -1;
        upmVersion = atoi(token);
        if (upmVersion != UNRELIABLE_PROTOCOL_MSG_VERSION) {
            Logger.logInfo2(F("AbstractMessageTransport::deserialiseInterNodeMsg - received UnreliableProtocolMsg with unhandled version - got [%d]; expected [%d]"), upmVersion, UNRELIABLE_PROTOCOL_MSG_VERSION);
            return -1;
        }

        // payload
        token = strtok(NULL, NODE_MSG_FIELD_SEP_STR);
        if (token == NULL) return -1;

        UnreliableProtocolMsg upm;
        _len = strlcpy(upm.payload, token, sizeof(upm.payload));
        if (_len >= sizeof(upm.payload)) return -1;
        
        InterNodeMsg* _nodeMsg = new InterNodeMsg;
        strcpy(_nodeMsg->dstNodeId, dstNodeId);
        strcpy(_nodeMsg->srcNodeId, srcNodeId);
        _nodeMsg->protocol = protocol;
        _nodeMsg->payload.unreliableProtocolMsg = upm;

        *nodeMsg = _nodeMsg;
    }
    else {
        Logger.logInfo2(F("AbstractMessageTransport::deserialiseInterNodeMsg - received InterNodeMsg with unhandled Protocol [%d]"), protocol);
        return -1;
    }

    return 0;
}

// *****************************************************************************************
//int AbstractMessageTransport::logInterNodeMsg(const InterNodeMsg& nodeMsg)
//{
//    LOG_FUNCTION("AbstractMessageTransport::logInterNodeMsg")
//
//    int rc = 0;
//    
//   /* Serial.println("InterNodeMsg");
//    Serial.println((String(F("  version:      ["))+nodeMsg.version+"]").c_str());
//    Serial.println((String(F("  dstNodeId:    ["))+nodeMsg.dstNodeId+"]").c_str());
//    Serial.println((String(F("  srcNodeId:    ["))+nodeMsg.srcNodeId+"]").c_str());
//    Serial.println((String(F("  payload:      ["))+nodeMsg.payload+"]").c_str());
//    */
//    return rc;
//}
//

