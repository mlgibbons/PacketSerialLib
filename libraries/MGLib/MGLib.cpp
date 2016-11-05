// 
// 
// 

#include "MGLib.h"

// Arduino libraries referenced by this library - need to include these in your sketch too 
#include <EEPROM.h>

#include <stdio.h>
#include <stdarg.h>

void LoggerClass::init(Stream* stream, int level)
{
	m_stream = stream;	
	m_level = level;
}

void LoggerClass::setBufferSize(int size)
{
    m_bufferSize = size;
}

void LoggerClass::log(int level, const char* msg)
{
	if ( (m_stream==NULL) || (level<m_level) ) {
		return;
	}
		
	m_stream->println(msg);
	m_stream->flush();
}

void LoggerClass::log(int level, const __FlashStringHelper * fsh)
{
    if ( (m_stream==NULL) || (level<m_level) ) {
        return;
    }
    
    m_stream->println(fsh);
	m_stream->flush();
}


void LoggerClass::logDebug(const char* msg)
{
	int level = DEBUG;
	log(level, msg);
}

void LoggerClass::logInfo(const char* msg)
{
	int level = INFO;
	log(level, msg);
}

void LoggerClass::logInfo2(const __FlashStringHelper * fmt, ...)
{
    char msgBuffer[m_bufferSize];
    va_list args;
    va_start(args, fmt);
#ifdef __AVR__
    vsnprintf_P(msgBuffer, sizeof(msgBuffer), (const char *)fmt, args); // progmem for AVR
#else
    vsnprintf(msgBuffer, sizeof(msgBuffer), (const char *)fmt, args); // for the rest of the world
#endif
    va_end(args);
    log(INFO, msgBuffer);
}

void LoggerClass::logError(const char* msg)
{
	int level = ERROR;
	log(level, msg);
}

void LoggerClass::logDebug(const __FlashStringHelper * fsh)
{
    int level = DEBUG;
    log(level, fsh);
}

void LoggerClass::logInfo(const __FlashStringHelper * fsh)
{
    int level = INFO;
    log(level, fsh);
}

void LoggerClass::logError(const __FlashStringHelper * fsh)
{
    int level = ERROR;
    log(level, fsh);
}


void LoggerClass::setLevel(int level)
{
	m_level = level;
}

LoggerClass Logger;

void LoggerClass::flush() {
	m_stream->flush();
}

LEDClass::LEDClass(int pin, int blinkPeriod)
{
	m_pin = pin;
	m_blinkPeriod = blinkPeriod;
	pinMode(m_pin, OUTPUT);
}


void LEDClass::blink()
{
	int oldState = digitalRead(m_pin);
	int newState = oldState==LOW ? HIGH : LOW;
	digitalWrite(m_pin, newState);
	delay(m_blinkPeriod);
	digitalWrite(m_pin, oldState);
}

LEDClass LEDPin13(13, 20);

BluetoothJoypadClass::BluetoothJoypadClass(Stream* serialPort)
{
	m_bluetoothPort = serialPort;
	m_lastDirection = CENTRE;
	m_lastDirectionTime = millis();
}

void BluetoothJoypadClass::init(Stream* serialPort, float throttleScaling, float directionScaling)
{
	m_bluetoothPort = serialPort;
	m_throttleScaling = throttleScaling;
	m_directionScaling = directionScaling;
}

