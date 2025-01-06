#define SERIAL_HARDWARE_FLOW_CONTROL 0
#include "MTISerial.h"

#ifdef MTI_UNIX
#include <poll.h>
#include <sys/ioctl.h>
#include <time.h>
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MTISerialIO Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MTISerialIO::MTISerialIO ()
{
	m_hFile = 0;
#ifdef MTI_WINDOWS
	m_hevtOverlapped = 0;
#endif
}

MTISerialIO::~MTISerialIO ()
{
	if(m_hFile != 0)
		Close();
}

bool MTISerialIO::IsPortAvailable (const char* port)
{
#ifdef MTI_WINDOWS
	HANDLE hFile = ::CreateFile(port, GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;
	::CloseHandle(hFile);
#endif

#ifdef MTI_UNIX
	int hFile = open( port, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if( hFile == -1 )
		return false;
	close(hFile);
#endif

	return true;
}

long MTISerialIO::Open (const char* port, unsigned int baudRate, unsigned int inQueue, unsigned int outQueue)
{
	if (m_hFile)
		return MTI_ERR_SERIALCOMM;

#ifdef MTI_WINDOWS
	// Open the device
	m_hFile = ::CreateFile(port,GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		m_hFile = 0;
		return MTI_ERR_INVALID_DEVICEID;
	}

	m_hevtOverlapped = ::CreateEvent(0,true,false,0);
	if (m_hevtOverlapped == 0)
	{
		::CloseHandle(m_hFile);
		m_hFile = 0;
		return MTI_ERR_SERIALCOMM;
	}

	// Setup the COM-port
	if (inQueue || outQueue)
	{
		if (!::SetupComm(m_hFile,inQueue,outQueue))
		{
			Close();
			return MTI_ERR_SERIALCOMM;
		}
	}

	DWORD dwMask = EV_BREAK|EV_ERR|EV_RXCHAR;
	if (!::SetCommMask(m_hFile,dwMask))
		return MTI_ERR_SERIALCOMM;

	// Setup the device for default settings
 	COMMCONFIG commConfig = {0};
	DWORD dwSize = sizeof(commConfig);
	commConfig.dwSize = dwSize;
	::GetDefaultCommConfig(port,&commConfig,&dwSize);
	::SetCommConfig(m_hFile,&commConfig,dwSize);
#endif

#ifdef MTI_UNIX
	m_hFile = open(port,O_RDWR|O_NOCTTY|O_NONBLOCK);		//Open in non blocking read/write mode
	if (m_hFile == -1)
	{
		m_hFile = 0;
		return MTI_ERR_INVALID_DEVICEID;
	}
#endif

	SetBlockingMode();
	SetSerialParams(baudRate);

	return MTI_SUCCESS;
}

long MTISerialIO::Close (void)
{
	if (m_hFile == 0)
		return MTI_SUCCESS;

#ifdef MTI_WINDOWS
	if (m_hevtOverlapped)
	{
		::CloseHandle(m_hevtOverlapped);
		m_hevtOverlapped = 0;
	}
	::CloseHandle(m_hFile);
#endif

#ifdef MTI_UNIX
	close( m_hFile ); 
#endif

	m_hFile = 0;
	return MTI_SUCCESS;
}

long MTISerialIO::SetSerialParams (unsigned int baudRate)
{
	if (m_hFile == 0)
		return MTI_ERR_INVALID_HANDLE;

#ifdef MTI_WINDOWS
	// Obtain the DCB structure for the device
	DCB dcb = {0};
    dcb.DCBlength=sizeof(dcb);

	if (!::GetCommState(m_hFile,&dcb))
		return MTI_ERR_SERIALCOMM;

	dcb.BaudRate = DWORD(baudRate);
	dcb.ByteSize = 8;
	dcb.Parity   = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	if (!SERIAL_HARDWARE_FLOW_CONTROL) {
		// Set the new data
		dcb.fOutxCtsFlow = false;					// Disable CTS monitoring
		dcb.fOutxDsrFlow = false;					// Disable DSR monitoring
		dcb.fDtrControl = DTR_CONTROL_DISABLE;		// Disable DTR monitoring
		dcb.fOutX = false;							// Disable XON/XOFF for transmission
		dcb.fInX = false;							// Disable XON/XOFF for receiving
		dcb.fRtsControl = RTS_CONTROL_DISABLE;		// Disable RTS (Ready To Send)
		dcb.fParity  = false;
	}
	else
	{
		// set XON/XOFF
		dcb.fOutX	= FALSE;
		dcb.fInX	= FALSE;
		// set RTSCTS
		dcb.fOutxCtsFlow = TRUE;
		dcb.fRtsControl = RTS_CONTROL_HANDSHAKE; 
		// set DSRDTR
		dcb.fOutxDsrFlow = FALSE;
		dcb.fDtrControl = DTR_CONTROL_DISABLE; 
	}

	if (!::SetCommState(m_hFile,&dcb))
		return MTI_ERR_SERIALCOMM;
#endif

#ifdef MTI_UNIX
	//CONFIGURE THE UART
	//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	//	CSIZE:- CS5, CS6, CS7, CS8
	//	CLOCAL - Ignore modem status lines
	//	CREAD - Enable receiver
	//	IGNPAR = Ignore characters with parity errors
	//	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
	//	PARENB - Parity enable
	//	PARODD - Odd parity (else even)
	struct termios options;
	tcgetattr(m_hFile, &options);

	int baud_flag;
	switch (baudRate) {
	case 9600:		baud_flag = B9600; break;
	case 19200:		baud_flag = B19200; break;
	case 38400:		baud_flag = B38400; break;
	case 57600:		baud_flag = B57600; break;
	case 115200:	baud_flag = B115200; break;
	case 230400:	baud_flag = B230400; break;
	case 460800:	baud_flag = B460800; break;
	case 500000:	baud_flag = B500000; break;
	case 576000:	baud_flag = B576000; break;
	case 921600:	baud_flag = B921600; break;
	default:		baud_flag = B9600; break;
	}

	cfsetspeed(&options, baud_flag);
	options.c_cflag &= ~CSTOPB;
	options.c_cflag |= CLOCAL;
	options.c_cflag |= CREAD;
	cfmakeraw(&options);
	// next may be good to try for timeout control
	//options.c_cc[VMIN]  = 1;
	//options.c_cc[VTIME] = 10;
	//
	tcflush(m_hFile, TCIFLUSH);
	tcsetattr(m_hFile, TCSANOW, &options);	
#endif

	return MTI_SUCCESS;
}

long MTISerialIO::Write (unsigned char* pData, size_t lData, unsigned int* lWritten, unsigned int timeout)
{
	if (m_hFile == 0)
		return MTI_ERR_INVALID_HANDLE;

#ifdef MTI_WINDOWS
	DWORD dwWritten = 0;
	if (!m_hevtOverlapped && timeout != INFINITE)
		return MTI_ERR_SERIALCOMM;

	// Wait for the event to happen
	OVERLAPPED ovInternal;
	LPOVERLAPPED lpOverlapped = 0;
	if (m_hevtOverlapped)
	{
		memset(&ovInternal,0,sizeof(ovInternal));
		ovInternal.hEvent = m_hevtOverlapped;
		lpOverlapped = &ovInternal;
	}
	// Make sure the overlapped structure isn't busy
	if(!(!m_hevtOverlapped || HasOverlappedIoCompleted(lpOverlapped)))
		return MTI_ERR_SERIALCOMM;

	// Write the data
	if (!::WriteFile(m_hFile,(LPCVOID)pData,lData,&dwWritten,lpOverlapped))
	{
		long lLastError = ::GetLastError();
		if (lLastError != ERROR_IO_PENDING)
			return MTI_ERR_SERIALCOMM;

		// We need to block if the client didn't specify an overlapped structure Wait for the overlapped operation to complete
		switch (::WaitForSingleObject(lpOverlapped->hEvent,timeout))
		{
		case WAIT_OBJECT_0:
			// The overlapped operation has completed
			if (!::GetOverlappedResult(m_hFile,lpOverlapped,&dwWritten,FALSE))
				return MTI_ERR_SERIALCOMM;
			break;
		case WAIT_TIMEOUT:
			::CancelIo(m_hFile);
			return MTI_ERR_SERIALCOMM_READ_TIMEOUT;
		default:
			return MTI_ERR_SERIALCOMM;
		}
	}
	else
	{
		// The operation completed immediatly. Just to be sure we'll set the overlapped structure's event handle.
		if (lpOverlapped)
			::SetEvent(lpOverlapped->hEvent);
	}

	if( lWritten != 0 )
		*lWritten = dwWritten;
#endif

#ifdef MTI_UNIX
	write(m_hFile, pData, lData);

	// For some channels like Bluetooth, the write immediately returns even though the output buffer
	// may take some time to clear. We implement a partially blocking write by polling the output buffer
	// to be sure it has been sent before continuing. A timeout is used in case the output write 
	// blocks indefinitely.
	if (!timeout)
		return MTI_SUCCESS;

	int out_bytes = -1;
	clock_t start = clock();
	while (out_bytes != 0) {
		double elapsed = (double)(clock()-start) / CLOCKS_PER_SEC * 1000; // milliseconds
		if (timeout != INFINITE && elapsed > timeout)
			return MTI_ERR_SERIALCOMM;
		ioctl(m_hFile, TIOCOUTQ, &out_bytes);
	}
#endif

	return MTI_SUCCESS;
}

long MTISerialIO::SetBlockingMode (int blockingMode)
{
	if (m_hFile == 0)
		return MTI_ERR_INVALID_HANDLE;

#ifdef MTI_WINDOWS
	COMMTIMEOUTS cto;
	if (!::GetCommTimeouts(m_hFile,&cto))
		return MTI_ERR_SERIALCOMM;

	if(blockingMode == MTI_BLOCKING_MODE_ON)
	{
		cto.ReadIntervalTimeout = 0;
		cto.ReadTotalTimeoutConstant = 0;
		cto.ReadTotalTimeoutMultiplier = 0;
	}
	else if(blockingMode == MTI_BLOCKING_MODE_OFF)
	{
		cto.ReadIntervalTimeout = MAXDWORD;
		cto.ReadTotalTimeoutConstant = 0;
		cto.ReadTotalTimeoutMultiplier = 0;
	}
	else
		return MTI_ERR_SERIALCOMM;

	if (!::SetCommTimeouts(m_hFile,&cto))
		return MTI_ERR_SERIALCOMM;
#endif

#ifdef MTI_UNIX
	const int flags = fcntl(m_hFile, F_GETFL);
	fcntl(m_hFile, F_SETFL, blockingMode == MTI_BLOCKING_MODE_ON ? flags & ~O_NONBLOCK : flags | O_NONBLOCK);
#endif
	return MTI_SUCCESS;
}

long MTISerialIO::Read (unsigned char* pData, size_t lData, unsigned int* lRead, unsigned int timeout, int blockingMode)
{
	if (m_hFile == 0)
		return MTI_ERR_INVALID_HANDLE;

#ifdef MTI_WINDOWS
	if( blockingMode != MTI_BLOCKING_MODE_ERR )
		SetBlockingMode( blockingMode );
	DWORD dwRead = 0;
#ifdef _DEBUG
	// The debug version fills the entire data structure with 0xDC bytes, to catch buffer errors as soon as possible.
	memset(pData,0xDC,lData);
#endif

	// Check if an overlapped structure has been specified
	if (!m_hevtOverlapped && timeout != INFINITE)
		return MTI_ERR_SERIALCOMM;
	// Wait for the event to happen
	OVERLAPPED ovInternal;
	memset(&ovInternal,0,sizeof(ovInternal));
	ovInternal.hEvent = m_hevtOverlapped;
	LPOVERLAPPED lpOverlapped = &ovInternal;
	// Make sure the overlapped structure isn't busy
	if(!(!m_hevtOverlapped || HasOverlappedIoCompleted(lpOverlapped)))
		return MTI_ERR_SERIALCOMM;

	// Read the data
	if (!::ReadFile(m_hFile,pData,lData,&dwRead,lpOverlapped))
	{
		long lLastError = ::GetLastError();
		if (lLastError != ERROR_IO_PENDING)
			return MTI_ERR_SERIALCOMM;

		// We need to block if the client didn't specify an overlapped structure
		switch (::WaitForSingleObject(lpOverlapped->hEvent,timeout))
		{
		case WAIT_OBJECT_0:
			// The overlapped operation has completed
			if (!::GetOverlappedResult(m_hFile,lpOverlapped,&dwRead,FALSE))
				return MTI_ERR_SERIALCOMM;
			break;
		case WAIT_TIMEOUT:
			::CancelIo(m_hFile);
			return MTI_ERR_SERIALCOMM_READ_TIMEOUT;
		default:
			return MTI_ERR_SERIALCOMM;
		}
	}
	else
	{
		// The operation completed immediatly. Just to be sure we'll set the overlapped structure's event handle.
		if (lpOverlapped)
			::SetEvent(lpOverlapped->hEvent);
	}

	if( lRead != 0 )
		*lRead = dwRead;
#endif

#ifdef MTI_UNIX
	if( blockingMode != MTI_BLOCKING_MODE_ERR )
		SetBlockingMode( blockingMode );
	
	// In each pass, the read() command returns the number of bytes read, which can be less than
	// the number of bytes requested. Here we keep reading until the number of bytes read
	// matches the number of bytes requested in lData.
	unsigned int rtot = 0;
	do
	{
		// If a timeout is specified, poll the file descriptor to make sure input is ready.
		// poll() returns 0 if timeout occurs, -1 if an error occurs.
		if (timeout != INFINITE)
		{
			struct pollfd fds[1];
			fds[0].fd = m_hFile;
			fds[0].events = POLLIN;
			int perr = poll(fds, 1, timeout);
			if (perr == 0)									// timeout
				return MTI_ERR_SERIALCOMM_READ_TIMEOUT;
			if (perr == -1 || !(fds[0].revents & POLLIN))	// other error or data not ready
				return MTI_ERR_SERIALCOMM;
		}
		rtot += read(m_hFile, pData + rtot, lData - rtot);
	} while (rtot < lData);

	if( lRead != 0 )
		*lRead = rtot;
	
#endif
	return MTI_SUCCESS;
}

long MTISerialIO::Purge()
{
	if (m_hFile == 0)
		return MTI_ERR_INVALID_HANDLE;

#ifdef MTI_WINDOWS
	if (!::PurgeComm(m_hFile, PURGE_TXCLEAR | PURGE_RXCLEAR))
		return MTI_ERR_SERIALCOMM;
#endif

#ifdef MTI_UNIX
	tcflush(m_hFile, TCIOFLUSH);
#endif

	return MTI_SUCCESS;
}

long MTISerialIO::ReadText (char* text, unsigned char delineationCharacter, unsigned int timeout)
{
	bool wait = true;
	long lastError;
	unsigned int textLength=0;
	unsigned char rdata[1];

	SetBlockingMode( true );
	while(wait) 
	{
		lastError = Read( rdata, 1, 0, timeout, MTI_BLOCKING_MODE_ERR );
		if (lastError != 0) 
		{
			wait = false;
			return lastError;
		}
		if (rdata[0] == delineationCharacter)
			wait = false;
		else 
		{
			text[textLength++] = (char)rdata[0];
			text[textLength] = 0;
		}
		if (textLength >= 80)
			wait = false;
	}
	return lastError;
}