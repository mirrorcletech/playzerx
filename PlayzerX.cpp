//////////////////////////////////////////////////////////////////////
// PlayzerX.cpp
// Version: 2.1.0.0
//////////////////////////////////////////////////////////////////////

#include "PlayzerX.h"

namespace playzerx
{
PlayzerX* PlayzerX::CreateDevice()
{
	PlayzerX* pl = new PlayzerX();
	return pl;
}

void PlayzerX::DeleteDevice(PlayzerX* device) { SAFE_DELETE(device); }

PlayzerX::PlayzerX()
{
	m_RGBCapable = false;
	m_CommandBytes.reserve(kMaxSendBytes);
	m_SamplesRemaining = 0;
	m_SampleRate = 10000;
	m_TimeOut = 1000;
	m_StreamingSamplesRemaining = false;
}

PlayzerX::~PlayzerX()
{
	if (m_SerialDevice != nullptr)
	{
		SAFE_DELETE(m_SerialDevice);
		m_LastError = PlayzerXError::SUCCESS;
	}
}

MTISerialIO* PlayzerX::ConnectDevice()
{
	PlayzerXAvailableDevices table;
	GetAvailableDevices(table);

	if (table.NumDevices == 0) { m_LastError = PlayzerXError::ERROR_CONNECTION; }
	else
		m_SerialDevice = ConnectDevice(table.CommPortName[0]);

	return m_SerialDevice;
}

MTISerialIO* PlayzerX::ConnectDevice(const std::string& portName)
{
	// TODO Add check if already connected!

	if (portName.empty()) { m_LastError = PlayzerXError::ERROR_INVALID_PARAM; }
	else
	{
		char* port_chr = new char[portName.length() + 1];
		std::strcpy(port_chr, portName.c_str());
		m_SerialDevice = ConnectDevice(port_chr);
		delete[] port_chr;
	}

	return m_SerialDevice;
}

MTISerialIO* PlayzerX::ConnectDevice(char* portName)
{
	// TODO Add check if already connected!
	char* connectPortName = new char[20];
	int deviceNum;
	MTISerialIO* socket = new MTISerialIO;

	// Check if portName was provided, then ignore
	// deviceNum and try to connect directly
	if (strlen(portName) < 4)
	{
		printf("Bad port");
		return NULL;
	}

	// Detect if comport name is missing escape sequence and add if necessary
#ifdef MTI_WINDOWS
	if (portName[0] == 67)  // Check if capital C for COM starts the string
		sprintf(connectPortName, "\\\\.\\%s", portName);
	else if (portName[0] == 92)  // Check if backslash is included in string
		strcpy(connectPortName, portName);
	else
	{
		printf("Bad port");
		return NULL;
	}

	// Now check if that port is available
	if (!MTISerialIO::IsPortAvailable(connectPortName))
	{
		printf("Bad port");
		return m_SerialDevice;
	}
#endif
#ifdef MTI_UNIX
	if (portName[0] == '/')  // Check if full path is passed in
		strcpy(connectPortName, portName);
	else  // Otherwise prepend '/dev/'
		sprintf(connectPortName, "/dev/%s", portName);
#endif

	deviceNum = 0;
	Sleep(50);
	long serialError = socket->Open(connectPortName, kBaudRate);
	if (serialError != 0)
	{
		SAFE_DELETE(socket);
		printf("Bad port");
		return m_SerialDevice;
	}
	m_SerialDevice = socket;

	// in case the device is not in PlayzerX mode, skip next commands
	if (IsDeviceConnected())
	{
		SetSampleRate(m_SampleRate);

		GetDeviceInfo();

		// This has to be last, after that bytes will come streaming on receive
		// Every xx milliseconds USB receives buffer remaining samples update
		SetBufferUpdateTimer(100);

		m_LastError = PlayzerXError::SUCCESS;
	}
	else
		m_LastError = PlayzerXError::ERROR_CONNECTION;

	return m_SerialDevice;
}

void PlayzerX::DisconnectDevice()
{
	// Stop receives buffer remaining samples update
	SetBufferUpdateTimer(0);

	if (m_SerialDevice != nullptr)
	{
		long serialError = m_SerialDevice->Close();
		if (serialError != 0) printf("Trouble trying to close com port");
		// TODO ERROR
		SAFE_DELETE(m_SerialDevice);
	}

	m_LastError = PlayzerXError::SUCCESS;
}

int PlayzerX::GetSamplesRemaining()
{
	long lastError;
	unsigned int pdwRead;
	unsigned int m_iTimeOut = 250;
	unsigned char data[MTI_SERIAL_QUEUE_SIZE];

	if (m_StreamingSamplesRemaining)
	{
		// if controller is set to regularly stream SamplesRemaining we assume serial
		// buffer already has latest value available. no need to request from controller
		// Clear data previously sent by the controller - except latest 6 bytes (latest value)
		m_SerialDevice->Read(data, MTI_SERIAL_QUEUE_SIZE - 6, 0, 0, MTI_BLOCKING_MODE_OFF);
		// now get the latest 6 bytes which represent response to SamplesRemaining
		lastError = m_SerialDevice->Read(data, 6, &pdwRead, m_iTimeOut);
	}

	unsigned char sendData[10];
	if (!m_StreamingSamplesRemaining || (lastError == 5))  // if timeout above, try requesting data
	{
		// If not streaming, need to ask for them
		sendData[0] = 'p';
		sendData[1] = 'l';
		sendData[2] = 'g';
		sendData[3] = 5;
		sendData[4] = 10;  // Include suffix here!
		lastError = m_SerialDevice->Write(sendData, 5, 0, 200);
		// now get the latest 6 bytes which represent response to SamplesRemaining
		lastError = m_SerialDevice->Read(data, 6, &pdwRead, m_iTimeOut);
	}

	if (lastError != 0) return -1;  // seems we could not get a good response

	// Got response - enforce byte alignment by sliding until finding the complete preamble
	unsigned char temp;
	long countTries = 0, maxTries = 6;
	bool match;

	match = (data[2] == 45) && (data[1] == 108) && (data[0] == 112);  // do we see "pl-" preamble
	while (!match)
	{
		temp = data[0];
		data[0] = data[5];
		data[5] = data[4];
		data[4] = data[3];
		data[3] = data[2];
		data[2] = data[1];
		data[1] = temp;
		match =
			(data[2] == 45) && (data[1] == 108) && (data[0] == 112);  // do we see "pl-" preamble
		countTries++;
		if (countTries > maxTries) return -1;
	}
	m_SamplesRemaining = (data[5] << 16) + (data[4] << 8) + data[3];

	return m_SamplesRemaining;
}

void PlayzerX::WaitForBufferLevel(int bufferLevel)
{
	if (bufferLevel < 0) return;

	float executionTimeMs;
	int waitTime;

	if (bufferLevel >= 0)
	{
		int getAS = GetSamplesRemaining(), getASPrev = 0;
#ifdef _DEBUG
#ifdef MTI_WINDOWS
		sprintf(scvtext, "\nSamplesRemaining = %d\tbufferLevel = %d", getAS, bufferLevel);
		OutputDebugStringA(scvtext);
#endif
#endif
		while (getAS > bufferLevel || (getAS < 0))
		{
			executionTimeMs = 1000.f * (float)(getAS - bufferLevel) / (float)m_SampleRate;
			waitTime = std::max(50, (int)std::floor(executionTimeMs * 0.4));
			Sleep(waitTime);
			getAS = GetSamplesRemaining();
#ifdef _DEBUG
#ifdef MTI_WINDOWS
			sprintf(scvtext,
					"\nSamplesRemaining = %d\tbufferLevel = %d\tdifference = %d\twaitTime = "
					"%d\tTime = %3.2f[ms]",
					getAS, bufferLevel, getASPrev - getAS, waitTime,
					1000.f * (float)(getASPrev - getAS) / (float)m_SampleRate);
			OutputDebugStringA(scvtext);
#endif
#endif
			getASPrev = getAS;
		}
	}
}

void PlayzerX::SendDataXYM(float x, float y, unsigned char m, int bufferLevelToSend)
{
	if (!m_SerialDevice)
	{
		m_LastError = PlayzerXError::ERROR_CONNECTION;
		return;
	}

	float xp[1], yp[1];
	unsigned char mp[1];
	xp[0] = x;
	yp[0] = y;
	mp[0] = m;
	SendDataXYM(xp, yp, mp, 1, bufferLevelToSend);

	m_LastError = PlayzerXError::SUCCESS;
}

void PlayzerX::SendDataXYM(float* x, float* y, unsigned char* m, unsigned int numSamples,
						   int bufferLevelToSend)
{
	if (!m_SerialDevice)
	{
		m_LastError = PlayzerXError::ERROR_CONNECTION;
		return;
	}

	numSamples = std::min(numSamples, kMaxSendSamples);

	unsigned int i;
	int mBytesPerSample = 9;

	m_CommandBytes.clear();
	m_CommandBytes.resize(mBytesPerSample * numSamples);

	// Populate buffer starting with closest origin point
	unsigned int xVal = 0, yVal = 0, mVal = 0;
	unsigned count = 0;
	// 12 bit X, 12 bit Y, 8 bit M
	// BYTES as they arrive to controller: X[7:0] {X[11:8], Y[3:0]} Y[11:4] M[7:0]
	for (i = 0; i < numSamples; i++)
	{
		xVal = (unsigned int)((x[i] + 1.f) * 2047.5f);
		yVal = (unsigned int)((y[i] + 1.f) * 2047.5f);
		mVal = m[i];
		m_CommandBytes[count++] = 'p';
		m_CommandBytes[count++] = 'l';
		m_CommandBytes[count++] = 'd';
		m_CommandBytes[count++] = mBytesPerSample;
		m_CommandBytes[count++] = (xVal & 0x00FF);
		m_CommandBytes[count++] = (((xVal & 0x0F00) >> 4) + (yVal & 0x000F));
		m_CommandBytes[count++] = ((yVal & 0x0FF0) >> 4);
		m_CommandBytes[count++] = mVal;
		m_CommandBytes[count++] = 10;  // including suffix here!
	}

	WaitForBufferLevel(bufferLevelToSend);

	unsigned int numWritten;
	long serialError = m_SerialDevice->Write(&m_CommandBytes[0], count, &numWritten, 2000);
#ifdef _DEBUG
#ifdef MTI_WINDOWS
	sprintf(scvtext, "\nWrite count = %d bytes | Written = %d bytes | serialError = %d", count,
			numWritten, serialError);
	OutputDebugStringA(scvtext);
#endif
#endif
}

void PlayzerX::SendDataXYM(std::vector<float>& x, std::vector<float>& y,
						   std::vector<unsigned char>& m, int bufferLevelToSend)
{
	if (m_SerialDevice == nullptr)
	{
		m_LastError = PlayzerXError::ERROR_CONNECTION;
		return;
	}

	unsigned int length = std::min((unsigned int)x.size(), (unsigned int)y.size());
	length = std::min(length, (unsigned int)m.size());
	SendDataXYM(&x[0], &y[0], &m[0], length, bufferLevelToSend);

	m_LastError = PlayzerXError::SUCCESS;
}

void PlayzerX::SendDataXY(float x, float y, int bufferLevelToSend)
{
	if (m_SerialDevice == nullptr)
	{
		m_LastError = PlayzerXError::ERROR_CONNECTION;
		return;
	}

	float xp[1], yp[1];
	xp[0] = x;
	yp[0] = y;
	SendDataXY(xp, yp, 1, bufferLevelToSend);

	m_LastError = PlayzerXError::SUCCESS;
}

void PlayzerX::SendDataXY(float* x, float* y, unsigned int numSamples, int bufferLevelToSend)
{
	if (m_SerialDevice == nullptr)
	{
		m_LastError = PlayzerXError::ERROR_CONNECTION;
		return;
	}

	numSamples = std::min(numSamples, kMaxSendSamples);

	unsigned int i;
	int mBytesPerSample = 8;

	m_CommandBytes.clear();
	m_CommandBytes.resize(mBytesPerSample * numSamples);

	// Populate buffer starting with closest origin point
	unsigned int xVal = 0, yVal = 0, mVal = 0, rVal = 0, gVal = 0, bVal = 0, rgbVal = 0;
	unsigned count = 0;
	// 12 bit X, 12 bit Y
	// BYTES as they arrive to controller: X[7:0] {X[11:8], Y[3:0]} Y[11:4]
	for (i = 0; i < numSamples; i++)
	{
		xVal = (unsigned int)((x[i] + 1.f) * 2047.5f);
		yVal = (unsigned int)((y[i] + 1.f) * 2047.5f);
		m_CommandBytes[count++] = 'p';
		m_CommandBytes[count++] = 'l';
		m_CommandBytes[count++] = 'D';
		m_CommandBytes[count++] = mBytesPerSample;
		m_CommandBytes[count++] = (xVal & 0x00FF);
		m_CommandBytes[count++] = (((xVal & 0x0F00) >> 4) + (yVal & 0x000F));
		m_CommandBytes[count++] = ((yVal & 0x0FF0) >> 4);
		m_CommandBytes[count++] = 10;  // including suffix here!
	}

	WaitForBufferLevel(bufferLevelToSend);

	unsigned int numWritten;
	long serialError = m_SerialDevice->Write(&m_CommandBytes[0], count, &numWritten, 2000);
#ifdef _DEBUG
#ifdef MTI_WINDOWS
	sprintf(scvtext, "\nWrite count = %d bytes | Written = %d bytes | serialError = %d", count,
			numWritten, serialError);
	OutputDebugStringA(scvtext);
#endif
#endif
}

void PlayzerX::SendDataXY(std::vector<float>& x, std::vector<float>& y, int bufferLevelToSend)
{
	if (!m_SerialDevice)
	{
		m_LastError = PlayzerXError::ERROR_CONNECTION;
		return;
	}

	unsigned int length = std::min((unsigned int)x.size(), (unsigned int)y.size());
	length = std::min(length, (unsigned int)x.size());
	SendDataXY(&x[0], &y[0], length, bufferLevelToSend);

	m_LastError = PlayzerXError::SUCCESS;
}

void PlayzerX::SendDataXYRGB(float x, float y, unsigned char r, unsigned char g, unsigned char b,
							 int bufferLevelToSend)
{
	if (!m_SerialDevice)
	{
		m_LastError = PlayzerXError::ERROR_CONNECTION;
		return;
	}

	float xp[1], yp[1];
	unsigned char rp[1], gp[1], bp[1];
	xp[0] = x;
	yp[0] = y;
	rp[0] = r;
	gp[0] = g;
	bp[0] = b;
	SendDataXYRGB(xp, yp, rp, gp, bp, 1, bufferLevelToSend);

	m_LastError = PlayzerXError::SUCCESS;
}

void PlayzerX::SendDataXYRGB(float* x, float* y, unsigned char* r, unsigned char* g,
							 unsigned char* b, unsigned int numSamples, int bufferLevelToSend)
{
	if (!m_SerialDevice)
	{
		m_LastError = PlayzerXError::ERROR_CONNECTION;
		return;
	}

	numSamples = std::min(numSamples, kMaxSendSamples);

	unsigned int i;
	int mBytesPerSample = 11;

	m_CommandBytes.clear();
	m_CommandBytes.resize(mBytesPerSample * numSamples);

	// Populate buffer
	unsigned int xVal = 0, yVal = 0, rVal = 0, gVal = 0, bVal = 0;
	unsigned count = 0;
	// 12 bit X, 12 bit Y, 8 bit R, 8 bit G, 8 bit B
	// BYTES as they arrive to controller: X[7:0] {X[11:8], Y[3:0]} Y[11:4] R[7:0] G[7:0] B[7:0]
	for (i = 0; i < numSamples; i++)
	{
		xVal = (unsigned int)((x[i] + 1.f) * 2047.5f);
		yVal = (unsigned int)((y[i] + 1.f) * 2047.5f);
		rVal = r[i];
		gVal = g[i];
		bVal = b[i];
		m_CommandBytes[count++] = 'p';
		m_CommandBytes[count++] = 'l';
		m_CommandBytes[count++] = 'd';
		m_CommandBytes[count++] = mBytesPerSample;
		m_CommandBytes[count++] = (xVal & 0x00FF);
		m_CommandBytes[count++] = (((xVal & 0x0F00) >> 4) + (yVal & 0x000F));
		m_CommandBytes[count++] = ((yVal & 0x0FF0) >> 4);
		m_CommandBytes[count++] = rVal;
		m_CommandBytes[count++] = gVal;
		m_CommandBytes[count++] = bVal;
		m_CommandBytes[count++] = 10;  // Include suffix here!
	}

	WaitForBufferLevel(bufferLevelToSend);

	unsigned int numWritten;
	long serialError = m_SerialDevice->Write(&m_CommandBytes[0], count, &numWritten, 2000);
#ifdef _DEBUG
#ifdef MTI_WINDOWS
	sprintf(scvtext, "\nWrite count = %d bytes | Written = %d bytes | serialError = %d", count,
			numWritten, serialError);
	OutputDebugStringA(scvtext);
#endif
#endif
}

void PlayzerX::SendDataXYRGB(std::vector<float>& x, std::vector<float>& y,
							 std::vector<unsigned char>& r, std::vector<unsigned char>& g,
							 std::vector<unsigned char>& b, int bufferLevelToSend)
{
	if (!m_SerialDevice)
	{
		m_LastError = PlayzerXError::ERROR_CONNECTION;
		return;
	}

	unsigned int length = std::min((unsigned int)x.size(), (unsigned int)y.size());
	length = std::min(length, (unsigned int)r.size());
	length = std::min(length, (unsigned int)g.size());
	length = std::min(length, (unsigned int)b.size());
	SendDataXYRGB(&x[0], &y[0], &r[0], &g[0], &b[0], length, bufferLevelToSend);

	m_LastError = PlayzerXError::SUCCESS;
}

void PlayzerX::ClearData()
{
	if (!m_SerialDevice)
	{
		m_LastError = PlayzerXError::ERROR_CONNECTION;
		return;
	}

	unsigned char sendData[20];
	sendData[0] = 'p';
	sendData[1] = 'l';
	sendData[2] = 'c';
	sendData[3] = 5;
	sendData[4] = 10;  // Include suffix here!
	long serialError = m_SerialDevice->Write(sendData, 5, 0, 200);
	if (serialError == 0)
		m_LastError = PlayzerXError::SUCCESS;
	else
		m_LastError = PlayzerXError::ERROR_GENERAL;
}

void PlayzerX::SetSampleRate(unsigned int sampleRate)
{
	if (!m_SerialDevice)
	{
		m_LastError = PlayzerXError::ERROR_CONNECTION;
		return;
	}
	sampleRate = std::min(std::max(sampleRate, 200u), 50000u);
	unsigned char sendData[20];
	sendData[0] = 'p';
	sendData[1] = 'l';
	sendData[2] = 'r';
	sendData[3] = 8;
	sendData[4] = (sampleRate & 0x0000FF);
	sendData[5] = (sampleRate & 0x00FF00) >> 8;
	sendData[6] = (sampleRate & 0xFF0000) >> 16;
	sendData[7] = 10;  // Include suffix here!
	long serialError = m_SerialDevice->Write(sendData, 8, 0, 200);
	if (serialError == 0) m_SampleRate = sampleRate;
}

void PlayzerX::SetBufferUpdateTimer(unsigned int bufferUpdateTimer)
{
	if (!m_SerialDevice)
	{
		m_LastError = PlayzerXError::ERROR_CONNECTION;
		return;
	}

	m_StreamingSamplesRemaining = (bufferUpdateTimer > 0);
	int updateRateLoops = std::min(std::max(bufferUpdateTimer, 0u), 1000u);

	unsigned char sendData[20];
	sendData[0] = 'p';
	sendData[1] = 'l';
	sendData[2] = 'u';
	sendData[3] = 7;
	sendData[4] = (updateRateLoops & 0x000000FF);
	sendData[5] = (updateRateLoops & 0x0000FF00) >> 8;
	sendData[6] = 10;  // Include suffix here!
	long serialError = m_SerialDevice->Write(sendData, 7, 0, 200);

	// If there is a change of this setting we will clear the serial data already sent
	// clear all data previously sent by the controller to the host
	unsigned char data[MTI_SERIAL_QUEUE_SIZE];
	m_SerialDevice->Read(data, MTI_SERIAL_QUEUE_SIZE, 0, 0, MTI_BLOCKING_MODE_OFF);
}

bool PlayzerX::GetDeviceInfo()
{
	if (!m_SerialDevice)
	{
		m_LastError = PlayzerXError::ERROR_CONNECTION;
		return false;
	}

	unsigned char sendData[10];
	sendData[0] = 'p';
	sendData[1] = 'l';
	sendData[2] = 'n';
	sendData[3] = 5;
	sendData[4] = 10;  // Include suffix here!
	long serialError = m_SerialDevice->Write(sendData, 5, 0, 200);
	if (serialError != 0) return false;

	char dataconf[128];
	unsigned char delineationChar = 0x0A;
	long lastError;
	// Enforce byte alignment by sliding until finding the complete preamble
	lastError = m_SerialDevice->ReadText(dataconf, delineationChar, m_TimeOut);
	if (strcmp(dataconf, "pl-info")) return false;
	lastError = m_SerialDevice->ReadText(dataconf, delineationChar, m_TimeOut);
	std::string reply1(dataconf);
	m_DeviceName = reply1;
	lastError = m_SerialDevice->ReadText(dataconf, delineationChar, m_TimeOut);
	std::string reply2(dataconf);
	m_FirmwareName = reply2;
	lastError = m_SerialDevice->ReadText(dataconf, delineationChar, m_TimeOut);
	if (lastError == 0)
	{
		std::string reply3(dataconf);
		m_DataFormat = reply3;
	}
	else
		m_DataFormat = "XYM";
	m_RGBCapable = (m_DataFormat == "XYRGB");

	if (lastError == 0)
		return true;
	else
		return false;
};

bool PlayzerX::IsDeviceConnected()
{
	if (!m_SerialDevice)
	{
		m_LastError = PlayzerXError::ERROR_CONNECTION;
		return false;
	}

	// let's purge serial buffers before this ping to controller
	PurgeSerialBuffers();

	unsigned char sendData[10];
	sendData[0] = 'p';
	sendData[1] = 'l';
	sendData[2] = 'p';
	sendData[3] = 5;
	sendData[4] = 10;  // Include suffix here!
	long serialError = m_SerialDevice->Write(sendData, 5, 0, 200);
	if (serialError != 0) return false;

	char dataconf[100];
	unsigned char delineationChar = 0x0A;
	long lastError;
	// Enforce byte alignment by sliding until finding the complete preamble
	lastError = m_SerialDevice->ReadText(dataconf, delineationChar, m_TimeOut);
	if (strcmp(dataconf, "pl-ok"))
		return false;
	else
		return true;
}

void PlayzerX::PurgeSerialBuffers()
{
	if (!m_SerialDevice)
	{
		m_LastError = PlayzerXError::ERROR_CONNECTION;
		return;
	}

	long serialError = m_SerialDevice->Purge();
	if (serialError != 0) m_LastError = PlayzerXError::ERROR_GENERAL;
}

void PlayzerX::ResetDevice()
{
	if (!m_SerialDevice)
	{
		m_LastError = PlayzerXError::ERROR_CONNECTION;
		return;
	}
	unsigned char sendData[10];
	sendData[0] = 'p';
	sendData[1] = 'l';
	sendData[2] = 'b';
	sendData[3] = 6;
	sendData[4] = 0xEE;
	sendData[5] = 10;  // Include suffix here!
	long serialError = m_SerialDevice->Write(sendData, 6, 0, 200);
	if (serialError == 0)
		m_LastError = PlayzerXError::SUCCESS;
	else
		m_LastError = PlayzerXError::ERROR_GENERAL;
	if (m_LastError == PlayzerXError::SUCCESS)
	{
		// we assume that the command was sent and unit reset successfully
		// then we can assume unit is in default mode not streaming SamplesRemaining
		m_StreamingSamplesRemaining = false;
		// let's pause any execution for 2.5s, for sure unit is not available
		Sleep(2500);
	}
}

void PlayzerX::SwitchPlayzerXMode(bool enableAINMode, bool flashBoot)
{
	if (!m_SerialDevice)
	{
		m_LastError = PlayzerXError::ERROR_CONNECTION;
		return;
	}
	unsigned char sendData[10];
	sendData[0] = 'p';
	sendData[1] = 'l';
	sendData[2] = (flashBoot) ? 'I' : 'i';
	sendData[3] = 8;
	if (enableAINMode)
	{
		sendData[4] = 'a';
		sendData[5] = 'i';
		sendData[6] = 'n';
	}
	else
	{
		sendData[4] = 'u';
		sendData[5] = 's';
		sendData[6] = 'b';
	}
	sendData[7] = 10;  // Include suffix here!
	long serialError = m_SerialDevice->Write(sendData, 8, 0, 200);
	if (serialError == 0)
		m_LastError = PlayzerXError::SUCCESS;
	else
		m_LastError = PlayzerXError::ERROR_GENERAL;
}

void PlayzerX::GetAvailableDevices(PlayzerXAvailableDevices& plad)
{
	plad.NumDevices = 0;

	// Pasted from GetAvailablePorts
	unsigned int NumPorts = 0;
	char portname[32];
	unsigned int AvailablePortNumbers[MTI_SERIAL_MAXPORTS];

#ifdef MTI_WINDOWS
	CEnumerateSerial::CPortsArray ports;
	CEnumerateSerial::UsingQueryDosDevice(ports);

	for (int i = 0; i < ports.GetSize(); i++) AvailablePortNumbers[NumPorts++] = ports[i];
#else
	for (unsigned int i = 0; i < MTI_SERIAL_MAXPORTS; i++)
	{
		sprintf(portname, MTI_PORT_PREFIX "%d", i);
		if (MTISerialIO::IsPortAvailable(portname)) AvailablePortNumbers[NumPorts++] = i;
	}
#endif

	if (NumPorts == 0) return;

	char s[16];
	long lastError = 0;
	unsigned int NumDevices = 0;

	m_CommandBytes.clear();
	m_CommandBytes.resize(5);

	for (unsigned int i = 0; i < NumPorts; i++)
	{
		m_SerialDevice = new MTISerialIO;
		sprintf(s, MTI_PORT_PREFIX "%d", AvailablePortNumbers[i]);
		lastError = m_SerialDevice->Open(s, kBaudRate);
		if (lastError == 0)
		{
			m_CommandBytes[0] = 0x0A;
			m_CommandBytes[1] = 0x0A;
			m_CommandBytes[2] = 0x0C;
			m_CommandBytes[3] = 0x0A;
			m_CommandBytes[4] = 0x0A;
			// Send 5 carriage returns
			lastError = m_SerialDevice->Write(&m_CommandBytes[0], 5, 0, 200);
			// Send 5 carriage returns
			lastError = m_SerialDevice->Write(&m_CommandBytes[0], 5, 0, 200);
			int keepTimeOut = m_TimeOut;
			m_TimeOut = 200;
			// Check if connected and query target information
			bool isResponding = IsDeviceConnected();
			if (isResponding)
			{
				bool success = GetDeviceInfo();
				plad.DeviceName[NumDevices] = m_DeviceName;
				plad.FirmwareName[NumDevices] = m_FirmwareName;
				plad.USARTBaudRate[NumDevices] = kBaudRate;
				plad.CommPortNumber[NumDevices] = AvailablePortNumbers[i];
				plad.CommPortName[NumDevices].assign(MTI_PORT_DISPLAY);
				plad.CommPortName[NumDevices].append(std::to_string(AvailablePortNumbers[i]));
				NumDevices++;
			}
			DisconnectDevice();
			m_TimeOut = keepTimeOut;
		}
		DisconnectDevice();
		PurgeSerialBuffers();
		SAFE_DELETE(m_SerialDevice);
	}
	plad.NumDevices = NumDevices;
	return;
}

void PlayzerX::ListAvailableDevices(PlayzerXAvailableDevices& plad)
{
	printf("\n");
	for (unsigned int i = 0; i < plad.NumDevices; i++)
		printf("PlayzerX Device:" TXT_GRN " %s " TXT_RST " is at serial port " TXT_GRN "%s" TXT_RST
			   "\n",
			   plad.DeviceName[i].c_str(), plad.CommPortName[i].c_str());
}

}  // namespace playzerx