BluetoothJoypadClass::Direction BluetoothJoypadClass::getPadDirection()
{
	// Command format is #b=1#
	// If two buttons are pressed then format is has both keys #b=13#
	// none =  0
	// fwd   = 1
	// bck   = 2
	// left  = 3
	// right = 4
	
	if (m_bluetoothPort==NULL) {
		return CENTRE;
	}
	
	Direction direction = NONE; 
	
	// read the joypad position
	if (m_bluetoothPort->available()) {
		
		// loop until we've read the command
		while (direction==NONE) {
			
			// create a zero terminated buffer
			// max packet size is 2 bytes
			char command[3];
			command[0] = NULL;
			command[1] = NULL;
			command[2] = NULL;
		
			// find start of packet
			m_bluetoothPort->find("#b=");
		
			// read packet contents
			int numBytesRead = m_bluetoothPort->readBytesUntil('#', command, 6);
		
			// if we read anything then convert to a direction
			if (numBytesRead>0) {
				String directionStr;
				directionStr += command;

				Logger.log(DEBUG, (String(F("Read from Bluetooth - ")) + directionStr).c_str());
			
				// map string to enum
				if (directionStr=="0")   direction=CENTRE;
				if (directionStr=="1")   direction=UP;
				if (directionStr=="2")   direction=DOWN;
				if (directionStr=="3")   direction=LEFT;
				if (directionStr=="4")   direction=RIGHT;
				if (directionStr=="13")  direction=UP_LEFT;
				if (directionStr=="14")  direction=UP_RIGHT;
				if (directionStr=="23")  direction=DOWN_LEFT;
				if (directionStr=="24")  direction=DOWN_RIGHT;			
			
				//LEDPin13.blink();
			}	
		}
	}
	
	if (direction!=BluetoothJoypadClass::NONE) {
		m_lastDirection = direction;
		m_lastDirectionTime = millis();
	}
	
	// no direction so use current one unless it has timed out
	// in which case just return to CENTRE
	else {
		unsigned long currTime = millis();
		unsigned long timeDiff = currTime - m_lastDirectionTime;
		
		if (timeDiff > 500) {
			m_lastDirection = BluetoothJoypadClass::CENTRE;
			m_lastDirectionTime = millis();
			Logger.log(INFO, String(F("Timeout for last command exceeded. Resetting to CENTRE")).c_str());
		}
	}

	return m_lastDirection;
}

JoystickValues BluetoothJoypadClass::getStickPosition()
{
	// Maps direction to analogue values equivalent to a stick
	Direction currDirection = getPadDirection();
	
	long throttle = 0;
	long direction = 0;
	
	int throttleMax = 100 * m_throttleScaling;
	int directionMax = 100 * m_directionScaling;

	switch (currDirection) {
		case CENTRE:
			break;
		case UP:
			throttle   = +throttleMax;
			break;
		case DOWN:
			throttle   = -throttleMax;
			break;
		case LEFT:
			direction  = -directionMax;
			break;
		case RIGHT:
			direction  = +directionMax;
			break;
			
		case UP_LEFT:
			throttle   = +throttleMax;
			direction  = -directionMax;
			break;
			
		case UP_RIGHT:
			throttle   = +throttleMax;
			direction  = +directionMax;
			break;
		case DOWN_LEFT:
			throttle   = -throttleMax;
			direction  = -directionMax;
			break;
		case DOWN_RIGHT:
			throttle   = -throttleMax;
			direction  = +directionMax;
			break;
	}
	
	Logger.log(DEBUG, (String(F("throttle")) + throttle + F(" direction") + direction).c_str());
				
	JoystickValues jv = {throttle, direction};
	return jv;
}

BluetoothJoypadClass BluetoothJoypad;



BluetoothJoystickClass::BluetoothJoystickClass(Stream* serialPort)
{
	m_bluetoothPort = serialPort;
	m_lastStickPosition = { 0, 0 };
	m_lastStickPositionTime = millis();
}

void BluetoothJoystickClass::init(Stream* serialPort)
{
	m_bluetoothPort = serialPort;
}

JoystickValues BluetoothJoystickClass::getStickPosition()
{
	if (m_bluetoothPort==NULL) {
		return  { 0, 0 };
	}
	
	JoystickValues jv = { 0, 0 };
	int jvUpdated = 0;
	
	#define    STX          0x02
	#define    ETX          0x03

	byte cmd[8] = {0, 0, 0, 0, 0, 0, 0, 0};    // bytes received

	// read update 
	if (m_bluetoothPort->available())  {		                           
		delay(2);
		
		cmd[0] =  m_bluetoothPort->read();
		
		if(cmd[0] == STX)  {
			int i=1;
			
			// read the full packet
			while (m_bluetoothPort->available())  {
				delay(1);
				cmd[i] = m_bluetoothPort->read();
				
				if(cmd[i]>127 || i>7)                 break;     // Communication error
				if((cmd[i]==ETX) && (i==2 || i==7))   break;     // Button or Joystick data end of packet marker
				
				i++;
			}
			
			// button update
			if (i==2)          
			{
				// 3 Bytes  ex: < STX "C" ETX >
			}
				    
			// stick update
			else if(i==7) 
			{
				int joyX = (cmd[1]-48)*100 + (cmd[2]-48)*10 + (cmd[3]-48);  // obtain the Int from the ASCII representation
				int joyY = (cmd[4]-48)*100 + (cmd[5]-48)*10 + (cmd[6]-48);
				
				joyX = joyX - 200;    // Offset to avoid
				joyY = joyY - 200;    // transmitting negative numbers

				if (joyX<-100 || joyX>100 || joyY<-100 || joyY>100)  {
					// commmunication error
					Logger.log(ERROR, "Communication error - joystick values out of range");
				}
				
				else {
					Logger.log(DEBUG, "Joystick updated");
					jv = { joyY, joyX };
					jvUpdated = 1;
				}
			}
			
		}
	}
	
	if (jvUpdated==1) {
		m_lastStickPosition = jv;
		m_lastStickPositionTime = millis();
	}
	
	// no direction so use current one unless it has timed out
	// in which case just return to centre
	else {
		unsigned long currTime = millis();
		unsigned long timeDiff = currTime - m_lastStickPositionTime;
		
		if (timeDiff > 5000) {
			m_lastStickPosition = { 0, 0 };
			m_lastStickPositionTime = millis();
			Logger.log(INFO, "Timeout for last command exceeded. Resetting to centre");
		}
	}
		
	return m_lastStickPosition;	
}

