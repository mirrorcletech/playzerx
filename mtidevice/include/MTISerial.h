#ifndef MTI_SERIAL_H
#define MTI_SERIAL_H

#include "MTIDefinitions.h"
//#include "stdafx.h"

///////////////////////////////////////////////////////////////////////////////////
// Constants
#define MTI_SERIAL_MAXPORTS			64
#define MTI_BAUDRATE_DEFAULT		921600
#define MTI_SERIAL_QUEUE_SIZE		8192*16
#define SERIAL_HARDWARE_FLOW_CONTROL 0

#define STRICT
#define NOMINMAX 
#ifndef INFINITE					
	#define INFINITE	0xFFFFFFFF 
#endif

#ifdef MTI_WINDOWS
	#include <windows.h>
	#include <tchar.h>
	#include "enumCOMs.h"
	#define MTI_PORT_PREFIX			"\\\\.\\COM"
	#define MTI_PORT_DISPLAY		"COM"
#endif
#ifdef MTI_UNIX
	#include <unistd.h>			
	#include <fcntl.h>			
	#include <termios.h>
// Change the port prefix to "/dev/serial" to use the GPIO serial port on the RaspPi
// Change the port prefix to "/dev/rfcomm" to use SPP over Bluetooth on the RaspPi
	#define MTI_PORT_PREFIX			"/dev/ttyUSB"
	#define MTI_PORT_DISPLAY		"ttyUSB"
#endif

///////////////////////////////////////////////////////////////////////////////////
//MTISerialIO is the basic Serial IO communication
class MTISerialIO
{
public:
	MTISerialIO();
	virtual ~MTISerialIO();

	// Check if particular COM-port is available (static method).
	static bool IsPortAvailable (const char* port);
	// Open the serial communications for a particular COM port. You need to use the full devicename (i.e. "COM1") to open the port.
	virtual long Open (const char* port, unsigned int baudRate = MTI_BAUDRATE_DEFAULT, unsigned int inQueue = MTI_SERIAL_QUEUE_SIZE, unsigned int outQueue = MTI_SERIAL_QUEUE_SIZE);
	// Close the serial port.
	virtual long Close (void);
	// Purge all buffers
	long Purge (void);

	// Set serial port params
	virtual long SetSerialParams (unsigned int baudRate = MTI_BAUDRATE_DEFAULT);
	// Read operations can be blocking or non-blocking.
	virtual long SetBlockingMode (int blockingMode = MTI_BLOCKING_MODE_ON);

	// Write data to the serial port.
	virtual long Write (unsigned char* pData, size_t lData, unsigned int* lWritten = 0, unsigned int timeout = INFINITE);

	// Read data from the serial port
	virtual long Read (unsigned char* pData, size_t lData, unsigned int* lRead = 0, unsigned int timeout = INFINITE, int blockingMode = MTI_BLOCKING_MODE_ON);
	// read text from serial port.  wait for characters until a \n is received, then return char*.  otherwise time out
	virtual long ReadText (char* text, unsigned char delineationCharacter, unsigned int timeout = INFINITE);

// Attributes
protected:
#ifdef MTI_WINDOWS
	HANDLE	m_hFile;			// File handle
	HANDLE	m_hevtOverlapped;	// Event handle for internal overlapped operations
#endif
#ifdef MTI_UNIX
	int m_hFile;
#endif

};

#endif