////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef MTI_DEFINITIONS_H
#define MTI_DEFINITIONS_H

#define NOMINMAX 
#define _USE_MATH_DEFINES

#define TXT_RED "\033[1;31m"
#define TXT_GRN "\033[1;32m"
#define TXT_YEL "\033[1;33m"
#define TXT_MAG "\033[1;35m"
#define TXT_CYN "\033[1;36m"
#define TXT_RST "\033[0m"
#define LINE_RESET "\33[2K\r"

///////////////////////////////////////////////////////////////////////////////////
// OS & DLL Export / Import Specifiers
#if defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS)
	#define MTI_WINDOWS
#endif
#if defined(__linux__) || defined( __unix__)
	#define MTI_UNIX
#endif

#ifdef MTI_UNIX
	#define DLLEXPORT
#elif MTI_ANDROID
	#define DLLEXPORT
#elif _MTIDLLEXPORT
	#define DLLEXPORT	__declspec(dllexport)
#else
	#define DLLEXPORT	__declspec(dllimport)
#endif
///////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Macros
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=0; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=0; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=0; } }
#define SIGN(x)				 ((x) > 0 ? 1 : -1)
#define SWAP_2(x)			 ((((x) & 0xff) << 8) | ((unsigned short)(x) >> 8))
#define LE_SHORT(x)			 (*(unsigned short *)&(x) = SWAP_2(*(unsigned short *)&(x)))
#define BOUND_COORD(x)		 (((x) > 1.0) ? 1.0 : (((x) < -1.0) ? -1.0 : (x)));
#ifdef MTI_UNIX
	#define Sleep(x) usleep((x)*1000)
	/* For cross-compatibility between OpenCV 2.4.13 (Windows API) and OpenCV 4.0+ (Linux API) */
	#ifdef _MTIOPENCV
	#include <opencv2/opencv.hpp>
	#define CV_FILLED 		cv::FILLED
	/* On Ubuntu 20.04, OpenCV 4.0+, we throw away the SHIFT key to match Windows behavior */
	inline int MTI_CV_WAITKEY(int delay) { int code; return ((code = cv::waitKey(delay)) == 0xE1) ? -1 : code; }
	#define cvWaitKey(x)	MTI_CV_WAITKEY(x) 
	#endif
#endif
	
#define MTI_DEGTORAD        1.745329251994330e-02f
#define MTI_RADTODEG        5.729577951308232e+01f
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Terminal commands for MTI console applications (formerly MTISwitchOS.h)
#ifdef MTI_WINDOWS
	#include <conio.h>
	#define CLEARSCREEN "cls"
#endif

#ifdef MTI_UNIX
	#include <termios.h>
	#include <sys/select.h>
	#include <sys/ioctl.h>
	#include <unistd.h>
	#include <stdio.h>
	#define CLEARSCREEN "clear"

/* Define _getch() and _kbhit() for Linux since they are Win-only functions */
static int _getch() 
{
	struct termios oldt, newt;
	int            ch;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return ch;
}

static int _kbhit() {
	static const int STDIN = 0;
	static bool initialized = false;

	if (!initialized) {
	    // Use termios to turn off line buffering
		termios term;
		tcgetattr(STDIN, &term);
		term.c_lflag &= ~ICANON;
		tcsetattr(STDIN, TCSANOW, &term);
		setbuf(stdin, NULL);
		initialized = true;
	}

	int bytesWaiting;
	ioctl(STDIN, FIONREAD, &bytesWaiting);
	return bytesWaiting;
}

#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MTI_MAXDEVICES	12

typedef void* MTIHandle;		// for the C API

