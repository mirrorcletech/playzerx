#include "PlayzerX.h"  // this header should be first, includes a lot of definitions
#ifdef MTI_WINDOWS
#include <windows.h>
#include <mmsystem.h>
#endif
#include <fstream>
#include <sstream>
#include <time.h>

using namespace playzerx;
PlayzerX* playzer;

float* xData;
float* yData;
unsigned char* mData;

unsigned int maxSendSamples = 50000;

bool rgbCapable = false;
bool applicationExitRequest = false;

unsigned int LoadPointFile(const char* filename, unsigned int& m_iPointFileSps, float* m_fXFile,
						   float* m_fYFile, unsigned char* m_iMFile)
{
	std::ifstream fp(filename);
	std::string line, str;
	unsigned int i = 0, j = 0;
	float xVal, yVal, mVal;
	bool spsFlag = false;
	m_iPointFileSps = 0;

	while (fp.good())
	{
		std::getline(fp, line);
		if (line.size() == 0) continue;
		if (i == 0)
		{
			// std::transform( line.begin(), line.end(), line.begin(), tolower );
			if ((line.find("sps") != std::string::npos) ||
				(line.find("Sps") != std::string::npos) || (line.find("SPS") != std::string::npos))
			{
				std::stringstream stream(line);
				stream >> str >> m_iPointFileSps;
				spsFlag = true;
			}
		}
		i++;
	}
	unsigned int numFilePoints = (spsFlag) ? i - 1 : i;

	fp.clear();
	i = 0;
	fp.seekg(0, fp.beg);
	while (fp.good())
	{
		std::getline(fp, line);
		if ((i == 0 && spsFlag) || line.size() == 0)
		{
			i++;
			continue;
		}

		std::stringstream stream(line);
		stream >> xVal >> yVal >> mVal;
		m_fXFile[j] = std::max(std::min(xVal, 1.f), -1.f);
		m_fYFile[j] = std::max(std::min(yVal, 1.f), -1.f);
		m_iMFile[j] = std::max(std::min((unsigned int)mVal, 255u), 0u);
		i++;
		j++;
	}
	fp.close();

	return numFilePoints;
}