BluetoothJoystickClass BluetoothJoystick;




/* This function places the current value of the heap and stack pointers in the
 * variables. You can call it from any place in your code and save the data for
 * outputting or displaying later. This allows you to check at different parts of
 * your program flow.
 * The stack pointer starts at the top of RAM and grows downwards. The heap pointer
 * starts just above the static variables etc. and grows upwards. SP should always
 * be larger than HP or you'll be in big trouble! The smaller the gap, the more
 * careful you need to be. Julian Gall 6-Feb-2009.
 */
uint8_t * heapptr, * stackptr;  // I declared these globally

void check_mem() {
  //uint8_t * heapptr, * stackptr;  // I declared these globally
  stackptr = (uint8_t *)malloc(4);  // use stackptr temporarily
  heapptr = stackptr;                  // save value of heap pointer
  free(stackptr);                        // free up the memory again (sets stackptr to 0)
  stackptr =  (uint8_t *)(SP);       // save value of stack pointer
  }

void format_mem_info(char* buffer, const char* label) {
  check_mem();
  sprintf(buffer, "label:%s, heap:%d, stack:%d", label, heapptr, stackptr);  
}

void logMemInfo(const char* label) {
    check_mem();
    char b[64];
    format_mem_info(b, label);
    Logger.logInfo(b);
}

void sprintf_vargs (char* buffer, int bufferlen, char * format, ...)
{
    va_list args;
    va_start (args, format);
    vsnprintf (buffer, bufferlen-1, format, args);
    va_end (args);
}


String getField(String* msg, char fieldSep) 
{
    String result = "";
    int delimPos = msg->indexOf(fieldSep);
      
    // if no delim found then scan to end of the string 
    if (delimPos != -1) {
        result = msg->substring(0, delimPos);
        *msg = msg->substring(delimPos+1);
    } else {
        result = msg->substring(0, msg->length());
        *msg = "";
    }
      
    return result;
}  

String getFieldOfSize(String* msg, int numChars)
{
    String result = "";

    result = msg->substring(0, numChars);
    
    *msg = msg->substring(numChars);
    
    return result;     
}


// *********************************************************************************************
//
// *********************************************************************************************
int readLineFromSerial(int readch, char*buffer, int bufferLen)
{
    static int pos = 0;
    int rpos;

    if (readch > 0) {
        switch (readch) {
        case '\n': // Ignore new-lines
            break;
        case '\r': // Return on CR
            rpos = pos;
            pos = 0;  // Reset position index ready for next time
            return rpos;
        default:
            if (pos < bufferLen - 1) {
                buffer[pos++] = readch;
                buffer[pos] = 0;
            }
        }
    }
    // No end of line has been found, so return -1.
    return -1;
}




// **********************************************************************************8*
// crc8.c
// Computes a 8-bit CRC
// 
// **********************************************************************************8*

#define DUMMY_CRC 1

#ifdef DUMMY_CRC

void crc8(unsigned char *crc, unsigned char m)
{
    *crc = 'X';
}

void crc8_buffer(unsigned char *crc, const unsigned char *buffer, int bufferLen)
{
    *crc = 'X';
}


