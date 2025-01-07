//////////////////////////////////////////////////////////////////////
// PlayzerXStub.h
// Version: 2.0.1.0
//////////////////////////////////////////////////////////////////////

#ifndef PLAYZERX_STUB_H
#define PLAYZERX_STUB_H

#include "PlayzerX.h"

namespace playzerx
{

class PlayzerXStub : public PlayzerX
{
   public:
	//!< Opens the COM port and connects to the addressed device.
	void ConnectDevice(const std::string &portName);
	void ConnectDevice();

	//!< Stops communication and closes the COM port.
	void DisconnectDevice();

	// Scanning Device
	void StartDevice();
	void StopDevice();

	// Send data to the device
	void SendData(float x, float y, unsigned char m, unsigned int bufferLevelForSend);

}  // namespace playzerx

#endif	// !PLAYZERX_STUB_H