// types.h

#ifndef _TYPES_h
#define _TYPES_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

struct Config {
    char id[5];
	byte ledPin;
    byte tempPin;
    byte switchPin;  
};


#endif