#ifdef MTI_WINDOWS
// Because this demo uses the Windows audio API,
// it is only available on Windows
void AudioDemo()
{
	const int NUMPTS = (int)((float)44100 / 100.f);  // 0.01 seconds of sound recording
	int sampleRate = 44100;
	short int waveIn[NUMPTS];  // 'short int' is a 16-bit type; I request 16-bit samples below
							   // for 8-bit capture, you'd use 'unsigned char' or 'BYTE' 8-bit types

	HWAVEIN hWaveIn;
	WAVEHDR WaveInHdr;
	MMRESULT result;

	// Specify recording parameters
	WAVEFORMATEX pFormat;
	pFormat.wFormatTag = WAVE_FORMAT_PCM;	  // simple, uncompressed format
	pFormat.nChannels = 1;					   //  1=mono, 2=stereo
	pFormat.nSamplesPerSec = sampleRate;	   // 44100
	pFormat.nAvgBytesPerSec = sampleRate * 2;  // = nSamplesPerSec * n.Channels * wBitsPerSample/8
	pFormat.nBlockAlign = 2;				   // = n.Channels * wBitsPerSample/8
	pFormat.wBitsPerSample = 16;			   //  16 for high quality, 8 for telephone-grade
	pFormat.cbSize = 0;

	result = waveInOpen(&hWaveIn, 0, &pFormat, 0L, 0L, WAVE_FORMAT_DIRECT);
	if (result)
	{
		char fault[256];
		waveInGetErrorText(result, fault, 256);
		printf("\nFailed to open waveform input device.");
		_getch();
		return;
	}

	// Set up and prepare header for input
	WaveInHdr.lpData = (LPSTR)waveIn;
	WaveInHdr.dwBufferLength = NUMPTS * 2;  // 16-bit, 2 bytes each sample
	WaveInHdr.dwBytesRecorded = 0;
	WaveInHdr.dwUser = 0L;
	WaveInHdr.dwFlags = 0L;
	WaveInHdr.dwLoops = 0L;

	// Commence sampling input
	float x = 0, y = 0;
	unsigned char m = 255;

	playzer->SetSampleRate(10000);  // Sample rate and number of points equal, so 1 second of data
									// in one repeated frame.

	do
	{
		waveInPrepareHeader(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));

		// Insert a wave input buffer
		result = waveInAddBuffer(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));
		if (result)
		{
			printf("\nFailed to read block from input device.");
			_getch();
			return;
		}
		result = waveInStart(hWaveIn);
		if (result)
		{
			printf("\nFailed to start recording .");
			_getch();
			return;
		}
		// Wait until finished recording
		do
		{
		} while (waveInUnprepareHeader(hWaveIn, &WaveInHdr, sizeof(WAVEHDR)) ==
				 WAVERR_STILLPLAYING);
		float rmsMean = 0;
		for (int i = 0; i < NUMPTS; i++)
		{
			rmsMean +=
				pow((float)(WaveInHdr.lpData[2 * i] + WaveInHdr.lpData[(2 * i) + 1] * 256), 2);
		}
		rmsMean = sqrt(rmsMean / NUMPTS);
		printf("rmsMean = %3.2f\n", rmsMean);
		y = 0.7 * y + 0.3 * std::min(sqrt(rmsMean / 10000), 1.f);
		m = 140 + (y * 115);
		playzer->SendDataXYM(x, y, m);
	} while (!_kbhit());
	_getch();  // consume the pressed key

	playzer->SendDataXYM(0, 0, 0);  // Reset beam to center with laser at lowest power
}

