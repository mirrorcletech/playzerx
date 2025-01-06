//////////////////////////////////////////////////////////////////////
// MTIDevice
// Version: 11.1.1.0
//////////////////////////////////////////////////////////////////////

#ifndef MTI_DEVICE_H
#define MTI_DEVICE_H

///////////////////////////////////////////////////////////////////////////////////

#include "MTIDataGenerator.h"
#include "MTISerial.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MTIDeviceParams
{
public:
	bool Success;						//read only
	char DeviceName[32];
	char FirmwareName[16];				//read only
	char CommType[16];					//read only
	char BluetoothMAC[16];				//read only
	unsigned int HardwareFilterOn;		//read only
	float VmaxMEMSDriver;				//read only
	unsigned int DeviceState;			//read only
	unsigned int DeviceErrorRegister;	//read only
	unsigned int USARTBaudRate;
	unsigned int SampleRate;
	unsigned int HardwareFilterBw;
	float VdifferenceMax;
	float Vbias;
	float XOffset;
	float YOffset;
	float DataScale;
	float DataRotation;
	unsigned int MEMSDriverEnable;
	unsigned int DigitalOutputEnable;
	unsigned int LaserModulationEnable;
	unsigned int BufferOffset;
	MTIAxes DeviceAxes;
	MTIBoot BootSetting;
	MTIDataMode DataMode;
	MTISync SyncMode;
	MTIDataFormat DataFormat;
	struct 
	{
		unsigned int SampleRate_Min;					// read only
		unsigned int SampleRate_Max;					// read only
		unsigned int SamplesPerFrame_Min;				// read only
		unsigned int SamplesPerFrame_Max;				// read only
		unsigned int HardwareFilterBw_Min;				// read only
		unsigned int HardwareFilterBw_Max;				// read only
		unsigned int FramesPerSecond_Min;				// read only
		unsigned int FramesPerSecond_Max;				// read only
		unsigned int VdifferenceMax_Min;				// read only
		unsigned int VdifferenceMax_Max;				// read only
		unsigned int Vbias_Min;							// read only
		unsigned int Vbias_Max;							// read only
	} DeviceLimits;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MTIAvailableDevices
{
public:
	char DeviceName[MTI_MAXDEVICES][32];				//read only
	char FirmwareName[MTI_MAXDEVICES][16];				//read only
	char CommType[MTI_MAXDEVICES][16];					//read only
	unsigned int USARTBaudRate[MTI_MAXDEVICES];			//read only
	unsigned int CommPortNumber[MTI_MAXDEVICES];		//read only
	char CommPortName[MTI_MAXDEVICES][10];				//read only
	float VmaxMEMSDriver[MTI_MAXDEVICES];				//read only
	unsigned int NumDevices;							//read only
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MTITrackParams
{
public:
	bool Success;
	float Threshold;
	int NormalGain;
	int TangentialGain;
	unsigned int BufferDelay;
	float HitRatio;
	unsigned int EnableSearch;
	unsigned int EnableOffsetStreaming;
	unsigned int EnableTrack;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MTITrackStatus
{
public:
	bool Success;
	bool TrackLocked;
	float XOffset;
	float YOffset;
	unsigned int HitCount;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
class MTIDeviceImpl;

class DLLEXPORT MTIDevice
{
public:

	/////// MTI Device ///////
	
	/** @name CREATION AND TERMINATION
	*  Constructor and Destructor of the MTIDevice class
	*/
	//@{
	MTIDevice();

	~MTIDevice();

	/*!
	Deletes the device object from the memory
	\verbatim embed:rst
	.. deprecated:: 11.0.0.0
	   Use :code:`delete` or allow MTIDevice object to go out of scope
	\endverbatim
	*/
	void DeleteDevice();						
	//@}

	/** @name SEARCH AND CONNECTION
	*  Open the COM port and connect to the addressed device.
	*/
	//@{

	void ConnectDevice( char* portName );		//!< Opens the serial (COM) port and connects to the addressed device.

	void ConnectDevice( MTISerialIO* socket );	//!< Connects to device at a previously connected MTISerialIO socket.

	void ConnectDevice( void );					//!< Connects to first available (compatible) device (at lowest available port number).

	void DisconnectDevice();					//!< Stops communication and closes the serial (COM) port.

	void GetAvailableDevices( MTIAvailableDevices* table );	//!< Searches for available devices to connect to at the host and returns
															//!< a table of the devices with port settings. 
	MTIAvailableDevices* GetAvailableDevices();	//!< DEPRECATED. Searches for available devices to connect to at the host and returns
												//!< a table of the devices with port settings. 

	//! Lists all available devices on COM Ports in a console window / terminal.
	//! @param[in]	table	The MTIAvailableDevices* object returned by GetAvailableDevices.
	void ListAvailableDevices( MTIAvailableDevices* table );
	//@}

	void GetAPIVersion(std::string *version);	//!< Returns string for the MTIDevice API version number in x.y.z.w format

	/** @name OPERATION
	*  Methods involved in operating the MEMS device
	*/
	//@{

	void StopDataStream();		//!< Stops the data output operation of the device. 
								//!< Sampling stops asynchronously as soon as command is received by device.
								//!< Outputs remain at values of last sample X, Y, M (or X, Y, R, G, B etc.)
								//!< and object obtains info about the last outputted sample.					

	//! SendDataStream sends (X,Y) positional coordinates, 8-bit Digital Output numbers, and additional settings
	//! for the output operation to the Controller.
	//!
	//! @param[in]	x	The array of X positional coordinates (Cartesian) to be sent with range of -1 to 1
	//! @param[in]	y	The array of Y positional coordinates (Cartesian) to be sent with range of -1 to 1
	//! @param[in]	m	The array of 8-bit Digital Output numbers (unsigned) to be sent with range of 0-255.
	//!
	//! @param[in]	numSamples	Number of samples to be sent to the Controller. 
	//!		Range is from 1 to the length of the passed X/Y/M arrays (or maximum buffer size for the Controller).
	//!
	//! @param[in]	delaySamples	delaySamples is a parameter that allows user to adjust output timing of the
	//!		digital output with respect to the X,Y outputs by delaying the digital output by a number of samples.
	//!		delaySamples should be a value between 0 and numSamples-1 which is maximum amount of m-samples rotation possible.
	//!		When value is 0, there is internal delay added to compensate for the hardware filter's group delay regardless,
	//!		so this compensation can be increased by this parameter.
	//!
	//! @param[in]	minimizeJump	A true/false value that dictates whether the function will rotate the data stream so that the distance
	//!		between the first sample in the new stream is closest to the last sample of the previously sent stream (or last sample outputted before a StopDataStream). 
	//!		This is done to minimize the jump and, in turn, any excitation of resonance when transitioning from one stream to next.
	//!
	//! @param[in]	compensateFilterDelay	A true/false value that dictates whether the function will rotate the m data (digital output) with
	//!		respect to the x,y data based on the estimated group delay of the hardware low-pass filter onboard the Controller. Default value  
	//!		is true: function will internally delay (buffer rotate) the m samples before streaming to the Controller. When false, the x,y,m
	//!		data will stream to the Controller exactly as arranged by the user in the function call.
	//!
	//! By default, the device will start outputting the data after everything is received. To prevent this, preface this function
	//! with a call to StopDataStream().
	void SendDataStream( float* x, float* y, unsigned char* m, unsigned int numSamples, unsigned int delaySamples = 0, bool minimizeJump = true, bool compensateFilterDelay = true );

	//! Starts the output operation of the device. 
	//! @param	repeatCount	The number of times the Controller should output the data stream.
	//!		If repeatCount is 0 or less, the Controller will output the data stream infinitely until stopped.
	//!	@param	confirmOnComplete	If confirmOnComplete = false, the function will not wait for a receipt from the device that the data stream was outputted.
	//!		confirmOnComplete is only useful when repeatCount is not -1.
	void StartDataStream( int repeatCount = -1, bool confirmOnComplete = true );

	//! Resets the MEMS device position back to its origin.  It is a special case of GoToDevicePosition with (X, Y) = (0, 0)
	//! ResetDevicePosition function also resets any output offsets to (0, 0)
	//! For more details see GoToDevicePosition
	void ResetDevicePosition();
	
	//! MEMS device will be stopped and position will be changed from stopped position (XStop, YStop) to new (X, Y)
	//! with a step of a specific duration of time, specified by user in as (mSec) in milliseconds.
	//!
	//! @param[in]	x, y	The (X,Y) Cartesian coordinates, with range of -1 to 1 
	//! @param[in]	m	The modulation data, with range of 0-255
	//! @param[in]	mSec	The duration of the step between (XStop, YStop) and new (X, Y).
	//!		Range of mSec is 1 to 5000 except in special case where 0 is acceptable. Use mSec=0 to move the device with minimal step time. 
	//!		In the case of mSec=0, only a single step is streamed and mirror will step in the time limited only by the HardwareFilterBw - therefore
	//!		this can be potentially damage or destroy the device if hardware filter bandwidth is not set to recommended or reasonably low limit.
	//!
	//! It should be noted that the step happens after approx. 100-200ms of communication and processing.
	//! If step time parameter (mSec) is omitted in the function call, the function will use a default value of 5ms.
	//! Note that the actual step time of the device from stopped position to new position is ultimately a function
	//! of both the stated (mSec) parameter and the bandwidth setting of the hardware filters or HardwareFilterBw
	//! which may additionally increase the step time.
	//!
	//! Furthermore, if the requested step time is too short for a given MEMS mirror design, and HardwareBandwidthBw
	//! is too high, the device may take a longer time to settle into new position while overshooting and ringing about the position.
	void GoToDevicePosition( float x, float y, unsigned char m, unsigned int mSec = 5 );

	//! Set MTIDevice::m_iRGBData array to an array of RGB values provided by this function
	//! @param[in]	rgbData	The array of RGB samples to be set in MTIDevice object's RGBData array
	//! @param[in]	numSamples	The number of samples from provided array to be copied to MTIDevice::m_iRGBData
	//!		Typically the length of the prepared data.
	//!
	//! Each rgbData unsigned int has 4 bytes with the format 0x00RRGGBB
	//! (most significant byte is not used, then 1 byte of red, 1 byte of green and 1 byte of blue)
	//! Each color's byte is a value from 0 to 255
	//! The RGBData will not be sent to the Controller until MTIDevice::SendDataStream is called.
	void SetRGBData( unsigned int* rgbData, unsigned int numSamples );

	//! Populate MTIDevice::m_iRGBData member with a single RGB color value provided by this function
	//! @param[in]	rgbColor	The single RGB value to populate the MTIDevice's RGBData array
	//! @param[in]	numSamples	The number of samples from provided array to be copied to MTIDevice::m_iRGBData
	//!		Typically the length of the prepared data.
	//!
	//! Each rgbData unsigned int has 4 bytes with the format 0x00RRGGBB
	//! (most significant byte is not used, then 1 byte of red, 1 byte of green and 1 byte of blue)
	//! Each color's byte is a value from 0 to 255
	//! The RGBData will not be sent to the Controller until MTIDevice::SendDataStream is called.
	void SetRGBData( unsigned int rgbColor, unsigned int numSamples = 0 );

	//! Apply gamma corrections to the m_iRGBData array. Gamma correction is only applied once SendDataStream is called and only to
	//! the RGB samples sent to the Controller. The m_iRGBData array will NOT be altered to reflect the gamma correction.
	void SetRGBGammas( float gamma_R, float gamma_G, float gamma_B );

	//! Get MTIDevice object's RGBData array
	//! @param[out]	rgbData	The pre-allocated array for the method to return values in the MTIDevice's RGBData array
	//! @param[in]	numSamples	The length of samples in the MTIDevice's RGBData array to get
	//!
	//! Each rgbData unsigned int has 4 bytes with the format 0x00RRGGBB
	//! (most significant byte is not used, then 1 byte of red, 1 byte of green and 1 byte of blue)
	//! Each color's byte is a value from 0 to 255
	void GetRGBData( unsigned int* rgbData, unsigned int numSamples );

	void SaveToFlash( MTIFlash flashsave );		//!< Sends settings to the target to save params, data, or both to flash memory.
	//@}

	/** @name CONTROL AND STATUS
	*  Resetting and querying the Controller for status
	*/
	//@{
	
	void ClearInputBuffer();	//!< Clears the input buffer of the host computer from any data previously received from device at serial port. 
	
	void SendSerialReset();		//!< Clears the SerialIO buffers on host and device and resets the communication. 
	
	void ResetDevice();			//!< Resets/restarts the MCU (processor) on the connected device (Controller).

	float GetXStop();			//!< Returns the last outputted X point where device stopped (after StopDataStream) or last X point sent to controller (after SendDataStream).

	float GetYStop();			//!< Returns the last outputted Y point where device stopped (after StopDataStream) or last Y point sent to controller (after SendDataStream).

	unsigned char GetMStop();	//!< Returns the last outputted M digital output where device stopped (after StopDataStream) or last M point sent to controller (after SendDataStream).

	unsigned int GetIStop();	//!< Returns the buffer index I of data sample where device stopped (after StopDataStream) or index I of last sample sent to controller (after SendDataStream).

	bool IsDeviceConnected();	//!< Verifies if a device socket is non-NULL - should be true when serial socket is connected 
	
	bool IsDeviceResponding();	//!< Verifies if a device is responding to serial commands - a ping of the Controller

	bool IsDeviceRunning();		//!< Verifies if the output operation is presently running and generating samples.

	bool IsTrackingSupported();	//!< Verifies if the Controller's firmware supports tracking functionality.

	MTIError GetLastError();	//!< Returns the last error. Called after each communication with the device to verify correct transfer.

	void SetDefaultDeviceName( char* text, const std::string& pass );
	//@}

	/** @name DEVICE PARAMS
	*  Methods for getting and setting Controller parameters
	*/
	//@{
	
	void GetDeviceParams( MTIDeviceParams* params );			//!< Get all device parameters from the device (Writable settings and read only firmware values).
	MTIDeviceParams* GetDeviceParams();							//!< DEPRECATED. Get all device parameters from the device (Writable settings and read only firmware values).

	void SetDeviceParams( MTIDeviceParams* params );			//!< Sends the complete device parameter structure to the device (including all settings).

	float GetDeviceParam( MTIParam param, int paramID = 0 );	//!< Gets a single parameter from the device.

	void SetDeviceParam( MTIParam param, float paramValue1, float paramValue2 = 0.f ); //!< Sets a single parameter on the device. 

	void LoadDeviceParams( MTIDeviceParams* params, const char* fileName );	//!< Loads a single or multiple device parameters in the parameter structure from a file (by default "mtidevice.ini"). 
	MTIDeviceParams* LoadDeviceParams( const char* fileName );				//!< DEPRECATED. Loads a single or multiple device parameters in the parameter structure from a file (by default "mtidevice.ini"). 

	void SaveDeviceParams( 	char* fileName );					//!< Saves the complete device parameter structure into a file.

	std::string EnumToString(std::string enumName, int enumValue);	//!< For an enum name listed in MTIDefinitions.h returns the string based on integer value of the enum

	char* GetEnumString(const char* enumName, int enumValue);	//!< DEPRECATED. use EnumToString instead 

	int EnumToInt(std::string myEnum);	//!< For an enum name listed in MTIDefinitions.h returns integer value
	//@}

	/** @name TRACK PARAMS
	*  Methods for getting and setting Controller tracking parameters
	*/
	//@{
	
	void GetTrackParams( MTITrackParams* params );				//!< Get the device's tracking parameter structure from the device (requires tracking functionality).
	MTITrackParams* GetTrackParams();							//!< DEPRECATED. Get the device's tracking parameter structure from the device (requires tracking functionality).

	void SetTrackParams( MTITrackParams *params );				//!< Sends the complete device tracking parameter structure to the device (requires tracking functionality).

	void GetTrackIntegrals( int &TrackIntegralX, int &TrackIntegralY );	//!< Get back the integration values for tuning parameters

	void GetTrackStatus( MTITrackStatus* trackStatus );
	MTITrackStatus* GetTrackStatus();							// DEPRECATED
	//@}

	/** @name ANALOG INPUTS
	*  Methods for handling Analog Input
	*/
	//@{

	float GetAnalogInputValue( unsigned int ChannelNumber ); //!< Samples immediately one value from Analog Input Channel 0 or 1.
	
	void GetAnalogInputBuffer( float* AI0, float* AI1, unsigned int DataLength );				//!< Gets the sampled values from channel 1 or 2. (Obtained before via MTIDataMode parameter).

	unsigned int GetAnalogInputStream( float* AI0, float* AI1, unsigned int DataLength = 0 );	//!< Gets the sampled values from channel 1 or 2 as they stream.  Get as many as are available, return how many were obtained.

	unsigned int GetSamplesRemaining();		//!< While running, provides the number of samples that remain to be read from the buffer.
											//!< Equal to (length of the data buffer - the index position + 1) at the requested moment in time
	//@}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	std::unique_ptr<MTIDeviceImpl> m_pImpl;
};

///////////////////////////////////////////////////////////////////////////////////

#endif