// 
// 
// 
//#define ENABLE_FUNCTION_LOGGING 1

#include "IOTNode.h"

#include <MGLib.h>
#include <EEPROM.h>

#include <Protocols.h>

// ------------------------------------------------------------------------
// DeviceNode
// ------------------------------------------------------------------------

DeviceNode::DeviceNode(const char* id, DeviceInfo* deviceInfo, int numDevices)
{
    LOG_FUNCTION("DeviceNode::DeviceNode")

    strlcpy(m_id, id, sizeof(m_id));

    m_numDevices = numDevices;
    for (int i=0; i<numDevices; i++) {
        m_devices[i] = deviceInfo[i]; 
    }

    setupDevices();
}    

int DeviceNode::setupDevices()
{
    LOG_FUNCTION("DeviceNode::setupDevices")
    
    for (int i=0; i < m_numDevices; i++) {
        int devType = m_devices[i].type;
      
        if (devType == DEVICE_INFO_TYPE_LED) {
            setupLEDDevice(&m_devices[i], false); 
        }

        else if (devType == DEVICE_INFO_TYPE_SWITCH) {
            setupSwitch(&m_devices[i]);
        }

        else if ((devType == DEVICE_INFO_TYPE_TEMP_DS18S20) ||
                 (devType == DEVICE_INFO_TYPE_TEMP_DS18B20) ||  
                 (devType == DEVICE_INFO_TYPE_TEMP_DS1822 )) {
            setupDallasTempSensor(&m_devices[i]);        
        }
    }
}

int DeviceNode::getDeviceList(DeviceInfo* deviceInfo, int* numDevices)
{
    LOG_FUNCTION("DeviceNode::getDeviceList")
    
    *numDevices = m_numDevices;
    for (int i=0; i<*numDevices; i++) {
        deviceInfo[i] = m_devices[i]; 
    }
    
    return 0;
}

void DeviceNode::logDeviceList()
{
  LOG_FUNCTION("DeviceNode::logDeviceList")

  DeviceInfo devices[DEVICE_NODE_MAX_DEVICES];
  int numDevices;
  
  getDeviceList(devices, &numDevices);

  for (int x=0; x<numDevices; x++) {
    Logger.logInfo2(F("Device [%d]"), x);
    Logger.logInfo2(F("------------------------"));
    Logger.logInfo2(F("   Id:%s"), devices[x].id);
    Logger.logInfo2(F("   Type:%d"), devices[x].type);
    Logger.logInfo2(F("   Pin:%d"), devices[x].pin);
  }

}
       
int DeviceNode::getDeviceState(const char* deviceId, char* deviceStateBuffer, int deviceStateBufferLen)
{
    LOG_FUNCTION("DeviceNode::getDeviceState")

    DeviceInfo* deviceInfo = NULL;
    findDeviceInfo(deviceId, &deviceInfo);
    
    if (deviceInfo==NULL) {
        return -1;
    } else {
        if (deviceInfo->type==DEVICE_INFO_TYPE_LED) {
            boolean ledState;
            getLEDState(deviceInfo, ledState);
            strlcpy(deviceStateBuffer, (ledState==true) ? "ON" : "OFF", deviceStateBufferLen);
        }

        else if (deviceInfo->type == DEVICE_INFO_TYPE_SWITCH) {
            boolean switchState;
            getSwitchState(deviceInfo, switchState);
            strlcpy(deviceStateBuffer, (switchState == true) ? "CLOSED" : "OPEN", deviceStateBufferLen);
        }

        else if ((deviceInfo->type == DEVICE_INFO_TYPE_TEMP_DS18B20) ||
                 (deviceInfo->type == DEVICE_INFO_TYPE_TEMP_DS1822)  ||
                 (deviceInfo->type == DEVICE_INFO_TYPE_TEMP_DS18S20))
        {
            float tempState;
            getDallasTempSensorTemp(deviceInfo, &tempState);

            // convert float to string - arduino hack
            char buffer[deviceStateBufferLen];
            dtostrf(tempState, 6, 2, buffer);
            char* startOfDigits = find_first_non_white_space(buffer);   // skip white space

            strlcpy(deviceStateBuffer, startOfDigits, deviceStateBufferLen);  
        }

        return 0;
    }
    
}
        
        
int DeviceNode::setDeviceState(const char* deviceId, const char* deviceState)
{
    LOG_FUNCTION("DeviceNode::setDeviceState")

    int rc = 0;
    
    DeviceInfo* deviceInfo = NULL;
    findDeviceInfo(deviceId, &deviceInfo);

    if (deviceInfo==NULL) {
        rc = -1;
    } else {
        if (deviceInfo->type==DEVICE_INFO_TYPE_LED) {
          
            if (strcmp("ON",deviceState)==0) {
              rc = setLEDState(deviceInfo, true);                                      
            } 
            else if (strcmp("OFF",deviceState)==0) {
              rc = setLEDState(deviceInfo, false);                                             
            } 
            else {
              rc = -1;
            }
        }
    }
            
    return rc;
}

