
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

// =====================================================================================
// Example of a simple node which will 
//		- loop every 2 seconds
//		- flash any leds on and off
//		- print out the reading for any temperature sensors
//		- print out the reading for any switches
// =====================================================================================

DeviceNode* node = NULL;

class SimpleNode : public DeviceNode
{
public:
	SimpleNode(const char* id, DeviceInfo* deviceInfo, int numDevices) :
		DeviceNode(id, deviceInfo, numDevices)
	{
	}

	void run() 
	{
		Logger.setLevel(DEBUG);

		// do your stuff here
		Logger.logInfo(F("Looping"));

		// --------------------------------------------
		// Blink the LED
		// --------------------------------------------

		Logger.logInfo(F("LED to ON"));

		if (setDeviceState("LED1", "ON") != 0) {
			Logger.logInfo(F("Error setting device state to ON"));
		}
		delay(200);

		Logger.logInfo(F("LED to OFF"));

		if (setDeviceState("LED1", "OFF") != 0) {
			Logger.logInfo(F("Error setting device state to OFF"));
		}
		delay(200);

	}

};

// == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == =
// Elements required for the configuration from EEPROM
// We store the config for the node id and the three devices attached (button, led, temp)

Config config = { "TEST", 13, 7, 6 };
#define CONFIG_TAG "IOTB"

void printConfigItemHelp()
{
	Logger.logInfo(F("NODE_ID - NODE_ID - char[4]"));
	Logger.logInfo(F("LED_PIN - LED_PIN - int"));
	Logger.logInfo(F("TEMP_PIN - TEMP_PIN - int"));
	Logger.logInfo(F("SWITCH_PIN - SWITCH_PIN - int"));
}

void printConfig()
{
	Logger.logInfo(F("Current config"));
	Logger.logInfo((String(F(" nodeId = [")) + config.id + F("]")).c_str());
	Logger.logInfo((String(F(" ledPin = [")) + config.ledPin + F("]")).c_str());
	Logger.logInfo((String(F(" tempPin = [")) + config.tempPin + F("]")).c_str());
	Logger.logInfo((String(F(" switchPin = [")) + config.switchPin + F("]")).c_str());
}

void setConfigItem(String key, String val)
{
	if (key == "NODE_ID") {
		strcpy(config.id, val.c_str());
	}
	else if (key == "LED_PIN") {
		config.ledPin = val.toInt();
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

    // ------------------------------------------------------------------------------------------------
    // Load config from EEPROM and allow user to change  
    // ------------------------------------------------------------------------------------------------
    initConfig(CONFIG_TAG, (unsigned char *)&config, sizeof(Config), printConfigItemHelp, printConfig, setConfigItem);

    // ------------------------------------------------------------------------------------------------
    // Setup the devices and node and display device list
    // ------------------------------------------------------------------------------------------------
    DeviceInfo deviceInfo[3] = { {String("LED1"), DEVICE_INFO_TYPE_LED, config.ledPin },
                                 {String("TEMP1"), DEVICE_INFO_TYPE_TEMP_DS18B20, config.tempPin },
                                 {String("SWITCH1"), DEVICE_INFO_TYPE_SWITCH, config.switchPin } };

    node = new SimpleNode(config.id, deviceInfo, 3);

    node->logDeviceList();

    Logger.logInfo(F("Setup complete"));
}

void loop()
{
	LOG_FUNCTION("loop")

	node->run();
}


