#include "Protocols.h"

int AM_serialiseApplicationMsg(const AM_ApplicationMsg& msg, char* buffer, int bufferSize)
{
    int _len;    
    char _tbuffer[8];

    // msg.protocol
    itoa(msg.protocol, _tbuffer, 10);
    _len = strlcat(buffer, _tbuffer, bufferSize);
    if (_len >= bufferSize) return -1;

    int rc = 0;

    _len = strlcat(buffer, PROTOCOLS_FIELD_SEP, bufferSize);
    if (_len >= bufferSize) return -1;

    // SIMPLE_SENSOR_PROTOCOL
    if (msg.protocol == SIMPLE_SENSOR_PROTOCOL) {
        rc = SSP_serialiseSSPMsg(msg.payload.SSP_Msg, buffer, bufferSize);
    } 
    else {
        rc = -1;
    }

    return rc;
}

int SSP_buildSensorReading(const char* sensorId, 
                           const SSP_SensorType& sensorType,
                           const char* readingValue, 
                           const int intervalInSecs,
                           SSP_SensorReading& sensorReading)
{ 
    int _len;

    _len = strlcpy(sensorReading.id, sensorId, sizeof(sensorReading.id));
    if (_len >= sizeof(sensorReading.id)) return -1;

    sensorReading.type = sensorType;

    _len = strlcpy(sensorReading.value, readingValue, sizeof(sensorReading.value));
    if (_len >= sizeof(sensorReading.value)) return -1;

    sensorReading.intervalInSecs = intervalInSecs;

    return 0;
}

int SSP_serialiseSSPMsg(const SSP_Msg& msg,
                        char* buffer, int bufferSize)

{
    int _len;
    char _tbuffer[16];

    // msg.version
    itoa(SSP_MSG_VERSION, _tbuffer, 10);
    _len = strlcat(buffer, _tbuffer, bufferSize);
    if (_len >= bufferSize) return -1;

    _len = strlcat(buffer, PROTOCOLS_FIELD_SEP, bufferSize);
    if (_len >= bufferSize) return -1;

    // msg.msgId
    itoa(msg.msgId, _tbuffer, 10);
    _len = strlcat(buffer, _tbuffer, bufferSize);
    if (_len >= bufferSize) return -1;

    _len = strlcat(buffer, PROTOCOLS_FIELD_SEP, bufferSize);
    if (_len >= bufferSize) return -1;

    // msg.msgType
    itoa(msg.msgType, _tbuffer, 10);
    _len = strlcat(buffer, _tbuffer, bufferSize);
    if (_len >= bufferSize) return -1;

    _len = strlcat(buffer, PROTOCOLS_FIELD_SEP, bufferSize);
    if (_len >= bufferSize) return -1;

    int rc = 0;

    if (msg.msgType == SENSOR_READING) {
        // msg.payload.SSP_SensorReading.id
        _len = strlcat(buffer, msg.payload.SSP_SensorReading.id, bufferSize);
        if (_len >= bufferSize) return -1;

        _len = strlcat(buffer, PROTOCOLS_FIELD_SEP, bufferSize);
        if (_len >= bufferSize) return -1;

        // msg.payload.SSP_SensorReading.type;
        itoa(msg.payload.SSP_SensorReading.type, _tbuffer, 10);
        _len = strlcat(buffer, _tbuffer, bufferSize);
        if (_len >= bufferSize) return -1;

        _len = strlcat(buffer, PROTOCOLS_FIELD_SEP, bufferSize);
        if (_len >= bufferSize) return -1;

        // msg.payload.SSP_SensorReading.value;
        _len = strlcat(buffer, msg.payload.SSP_SensorReading.value, bufferSize);
        if (_len >= bufferSize) return -1;

        _len = strlcat(buffer, PROTOCOLS_FIELD_SEP, bufferSize);
        if (_len >= bufferSize) return -1;

        // msg.payload.SSP_SensorReading.intervalInSecs;
        itoa(msg.payload.SSP_SensorReading.intervalInSecs, _tbuffer, 10);
        _len = strlcat(buffer, _tbuffer, bufferSize);
        if (_len >= bufferSize) return -1;
    } 
    else {
        rc = -1;
    }

    return rc;
}