void DeviceNode::findDeviceInfo(const char* deviceId, DeviceInfo** deviceInfo)
{
    LOG_FUNCTION("DeviceNode::findDeviceInfo")
    
    DeviceInfo* _devInfo = NULL;
    for (int i=0; i < m_numDevices; i++) {
        if (strcmp(m_devices[i].id, deviceId)==0) {
            _devInfo = &m_devices[i];
        } 
    }

    *deviceInfo = _devInfo;
}


// ------------------------------------------------------------------------
// LED functions
// ------------------------------------------------------------------------

int DeviceNode::setupLEDDevice(DeviceInfo* deviceInfo, boolean initialLEDState)
{
    LOG_FUNCTION("DeviceNode::setupLEDDevice")
    
    pinMode(deviceInfo->pin, OUTPUT);
    setLEDState(deviceInfo, initialLEDState);
}

int DeviceNode::getLEDState(DeviceInfo* deviceInfo, boolean& ledState)
{
    LOG_FUNCTION("DeviceNode::getLEDState")
    
    ledState = ( digitalRead(deviceInfo->pin) == HIGH ) ? true : false;

	return 0;
}
        
int DeviceNode::setLEDState(DeviceInfo* deviceInfo, boolean ledState)
{
    LOG_FUNCTION("DeviceNode::setLEDState")
    
    if (ledState==true) {
        digitalWrite(deviceInfo->pin, HIGH);
    } else {
        digitalWrite(deviceInfo->pin, LOW);
    }

	return 0;
}

// ------------------------------------------------------------------------
// Dallas temperature sensor functions
// ------------------------------------------------------------------------

int DeviceNode::setupDallasTempSensor(DeviceInfo* deviceInfo)
{
    LOG_FUNCTION("DeviceNode::setupDallasTempSensor")
    //:TODO:dummy
}

int DeviceNode::getDallasTempSensorTemp(DeviceInfo* deviceInfo, float* temperature)
{
    LOG_FUNCTION("DeviceNode::getDallasTempSensorTemp")
    //:TODO:
    *temperature = 21.5;  
}

// ------------------------------------------------------------------------
// Switch functions
// ------------------------------------------------------------------------

int DeviceNode::setupSwitch(DeviceInfo* deviceInfo)
{
	LOG_FUNCTION("DeviceNode::setupSwitch")

	pinMode(deviceInfo->pin, INPUT);
}

int DeviceNode::getSwitchState(DeviceInfo* deviceInfo, boolean& switchState)
{
	LOG_FUNCTION("DeviceNode::getSwitchState")

	switchState = (digitalRead(deviceInfo->pin) == HIGH) ? true : false;
	return 0;
}


// ------------------------------------------------------------------------
// SensorNode
// ------------------------------------------------------------------------

SensorNode::SensorNode(const char* id, DeviceInfo* deviceInfo, int numDevices, int interval) :
    DeviceNode(id, deviceInfo, numDevices)
{
    LOG_FUNCTION("SensorNode::SensorNode")

    m_interval = interval;
}

void SensorNode::logReading(const char* sensorId, int sensorType, const char* sensorReading)
{
    LOG_FUNCTION("SensorNode::logReading")

    Logger.logInfo2(F("%s:%d:%s"), sensorId, sensorType, sensorReading);
}


boolean SensorNode::isSensor(DeviceInfo& devInfo)
{
    boolean rc = false;

    if ((devInfo.type == DEVICE_INFO_TYPE_SWITCH) ||
        (devInfo.type == DEVICE_INFO_TYPE_TEMP_DS18B20) ||
        (devInfo.type == DEVICE_INFO_TYPE_TEMP_DS1822) ||
        (devInfo.type == DEVICE_INFO_TYPE_TEMP_DS18S20))
    {
        rc = true;
    }

    return rc;
}

