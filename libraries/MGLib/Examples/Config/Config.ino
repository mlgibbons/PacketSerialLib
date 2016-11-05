
#include "types.h"
#include <MGLib.h>
#include <EEPROM.h>

// Default config
Config config = { 100, 199, "AAA" };

#define CONFIG_TAG "ESWC"

void printConfigItemHelp() {
	Logger.logInfo2(F("RFM_NODE_ID       RFM_NODE_ID     int"));
	Logger.logInfo2(F("RFM_NETWORK_ID    RFM_NETWORK_ID  int"));
	Logger.logInfo2(F("NODE_ID           NODE_ID         char[%d]"), sizeof(config.nodeId));
}

void printConfig() {
	Logger.logInfo2(F("Current config"));
	Logger.logInfo2(F("  RFM_NODE_ID     = [%d]"), config.rfmNodeId);
	Logger.logInfo2(F("  RFM_NETWORK_ID  = [%d]"), config.rfmNetworkId);
	Logger.logInfo2(F("  NODE_ID         = [%s]"), config.nodeId);
}

void setConfigItem(const char* key, const char* val) {
	if (strcmp(key, "RFM_NODE_ID")==0) {
		config.rfmNodeId = atoi(val);
	}
	else if (strcmp(key, "RFM_NETWORK_ID")==0) {
		config.rfmNetworkId = atoi(val);
	}
	else if (strcmp(key, "NODE_ID") == 0) {
		int _len = strlcpy(config.nodeId, val, sizeof(config.nodeId));
	}
	else {
		Logger.logInfo2(F("Unknown key type"));
	}
}

void setup()
{
	Serial.begin(57600);
	Logger.init(&Serial, INFO);
	Logger.logInfo(String(F("Setup start")).c_str());
	initConfig(CONFIG_TAG, (unsigned char *) &config, sizeof(Config), printConfigItemHelp, printConfig, setConfigItem);
	Logger.logInfo(String(F("Setup complete")).c_str());
}

void loop()
{
	// do your stuff here
	Logger.logInfo("Looping");
	delay(1000);
}


