
#include <MGLib.h>
#include <EEPROM.h>

void setup()
{
	Serial.begin(115200);
	Logger.init(&Serial, INFO);
	Logger.logInfo("Setup complete");
}

int loopCount = 0;

void loop()
{
	// prevent looping and wiping out out EPROM
	if (loopCount==0) {
		runTests();
		loopCount=1;
    }
}

void runTests()
{
	Logger.logInfo("Running tests");

	//testConfig();
	configureEEPROMInInteractiveMode();

	Logger.logInfo("Test complete");
}

int testEEPROMBlockReadWrite(char tag[EPROM_TAG_SIZE], char* bufferWrite, int bufferWriteLen) 
{
	int blockPos = 100;
	int blockLen;
	int writePos = writeBlockToEEPROM(tag, bufferWrite, bufferWriteLen, blockPos, blockLen);

	dumpBytesFromEEPROMToConsole(100, 16);

	/*
	char bufferRead[16];
	if (readBlockFromEEPROM(tag, bufferRead, 16)!=0) {
		Logger.logError("Error returned by readBlockFromEPROM");
		return -1;
	}

	Logger.logInfo("Buffer contents");
	Logger.logInfo((String("  len:") + strlen(&bufferRead[0])).c_str());
	Logger.logInfo((char*) (&bufferRead[0]));

	if (memcmp(bufferRead, bufferWrite, bufferWriteLen)) {
		Logger.logInfo("Buffer read matches buffer written");
		return 0;
	}
	else {
		Logger.logError("Buffer read does not matched buffer written");
		return -1;
	}
	*/
}


int testEEPROMReadWrite()
{
	char tag1[EPROM_TAG_SIZE] = { 'T','A','G','1' };
	
	char* buffer1 = "AAAAAAAAAAAAA";
	
	//char tag2[EPROM_TAG_SIZE] = { 'T','A','G','2' };
	//char* buffer2 = "GOODBYE";
	//char tag3[EPROM_TAG_SIZE] = { 'T','A','G','3' };
	//char* buffer3 = "AUREVOIS";

	testEEPROMBlockReadWrite(tag1, buffer1, strlen(buffer1)+1);
	//testEEPROMBlockReadWrite(tag2, buffer2, strlen(buffer2));
	//testEEPROMBlockReadWrite(tag3, buffer3, strlen(buffer3));

}

void printCommandHelp() {
	Serial.println(F("Commands are"));
	Serial.println(F("E:S,L,V = set EEPROM to value V starting at pos S and for len L"));
	Serial.println(F("R:T     = read block from EEPROM with tag T"));
	Serial.println(F("S:K,V   = set block item K to value V"));
	Serial.println(F("            Item Id		Item Name       Value"));
	Serial.println(F("            BT			BLOCK_TAG       char[3]"));
	Serial.println(F("            BD			BLOCK_DATA      string"));
	Serial.println(F("P       = print items"));
	Serial.println(F("W:P     = write block to EEPROM at position P (use X to autoselect next free position)"));
	Serial.println(F("D:S,L   = dump EEPROM memory from S for len L"));
	Serial.println(F("B:S     = dump all EEPROM blocks form pos S"));
	Serial.println(F("H		  = print this help text"));
	Serial.println(F("Q       = quit"));
}

