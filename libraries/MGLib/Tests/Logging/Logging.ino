#include <MGLib.h>
#include <EEPROM.h>

void setup()
{
	Serial.begin(115200);
	Logger.init(&Serial, INFO);
}

void loop()
{
	runTests();
}

void runTests()
{
	testLogging();
}

void testLogging()
{
	Logger.setLevel(INFO);
	Logger.log(INFO, "Start-Logging test");

	Logger.log(INFO, "This is an INFO test message.");
	Logger.log(INFO, "The next message is a debug msg and should not be seen.");
	Logger.log(DEBUG, "This is a debug message - YOU SHOULD NOT SEE IT");
	Logger.log(INFO, "Did you see the debug message?");

	Logger.log(INFO, "Switching to debug level. You should now see a debug message.");

	Logger.setLevel(DEBUG);
	Logger.log(DEBUG, "This is a debug message - YOU SHOULD SEE IT");

	Logger.setLevel(INFO);
	Logger.log(INFO, "End-Logging test");
}

