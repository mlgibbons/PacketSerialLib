
//#define ENABLE_FUNCTION_LOGGING 1

// IOTNode.h includes
#include "IOTNode.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "InterNodeMessagingLib.h"
#include "RFM12B.h"

// MGLib.h includes
#include <MGLib.h>
#include <EEPROM.h>

#include "types.h"

DeviceNode* node = NULL;

// ========================================================================================================
// Elements required for the configuration from EEPROM

typedef struct {
    char id[5];
    byte rfmNodeId;
    byte rfmNetworkId;
    byte rfmGatewayId;
    byte tempPin;
    byte switchPin;
    int  interval;
    char dstNodeId[5];
} Config;

Config config;

#define CONFIG_TAG "CSN-"

void initDefaultConfig()
{
    strlcpy(config.id, "AAA", sizeof(config.id));
    config.rfmNodeId = 99;
    config.rfmNetworkId = 201;
    config.rfmGatewayId = 1;
    config.tempPin = 3;
    config.switchPin = 4;
    config.interval = 5;
    strlcpy(config.dstNodeId, "SNM", sizeof(config.dstNodeId));
}

void printConfig()
{
	Logger.logInfo2(F("Current config"));
    Logger.logInfo2(F(" NODE_ID  (char[4]) = [%s]"), config.id);
    Logger.logInfo2(F(" TEMP_PIN (int) = [%d]"), config.tempPin);
    Logger.logInfo2(F(" SWITCH_PIN (int)= [%d]"), config.switchPin);
    Logger.logInfo2(F(" RFM_NODE_ID (int)= [%d]"), config.rfmNodeId);
    Logger.logInfo2(F(" RFM_NETWORK_ID (int)= [%d]"), config.rfmNetworkId);
    Logger.logInfo2(F(" RFM_GATEWAY_ID (int)= [%d]"), config.rfmGatewayId);
    Logger.logInfo2(F(" SAMPLE_INTERVAL (int)= [%d]"), config.interval);
    Logger.logInfo2(F(" DST_NODE_ID (char[4])= [%s]"), config.dstNodeId);
}

void setConfigItem(const char* key, const char* val)
{
	if (strcmp(key, "NODE_ID")==0) {
		strlcpy(config.id, val, sizeof(config.id));
	}
    else if (strcmp(key,"DST_NODE_ID") == 0) {
        strlcpy(config.dstNodeId, val, sizeof(config.dstNodeId));
    }
    else if (strcmp(key, "TEMP_PIN") == 0) {
		config.tempPin = atoi(val);
	}
    else if (strcmp(key, "SWITCH_PIN") == 0) {
        config.switchPin = atoi(val);
    }
    else if (strcmp(key, "RFM_NODE_ID") == 0) {
        config.rfmNodeId = atoi(val);
    }
    else if (strcmp(key, "RFM_NETWORK_ID") == 0) {
        config.rfmNetworkId = atoi(val);
    }
    else if (strcmp(key, "RFM_GATEWAY_ID") == 0) {
        config.rfmGatewayId = atoi(val);
    }
    else if (strcmp(key, "SAMPLE_INTERVAL") == 0) {
        config.interval = atoi(val);
    }
    else {
		Logger.logError(F("Unknown key type"));
	}
}

// ========================================================================================================
void setup()
{
    LOG_FUNCTION("setup")

    Serial.begin(115200);
    Logger.init(&Serial, INFO);
    logMemInfo("setup start");
    Logger.logInfo(F("Setup start"));
    Logger.logInfo(F(__FILE__));

    initDefaultConfig();

    // Load config from EEPROM if possible and allow user to change  
    initConfig(CONFIG_TAG, (unsigned char *)&config, sizeof(Config), NULL, printConfig, setConfigItem);

    // Display the config
    printConfig();

    // Setup the devices
    DeviceInfo deviceInfo[2] = { {"TEMP1", DEVICE_INFO_TYPE_TEMP_DS18B20, config.tempPin },
                                 {"SW1", DEVICE_INFO_TYPE_SWITCH, config.switchPin } };

    // Create messaging endpoint for use by the app and pass in the transport to use
    AbstractMessageTransport* msgTransport = new RFMTransport(config.rfmNodeId, config.rfmNetworkId, config.rfmGatewayId);
    MessagingEndpoint* msgEndPoint = new MessagingEndpoint(msgTransport);

    // Create the node
    node = new ConnectedSensorNode(config.id, deviceInfo, 2, config.interval, msgEndPoint, config.dstNodeId);

    // Display the devices
    node->logDeviceList();

    logMemInfo("setup end");
    Logger.logInfo2(F("Setup complete"));
}

void loop()
{
	LOG_FUNCTION("loop")

    node->run();
}