// MTIDEVICE SECTION
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum MTIError
{
	// all of the items must be in the specific order
	MTI_SUCCESS,								//!< Last operation/ communication was succesful
	MTI_ERR_INVALID_HANDLE,						//!< Handle is not existing
	MTI_ERR_INVALID_DEVICEID,					//!< No or wrong device ID
	MTI_ERR_GENERAL,							//!< General error
	MTI_ERR_SERIALCOMM,							//!< Communication error ocurred
	MTI_ERR_SERIALCOMM_READ_TIMEOUT,			//!< Communication timed out
	MTI_ERR_SERIALCOMM_SERIAL_RESET_FAIL,		//!< Reset of the communication failed
	MTI_ERR_INCORRECT_TARGET_RESPONSE,			//!< Incorrect response received from target device
	MTI_ERR_PARAMETER_OUT_OF_RANGE,				//!< Parameter value out of range
	MTI_ERR_SOME_PARAMETERS_CANNOT_BE_SET,		//!< Some parameters can not be set in the device
	MTI_ERR_INVALID_PARAMETER_NAME,				//!< Invalid parameter name
	MTI_ERR_CONNECTDEVICE_INVALID_PORTNAME,		//!< Invalid port name
	MTI_ERR_CONNECTDEVICE_PORT_NOT_AVAILABLE,	//!< Port not available
	MTI_ERR_LOADDEVICEPARAMS_FILE_NOT_FOUND,	//!< INI file not found
} 
MTIError;

typedef enum MTIBlockingMode
{
	MTI_BLOCKING_MODE_ERR = -1,
	MTI_BLOCKING_MODE_OFF =  0,
	MTI_BLOCKING_MODE_ON  =  1,
}
MTIBlockingMode;

typedef enum MTIParam
{
	HardwareFilterOn,			//!< Hardware Filter Available on Driver (Read only)
	VmaxMEMSDriver,				//!< Maximum Voltage range of MEMS driver (Read only)
	DeviceState,				//!< 16-Bit register with various device states and flags (Read only)
	DeviceErrorRegister,		//!< Register for device error states (Read only)
	USARTBaudRate,				//!< USART communication Baud Rate - Default of 460kBaud
	SampleRate,					//!< Sample Rate of the controller to update MEMS position, DOut, read AIs, etc.
	HardwareFilterBw,			//!< Hardware filter cut-off frequency - Recommended by MEMS Datasheet
	VdifferenceMax,				//!< Maximum differential voltage - Recommended by MEMS Datasheet
	Vbias,						//!< Bias Voltage for the MEMS  - Recommended by MEMS Datasheet
	OutputOffsets,				//!< Offsets for MEMS driver output, Normalized value from -1 to +1
	DataScale,					//!< Normalized scale of VdifferenceMax MEMS output from 0.0 to 1.0
	DataRotation,				//!< Rotation of MEMS output in radians
	MEMSDriverEnable,			//!< Enable/ disable MEMS driver
	DigitalOutputEnable,		//!< Enable/ disable digital outputs
	LaserModulationEnable,		//!< Enable/ disable laser modulation at 2x sample rate (for Tracking)
	BufferOffset,				//!< Offset memory buffer
	DeviceAxes,					//!< Device Axes orientation (Device Parameter Structure)
	BootSetting,				//!< Device startup settings (Device Parameter Structure)
	DataMode,					//!< Defines the data mode for operation (Device Parameter Structure)
	SyncMode,					//!< Defines the sync port operation (Device Parameter Structure)
	DataFormat,					//!< The format of the data sent to the Controller. Should align with the Controller's firmware.
	FramesPerSecond,			
	InterpolationType,			
	WaveformType,				
}
MTIParam;

typedef enum MTIAxes
{
	// all of the items must be in the specific order
	Normal													= 0, //!< Normal X,Y Axes Definition
	FlipXY													= 1, //!< Flip both axes
	MirrorY													= 2, //!< Mirror Y axis
	MirrorY_FlipXY											= 3, //!< Mirror Y, flip X with Y axis
	MirrorX													= 4, //!< Mirror X axis
	MirrorX_FlipXY											= 5, //!< Mirror X, flip X with Y axis
	MirrorX_MirrorY											= 6, //!< Mirror X,Y axes
	MirrorX_MirrorY_FlipXY									= 7, //!< Mirror and flip both axes
}
MTIAxes;

