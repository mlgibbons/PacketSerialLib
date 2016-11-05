// Node.h

#ifndef _IOTNODE_h
#define _IOTNODE_h

#include "OneWire.h"
#include <DallasTemperature.h>

#include "InterNodeMessagingLib.h"

//#include "NodeUtils.h"

// ---------------------------------------------------------------------------
// DeviceNode
//      Base class which handles devices connected to it
//          - device setup
//          - device state reading and setting
// ---------------------------------------------------------------------------

#define DEVICE_INFO_ID_MAX_SIZE 8

#define DEVICE_INFO_TYPE_LED          3
#define DEVICE_INFO_TYPE_TEMP_DS18S20 4
#define DEVICE_INFO_TYPE_TEMP_DS18B20 5
#define DEVICE_INFO_TYPE_TEMP_DS1822  6
#define DEVICE_INFO_TYPE_SWITCH       7

typedef struct {
    char id[DEVICE_INFO_ID_MAX_SIZE];
    int type;
    int pin;
} DeviceInfo;


#define DEVICE_NODE_NODE_ID_MAX_SIZE 8
#define DEVICE_NODE_MAX_DEVICES 4

class DeviceNode
{
    public:
		DeviceNode(const char* id, DeviceInfo* deviceInfo, int numDevices);
    
        int getDeviceList(DeviceInfo* deviceInfo, int* numDevices);
        void logDeviceList();

        int getDeviceState(const char* deviceId, char* deviceStateBuffer, int deviceStateBufferLen);
        int setDeviceState(const char* deviceId, const char* deviceState);

		virtual void run() {};

    protected:
        char m_id[DEVICE_NODE_NODE_ID_MAX_SIZE];
        int m_numDevices;
        DeviceInfo m_devices[DEVICE_NODE_MAX_DEVICES];
        
        int setupDevices();
        
        void findDeviceInfo(const char* deviceId, DeviceInfo** deviceInfo);     

        // LED
        int setupLEDDevice(DeviceInfo* deviceInfo, boolean initialLEDState);
        int getLEDState(DeviceInfo* deviceInfo, boolean& ledState);
        int setLEDState(DeviceInfo* deviceInfo, boolean ledState);

        // TEMP
        int setupDallasTempSensor(DeviceInfo* deviceInfo);
        int getDallasTempSensorTemp(DeviceInfo* deviceInfo, float* temperature);

        // SWITCH
        int setupSwitch(DeviceInfo* deviceInfo);
        int getSwitchState(DeviceInfo* deviceInfo, boolean& switchState);        
};

// ---------------------------------------------------------------------------
// SensorNode
// A DeviceNode which periodically logs (using the Logger class) 
// the state of any connected sensors as well as outputing changes to 
// sensors such as a button press.
// 
// ---------------------------------------------------------------------------
class SensorNode : public DeviceNode
{
public:
    SensorNode(const char* id, DeviceInfo* deviceInfo, int numDevices, int interval);

    void run();

    virtual void sleep();

protected:
    int m_interval;

    boolean isSensor(DeviceInfo& devInfo);
    virtual void logReading(const char* sensorId, int sensorType, const char* sensorReading);
};


// ----------------------------------------------------------------------------
// ConnectedSensorNode
// Sends sensor readings to a SensorNodeManager using the SimpleSensorProtocol
// and a MessagingEndpoint 
// ----------------------------------------------------------------------------

class ConnectedSensorNode : public SensorNode
{
public:
    ConnectedSensorNode(const char* id, DeviceInfo* deviceInfo, int numDevices, int interval,
                        MessagingEndpoint* endPoint, const char* sensorNodeManagerId);

protected:
    MessagingEndpoint* m_endPoint;
    char m_sensorNodeMgrId[4];
    int m_nextMsgId;

    void logReading(const char* sensorId, int sensorType, const char* sensorReading);

    int getNextSSPMsgId();
};



#endif