void configureEEPROMInInteractiveMode() {
    String blockTag;
	String blockData;
	String lineBuffer;

	// allow user to modify config
	Serial.println(F("Config mode entered"));
	printCommandHelp();

	while (1 == 1) {
		if (readLineFromSerial(lineBuffer) == 1) {

			// ** DUMP **************************************************************	
			if (lineBuffer[0] == 'D') {
				String cmd = getField(&lineBuffer, ':');

				int startPos  = getField(&lineBuffer, ',').toInt();
				int numBytes  = getField(&lineBuffer, ',').toInt();

				String logMsg = F("Dumping EEPROM from pos ");
				logMsg += startPos;
				logMsg += " for numBytes ";
				logMsg += numBytes;
				Serial.println(logMsg);

				dumpBytesFromEEPROMToConsole(startPos, numBytes);

				lineBuffer = "";
			}

			// ** HELP ******************************************************	
			else if (lineBuffer[0] == 'H') {
				printCommandHelp();
				lineBuffer = "";
			}

			// ** DUMP BLOCKS ******************************************************	
			else if (lineBuffer[0] == 'B') {
				String cmd = getField(&lineBuffer, ':');
				int startPos = getField(&lineBuffer, ',').toInt();

				String logMsg = F("Dumping all EEPROM blocks from pos ");
				logMsg += startPos;
				Serial.println(logMsg);

				dumpBlocksToConsole(startPos);

				Serial.println("Done");
				lineBuffer = "";
			}

			// ** QUIT **************************************************************	
			else if (lineBuffer == "Q") {
				Serial.println(F("Exiting interactive mode"));
				
				lineBuffer = "";
			}

			// ** PRINT *************************************************************	
			else if (lineBuffer == "P") {
				Serial.println(F("Block items"));
								
				String logMsg = F("    tag = ");
				logMsg += blockTag;
				Serial.println(logMsg);

				logMsg = F("    data = ");
				logMsg += blockData;
				Serial.println(logMsg);

				lineBuffer = "";
			}

			// ** WRITE *************************************************************	
			else if (lineBuffer[0] == 'W') {
				String cmd    = getField(&lineBuffer, ':');
				String posStr = getField(&lineBuffer, ',');

				String logMsg = F("Writing block to EEPROM at location ");
			
				int blockStartPos;
				if (posStr=="X") { // autoselect position for block
					blockStartPos = -1;     
					logMsg += "<autoSelect>";
				} else {
					blockStartPos = posStr.toInt();
					logMsg += blockStartPos;
				}

				Serial.println(logMsg);
				
				int blockLen;
				int rc = writeBlockToEEPROM(blockTag.c_str(), blockData.c_str(), blockData.length(), blockStartPos, blockLen);
				
				if (rc<0) {
					Serial.println("Failed to write block");
				} else {
					Serial.println("Wrote block");
				}
				lineBuffer = "";
			}

			// ** ERASE *************************************************************	
			else if (lineBuffer[0] == 'E') {
				String cmd = getField(&lineBuffer, ':');
				int pos = getField(&lineBuffer, ',').toInt();
				int numBytes = getField(&lineBuffer, ',').toInt();
				String hexStr = getField(&lineBuffer, ',');
				int intToWrite = (int) strtol(hexStr.c_str(), NULL, 16);
				char charToWrite = (char) intToWrite;

				String logMsg = F("Writing to EEPROM at location ");
				logMsg += pos;
				logMsg += " for numBytes ";
				logMsg += numBytes;
				logMsg += " using value ";
				logMsg += intToWrite;
				Serial.println(logMsg);

				int writtenTo = -1;
				for (int i=0; i<numBytes; i++)
					writtenTo = writeBytesToEEPROM(pos+i, &charToWrite, 1, NULL);

				logMsg = F("Write completed");
				Serial.println(logMsg);

				lineBuffer = "";
			}

			// ** READ **************************************************************	
			else if (lineBuffer[0] == 'R') {
				String cmd = getField(&lineBuffer, ':');
				String tag = getField(&lineBuffer, ',');

				String logMsg = F("Reading block with tag ");
				logMsg += tag;
				Serial.println(logMsg);

				char buffer[64];
				int blockStartPos, blockLen;
				int numBytesRead;
				int rc = readBlockFromEEPROM(tag.c_str(), (char*) &buffer, 64, numBytesRead, blockStartPos, blockLen);

				logMsg = "Buffer contents [";
				for (int i=0; i<numBytesRead;i++)
					logMsg += buffer[i];
				logMsg += "]";
				Serial.println(logMsg);

				lineBuffer = "";
			}

			// ** SET ***************************************************************	
			else if (lineBuffer[0] == 'S') {
				String cmd = getField(&lineBuffer, ':');
				String key = getField(&lineBuffer, ',');
				String val = getField(&lineBuffer, ',');

				String logMsg = F("Setting item [");
				logMsg += key;
				logMsg += F("] to value [");
				logMsg += val;
				logMsg += F("]");
				Serial.println(logMsg);

				if (key == "BT") {
					blockTag = val;
				}
				else if (key == "BD") {
				    blockData = val;
				}
				else {
					Serial.println("Unknown key type {BT, BD}");
				}

				lineBuffer = "";
			}

			else {
				Serial.println("Unknown command");
				Serial.println(lineBuffer);
				lineBuffer = "";
			}
		}
	}

}


void testConfig()
{
	testEEPROMReadWrite();
}