typedef enum MTIDataMode
{
	// all of the items must be in the specific order
	Sample_Output											= 0, //!< Output mode at sample rate (default) (Continuous Run)
	Sample_And_Analog_Input_Buffer							= 1, //!< Output mode for sampling analog input signals into buffer while outputting samples (Single Run)
	Keypoint												= 2, //!< Output mode keypoints
	Sample_Output_FIFO										= 3, /*!< Output mode for continuous data streaming. Data streams are stored into a FIFO buffer and sampled
																 out at the current sample rate. Requires new samples to be continuously streamed to the Controller.*/
	Waveform												= 4, //!< Output mode waveforms
	Analog_Input_To_Output									= 5, //!< Output mode passing analog inputs (+/-10V) to MEMS driver
	Sample_And_Analog_Input_Stream							= 6, //!< Output mode for streaming analog inputs while outputting samples
	Sample_And_Analog_Input_Track							= 7, //!< Output mode for streaming analog inputs and streaming samples
	Auto_Track												= 8, //!< Automatic tracking
	Sample_And_SyMPL1_Stream								= 9, //!< Output mode for streaming distances with SyMPL1 lidar architecture
}
MTIDataMode;

typedef enum MTISync
{
	// all of the items must be in the specific order
	Output_DOut0											= 0, //!< Default digital output levels at pin 0
	Output_Inverted_DOut0									= 1, //!< Inverted digital output levels at pin 0
	Output_Sample_Clock										= 2, //!< Output a clock signal at sync connector
	Output_Start_Trigger									= 3, //!< Output a trigger signal at sync connector
	External_Sample_Clock									= 4, //!< Input a external clock signal at sync connector
	External_Start_Trigger									= 5, //!< Input a start trigger signal at sync connector
	External_Frame_Trigger									= 6, //!< Input an external trigger signal at sync connector to start single scan of frame

} MTISync;

typedef enum MTIBoot
{
	// all of the items must be in the specific order
	Boot_With_Factory_Defaults								= 0, //!< Boot with factory settings (default)
	Boot_With_Flash_Device_Params							= 1, //!< Boot with (customized) device parameters from flash
	Boot_With_Flash_Data_And_Autorun						= 2, //!< Boot with (customized) device parameters and data from flash and auto run
	Boot_With_Flash_Data_No_Autorun							= 3, //!< Boot with (customized) device parameters and data from flash and wait for commands
} MTIBoot;

typedef enum MTIFlash
{
	// all of the items must be in the specific order
	Device_Params,						//!< Store device parameters in flash
	Data_In_Buffer,						//!< Store data in buffer in flash
} MTIFlash;

typedef enum MTIDataFormat
{
	// all of the items must be in the specific order
	Standard_XYM								= 0, //!< Standard 5 byte data with 16 bits X, 16 bits Y, 8 bits M
	Reduced_XYM									= 1, //!< Reduced length 3 byte data with 12 bits X, 11 bits Y, 1 bits M
	Reduced_XYRGB								= 2, //!< RGB 5 byte data with 12 bits X, 12 bits Y, 5 bits R, 6 bits G, 5 bits B
	Standard_XYRGB								= 3, //!< RGB 6 byte data with 12 bits X, 12 bits Y, 8 bits R, 8 bits G, 8 bits B
	Standard_XYMRGB								= 4, //!< RGB 7 byte data with 12 bits X, 12 bits Y, 8 bits M, 8 bits R, 8 bits G, 8 bits B
} MTIDataFormat;

