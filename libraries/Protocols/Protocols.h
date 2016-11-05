/****************************************************************
 *
 * See
 *    docs\protocols
 *
 ****************************************************************/
#ifndef _PROTOCOLS_h
#define _PROTOCOLS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define PROTOCOLS_FIELD_SEP ";"

// ***************************************************************
// SimpleSensorProtocol V1.0
// ***************************************************************

enum SSP_SensorType {
    TEMPERATURE = 1,
    HUMIDITY = 2,
    BUTTON = 3,
    SWITCH = 4,
    LIGHT = 5,
    PIR = 6,
    OTHER = 98,
    BATTERY = 99,
};

// SimpleSensorProtocol::SensorReading

#define SSP_SENSOR_READING_ID_SIZE 8
#define SSP_SENSOR_READING_VALUE_SIZE 16

struct SSP_SensorReading {
    char				id[SSP_SENSOR_READING_ID_SIZE];
    SSP_SensorType		type;
    char                value[SSP_SENSOR_READING_VALUE_SIZE];
    int					intervalInSecs;
};

int SSP_buildSensorReading(const char* sensorId, const SSP_SensorType& sensorType,
    const char* readingValue, const int intervalInSecs,
    SSP_SensorReading& sensorReading);

// SimpleSensorProtocol::Message

enum SSP_MsgType {
    SENSOR_READING = 1,
    //SENSOR_EVENT = 2,
    //SENSOR_NODE_ANNOUNCEMENT = 3,
    //HEARTBEAT = 4
};

union SSP_MsgPayload {
    struct SSP_SensorReading SSP_SensorReading;
};

#define SSP_MSG_MAX_MESSAGE_ID 256
#define SSP_MSG_VERSION 1

struct SSP_Msg {
    int			msgId;
    SSP_MsgType	msgType;
    union SSP_MsgPayload payload;
};

int SSP_serialiseSSPMsg(const SSP_Msg& msg, 
                        char* buffer, int bufferSize);

// ***************************************************************
// ApplicationMessaging
// ***************************************************************

enum AM_ApplicationMessageProtocol {
    SIMPLE_SENSOR_PROTOCOL = 1,
};

union AM_ApplicationMsgPayload {
    struct SSP_Msg SSP_Msg;
};

struct AM_ApplicationMsg {
    AM_ApplicationMessageProtocol protocol;
    union AM_ApplicationMsgPayload payload;
};

int AM_serialiseApplicationMsg(const AM_ApplicationMsg& msg, char* buffer, int bufferSize);



#endif 



