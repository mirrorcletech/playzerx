//////////////////////////////////////////////////////////////////////
// PlayzerX
// Version: 2.1.0.0
//////////////////////////////////////////////////////////////////////

/**
 * \file PlayzerX.h
 * \brief Defines the PlayzerX class and associated data structures.
 * \version 2.1.0.0
 *
 * The PlayzerX library facilitates communication with Mirrorcle Technologies, Inc.
 * Vector Graphics Laser Projection (VGLP) PlayzerX-compatible devices
 * via serial connections (COM ports). This file declares the main interface
 * for discovering and controlling PlayzerX devices.
 */

#ifndef PLAYZERX_H
#define PLAYZERX_H

#include <string>
#include <vector>
#include <algorithm>

/**
 * \brief Global buffer for text debug/log messages.
 * \note This global buffer is shared across the application.
 */
char scvtext[100];

#include "MTISerial.h"
#include "PlayzerXDefinitions.h"

namespace playzerx
{
/**
 * \class PlayzerXAvailableDevices
 * \brief Holds details for all discovered PlayzerX devices on the system.
 *
 * Objects of this class store metadata for each detected device, including the
 * device's name, firmware name, data format, and COM port information.
 */
class PlayzerXAvailableDevices
{
   public:
	/** \brief Read-only array of device names. */
	std::string DeviceName[MTI_MAXDEVICES];
	/** \brief Read-only array of firmware names. */
	std::string FirmwareName[MTI_MAXDEVICES];
	/** \brief Read-only array of data formats (e.g., "XYM" or "XYRGB"). */
	std::string DataFormat[MTI_MAXDEVICES];
	/** \brief Read-only array of UART baud rates for each device. */
	unsigned int USARTBaudRate[MTI_MAXDEVICES];
	/** \brief Read-only array of COM port numbers. */
	unsigned int CommPortNumber[MTI_MAXDEVICES];
	/** \brief Read-only array of COM port names (e.g., "COM3", "/dev/ttyUSB0"). */
	std::string CommPortName[MTI_MAXDEVICES];
	/** \brief Number of devices detected. */
	unsigned int NumDevices;
};

/**
 * \class PlayzerX
 * \brief Main interface to connect and control PlayzerX devices.
 *
 * The PlayzerX class provides high-level methods to manage a device's life cycle,
 * exchange data, and query status. It supports both XYM and XYRGB data formats.
 */
class DLLEXPORT PlayzerX
{
   public:
	/** \brief Factory method to create a new PlayzerX device object. */
	static PlayzerX* CreateDevice();
	/** \brief Safely deletes a PlayzerX device object created by CreateDevice(). */
	static void DeleteDevice(PlayzerX* device);

	/**
	 * \brief Default constructor.
	 */
	PlayzerX();

	/**
	 * \brief Destructor.
	 *
	 * Automatically disconnects and cleans up resources if still connected.
	 */
	~PlayzerX();

	/**
	 * \brief Opens the COM port and connects to the first detected device.
	 * \return A pointer to an MTISerialIO instance if successful, otherwise \c nullptr.
	 * \note This method uses the first available device from the discovery list.
	 */
	MTISerialIO* ConnectDevice();

	/**
	 * \brief Opens the COM port and connects to the device using a specified port name.
	 * \param portName The COM port name (e.g., "COM3", "/dev/ttyUSB0").
	 * \return A pointer to an MTISerialIO instance if successful, otherwise \c nullptr.
	 */
	MTISerialIO* ConnectDevice(const std::string& portName);

	/**
	 * \brief Opens the COM port and connects to the device using a C-string port name.
	 * \param portName The COM port name in a C-string format (e.g., "COM3").
	 * \return A pointer to an MTISerialIO instance if successful, otherwise \c nullptr.
	 */
	MTISerialIO* ConnectDevice(char* portName);

	/**
	 * \brief Checks if a device is currently connected.
	 * \return \c true if the device is connected; otherwise \c false.
	 */
	bool IsDeviceConnected();

	/**
	 * \brief Stops communication and closes the COM port.
	 */
	void DisconnectDevice();

	/**
	 * \brief Retrieves the current number of remaining (unsent) samples.
	 * \return Number of samples left in the device buffer, or \c -1 on failure.
	 */
	int GetSamplesRemaining();

	/**
	 * \brief Sets the timer interval (in milliseconds) to update buffer levels.
	 * \param bufferUpdateTimer A value greater than zero enables periodic buffering updates.
	 */
	void SetBufferUpdateTimer(unsigned int bufferUpdateTimer);

