//////////////////////////////////////////////////////////////////////
// PlayzerXStub.cpp
// Version: 2.0.1.0
//////////////////////////////////////////////////////////////////////

#include "PlayzerXStub.h"

namespace playzerx
{

void PlayzerXStub::ConnectDevice(const std::string &port_name)
{
	// Do nothing
	m_DeviceName = "DeviceStub";
	m_LastError = PlayzerXError::SUCCESS;
}

void PlayzerXStub::ConnectDevice() { ConnectDevice(""); }

void PlayzerXStub::DisconnectDevice()
{
	// Do nothing
	m_LastError = PlayzerXError::SUCCESS;
}

void PlayzerXStub::StartDevice()
{
	// Do nothing
	m_LastError = PlayzerXError::SUCCESS;
}

void PlayzerXStub::StopDevice()
{
	// Do nothing
	m_LastError = PlayzerXError::SUCCESS;
}

void PlayzerXStub::SendData(float x, float y, unsigned char m, unsigned int bufferLevelForSend)
{
	// Do nothing
	m_LastError = PlayzerXError::SUCCESS;
}

}  // namespace playzerx