// Because this demo uses the Windows audio API,
// it is only available on Windows
void AudioDemoScope()
{
	const int NUMPTS = (int)(200);  // 0.05 seconds of sound recording
	int sampleRate = 44100;
	// 'short int' is a 16-bit type; I request 16-bit samples below
	// for 8-bit capture, you'd use 'unsigned char' or 'BYTE' 8-bit types
	short int waveIn[NUMPTS];

	HWAVEIN hWaveIn;
	WAVEHDR WaveInHdr;
	MMRESULT result;

	// Specify recording parameters
	WAVEFORMATEX pFormat;
	pFormat.wFormatTag = WAVE_FORMAT_PCM;	  // simple, uncompressed format
	pFormat.nChannels = 1;					   //  1=mono, 2=stereo
	pFormat.nSamplesPerSec = sampleRate;	   // 44100
	pFormat.nAvgBytesPerSec = sampleRate * 2;  // = nSamplesPerSec * n.Channels * wBitsPerSample/8
	pFormat.nBlockAlign = 2;				   // = n.Channels * wBitsPerSample/8
	pFormat.wBitsPerSample = 16;			   //  16 for high quality, 8 for telephone-grade
	pFormat.cbSize = 0;

	result = waveInOpen(&hWaveIn, 0, &pFormat, 0L, 0L, WAVE_FORMAT_DIRECT);
	if (result)
	{
		char fault[256];
		waveInGetErrorText(result, fault, 256);
		printf("\nFailed to open waveform input device.");
		_getch();
		return;
	}

	// Set up and prepare header for input
	WaveInHdr.lpData = (LPSTR)waveIn;
	WaveInHdr.dwBufferLength = NUMPTS * 2;  // 16-bit, 2 bytes each sample
	WaveInHdr.dwBytesRecorded = 0;
	WaveInHdr.dwUser = 0L;
	WaveInHdr.dwFlags = 0L;
	WaveInHdr.dwLoops = 0L;

	// Commence sampling input

	int npts = 300;
	int nptsh = npts / 2;
	std::vector<unsigned char> mDataForward, mDataReverse, mData;
	mData.reserve(npts);
	mDataForward.reserve(nptsh);
	mDataForward.assign(nptsh, 200);
	mDataReverse.reserve(nptsh);
	unsigned char m;

	// Sample rate and number of points equal, so 1 second of data in
	// one repeated frame.
	playzer->SetSampleRate(4000);
	float y = 0;
	std::vector<float> xData;
	std::vector<float> yDataForward, yDataReverse, yData;
	xData.reserve(npts);
	yData.reserve(npts);
	yDataForward.reserve(nptsh);
	yDataForward.assign(nptsh, 0);
	yDataReverse.reserve(nptsh);
	// prepare xData scan back and forth
	for (int i = 0; i < nptsh; i++) xData.push_back(-0.5f + 1 * ((float)i / (float)(nptsh - 1)));
	for (int i = 0; i < nptsh; i++) xData.push_back(0.5f - 1 * ((float)i / (float)(nptsh - 1)));
	do
	{
		waveInPrepareHeader(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));

		// Insert a wave input buffer
		result = waveInAddBuffer(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));
		if (result)
		{
			printf("\nFailed to read block from input device.");
			_getch();
			return;
		}
		result = waveInStart(hWaveIn);
		if (result)
		{
			printf("\nFailed to start recording .");
			_getch();
			return;
		}
		// Wait until finished recording
		do
		{
		} while (waveInUnprepareHeader(hWaveIn, &WaveInHdr, sizeof(WAVEHDR)) ==
				 WAVERR_STILLPLAYING);
		float rmsMean = 0;
		for (int i = 0; i < NUMPTS; i++)
		{
			rmsMean +=
				pow((float)(WaveInHdr.lpData[2 * i] * 256 + WaveInHdr.lpData[(2 * i) + 1]), 2);
		}
		rmsMean = sqrt(rmsMean / NUMPTS);
		// printf("rmsMean = %3.2f\n",rmsMean);
		y = 0.01 * y + 0.99 * std::min(sqrt(rmsMean / 40000), 1.f);
		m = 180 + (y * 75);
		// rotate data arrays to allow new data at end
		std::rotate(yDataForward.begin(), yDataForward.begin() + 2, yDataForward.end());
		std::rotate(mDataForward.begin(), mDataForward.begin() + 2, mDataForward.end());
		mDataForward.at(nptsh - 1) = m;
		yDataForward.at(nptsh - 1) = y;
		mDataForward.at(nptsh - 2) = m;
		yDataForward.at(nptsh - 2) = y;
		yDataReverse.clear();
		yDataReverse.insert(yDataReverse.begin(), yDataForward.begin(), yDataForward.end());
		std::reverse(yDataReverse.begin(), yDataReverse.end());
		mDataReverse.clear();
		mDataReverse.insert(mDataReverse.begin(), mDataForward.begin(), mDataForward.end());
		std::reverse(mDataReverse.begin(), mDataReverse.end());
		yData.clear();
		mData.clear();
		yData.insert(yData.begin(), yDataForward.begin(), yDataForward.end());
		yData.insert(yData.end(), yDataReverse.begin(), yDataReverse.end());
		mData.insert(mData.begin(), mDataForward.begin(), mDataForward.end());
		mData.insert(mData.end(), mDataReverse.begin(), mDataReverse.end());
		playzer->SendDataXYM(&xData[0], &yData[0], &mData[0], npts, npts);
	} while (!_kbhit());
	_getch();  // consume the pressed key

	playzer->SendDataXYM(0, 0, 0);  // Reset beam to center with laser at lowest power
}
#endif