typedef enum MTIDeviceLimits
{
	SampleRate_Min,						//!< Minimum sample rate (Read only firmware)
	SampleRate_Max,						//!< Maximum sample rate (Read only firmware)
	SamplesPerFrame_Min,				//!< Minimum samples per frame (Read only firmware)
	SamplesPerFrame_Max,				//!< Maximum samples per frame (Read only firmware)
	HardwareFilterBw_Min,				//!< Minimum hardware filter bandwidth (Read only firmware)
	HardwareFilterBw_Max,				//!< Maximum hardware filter bandwidth (Read only firmware)
	FramesPerSecond_Min,				//!< Minimum frames per second (Read only firmware)
	FramesPerSecond_Max,				//!< Maximum frames per second (Read only firmware)
	VdifferenceMax_Min,					//!< Minimum differential voltage (Read only firmware)
	VdifferenceMax_Max,					//!< Maximum differential voltage (Read only firmware)
	Vbias_Min,							//!< Minimum bias voltage (Read only firmware)
	Vbias_Max,							//!< Maximum bias voltage (Read only firmware)
} MTIDeviceLimits;

typedef enum MTITrack
{
	Threshold,							/*!< Analog input voltage threshold to trigger tracking.*/
	NormalGain,							/*!< Tracking gain to follow target - corrections toward the estimated center of the target.
										Actual gain is applied in the Controller as 2^NormalGain, and therefore NormalGain is often a negative value.*/
	TangentialGain,						/*!< Tracking gain to move along the perimeter of target (Positive is clockwise and negative is counterclockwise).
										Corrections perpendicular to the estimated vector from current position to the center of the target.*/
	BufferDelay,						/*!< Tracking setting which corrects the time-correlation of XY MEMS positions and the M Digital Output,
										due to the fact that XY MEMS positions are delayed by filtering.*/
	HitRatio,							/*!< The ratio of the MEMS scan (nutation) that should hit/cover the target.
										(e.g. 1 to fully cover the target, 0.5 to cover half of the target).*/
	EnableSearch,						/*!< Toggle to enable / disable the search feature.  When all points are below threshold,
										system can start a search raster pattern or stay in the same position depending on this setting.*/
	EnableOffsetStreaming,				/*!< Toggle to enable / disable offset streaming feature.  When on, Controller will continuously stream
										current X and Y offset positions - during tracking this provides real time azimuth and elevation data about the target location.
										The other option is to disable this feature and asynchronously poll the positions using the GetDeviceParam( MTIParam::OutputOffsets, n)*/
	EnableTrack,						/*!< Toggle to enable / disable the track feature.  When disabled, position will not be updated based on one nutation cycle.
										When enabled, offsets are updated to improve centering on the target, at each end of frame/buffer.*/
} MTITrack;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MTIDATAGENERATOR SECTION
// Enums for DataGenerator
typedef enum FilterType
{
	FilterNone			= 0,
	FilterBessel		= 1,
	FilterButterworth	= 2,
	FilterChebyshevI	= 3,
	FilterChebyshevII	= 4,
	FilterElliptic		= 5,
	FilterLegendre		= 6,
	FilterIIRGeneric	= 7,
}
FilterType;

typedef enum SpirographType
{
	CurveRose			= 0,
	CurveEpicycloid		= 1,
	CurveEpitrochoid	= 2,
	CurveHypocycloid	= 3,
	CurveHypotrochoid	= 4,
}
SpirographType;

typedef enum ModulationType
{
	ModulationNone	= 0,
	ModulationAmpl	= 1,
	ModulationFreq	= 2,
	ModulationAmplUnipolar = 3,
}
ModulationType;

typedef enum WaveType
{
	WaveSine	 = 0,
	WaveTriangle = 1,
	WaveSquare   = 2,
}
WaveType;

/**
 * @enum PlotType
 * The style of plot to use when using MTIDataGenerator::PlotData
*/
typedef enum PlotType
{
	TimeVsXYData = 0,
	XDataVsYData = 1,
	TimeVsXData = 2,
	TimeVsYData = 3,
	TimeVsMData = 4,
	TimeVsXMData = 5,
	TimeVsYMData = 6,
	TimeVsXYMData = 7,
}
PlotType;

#endif
