
#define ENABLE_FUNCTION_LOGGING 1

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

// == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == =
// Elements required for the configuration from EEPROM
// We store the config for the node id and the three devices attached (button, led, temp)

Config config = { "TEST", 7, 6 };
#define CONFIG_TAG "IOTB"

void printConfigItemHelp()
{
	Logger.logInfo(F("NODE_ID - NODE_ID - char[4]"));
	Logger.logInfo(F("TEMP_PIN - TEMP_PIN - int"));
	Logger.logInfo(F("SWITCH_PIN - SWITCH_PIN - int"));
}

void printConfig()
{
	Logger.logInfo(F("Current config"));
	Logger.logInfo((String(F(" nodeId = [")) + config.id + F("]")).c_str());
	Logger.logInfo((String(F(" tempPin = [")) + config.tempPin + F("]")).c_str());
	Logger.logInfo((String(F(" switchPin = [")) + config.switchPin + F("]")).c_str());
}

void setConfigItem(String key, String val)
{
	if (key == "NODE_ID") {
		strcpy(config.id, val.c_str());
	}
	else if (key == "TEMP_PIN") {
		config.tempPin = val.toInt();
	}
	else if (key == "SWITCH_PIN") {
		config.tempPin = val.toInt();
	}
	else {
		Serial.println("Unknown key type");
	}
}

// == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == =
void setup()
{
    LOG_FUNCTION("setup")

    Serial.begin(115200);
    Logger.init(&Serial, INFO);

    Logger.logInfo(F("Setup start"));
    Logger.logInfo(F(__FILE__));

    // Load config
    initConfig(CONFIG_TAG, (unsigned char *)&config, sizeof(Config), printConfigItemHelp, printConfig, setConfigItem);

    // Define devices
    DeviceInfo deviceInfo[2] = { {String("TEMP1"), DEVICE_INFO_TYPE_TEMP_DS18B20, config.tempPin },
                                 {String("SWITCH1"), DEVICE_INFO_TYPE_SWITCH, config.switchPin } };

    // Create node
    node = new SensorNode(config.id, deviceInfo, 2, 1);

    node->logDeviceList();

    Logger.logInfo(F("Setup complete"));
}

void loop()
{
	LOG_FUNCTION("loop")

	node->run();
}