// ScanningDemo demonstrates basic content generation and execution functions
// This demo prepares a lissajous pattern, then sends the data to the Controller
void ScanningDemo()
{
	int i = 0, j = 0, k = 0, key, npts = 256 * 100;
	float *x, *y, dt = (float)M_PI * 2.f / npts;
	unsigned char *m, *r, *g, *b;

	// Sample rate and number of points equal, so 1 second of data
	// in one repeated frame.
	playzer->SetSampleRate(npts);

	printf("\nStarting scanning demo...\n\n");

	// Create some sample data
	x = new float[npts];
	y = new float[npts];
	m = new unsigned char[npts];
	r = new unsigned char[npts];
	g = new unsigned char[npts];
	b = new unsigned char[npts];

	while (true)
	{
		if (j > 0)
		{
			if (!_kbhit()) continue;
			key = _getch();
			if (key == 224)
				continue;
			else if (key == 27)
				break;
		}
		// Integer that changes every iteration to generate different Lissajous patterns.
		k = j % 8 + 1;
		// Prepare 1 second of data to be repeated
		for (i = 0; i < npts; i++)
		{
			// X-Axis position follows a sin curve from -1.0 to +1.0
			// normalized position (* DataScale setting)
			x[i] = sin(20.f * k * i * dt);
			// Y-Axis position follows a cos curve from -0.9 to +0.9
			// normalized position at another frequency
			y[i] = 0.9f * sin((10.f * (k + 1) + 1) * i * dt);
			// 8-bit laser modulation increases every four samples
			m[i] = (unsigned char)(20 + (i / 4) % 236);

			// if RGB Playzer
			r[i] = (unsigned char)((i / 100) % 256);
			g[i] = (unsigned char)(((i * 2) / 100) % 256);
			b[i] = (unsigned char)(((i * 3) / 100) % 256);
		}
		printf(TXT_YEL
			   "Cycle: %d. Press any key to change waveform or ESC to exit demo...\n" TXT_RST,
			   ++j);
		do
		{
			// This call will download the buffer of data to the
			// controller and run it when existing frame ends
			if (!rgbCapable)
				playzer->SendDataXYM(x, y, m, npts, 10000);
			else
				playzer->SendDataXYRGB(x, y, r, g, b, npts, 10000);
		} while (!_kbhit());
	}

	playzer->SendDataXYM(0, 0, 0);  // Reset beam to center with laser at lowest power

	SAFE_DELETE_ARRAY(x);
	SAFE_DELETE_ARRAY(y);
	SAFE_DELETE_ARRAY(m);
}

// PointToPointDemo demonstrates MTIDevice's GoToDevicePosition method
// for Point-To-Point beam steering.
//
// Loops and ask user which position to go to.
// If position exceeds -1 to 1, exit program.
void PointToPointDemo()
{
	int tStep = 5;

	printf("\nInput X,Y position the device should step to.\n");
	printf("Use normalized positions from -1 to 1 for each axis\n");
	printf(TXT_YEL "To EXIT, enter any value out of the valid range\n\n" TXT_RST);
	if (rgbCapable) { printf("\nR,G,B color values should be provided from 0 to 255.\n"); }
	else
	{
		printf("Laser modulation M value should be provided from 0 to 255\n");
	}

	// First get the desired step time between last point and the next point

	float x = 0, y = 0;
	int m = 200;
	int r = 200, g = 200, b = 200;

	playzer->SendDataXYM(0, 0, 0);  // Reset beam to center with laser at lowest power
	bool runFlag = true;
	do
	{
		if (!rgbCapable)  // for Monochrome Playzer
		{
			// Point beam to position x, y (-1 to +1) and modulate laser to m (0 to 255)
			playzer->SendDataXYM(x, y, m);
			x = y = -10;
			printf(
				"\033[8;6HInput go to position in X,Y or X,Y,M format:                      "
				"\033[8;51H");
			char line[256];
			fgets(line, 256, stdin);
			sscanf(line, "%f , %f , %d", &x, &y, &m);
		}
		else
		{
			// Point beam to position x, y (-1 to +1) and modulate laser to R, G, B (0 to 255)
			playzer->SendDataXYRGB(x, y, r, g, b);
			x = y = -10;
			printf(
				"\033[8;6HInput go to position in X,Y,R,G,B format:                         "
				"\033[8;51H");
			char line[256];
			fgets(line, 256, stdin);
			sscanf(line, "%f , %f , %d, %d, %d", &x, &y, &r, &g, &b);
		}
		runFlag = abs(x) <= 1 && abs(y) <= 1;
	} while (runFlag);

	playzer->ClearData();  // Reset beam to center with laser at lowest power
}