#else

#define GP  0x107   /* x^8 + x^2 + x + 1 */
#define DI  0x07

static unsigned char crc8_table[256];     /* 8-bit table */
static int made_table = 0;

static void init_crc8()
/*
* Should be called before any other crc function.
*/
{
	int i, j;
	unsigned char crc;

	if (!made_table) {
		for (i = 0; i<256; i++) {
			crc = i;
			for (j = 0; j<8; j++)
				crc = (crc << 1) ^ ((crc & 0x80) ? DI : 0);
			crc8_table[i] = crc & 0xFF;
			/* printf("table[%d] = %d (0x%X)\n", i, crc, crc); */
		}
		made_table = 1;
	}
}

void crc8(unsigned char *crc, unsigned char m)
/*
* For a byte array whose accumulated crc value is stored in *crc, computes
* resultant crc obtained by appending m to the byte array
*/
{
	if (!made_table)
		init_crc8();

	*crc = crc8_table[(*crc) ^ m];
	*crc &= 0xFF;
}

void crc8_buffer(unsigned char *crc, const unsigned char *buffer, int bufferLen)
{
	for (int i = 0; i < bufferLen; i++) {
		crc8(crc, buffer[i]);
	}
}

#endif

// *********************************************************************************************
// 
// Config
//
//	EPROM format is a collection of blocks starting at EPROM_CONFIG_START
//  
//  Each block starts with a magic string "MGGG"
//  Next comes a block tag of 4 characters e.g. "MBT1"
//  Then a byte for the block length
//  Then the actual data 
//  Then a checksum
//
// *********************************************************************************************

#define EPROM_BLOCK_START_MAGIC_STRING "MGGG"
#define EPROM_BLOCK_START_MAGIC_STRING_LEN 4
#define EPROM_CONFIG_START 0
#define EPROM_CONFIG_END   1024


boolean atBlockStart(int location) {
	boolean rc = false;

	if (EEPROM.read(location + 0) == EPROM_BLOCK_START_MAGIC_STRING[0] &&
		EEPROM.read(location + 1) == EPROM_BLOCK_START_MAGIC_STRING[1] &&
		EEPROM.read(location + 2) == EPROM_BLOCK_START_MAGIC_STRING[2] &&
		EEPROM.read(location + 3) == EPROM_BLOCK_START_MAGIC_STRING[3]) 
	{
		rc = true;
	}

	return rc;
}

boolean checkBlockTagMatches(int location, const char* tag) 
{
	boolean rc = false;

	if (EEPROM.read(location + 0) == tag[0] &&
		EEPROM.read(location + 1) == tag[1] &&
		EEPROM.read(location + 2) == tag[2] &&
		EEPROM.read(location + 3) == tag[3])
	{
		rc = true;
	}

	return rc;
}

int locateBlock(const char* tag, int startPos=0)
{
	int rc;
	boolean blockFound = false;
	int blockLocation = 0;

	int currLocation = startPos;

	while ((blockFound!=true) && (currLocation<EPROM_CONFIG_END)) {		
		
		// are we at the start of a block
		if (atBlockStart(currLocation) == true) {
			
			// check block tag matches if one was passed
			if (tag!=NULL) {
				int tagPos = currLocation + EPROM_BLOCK_START_MAGIC_STRING_LEN;		
				if (checkBlockTagMatches(tagPos, tag) == true) {
					blockFound = true;
				}
			}
			else {
				blockFound = true;
			}

			if (blockFound==true) {
				blockLocation = currLocation;
				break;
			}
		}
		
		currLocation++;		
	}	

	rc = blockFound ? blockLocation: -1;

	return rc;
}

int writeBytesToEEPROM(int location, const unsigned char* buffer, int bufferLen, unsigned char* crc)
{
	for (unsigned int t = 0; t<bufferLen; t++) {
		EEPROM.write(location + t, buffer[t]);
	}

	if (crc!=NULL) {
		crc8_buffer(crc, buffer, bufferLen);
	}
	
	return location + bufferLen;
}

int writeByteToEEPROM(int location, int numBytes, char byte)
{
	for (unsigned int t = 0; t<numBytes; t++) {
		EEPROM.write(location + t, byte);
	}
}

