// MGLib.h

#ifndef _MGLIB_h
#define _MGLIB_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

// *****************************************************************************
// Logging 
// *****************************************************************************

#define DEBUG 0
#define INFO  3
#define ERROR 6

class LoggerClass
{
 protected:
	Stream* m_stream = NULL;
	int m_level = ERROR;
    int m_bufferSize = 128;         // default size
	
 public:
	//static const int DEBUG = 0;
	//static const int INFO  = 3;
	//static const int ERROR = 6;

	void init(Stream* stream, int level=INFO);
	void setLevel(int level);
	void log(int level, const char* msg);
	void log(int level, const __FlashStringHelper * fsh);

	void flush();
		
	void logDebug(const char* msg);
	void logInfo(const char* msg);
	void logError(const char* msg);
    
	void logDebug(const __FlashStringHelper * fsh);
	void logInfo(const __FlashStringHelper * fsh);
	void logError(const __FlashStringHelper * fsh);

    void logInfo2(const __FlashStringHelper * fmt, ...);

    void setBufferSize(int bufferSize);

};

extern LoggerClass Logger;

//#define LOG_INFO
//#define LOG_ERROR
//#define LOG_DEBUG

#ifdef LOG_INFO
#define LOG_INFO(M, ...) { sprintf_P(Logger.msgBuffer, PSTR(M), ##__VA_ARGS__); Logger.logInfo(Logger.msgBuffer); }
#else
#define LOG_INFO(M, ...) 
#endif

#ifdef LOG_ERROR
#define LOG_ERROR(M, ...) { sprintf_P(Logger.msgBuffer, PSTR(M), ##__VA_ARGS__); Logger.logError(Logger.msgBuffer); }
#else
#define LOG_ERROR(M, ...)
#endif

#ifdef LOG_DEBUG
#define LOG_DEBUG(M, ...) { sprintf_P(Logger.msgBuffer, PSTR(M), ##__VA_ARGS__); Logger.logDebug(Logger.msgBuffer); }
#else
#define LOG_DEBUG(M, ...)
#endif

class FunctionLogger
{
	public:
		FunctionLogger(String functionName) {
			m_functionName = functionName;
			String logMsg = F("Enter [");
			logMsg += functionName;
			logMsg += F("]");
			Logger.logInfo(logMsg.c_str());
		}

		~FunctionLogger() {
			String logMsg = F("Exit  [");
			logMsg += m_functionName;
			logMsg += F("]");
			Logger.logInfo(logMsg.c_str());
		}

	private:
		String m_functionName;
};

#ifdef ENABLE_FUNCTION_LOGGING
#define LOG_FUNCTION(M) FunctionLogger(F(M));
#else
#define LOG_FUNCTION(M) 
#endif


// *****************************************************************************
//
// *****************************************************************************
class LEDClass
{
	protected:
	int m_pin = 13;
	int m_blinkPeriod = 20;
	
	public:
	LEDClass(int pin=13, int blinkPeriod=20);
	void blink();
};

extern LEDClass LEDPin13;

// *****************************************************************************
// Joystick functions
// *****************************************************************************

struct JoystickValues {
	long throttle;
	long direction;
};

class BluetoothJoystickBaseClass {
	public:
		virtual JoystickValues getStickPosition() = 0;
		
};

class BluetoothJoypadClass: public BluetoothJoystickBaseClass
{
	public:
		enum Direction {
			NONE,
			CENTRE,
			UP,
			DOWN,
			LEFT,
			RIGHT,
			UP_LEFT,
			UP_RIGHT,
			DOWN_LEFT,
			DOWN_RIGHT
		};
		
		BluetoothJoypadClass(Stream* serialPort=NULL);
		void init(Stream* serialPort, float throttleScaling=1, float directionScaling=1);
		
		Direction getPadDirection();
		JoystickValues getStickPosition();
		
	protected:
		Stream* m_bluetoothPort = NULL;
		Direction m_lastDirection;
		unsigned long m_lastDirectionTime;
		float m_throttleScaling;
		float m_directionScaling;
};

extern BluetoothJoypadClass BluetoothJoypad;

class BluetoothJoystickClass: BluetoothJoystickBaseClass
{
	public:
	
	BluetoothJoystickClass(Stream* serialPort=NULL);
	void init(Stream* serialPort);
	
	JoystickValues getStickPosition();
	
	protected:
	Stream* m_bluetoothPort = NULL;
	JoystickValues m_lastStickPosition;
	unsigned long m_lastStickPositionTime;
};

extern BluetoothJoystickClass BluetoothJoystick;

// *****************************************************************************
// Memory utility functions
// *****************************************************************************

void check_mem();
void format_mem_info(const char* buffer);    
void sprintf_vargs (char* buffer, int bufferlen, char * format, ...);
void logMemInfo(const char* label="NONE");

// *****************************************************************************
// String parsing functions - handy in extracting fields from messages
// *****************************************************************************

String getField(String* msg, char fieldSep);
String getFieldOfSize(String* msg, int numChars);

// *****************************************************************************
// Misc utility functions
// *****************************************************************************

int readLineFromSerial(int readch, char*buffer, int bufferLen);

char* find_first_non_white_space(const char *line);


// *****************************************************************************
// Config - functions to allow sketch config to be stored in EPROM
// *****************************************************************************

#define EPROM_TAG_SIZE 4

int writeBytesToEEPROM(int location, const unsigned char* buffer, int bufferLen, unsigned char* crc);

int readBytesFromEEPROM(int location, int numBytes, unsigned char* buffer, unsigned char* crc);

void dumpBytesFromEEPROMToConsole(int location, int numBytes);

int writeBlockToEEPROM(const char* tag, const unsigned char* bufferStart, int bufferLen, int& blockStartPos, int& blockLen);

int readBlockFromEEPROM(const char* tag, unsigned char* buffer, int bufferLen, int&bytesRead, int& blockStartPos, int& blockLen);

void dumpBlocksToConsole(int startPos = 0);

void initConfig(const char* configTag,
				unsigned char* config,
				int configLen,
				void(*printConfigItemHelp)(),
				void(*printConfig)(),
				void(*setConfigItem)(const char*, const char*));


#endif