// ImportFileDemo demonstrates the use of the MTIDataGenerator class to load user-defined
// point files (in x/y/m format)
//
// The demo loads the butterfly.smp point file from the resources folder, then sends it directly
// to the Controller for display
void ImportFileDemo()
{
	unsigned int sps, npts;
	float *x, *y;
	unsigned char* m;

	// let's allocate a lot of points if we don't know how many in file
	npts = 100000;
	x = new float[npts];
	y = new float[npts];
	m = new unsigned char[npts];

	sps = 20000;  // sample rate default but it may return different after reading file
	npts = LoadPointFile("butterfly.smp", sps, x, y, m);

	if (npts < 1)  // no points were returned (maybe file not found)
		return;

	if (sps > 0)
		printf("\nSample rate of %d specified in the file\n\n", sps);
	else
	{
		printf(
			"Sample file did not specify SPS setting.  Using default value of 20000 samples per "
			"second.\n\n");
		sps = 20000;
	}

	// Ensure that the sample rate from the .smp does not exceed the device limits
	if (sps > 60000 || sps <= 500)
	{
		printf("Invalid sample rate. Defaulting to 5000\n\n");
		sps = 5000;
	}
	playzer->SetSampleRate(sps);  // Set the sample rate to that specified by .smp

	// Send the data stream read from .smp to the device and run indefinitely
	unsigned int delaySamples = 0;  // default value - no additional delay of m data samples needed
	// ok to rotate xym data so first point in buffer closest to previous scan end
	bool minimizeJump = true;
	// ok to rotate m data to minimize group delay difference
	// between xy and m data output
	bool compensateFilterDelay = true;

	printf(TXT_YEL "Press any key to stop the waveform and device scanning...\n" TXT_RST);

	do
	{
		// This call will download the buffer of data to the
		// controller and run it when existing frame ends
		playzer->SendDataXYM(x, y, m, npts, 10000);

	} while (!_kbhit());  // wait for a keypress in console
	_getch();			  // consume the pressed key

	// Send device safely to origin, continue running at origin (0,0) position
	playzer->ClearData();

	SAFE_DELETE_ARRAY(x);
	SAFE_DELETE_ARRAY(y);
	SAFE_DELETE_ARRAY(m);
}

// returns true if connected playzer is RGB Playzer
// false if Monochrome Playzer
bool IsRGBModule()
{
	std::string dataFormat = playzer->GetDataFormat();
	if (dataFormat == "XYRGB")
		return true;
	else
		return false;
}