void SensorNode::run()
{
    LOG_FUNCTION("SensorNode::run")
    while (true) {
        logMemInfo("SensorNode::run-start");

        Logger.logInfo2(F("SensorNode::run - reading sensors"));
        
        // iterate over each sensor
        for (int x = 0; x < m_numDevices; x++) 
        {
            DeviceInfo devInfo = m_devices[x];

            // if the device is a sensor log the reading
			if (isSensor(devInfo))
            {
                // get state
				char devState[16];
                int rc = getDeviceState(devInfo.id, devState, 16);

                if (rc != 0) {
                    //:TODO:String logMsg = F("Error reading device: ");
                    //logMsg += devInfo.id;
                    //'Logger.logError(logMsg.c_str());
                }
                else {
                    logReading(devInfo.id, devInfo.type, devState);
	            }
            }

        }

        logMemInfo("SensorNode::run-end");

        Logger.logInfo2(F("SensorNode::run - waiting for [%d] secs before next reading"), m_interval);

        sleep();
    }
}


void SensorNode::sleep()
{
    delay(m_interval * 1000);
}


// ------------------------------------------------------------------------
// ConnectedSensorNode
// ------------------------------------------------------------------------

ConnectedSensorNode::ConnectedSensorNode(const char* id, DeviceInfo* deviceInfo, int numDevices, 
                                         int interval, MessagingEndpoint* endPoint, const char* sensorNodeMgrId)
    :SensorNode(id, deviceInfo, numDevices, interval)
{
    LOG_FUNCTION("ConnectedSensorNode::ConnectedSensorNode")
    m_endPoint = endPoint;

    int len = strlcpy(m_sensorNodeMgrId, sensorNodeMgrId, sizeof(m_sensorNodeMgrId));
    if (len >= sizeof(m_sensorNodeMgrId)) {
        Logger.logInfo2(F("WARNING:Sensor node id truncated from [%s] to [%s]"), id, m_sensorNodeMgrId);
    }

    m_endPoint->bind(id);
    m_nextMsgId=1;
}

int ConnectedSensorNode::getNextSSPMsgId()
{
    int rc = m_nextMsgId++;
    if (m_nextMsgId >= SSP_MSG_MAX_MESSAGE_ID) {
        m_nextMsgId = 1;
    }
    return rc;
}

SSP_SensorType mapSensorTypeToSSPSensorType(int sensorType)
{
    SSP_SensorType sspSensorType;

    switch (sensorType) {
        case DEVICE_INFO_TYPE_TEMP_DS18S20:
        case DEVICE_INFO_TYPE_TEMP_DS18B20:
        case DEVICE_INFO_TYPE_TEMP_DS1822:
            sspSensorType = TEMPERATURE;
            break;

        case DEVICE_INFO_TYPE_SWITCH:
            sspSensorType = SWITCH;
            break;

        default:
            sspSensorType = OTHER;
    }

    return sspSensorType;
}

void ConnectedSensorNode::logReading(const char * sensorId, int sensorType, const char * sensorReading)
{
    LOG_FUNCTION("ConnectedSensorNode::logReading")

    // Build the sensor reading
    SSP_SensorReading ssp_sensorReading;
    SSP_SensorType sspSensorType = mapSensorTypeToSSPSensorType(sensorType);
    SSP_buildSensorReading(sensorId, sspSensorType, sensorReading, m_interval, ssp_sensorReading );

    // Build SimpleSensorProtocol message
    SSP_Msg sspMsg;
    sspMsg.msgId = getNextSSPMsgId();
    sspMsg.msgType = SENSOR_READING;
    sspMsg.payload.SSP_SensorReading = ssp_sensorReading;

    // Build ApplicatioMessage
    AM_ApplicationMsg amMsg = { SIMPLE_SENSOR_PROTOCOL, sspMsg };

    // serialise the message for sending
    char buffer[64];
    buffer[0] = '\0';
    int rc = AM_serialiseApplicationMsg(amMsg, buffer, sizeof(buffer));

    if (rc != 0) {
        Logger.logInfo2(F("ConnectedSensorNode::logReading - failed to serialise message for sending"));
        return;
    }

    Logger.logInfo2(F("ConnectedSensorNode::logReading - Sending msg [%s] to sensorNodeMgr [%s]"), buffer, m_sensorNodeMgrId);

    // send to sensor node manager
    m_endPoint->sendMsg(m_sensorNodeMgrId, buffer);

    Logger.logInfo2(F("ConnectedSensorNode::logReading - Done"));
}



