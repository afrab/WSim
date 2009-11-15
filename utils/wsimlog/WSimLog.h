/*
WSimLog.h

Purpose:
	This header file supports logging ints, bytes, characters, and 
	strings to the WSim log file.

Author: 
	Mohamed M. Elwakil (mohamed@elwakil.net) 
        modified by Antoine Fraboulet
 
Usage:
	1. Include that file in your code.
	2. Use any of the log macros defined here. For example:
		logChar('m'); 
		logByte(10);
		logInt(33222);
		logString("Mickey"); 
	3. Compile your code.
	4. Run WSim with this option: --monitor=__WSIMLOGBUFFER:w
	
Notes: - If you need strings larger than 100 characters, modify the
         value set to __WSIMLOGBUFFER_LENGTH to suit your needs.
       - data are stored LSB first	
*/

#define __WSIMLOGBUFFER_LENGTH 100
char __WSIMLOGBUFFER[__WSIMLOGBUFFER_LENGTH+1];

void logChar(char ch) 
{ 
	__WSIMLOGBUFFER[2] = '\0';	
	__WSIMLOGBUFFER[1] = ch; 
	__WSIMLOGBUFFER[0] = '1'; 
}

void logByte(char bt) 
{ 
	__WSIMLOGBUFFER[2] = '\0';	
	__WSIMLOGBUFFER[1] = bt; 
	__WSIMLOGBUFFER[0] = '2'; 
}

void logInt(int it) 
{ 
	__WSIMLOGBUFFER[3] = '\0';	
	__WSIMLOGBUFFER[2] = (uint8_t)((uint16_t)it >> 8) & 0x00ff;
	__WSIMLOGBUFFER[1] = (uint8_t)((uint16_t)it     ) & 0x00ff;
	__WSIMLOGBUFFER[0] = '3'; 
}

void logString(char* str)
{
	strcpy(&__WSIMLOGBUFFER[1],str);
	__WSIMLOGBUFFER[0] = '4';
}