int writeBlockToEEPROM(const char* tag, const unsigned char* buffer, int bufferLen, int& blockStartPos, int& blockLen)
{
	if (strlen(tag) < EPROM_TAG_SIZE) {
		Logger.logError(String(F("Write aborted: tag size incorrect")).c_str());
		return -1;
	}

	int _blockStart = 0;

	// find the block if writePos was not set
	if (blockStartPos==-1)  {
		_blockStart = locateBlock(tag);

		if (_blockStart < 0) {
			Logger.logError(String(F("Block not found")).c_str());
			return -1;
		}
		else {
		    String logMsg = String(F("Block found at ["));
			logMsg += _blockStart;
			logMsg += "]";
			Logger.logInfo(logMsg.c_str());
		}
	}
	else {
		_blockStart = blockStartPos;
		String logMsg = String(F("Writing block at ["));
		logMsg += _blockStart;
		logMsg += "]";
		Logger.logInfo(logMsg.c_str());
	}

	int currWritePos = _blockStart;

	// now write out the block
	unsigned char crc = 0;

	// magic string
	currWritePos = writeBytesToEEPROM(currWritePos, (unsigned  char *) EPROM_BLOCK_START_MAGIC_STRING, strlen(EPROM_BLOCK_START_MAGIC_STRING), NULL);
	
	// tag
	currWritePos = writeBytesToEEPROM(currWritePos, (unsigned char*) tag, EPROM_TAG_SIZE, &crc);
	
	// data len
	unsigned char blockDataLenChar = (char) bufferLen;
	currWritePos = writeBytesToEEPROM(currWritePos, &blockDataLenChar, 1, &crc);

	// data
	currWritePos = writeBytesToEEPROM(currWritePos, buffer, bufferLen, &crc);

	// checksum
	unsigned char blockChecksumChar = (unsigned char) crc;
	currWritePos = writeBytesToEEPROM(currWritePos, &blockChecksumChar, 1, NULL);

	blockStartPos = _blockStart;
	blockLen = currWritePos - _blockStart;

	return 0;
}

int readBytesFromEEPROM(int location, int numBytes, unsigned char* buffer, unsigned char* crc)
{
	for (unsigned int t = 0; t<numBytes; t++) {
		*(buffer + t) = EEPROM.read(location + t);
		if (crc != NULL) {
			crc8(crc, *((char*)buffer + t));
		}
	}

	return location + numBytes;
}

int readBlockAtPosFromEEPROM(int blockLocation, unsigned char* buffer, int bufferLen, int& bytesRead, int& blockLen, char* tag=NULL)
{
	int currReadPos = blockLocation;

    // read in the block

	// magic string
	char blockStartStr[EPROM_BLOCK_START_MAGIC_STRING_LEN];

	currReadPos = readBytesFromEEPROM(currReadPos, EPROM_BLOCK_START_MAGIC_STRING_LEN, (unsigned char*)&blockStartStr[0], NULL);

    unsigned char crc = 0;

	// tag
	char _tag[EPROM_TAG_SIZE];
	currReadPos = readBytesFromEEPROM(currReadPos, EPROM_TAG_SIZE, (unsigned char*) &_tag[0], &crc) ;

	if (tag != NULL) {
		memcpy(tag, _tag, 4);
	}

	// data len
	unsigned char blockDataLenChar = (unsigned char) bufferLen;
	currReadPos = readBytesFromEEPROM(currReadPos, 1, (unsigned char*)&blockDataLenChar, &crc);
	int blockDataLen = (int) blockDataLenChar;

	// data
	currReadPos = readBytesFromEEPROM(currReadPos, blockDataLen, buffer, &crc);

	// checksum
	unsigned char blockChecksumChar;
	currReadPos = readBytesFromEEPROM(currReadPos, 1, (unsigned char*) &blockChecksumChar, NULL);

	if (blockChecksumChar != crc) {
		Logger.logError(String(F("Block read errro: checksum mismatch")).c_str());
		return -1;
	}

	blockLen = currReadPos - blockLocation;
	bytesRead = blockDataLen;

	return 0;
}

int readBlockFromEEPROM(const char* tag, unsigned char* buffer, int bufferLen, int& bytesRead, int& blockStartPos, int& blockLen)
{
	// find the block
	blockStartPos = locateBlock(tag);
	if (blockStartPos<0)  return -1;

	return readBlockAtPosFromEEPROM(blockStartPos, buffer, bufferLen, bytesRead, blockLen);
}