// ArrowKeysDemo uses the MTIParam OutputOffsets to quickly send the (x,y) coordinates
// received from user WASD-key input to the Controller and, in-turn, the MEMS
void ArrowKeysDemo()
{
	int key;
	bool runFlag = true;
	float x = 0, y = 0;
	unsigned int m;	// for monochrome playzer laser power control
	unsigned int r, g, b; // for RGB playzer laser power control
	r = g = b = m = 100;

	printf("\nUse WASD Keys to Control Device Tip / Tilt Angle.\n");
	printf("A and D Keys Control X-Axis [-1.0 to +1.0].\n");
	printf("W and S Keys Control Y-Axis [-1.0 to +1.0].\n");
	if (!rgbCapable)
	{
		printf("Use P/p Keys to Control Laser Power Up/down [0 to 255].\n");
		printf(TXT_YEL "Hit ESC to exit this mode\n" TXT_RST);
		printf("\033[8;6HCurrent X, Y, M data: %3.2f, %3.2f, %d    ", x, y, m);
	}
	else
	{
		printf("Use R/r | G/g | B/b Keys to Control Laser Power Up/down [0 to 255].\n");
		printf(TXT_YEL "Hit ESC to exit this mode\n" TXT_RST);
		printf("\033[8;6HCurrent X, Y, R, G, B data: %3.2f, %3.2f, %d, %d, %d                ", 
			x, y, r, g, b);
	}

	// Move to origin and output m on digital outputs and laser power
	if (!rgbCapable)
		playzer->SendDataXYM(0.f, 0.f, m, 1);
	else
		playzer->SendDataXYRGB(0.f, 0.f, r, g, b, 1);

	while (runFlag)
	{
		if (_kbhit())
		{
			key = (int)_getch();
			switch (key)
			{
			case 'P':  //
				m = std::min(m, 250u);
				m += 5;
				break;
			case 'p':  //
				m = std::max(m, 5u);
				m -= 5;
				break;
			case 65:  // 'A' key
			case 97:  // 'a' key
				x -= 0.05f;
				x = std::max(x, -1.f);
				break;
			case 68:   // 'D' key
			case 100:  // 'd' key
				x += 0.05f;
				x = std::min(x, +1.f);
				break;
			case 83:   // 'S' key
			case 115:  // 's' key
				y -= 0.05f;
				y = std::max(y, -1.f);
				break;
			case 87:   // 'W' key
			case 119:  // 'w' key
				y += 0.05f;
				y = std::min(y, +1.f);
				break;
			// next 6 keypresses apply only for RGB Playzer
			case 'R':  //
				r = std::min(r, 250u);
				r += 5;
				break;
			case 'r':  //
				r = std::max(r, 5u);
				r -= 5;
				break;
			case 'G':  //
				g = std::min(g, 250u);
				g += 5;
				break;
			case 'g':  //
				g = std::max(g, 5u);
				g -= 5;
				break;
			case 'B':  //
				b = std::min(b, 250u);
				b += 5;
				break;
			case 'b':  //
				b = std::max(b, 5u);
				b -= 5;
				break;

			case 27: runFlag = false; continue;
			default: continue;
			}
			if (!rgbCapable)
			{
				playzer->SendDataXYM(x, y, m, 1);
				printf(
					"\033[8;6HCurrent X, Y, M data: %3.2f, %3.2f, %d    ", x, y, m);
			}
			else
			{
				playzer->SendDataXYRGB(x, y, r, g, b, 1);
				printf("\033[8;6HCurrent X, Y, R, G, B data: %3.2f, %3.2f, %d, %d, %d                ",
					   x, y, r, g, b);
			}
		}
	}
	playzer->ClearData();  // Reset beam to center with laser at lowest power
}