	/**
	 * \brief Blocks execution until the buffer level is below a specified threshold.
	 * \param bufferLevelToSend Desired buffer threshold, default is \c 25000.
	 */
	void WaitForBufferLevel(int bufferLevelToSend = 25000);

	/**
	 * \brief Sends a single XYM sample (X, Y, modulation).
	 * \param x Normalized X coordinate in the range [-1.0, 1.0].
	 * \param y Normalized Y coordinate in the range [-1.0, 1.0].
	 * \param m Modulation value (e.g., intensity), range [0, 255].
	 * \param bufferLevelToSend Desired buffer threshold to wait for before sending.
	 */
	void SendDataXYM(float x, float y, unsigned char m, int bufferLevelToSend = -1);

	/**
	 * \brief Sends multiple XYM samples in array form.
	 * \param x Pointer to array of normalized X coordinates.
	 * \param y Pointer to array of normalized Y coordinates.
	 * \param m Pointer to array of modulation values.
	 * \param numSamples Number of samples to send.
	 * \param bufferLevelToSend Desired buffer threshold to wait for before sending.
	 */
	void SendDataXYM(float* x, float* y, unsigned char* m, unsigned int numSamples,
					 int bufferLevelToSend = -1);

	/**
	 * \brief Sends multiple XYM samples using \c std::vector containers.
	 * \param x Vector of normalized X coordinates.
	 * \param y Vector of normalized Y coordinates.
	 * \param m Vector of modulation values.
	 * \param bufferLevelToSend Desired buffer threshold to wait for before sending.
	 */
	void SendDataXYM(std::vector<float>& x, std::vector<float>& y, std::vector<unsigned char>& m,
					 int bufferLevelToSend = -1);

	/**
	 * \brief Sends a single XY sample (no modulation).
	 * \param x Normalized X coordinate in the range [-1.0, 1.0].
	 * \param y Normalized Y coordinate in the range [-1.0, 1.0].
	 * \param bufferLevelToSend Desired buffer threshold to wait for before sending.
	 */
	void SendDataXY(float x, float y, int bufferLevelToSend = -1);

	/**
	 * \brief Sends multiple XY samples in array form.
	 * \param x Pointer to array of normalized X coordinates.
	 * \param y Pointer to array of normalized Y coordinates.
	 * \param numSamples Number of samples to send.
	 * \param bufferLevelToSend Desired buffer threshold to wait for before sending.
	 */
	void SendDataXY(float* x, float* y, unsigned int numSamples, int bufferLevelToSend = -1);

	/**
	 * \brief Sends multiple XY samples using \c std::vector containers.
	 * \param x Vector of normalized X coordinates.
	 * \param y Vector of normalized Y coordinates.
	 * \param bufferLevelToSend Desired buffer threshold to wait for before sending.
	 */
	void SendDataXY(std::vector<float>& x, std::vector<float>& y, int bufferLevelToSend = -1);

	/**
	 * \brief Sends a single XYRGB sample.
	 * \param x Normalized X coordinate.
	 * \param y Normalized Y coordinate.
	 * \param r Red color component [0..255].
	 * \param g Green color component [0..255].
	 * \param b Blue color component [0..255].
	 * \param bufferLevelToSend Desired buffer threshold to wait for before sending.
	 */
	void SendDataXYRGB(float x, float y, unsigned char r, unsigned char g, unsigned char b,
					   int bufferLevelToSend = -1);

	/**
	 * \brief Sends multiple XYRGB samples in array form.
	 * \param x Pointer to an array of normalized X coordinates.
	 * \param y Pointer to an array of normalized Y coordinates.
	 * \param r Pointer to an array of red color values [0..255].
	 * \param g Pointer to an array of green color values [0..255].
	 * \param b Pointer to an array of blue color values [0..255].
	 * \param numSamples Number of samples to send.
	 * \param bufferLevelToSend Desired buffer threshold to wait for before sending.
	 */
	void SendDataXYRGB(float* x, float* y, unsigned char* r, unsigned char* g, unsigned char* b,
					   unsigned int numSamples, int bufferLevelToSend = -1);

	/**
	 * \brief Sends multiple XYRGB samples using \c std::vector containers.
	 * \param x Vector of normalized X coordinates.
	 * \param y Vector of normalized Y coordinates.
	 * \param r Vector of red color values [0..255].
	 * \param g Vector of green color values [0..255].
	 * \param b Vector of blue color values [0..255].
	 * \param bufferLevelToSend Desired buffer threshold to wait for before sending.
	 */
	void SendDataXYRGB(std::vector<float>& x, std::vector<float>& y, std::vector<unsigned char>& r,
					   std::vector<unsigned char>& g, std::vector<unsigned char>& b,
					   int bufferLevelToSend = -1);