void dumpBytesFromEEPROMToConsole(int location, int numBytes)
{
	for (unsigned int t = 0; t<numBytes; t++) {
		int curr = (int) EEPROM.read(location+t);
		String currStr = String( curr, HEX);
		Logger.logInfo(currStr.c_str());
	}

}

void dumpBlocksToConsole(int startPos)
{
	int currPos = startPos;
	while (currPos<EPROM_CONFIG_END) {
		int blockFoundPos = locateBlock(NULL, currPos);

		if (blockFoundPos>=0) {
			unsigned char buffer[64];
			int blockLen;
			char tag[4];
			int numBytesRead;
			int rc = readBlockAtPosFromEEPROM(blockFoundPos, (unsigned char*)&buffer, 64, numBytesRead, blockLen, &tag[0]);

			String logMsg = "Block with tag [";
			for (int i=0; i<4;i++) {
				logMsg += tag[i];
			}
			
			logMsg += "] found at location [";
			logMsg += blockFoundPos;

			logMsg += "] length [";
			logMsg += blockLen;

			logMsg += "] with contents [";
			for (int i = 0; i<numBytesRead;i++)
				logMsg += String(buffer[i], HEX);
			logMsg += "]";

			Serial.println(logMsg);

			currPos+=blockLen+1;
		} else {
			currPos++;
		}
	}
}

void printConfigCommandHelp(void(*printConfigItemHelp)()) {
	Logger.logInfo2(F("Commands are"));
	Logger.logInfo2(F("S:K,V   = set item K to value V"));

    if (printConfigItemHelp != NULL) {
        Logger.logInfo2(F("Item Id           Item Name       Value"));
        printConfigItemHelp();
    }

	Logger.logInfo2(F("P       = print config"));
	Logger.logInfo2(F("W:P     = write config to EEPROM Optional (P=Pos) "));
	Logger.logInfo2(F("R       = read config from EEPROM"));
	Logger.logInfo2(F("H       = print this help text"));
	Logger.logInfo2(F("E       = erase all config in EEPROM"));
	Logger.logInfo2(F("C       = dump all config blocks to console"));
	Logger.logInfo2(F("D:P,N   = dump N bytes from EEPROM at pos P console"));
	Logger.logInfo2(F("Q       = quit"));
}

void writeConfigToEEPROM(const char* tag, const unsigned char* config, int configLen, int _blockStartPos = -1) {
	Logger.logInfo2(F("Writing config to EEPROM"));

	int blockStartPos = _blockStartPos;
	int blockLen;
	int rc = writeBlockToEEPROM(tag, (const unsigned char*) config, configLen, blockStartPos, blockLen);

	if (rc<0) {
		Logger.logInfo2(F("Failed to write config to EEPROM"));
	}
	else {
		Logger.logInfo2(F("Successfully wrote config to EEPROM"));
	}
}

void loadConfigFromEEPROM(const char* tag, unsigned char* config, int configLen) {
	Logger.logInfo2(F("Reading config from EEPROM."));

	int numBytesRead, blockStartPos, blockLen;

	if (readBlockFromEEPROM(tag, config, configLen, numBytesRead, blockStartPos, blockLen) == 0) {
		Logger.logInfo2(F("Successfully read config from EEPROM."));
	}
	else {
		Logger.logInfo2(F("Failed to read config from EEPROM. Using default config."));
	};
}