// In this example we test how fast laser beam can scan when we provide
// only one sample per SendData call. We are continuously sending a new position
// to the Playzer but only one sample point at a time
void BasicCircle()
{
	float x, y;
	unsigned char m;

	printf("\nThis demo tests speed of sending a single sample with each SendData call.\n");
	printf("There are two options tested here, XYM sample with 9 bytes/sample load\n");
	printf("And XY sample with 8 bytes/sample load\n");
	printf("Press any key to EXIT the loop earlier\n\n");

	// setting sample rate here but it is not critical since effective
	// sample rate will be limited to the for-loop rate of SendDataXYM calls
	playzer->SetSampleRate(50000);

	printf("Loop for XYM samples to form 12 circles running (720 samples per circle)...\n");
	for (int i = 0; i < 12 * 720; i++)
	{
		// x and y to form a circle with 720 samples per circle
		x = cos((float)i * 2.f * M_PI / 720);
		y = sin((float)i * 2.f * M_PI / 720);
		// laser modulation goes up/down
		m = (0.5f + 0.5f * sin(((float)i / 3) * 2.f * M_PI / 720)) * 255u;
		playzer->SendDataXYM(x, y, m);  // send single XYM sample right away
		// Sleep(1);	// optional slow-down of the point-to-point movement
		if (_kbhit())  // we let user get out sooner if needed
		{
			_getch();  // consume character
			break;
		}
	}

	printf("Loop for XYM samples to form 10 circles running (720 samples per circle)...\n");
	for (int i = 0; i < 10 * 720; i++)
	{
		// x and y to form a circle with 720 samples per circle
		x = cos((float)i * 2.f * M_PI / 720);
		y = sin((float)i * 2.f * M_PI / 720);
		playzer->SendDataXY(x, y);  // send single XY sample right away (M defaults to 255)
		// Sleep(1);	// optional slow-down of the point-to-point movement
		if (_kbhit())  // we let user get out sooner if needed
		{
			_getch();  // consume character
			break;
		}
	}

	// let's shut off the beam and point to origin now
	playzer->ClearData();
}

// This function provides options for switching between playzerX modes and flashing its settings.
void PlayzerXModeSelection()
{
	bool menuFlag = true, runFlag = true;
	int mode = 0;

	system(CLEARSCREEN);
	menuFlag = false;

	if (rgbCapable)
	{
		printf(TXT_RED "\nThese options are not available for RGB PlayzerX.\n" TXT_RST);
		printf(TXT_RED "\nRGB PlayzerX is currently not taking AIN input.\n" TXT_RST);
		printf(TXT_YEL "\n\nPress any key to return to main menu..." TXT_RST);
		_getch();
		return;
	}

	printf("PlayzerX modes:\n");
	printf("\t0: Quit.\n");
	printf("\t1: Flash PlayzerX-USB Setting into the controller.\n");
	printf("\t2: Switch device into PlayzerX-AIN Mode.\n");
	printf("\t3: Switch device into PlayzerX-AIN Mode and Flash Setting.\n");

	while (runFlag)
	{
		if (_kbhit())
		{
			mode = (int)_getch() - 48;
			menuFlag = true;
			switch (mode)
			{
			case 0: runFlag = false; break;
			case 1: playzer->SwitchPlayzerXMode(false, true); break;
			case 2: playzer->SwitchPlayzerXMode(true, false); break;
			case 3: playzer->SwitchPlayzerXMode(true, true); break;
			case -21: runFlag = false; break;
			default: break;
			}

			if (mode == 1)
			{
				printf(TXT_YEL "\n\nPlayzerX-USB Settings Flashed into the controller..." TXT_RST);
				Sleep(2000);
			}
			else if ((mode == 2) || (mode == 3))
			{
				printf(
					"\nController switched to PlayzerX-AIN mode. Use a function generator or "
					"Pangolin to feed +-5V signals to the analog inputs");
				printf(
					"\nSending commands to the controller changes the state of the controller to "
					"PlayzerX-USB.");
				if (mode == 3)
				{
					printf(TXT_GRN
						   "\n\nWhen settings are flashed into the controller, the controller "
						   "state change to PlayzerX-USB after sending commands in the application "
						   "is temporarily. After power cycling, the controller reboots with the "
						   "flashed settings.");
				}
				printf(TXT_YEL
					   "\n\nTo retain the PlayzerX-AIN settings and prevent any state changes in "
					   "the controller,\npress ESC to exit the application...\n" TXT_RST);
				printf(TXT_YEL
					   "\nOtherwise, press any key to return to the main menu...\n" TXT_RST);

				int key = _getch();
				if (key == 27) applicationExitRequest = true;
			}

			break;
		}
	}
}