	/**
	 * \brief Clears any queued data on the device and sends it to the origin.
	 */
	void ClearData();

	/**
	 * \brief Sets the sample rate for data output (e.g., 10,000 samples/sec).
	 * \param sampleRate The desired sample rate in samples per second (200..50000).
	 */
	void SetSampleRate(unsigned int sampleRate);

	/**
	 * \brief Obtains the name of the connected device.
	 * \return String containing the device name.
	 */
	std::string GetDeviceName() { return m_DeviceName; };

	/**
	 * \brief Obtains the firmware version name of the connected device.
	 * \return String containing the firmware name.
	 */
	std::string GetFirmwareName() { return m_FirmwareName; };

	/**
	 * \brief Obtains the data format ("XYM" or "XYRGB").
	 * \return String indicating the current data format of the device.
	 */
	std::string GetDataFormat() { return m_DataFormat; };

	/**
	 * \brief Returns the version number of the PlayzerX API in x.y.z.w format.
	 * \return The version string, e.g., "2.1.0.0".
	 */
	std::string GetAPIVersion() { return "2.1.0.0"; };

	/**
	 * \brief Gets the last error code generated by any operation on this device.
	 * \return A \c PlayzerXError enumeration value.
	 */
	PlayzerXError GetLastError() { return m_LastError; }

	/**
	 * \brief Checks if an error has been raised in the most recent operation.
	 * \return \c true if the last error was not \c SUCCESS; otherwise \c false.
	 */
	bool HasError() { return m_LastError != PlayzerXError::SUCCESS; }

	/**
	 * \brief Discovers all available PlayzerX devices connected to the system.
	 * \param plad A reference to \c PlayzerXAvailableDevices for storing the results.
	 */
	void GetAvailableDevices(PlayzerXAvailableDevices& plad);

	/**
	 * \brief Prints a summary of all discovered devices to the standard output.
	 * \param plad A \c PlayzerXAvailableDevices object containing the discovery results.
	 */
	void ListAvailableDevices(PlayzerXAvailableDevices& plad);

	/** \brief Resets / restarts the MCU(processor) on the connected device(Controller). */
	void ResetDevice();

	/** \brief Switches between PlayzerX-AIN and PlayzerX-USB modes. */
	void SwitchPlayzerXMode(bool enableAINMode, bool flashBoot);

   protected:
	/** \brief Pointer to the underlying serial communication object. */
	MTISerialIO* m_SerialDevice;

	/** \brief Maximum number of samples that can be sent at once. */
	const unsigned int kMaxSendSamples = 100000u;

	/** \brief Maximum number of bytes that can be sent at once (11 bytes/sample in RGB mode). */
	const unsigned int kMaxSendBytes = kMaxSendSamples * 11;

	/** \brief Nominal baud rate used for the device (921600 baud). */
	const unsigned int kBaudRate = 921600;

	/** \brief Indicates if the device supports RGB mode. */
	bool m_RGBCapable;

	/** \brief Tracks whether streaming of samples remaining is active. */
	bool m_StreamingSamplesRemaining = false;

	/** \brief Tracks how many samples remain in the device buffer. */
	unsigned int m_SamplesRemaining;

	/** \brief Current sample rate, in samples per second. */
	unsigned int m_SampleRate;

	/** \brief Timeout duration for serial read/write operations, in milliseconds. */
	unsigned int m_TimeOut;

	/** \brief Tracks the last error reported by a PlayzerX operation. */
	PlayzerXError m_LastError = PlayzerXError::SUCCESS;

   private:
	/** \brief Outgoing command buffer used for sending data. */
	std::vector<unsigned char> m_CommandBytes;

	/** \brief Cached string for the device name. */
	std::string m_DeviceName;

	/** \brief Cached string for the device firmware name. */
	std::string m_FirmwareName;

	/** \brief Cached string for the device data format (e.g., "XYM", "XYRGB"). */
	std::string m_DataFormat;

	/**
	 * \brief Queries the device for identification and format details.
	 * \return \c true if successful, otherwise \c false.
	 */
	bool GetDeviceInfo();

	/** \brief Purges any pending data in the serial I/O buffers. */
	void PurgeSerialBuffers();
};

}  // namespace playzerx

#endif  // !PLAYZERX_H