void runConfigUI(const char* configTag,
	unsigned char* config,
	int configLen,
	void(*printConfigItemHelp)(),
	void(*printConfig)(),
	void(*setConfigItem)(const char*, const char*))
{
	char lineBuffer[32];

	// allow user to modify config
	Logger.logInfo2(F("Config mode entered"));
	printConfigCommandHelp(printConfigItemHelp);

	while (1 == 1) {
		if (readLineFromSerial(Serial.read(), lineBuffer, sizeof(lineBuffer)) > 0) {

            Logger.logInfo2(F("Line = [%s]"), lineBuffer);

			// ** HELP ******************************************************	
			if (lineBuffer[0] == 'H') {
				printConfigCommandHelp(printConfigItemHelp);
				strcpy(lineBuffer, "");
			}

			// ** QUIT **************************************************************	
			else if (strcmp(lineBuffer,"Q")==0) {
				Logger.logInfo2(F("Exiting interactive config mode"));
                strcpy(lineBuffer, "");
                break;
			}

			// ** BLOCKS **************************************************************	
			else if (strcmp(lineBuffer,"C")==0) {
				Logger.logInfo2(F("Dumping config blocks"));
				dumpBlocksToConsole();
				Logger.logInfo2(F("Done"));
                strcpy(lineBuffer, "");
            }

			// ** DUMP **************************************************************	
			else if (lineBuffer[0] == 'D') {
				Logger.logInfo2(F("Dumping EEPROM contents"));
				//String posStr = getField(&lineBuffer, ',');
				//String numStr = getField(&lineBuffer, ',');
				//dumpBytesFromEEPROMToConsole(posStr.toInt(), numStr.toInt());
				//Logger.logInfo2.println(F("Done"));
                strcpy(lineBuffer, "");
            }

			// ** ERASE **************************************************************	
			else if (strcmp(lineBuffer,"E") == 0) {
				Logger.logInfo2(F("Erasing all config"));
				writeByteToEEPROM(0, EPROM_CONFIG_END, 'X');
				Logger.logInfo2(F("Done"));
                strcpy(lineBuffer, "");
            }

			// ** PRINT *************************************************************	
			else if (strcmp(lineBuffer,"P") == 0) {
				printConfig();
                strcpy(lineBuffer, "");
            }

			// ** WRITE *************************************************************	
			else if (lineBuffer[0] == 'W') {
				char *posStr = strtok(lineBuffer, ",");

				if (strcmp(posStr,"")==0) {
					writeConfigToEEPROM(configTag, config, configLen, -1);
				}
				else {
                    int pos;
                    pos = atoi(posStr);
					writeConfigToEEPROM(configTag, config, configLen, pos);
				}
                strcpy(lineBuffer, "");
            }

			// ** READ *************************************************************	
			else if (lineBuffer[0] == 'R') {
				loadConfigFromEEPROM(configTag, config, configLen);
                strcpy(lineBuffer, "");
            }

			// ** SET ***************************************************************	
			else if (lineBuffer[0] == 'S') {
                char* cmd = strtok(lineBuffer, ":");
                char* key = strtok(NULL, ",");
                char* val = strtok(NULL, ",");

				Logger.logInfo2(F("Setting item [%s] to [%s]"), key, val);

				setConfigItem(key, val);

                strcpy(lineBuffer, "");
            }

			else {
				Logger.logInfo2(F("Unknown command [%s]"), lineBuffer);
                strcpy(lineBuffer, "");
            }
		}
	}

}

void initConfig(const char* configTag,
	unsigned char* config,
	int configLen,
	void(*printConfigItemHelp)(),
	void(*printConfig)(),
	void(*setConfigItem)(const char*, const char*))
{
	int  initPeriod = 10;        // time to wait for user to enter config mode  in secs
	int  initCycleDelay = 1000;   // cycle time in ms
	long initPeriodCountMax = (initPeriod * 1000) / initCycleDelay;
	long initPeriodCountCur = 0;

	char lineBuffer[32];

	Logger.logInfo2(F("Starting up in [%d]"), initPeriod);
	Logger.logInfo2(F("Using config"));

	loadConfigFromEEPROM(configTag, config, configLen);
	printConfig();

	Logger.logInfo2(F("Press 'C' and 'Enter' to enter config mode or 'Q' to continue immediately"));

	int configModeSelected = 0;

	while ((initPeriodCountCur < initPeriodCountMax) && (configModeSelected == 0)) {

		initPeriodCountCur++;
		Logger.logInfo2(F("."));
		delay(initCycleDelay);

        if (readLineFromSerial(Serial.read(), lineBuffer, sizeof(lineBuffer)) > 0) {
			if (strcmp(lineBuffer,"C")==0) {
				configModeSelected = 1;
				strcpy(lineBuffer, "");
			}
			else if (strcmp(lineBuffer,"Q")==0) {
				break;
			}
		}
	}

	if (configModeSelected == 1) {
		Logger.logInfo2(F("Entering manual config mode"));
		runConfigUI(configTag, config, configLen, printConfigItemHelp, printConfig, setConfigItem);
	}

	Logger.logInfo2(F("Continuing startup"));
}


char* find_first_non_white_space(const char *line)
{
    char * rc = (char *) line;
    while (isspace((unsigned char)*rc))
        rc++;

    return rc;
}