void ResetDeviceDemo()
{
	// send reset command to the unit - this will reboot the MCU but serial connection should remain
	printf(TXT_RST "\nSent ResetDevice() command to Controller. Need ~3-4s to reconnect.");
	printf("\nController will boot in mode saved in flash with default settings.");
	playzer->ResetDevice();
	// user can note LED changes on the controller. when blue LED restarts firmware is running
	// when red LED is on, driver is enabled and unit is fully ready

	// setup a timer to try to reconnect for 4s max, then time out
	time_t tstart, tend;
	tstart = time(0);
	while (!playzer->IsDeviceConnected())
	{
		Sleep(100);
		if (difftime(time(0), tstart) > 4.0f)  // if no response after 3s of trying
			break;
	}

	if (!playzer->IsDeviceConnected())  // if failed to reconnect
	{ printf(TXT_RED "\nFailed to reconnect with Controller (Time-out). Test Fails.\n" TXT_RST); }
	else
	{
		printf(TXT_GRN "\nController reconnnected successfully ... \n" TXT_RST);
		Sleep(2000);
	}
}

int main(int argc, char* argv[])
{
	// Initialize the Playzer Device
	playzer = PlayzerX::CreateDevice();

	playzer->ConnectDevice();
	if (playzer->HasError())
	{
		printf("\n\nUnable to connect with any PlayzerX Controller.  Press any key to Exit.\n");
		PlayzerX::DeleteDevice(playzer);
		_getch();
		return -1;  // leave demo if not successfully connected
	}

	rgbCapable = IsRGBModule();

	std::string deviceName = playzer->GetDeviceName();
	std::string firmwareName = playzer->GetFirmwareName();
	bool menuFlag = true, runFlag = true;
	int mode = 0;
	while (runFlag)
	{
		if (menuFlag)
		{
			system(CLEARSCREEN);
			menuFlag = false;

			printf(
				"\n******************** PlayzerX-Demo 2.1 - C++ SDK Examples "
				"********************\n");
			printf("Demonstrates uses of PlayzerX class.\n");
			std::string version = playzer->GetAPIVersion();
			printf("Device Name:" TXT_GRN " %s " TXT_RST "     Firmware Name:" TXT_GRN
				   " %s " TXT_RST "     API version" TXT_GRN " %s " TXT_RST "\n\n",
				   deviceName.c_str(), firmwareName.c_str(), version.c_str());
			printf("General Examples:\n");
			printf(
				"\t0: Quit\n\t1: Point to Point Demo\n\t2: Scanning Demo\n\t3: Import File with "
				"Samples Demo\n");
			printf("\t4: Follow WASD Keys Demo\n");
			printf("\t5: Basic Circle\n");
#ifdef MTI_WINDOWS
			printf("\t6: Audio Demo Y-axis\n\t7: Audio Demo O-scope\n");
#endif
			printf("\t8: Switch PlayzerX Mode\n");
			printf("\t9: Reset Device\n");
		}

		if (_kbhit())
		{
			mode = (int)_getch() - 48;
			menuFlag = true;
			// Stop and send analog outputs (and device) back to origin
			playzer->ClearData();
			system(CLEARSCREEN);
			switch (mode)
			{
			case 0: runFlag = false; break;
			case 1: PointToPointDemo(); break;
			case 2: ScanningDemo(); break;
			case 3: ImportFileDemo(); break;
			case 4: ArrowKeysDemo(); break;
			case 5: BasicCircle(); break;
#ifdef MTI_WINDOWS
			case 6: AudioDemo(); break;
			case 7: AudioDemoScope(); break;
#endif
			case 8: PlayzerXModeSelection(); break;
			case 9: ResetDeviceDemo(); break;
			case -21: runFlag = false; break;
			default: break;
			}
		}
		runFlag = runFlag & (!applicationExitRequest);
	}

	// Shut down the PlayzerX device
	if (!applicationExitRequest) playzer->DisconnectDevice();
	playzerx::PlayzerX::DeleteDevice(playzer);
	return 0;
}