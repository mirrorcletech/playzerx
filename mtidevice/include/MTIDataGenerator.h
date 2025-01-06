//////////////////////////////////////////////////////////////////////
// MTIDataGenerator
// Version: 11.1.1.0
//////////////////////////////////////////////////////////////////////

#ifndef MTI_DATA_GENERATOR_H
#define MTI_DATA_GENERATOR_H

#include "MTIDefinitions.h"

#ifdef _MSC_VER
	#define _CRT_SECURE_NO_WARNINGS
#endif

constexpr unsigned int mtiMaxNumSamples = 100000u;

#ifdef MTI_ANDROID
	#define DLLEXPORT
#endif
///////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <time.h>
#include "DspFilter.h"

#ifdef _MTIOPENCV
	#include <opencv2/opencv.hpp>
#endif

///////////////////////////////////////////////////////////////////////////////////
class MTIDataGeneratorImpl;

///////////////////////////////////////////////////////////////////////////////////
//MTIDataGenerator encapsulates the content generation API
class DLLEXPORT MTIDataGenerator
{
public:

	/** @name Creation and Termination
	*  Constructor and Destructor of the MTIDataGenerator class
	*/
	//@{
	MTIDataGenerator();
	~MTIDataGenerator();
	//@}

	/** @name Transforms
	*  Transform data for content generation
	*/
	//@{
	/**
	 * @brief BoundingBox calculates the bounding box limits of the given set of points.
	 *
	 * This function processes the xData and yData arrays to find the minimum and maximum values
	 * along the X and Y axes respectively. It returns these minimum and maximum values,
	 * effectively defining the bounding box for the set of points. If the number of points is less than one,
	 * it sets the bounding box limits to zero.
	 *
	 * @param[in]	xData		Array of X position data.
	 * @param[in]	yData		Array of Y position data.
	 * @param[in]	numPoints	The number of points in the arrays xData and yData.
	 *
	 * @param[out]	xMin		The minimum X coordinate from the given set of points.
	 * @param[out]	xMax		The maximum X coordinate from the given set of points.
	 * @param[out]	yMin		The minimum Y coordinate from the given set of points.
	 * @param[out]	yMax		The maximum Y coordinate from the given set of points.
	 */
	void BoundingBox( float* xData, float* yData, unsigned int numPoints, float& xMin, float& xMax, float& yMin, float& yMax );
	
	/**
	 * @brief NormalizeData performs a normalization and rotation on a given set of points.
	 *
	 * This function operates on xData and yData, normalizing their values by a defined amplitude,
	 * centered around their mean, and rotating the set of points by a given angle theta. The range
	 * of normalization is determined based on the maximum and minimum bounds of the points. The rotation
	 * is performed via a standard 2D rotation matrix operation.
	 *
	 * @param[in, out]	xData		Array of X position data, which will be modified in-place.
	 * @param[in, out]	yData		Array of Y position data, which will be modified in-place.
	 * @param[in]		numPoints	The number of points in the arrays xData and yData.
	 * @param[in]		amplitude	The amplitude for normalization.
	 * @param[in]		theta		The angle in radians by which to rotate the data.
	 * @param[in]		xMin		The minimum X coordinate from the given set of points.
	 * @param[in]		xMax		The maximum X coordinate from the given set of points.
	 * @param[in]		yMin		The minimum Y coordinate from the given set of points.
	 * @param[in]		yMax		The maximum Y coordinate from the given set of points.
	 *
	 * @return The number of points processed.
	 */
	unsigned int NormalizeData( float* xData, float* yData, unsigned int numPoints, float amplitude, float theta, float& xMin, float& xMax, float& yMin, float& yMax ); 
	
	/**
	 * @brief AffineTransformData performs an affine transformation (rotation, scaling, translation) on a given set of points.
	 *
	 * This function first checks if rotation or translation is necessary. Rotation is performed only when
	 * the amplitude is not close to 1 or theta is not close to 0. Translation is performed only when either
	 * xof or yof is not close to 0.
	 *
	 * For rotation, the function normalizes the xData and yData around their means and then rotates the points
	 * by the angle theta. It then translates the data if necessary. After the transformation, the xData and yData
	 * are clamped to the range [-1, 1].
	 *
	 * For translation only, the function translates the xData and yData by xof and yof respectively, and then clamps
	 * the data to the range [-1, 1].
	 *
	 * @param[in, out]	xData		Array of X position data, which will be modified in-place.
	 * @param[in, out]	yData		Array of Y position data, which will be modified in-place.
	 * @param[in]		numPoints	The number of points in the arrays xData and yData.
	 * @param[in]		amplitude	The amplitude for normalization.
	 * @param[in]		theta		The angle in radians by which to rotate the data.
	 * @param[in]		xof			The offset for X position data.
	 * @param[in]		yof			The offset for Y position data.
	 *
	 * @return The number of points processed.
	 */
	unsigned int AffineTransformData( float* xData, float* yData, unsigned int numPoints, float amplitude, float theta, float xof, float yof );
	
	/**
	 * @brief RotateData performs transformations (rotations about 3 axes, sequentially) on a given set of points.
	 *
	 * This function first checks if rotation or translation is necessary. Rotation is performed only when
	 * thetas are not close to 0.
	 *
	 * For rotation, the function firstly determines the rotation Origin (offset for x-axis, y-axis, z-axis for the respective rotations).
	 * default (omitting last 3 parameters) and useDataOrigin==true will set that origin to center of data bounding box ('data origin')
	 * omitting last 2 parameters and useDataOrigin==false will set that to 0,0
	 * user entries xOrigin and yOrigin and useDataOrigin==false will set that to xOrigin, yOrigin
	 * Then data is rotated about the offset x-axis, y-axis, z-axis, in that sequential order.
	 * Finally, all data is clamped to the range [-1, 1].
	 *
	 * @param[in, out]	xData		Array of X position data, which will be modified in-place.
	 * @param[in, out]	yData		Array of Y position data, which will be modified in-place.
	 * @param[in]		numPoints	The number of points in the arrays xData and yData.
	 * @param[in]		thetaAboutX	The angle in radians by which to rotate the data about origin, parallel with x-axis
	 * @param[in]		thetaAboutY	The angle in radians by which to rotate the data about origin, parallel with y-axis
	 * @param[in]		thetaAboutZ	The angle in radians by which to rotate the data about origin, parallel with z-axis
	 * @param[in]		useDataOrigin	If true, data is rotated about origin of the data itself (center of bounding box), rest of parameters ignored
	 * @param[in]		xOrigin		if useDataOrigin is false, this value places the rotation origin for thetaAboutY to xOrigin
	 * @param[in]		yOrigin		if useDataOrigin is false, this value places the rotation origin for thetaAboutX to yOrigin
	 *
	 * @return The number of points processed.
	 */
	unsigned int RotateData( float* xData, float* yData, unsigned int numPoints, float thetaAboutX, float thetaAboutY, float thetaAboutZ, bool useDataOrigin = true, float xOrigin = 0.f, float yOrigin = 0.f );
	
	/**
	 * @brief Initializes the projective transformation matrix with the provided data.
	 *
	 * This function creates a 3x3 transformation matrix used for projective transformations of data points.
	 * If x is NULL, the function initializes a basic transformation matrix that either scales and translates
	 * points to the range [-1, 1] or transforms points from the range [-1, 1] to the unit square [0, 1].
	 *
	 * If x is not NULL, the function initializes a more complex projective transformation matrix based on
	 * the four points provided in x and y.
	 *
	 * If the transformation matrix needs to be inverted (e.g., for performing the inverse transformation),
	 * the function calculates and stores the inverse of the matrix.
	 *
	 * @param[in]		x		Array containing the x-coordinates of four points defining a quadrilateral, or NULL.
	 * @param[in]		y		Array containing the y-coordinates of four points defining a quadrilateral, or NULL.
	 * @param[in]		inverse	If true, calculates and stores the inverse of the transformation matrix.
	 */
	void InitProjectiveTransformMatrix( float* x, float* y, bool inverse );
	
	/**
	 * @brief Performs a projective transformation on a given set of points.
	 *
	 * This function applies a projective transformation to the provided xData and yData, which are modified in-place.
	 * The transformation is defined by the matrix initialized by `InitProjectiveTransformMatrix()`. If the inverse flag
	 * was set to true during initialization, the function applies the inverse transformation; otherwise, it applies the
	 * forward transformation.
	 *
	 * The resulting points are then scaled to the range [-1, 1] (for the inverse transformation) or left in the unit
	 * square [0, 1] (for the forward transformation).
	 *
	 * @param[in, out]	xData		Array of X position data, which will be modified in-place.
	 * @param[in, out]	yData		Array of Y position data, which will be modified in-place.
	 * @param[in]		numPoints	The number of points in the arrays xData and yData.
	 *
	 * @return The number of points processed.
	 */
	unsigned int ProjectiveTransformData( float* xData, float* yData, unsigned int numPoints );
	
	/**
	 * @brief Performs a tangential distortion transformation on a given set of points.
	 *
	 * This function applies a tangential distortion to the provided xData and yData, which are modified in-place.
	 * The distortion factor is calculated based on the distance from the origin and the maximum distortion angle (thetaMax).
	 *
	 * @param[in, out]	xData		Array of X position data, which will be modified in-place.
	 * @param[in, out]	yData		Array of Y position data, which will be modified in-place.
	 * @param[in]		numPoints	The number of points in the arrays xData and yData.
	 * @param[in]		thetaMax	Maximum distortion angle in radians.
	 *
	 * @return The number of points processed.
	 */
	unsigned int TangentialTransformData( float* xData, float* yData, unsigned int numPoints, float thetaMax );
	
	/**
	 * @brief Initializes the lookup table used in radial distortion transformations.
	 *
	 * This function calculates and stores a set of multipliers used to quickly perform radial distortions.
	 * It should be called before performing any radial distortions.
	 *
	 * @param[in]		inverse	If true, calculates and stores the inverse transformation multipliers.
	 */
	void InitRadialTransformMult( bool inverse );

	/**
	 * @brief Performs a radial distortion transformation on a given set of points.
	 *
	 * This function applies a radial distortion to the provided xData and yData, which are modified in-place.
	 * The distortion factor is calculated based on the distance from the origin and the radial distortion coefficient (k).
	 *
	 * The function uses a lookup table initialized by `InitRadialTransformMult()` to quickly calculate distortion multipliers.
	 *
	 * @param[in, out]	xData		Array of X position data, which will be modified in-place.
	 * @param[in, out]	yData		Array of Y position data, which will be modified in-place.
	 * @param[in]		numPoints	The number of points in the arrays xData and yData.
	 * @param[in]		k			Radial distortion coefficient.
	 *
	 * @return The number of points processed.
	 */
	unsigned int RadialTransformData( float* xData, float* yData, unsigned int numPoints, float k );

	/**
	 * @brief Performs a sequence of transformations on a given set of points.
	 *
	 * This function applies a sequence of transformations to the provided xData and yData, which are modified in-place.
	 * The sequence includes an affine transformation (always performed), followed by optional projective, radial, and tangential transformations.
	 *
	 * The projective transformation is performed if projXfm is true.
	 * The radial transformation is performed if the absolute value of barXfmK is greater than 1e-3, and requires that `InitRadialTransformMult()` has been called before.
	 * The tangential transformation is performed if the absolute value of tanXfmTheta is greater than 1e-3.
	 *
	 * @param[in, out]	xData		Array of X position data, which will be modified in-place.
	 * @param[in, out]	yData		Array of Y position data, which will be modified in-place.
	 * @param[in]		numPoints	The number of points in the arrays xData and yData.
	 * @param[in]		amplitude	Amplitude for the affine transformation.
	 * @param[in]		theta		Angle for the affine transformation.
	 * @param[in]		xof			X offset for the affine transformation.
	 * @param[in]		yof			Y offset for the affine transformation.
	 * @param[in]		projXfm		Flag indicating whether to perform the projective transformation.
	 * @param[in]		tanXfmTheta	Maximum distortion angle for the tangential transformation.
	 * @param[in]		barXfmK		Radial distortion coefficient for the radial transformation.
	 *
	 * @return The number of points processed.
	 */
	unsigned int TransformData( float* xData, float* yData, unsigned int numPoints, float amplitude, float theta, float xof, float yof, bool projXfm, float tanXfmTheta, float barXfmK );
	//@}

	/** @name Utilities
	*  Utilities for content generation
	*/
	//@{
	int gcd( int a, int b );
	float saw( float theta );
	unsigned int AnimationRotate( float* xData, float* yData, unsigned char* mData, unsigned int numPoints, unsigned int numFrames );
	unsigned int AnimationRotate( float* xData, float* yData, unsigned char* mData, unsigned int* rgbData, unsigned int numPoints, unsigned int numFrames );
	unsigned int AnimationScroll( float* xData, float* yData, unsigned char* mData, unsigned int numPoints, unsigned int numFrames, bool leftToRight = true );
	unsigned int AnimationScroll( float* xData, float* yData, unsigned char* mData, unsigned int* rgbData, unsigned int numPoints, unsigned int numFrames, bool leftToRight = true );
	unsigned int AnimationBounce( float* xData, float* yData, unsigned char* mData, unsigned int numPoints, unsigned int numFrames, float theta );
	unsigned int AnimationBounce( float* xData, float* yData, unsigned char* mData, unsigned int* rgbData, unsigned int numPoints, unsigned int numFrames, float theta );

	/**
	 * @brief Closes a curve described by the input points and modifies the data in-place.
	 *
	 * This function closes a curve by either retracing it or interpolating between the first and the last points.
	 * The operation depends on the `retrace` flag and the `closePoints` value. If `retrace` is true, the curve is retraced - 
	 * points are added to the curve in reverse order from last point to first.
	 * If `retrace` is false and `closePoints` is more than 0, `closePoints` new points are added to the curve by interpolating
	 * from last point back to the first point. If neither condition is satisfied, the curve is not closed and the original data is returned.
	 * Function assumes memory pre-allocation of at least numPoints+closePoints for each array
	 *
	 * @param[in, out] xData		Array of X position data, which will be modified in-place.
	 * @param[in, out] yData		Array of Y position data, which will be modified in-place.
	 * @param[in, out] mData		Array of 8-bit Digital Output data, which will be modified in-place.
	 * @param[in]	  numPoints		The number of points in the curve.
	 * @param[in]	  closePoints	The number of points for the curve closure.
	 * @param[in]	  retrace		Flag indicating whether to retrace the curve.
	 *
	 * @return The number of points processed.
	 */
	unsigned int CloseCurve( float* xData, float* yData, unsigned char* mData, unsigned int numPoints, unsigned int closePoints, bool retrace = false );

	/**
	 * @brief Closes a curve described by the input points and modifies the data in-place. This function is an overload of the previous function
	 * and supports RGB data for use with Mirrorcle's RGB Playzers
	 *
	 * @param[in, out] xData		Array of X position data, which will be modified in-place.
	 * @param[in, out] yData		Array of Y position data, which will be modified in-place.
	 * @param[in, out] mData		Array of 8-bit Digital Output data, which will be modified in-place.
	 * @param[in, out] rgbData		Array of RGB data for each point, which will be modified in-place. NULL if no RGB data.
	 * @param[in]	  numPoints		The number of points in the curve.
	 * @param[in]	  closePoints	The number of points for the curve closure.
	 * @param[in]	  retrace		Flag indicating whether to retrace the curve.
	 *
	 * @return The number of points processed.
	 */
	unsigned int CloseCurve( float* xData, float* yData, unsigned char* mData, unsigned int* rgbData, unsigned int numPoints, unsigned int closePoints, bool retrace = false );

	/** 
	 *  ConcatenateData combines multiple lists of data into a single list with smooth connections with user - defined number of points("connectionPoints")
	 * connectionPoints is defined as the number of points to be interpolated between the last point of each list and the first
	 * point of the next list (exclusive of those two points), using triangular (velocity) interpolation.
	 *
	 *	Function returns the number of points in the concatenated list (in output arrays xConc, yConc, mConc).
	 * (xConc, yConc, mConc) should be allocated for enough space to contain arrays of length at least:
	 * (sum of the elements of numPoints + (connectionPoints * numLists))
	 *
	 * If a list of data has been closed where the last point is the same as the first point 
	 * and points preceding the last have an M value of 0 (i.e. it has been passed through `CloseCurve()`),
	 * then the last point and all preceding points with M of 0 until the first non-zero will be removed as an optimization 
	 * during concatenation.
	 *
	 * @param[in]	xData	Array of X position arrays 
	 * @param[in]	yData	Array of Y position arrays
	 * @param[in]	mData	Array of M digital output number arrays
	 *
	 *	@param[in]	numLists	The number of lists to be concatenated. xData, yData, mData must have at least this many arrays.
	 *							i.e. the number of pointers in xData/yData/mData to be concatenated
	 *
	 *	@param[in]	numPoints	Array containing the number of points in each list. The length of the array should be at least
	 *							equivalent (or greater) than numLists.
	 *
	 *	@param[out]	xConc	Resulting X position data after concatenation
	 *	@param[out]	yConc	Resulting Y position data after concatenation
	 *	@param[out]	mConc	Resulting M digital output data after concatenation
	 *
	 *	@param[in]	connectionPoints	The number of points to be interpolated between the last point of 
	 *									each list and the first point of the next list (defaults to 0 if not used)
	 */
	unsigned int ConcatenateData( float** xData, float** yData, unsigned char** mData, float* xConc, float* yConc, unsigned char* mConc, unsigned int numLists, unsigned int* numPoints, unsigned int connectionPoints = 0 );
	//@}

	/** @name Waveforms
	*  Useful waveforms for content generation
	*/
	//@{
	/**
	 * @brief Generates a sine waveform.
	 *
	 * This function generates a sine waveform with a given amplitude, frequency, and phase. It can operate in two modes
	 * depending on whether yData is provided or not. If the pointer for yData is null, the function generates a 1D sine waveform (stored in xData).
	 * This waveform has N complete periods of a sine waveform noted by 'frequency' parameter described by numPoints samples.
	 * If yData is provided as a non-null address, the function generates a sine waveform "graph" (x-axis of the graph with points uniformly
	 * increasing from -1 to +1 is in xData and sin() is in yData again with 'frequency' periods, and given amplitude and phase offset).
	 *
	 * @param[in] xData Pointer to the array for X data samples, should be pre-allocated.
	 * @param[in] yData Pointer to the array for Y data samples, should be pre-allocated.
	 * @param[in] mData Pointer to the array for M data samples, should be pre-allocated.
	 * @param[in] numPoints Number of points to be generated for the waveform.
	 * @param[in] amplitude Amplitude of the sine waveform.
	 * @param[in] frequency Frequency of the sine waveform.
	 * @param[in] phase Phase of the sine waveform.
	 *
	 * @return Number of points in the generated waveform.
	 */
	unsigned int SineWaveform( float* xData, float* yData, unsigned char* mData, unsigned int numPoints, float amplitude, unsigned int frequency, float phase );

	/**
	 * @brief Generates a sawtooth waveform.
	 *
	 * This function generates a sawtooth waveform with a given amplitude, frequency, width, and phase. It can operate in two modes
	 * depending on whether yData is provided or not. If yData is null, the function generates a 1D sawtooth waveform (stored in xData).
	 * If yData is provided as a non-null address, the function generates a 2D sawtooth waveform (stored in xData and yData).
	 *
	 * @param[in] xData Pointer to the array for X data samples, should be pre-allocated.
	 * @param[in] yData Pointer to the array for Y data samples, should be pre-allocated.
	 * @param[in] mData Pointer to the array for M data samples, should be pre-allocated.
	 * @param[in] numPoints Number of points to be generated for the waveform.
	 * @param[in] amplitude Amplitude of the sawtooth waveform.
	 * @param[in] frequency Frequency of the sawtooth waveform.
	 * @param[in] width Width (duty cycle) of the sawtooth waveform.
	 * @param[in] phase Phase of the sawtooth waveform.
	 *
	 * @return Number of points in the generated waveform.
	 */
	unsigned int SawtoothWaveform( float* xData, float* yData, unsigned char* mData, unsigned int numPoints, float amplitude, unsigned int frequency, float width, float phase );

	/**
	 * @brief Generates a square waveform.
	 *
	 * This function generates a square waveform with a given amplitude, frequency, width, and phase. It can operate in two modes
	 * depending on whether yData is provided or not. If yData is null, the function generates a 1D square waveform (stored in xData).
	 * If yData is provided as a non-null address, the function generates a 2D square waveform (stored in xData and yData).
	 *
	 * @param[in] xData Pointer to the array for X data samples, should be pre-allocated.
	 * @param[in] yData Pointer to the array for Y data samples, should be pre-allocated.
	 * @param[in] mData Pointer to the array for M data samples, should be pre-allocated.
	 * @param[in] numPoints Number of points to be generated for the waveform.
	 * @param[in] amplitude Amplitude of the square waveform.
	 * @param[in] frequency Frequency of the square waveform.
	 * @param[in] width Width (duty cycle) of the square waveform.
	 * @param[in] phase Phase of the square waveform.
	 *
	 * @return Number of points in the generated waveform.
	 */
	unsigned int SquareWaveform( float* xData, float* yData, unsigned char* mData, unsigned int numPoints, float amplitude, unsigned int frequency, float width, float phase );

	/**
	 * @brief Generates a Lissajous waveform.
	 *
	 * This function generates a Lissajous waveform with the given parameters. The Lissajous waveform is determined by the type of wave,
	 * the amplitudes and frequencies for x and y, the phase, and the modulation parameters.
	 * Modulation can be none, amplitude modulation, unipolar amplitude modulation, or frequency modulation.
	 *
	 * @param[in] xData Pointer to the array for X data samples, should be pre-allocated.
	 * @param[in] yData Pointer to the array for Y data samples, should be pre-allocated.
	 * @param[in] mData Pointer to the array for M data samples, should be pre-allocated.
	 * @param[in] numPoints Number of points to be generated for the waveform.
	 * @param[in] waveType Type of the waveform. Determines if the waveform is sinusoidal or sawtooth.
	 * @param[in, out] sps Sampling rate that is calculated in the function based on the number of points and a 5 second
	 *						total waveform time.
	 * @param[in] xAmplitude Amplitude of the X waveform.
	 * @param[in] yAmplitude Amplitude of the Y waveform.
	 * @param[in] xFreq Frequency of the X waveform.
	 * @param[in] yFreq Frequency of the Y waveform.
	 * @param[in] phase Phase of the waveform.
	 * @param[in] modType Modulation type of the waveform. Can be `ModulationType::ModulationNone`, `ModulationType::ModulationAmpl`, 
		`ModulationType::ModulationAmplUnipolar`, `ModulationType::ModulationFreq`, or any other value (which defaults to `ModulationType::ModulationAmplUnipolar`).
	 * @param[in] modAmplitude Modulation amplitude of the waveform.
	 * @param[in] modFreq Modulation frequency of the waveform.
	 * @param[in] modPhase Modulation phase of the waveform.
	 *
	 * @return Number of points in the generated waveform.
	 */
	unsigned int LissajousWaveform( float* xData, float* yData, unsigned char* mData, unsigned int numPoints, unsigned int waveType, unsigned int& sps, float xAmplitude, float yAmplitude, float xFreq, float yFreq, float phase, unsigned int modType, float modAmplitude, float modFreq, float modPhase );

	/**
	 * @brief Generates a random noise waveform.
	 *
	 * This function generates a random noise waveform with a given amplitude.
	 *
	 * @param[in] xData Pointer to the array for X data samples, should be pre-allocated.
	 * @param[in] numPoints Number of points to be generated for the waveform.
	 * @param[in] amplitude Amplitude of the random noise.
	 *
	 * @return Number of points in the generated waveform.
	 */
	unsigned int NoiseWaveform( float* xData, unsigned int numPoints, float amplitude );

	/**
	 * @brief Generates Resonant-Quasistatic (RQ) waveform data where one axis of the MEMS is driven at resonance while the other 
	 * is driven point-to-point.
	 *
	 * The function attempts to generate an RQ waveform where the xData will contain the fast-axis (resonant) waveform while the 
	 * yData will contain the slow-axis (quasi-static) waveform. It will adhere to the requested frequency parameters
	 * up to an enforced limit for sample rate (sps) of 90,000 and the maximum samples per frame of the USB MEMS Controller.
	 * 
	 * The slow-axis waveform on Y-axis can be either sawtooth or sine based on the boolean flag sawtoothOnY. 
	 *
	 * Different DOut (M) bits are enabled and disabled at specific events/conditions throughout the generation of data.
	 *
	 * @param[out] xData Pointer to output array for X position data, should be pre-allocated.
	 * @param[out] yData Pointer to output array for Y position data, should be pre-allocated.
	 * @param[out] mData Pointer to output array for M digital output data, should be pre-allocated.
	 * @param[in,out] sps Sampling rate that is calculated in the function based on xFrequency and points per X period.
	 * @param[in] sawtoothOnY Boolean flag to decide if the wave on Y-axis should be sawtooth (true) or sine (false).
	 * @param[in] xAmplitude Amplitude for X position data.
	 * @param[in] yAmplitude Amplitude for Y position data.
	 * @param[in] xFrequency Frequency for X position data.
	 * @param[in] numPeriods Number of periods for the waveform.
	 * @param[in] dutyCycle Duty cycle for the waveform.
	 * @param[in] yBandwidth Bandwidth of the software filter applied to the Y position data.
	 *
	 * @return Number of points in the generated waveform.
	 *
	 * @note DOut bits have the following functionality:
	 * - DOut0, DOut6, DOut7: Turn on laser at every point and keep it bright.
	 * - DOut1: Show half sample rate.
	 * - DOut2: Indicate horizontal direction (high for each left-right scan).
	 * - DOut3: Indicate horizontal active line (may need phase adjust) for each left-right scan.
	 * - DOut4: Vsync. High during the rising portion, low during the falling portion of the waveform.
	 * - DOut5: Frame sync at start of complete pattern.
	 */
	unsigned int RQWaveform( float* xData, float* yData, unsigned char* mData, unsigned int& sps, bool sawtoothOnY, float xAmplitude, float yAmplitude, unsigned int xFrequency,
		unsigned int numPeriods, float dutyCycle, float yBandwidth );
	//@}

	/** @name Curves
	*  Curves for content generation
	*/
	//@{
	unsigned int SpiralCurve( float* xData, float* yData, unsigned char* mData, unsigned int numPoints, float xAmplitude, float yAmplitude, unsigned int frequency, float phase );
	unsigned int PolygonCurve( float* xData, float* yData, unsigned char* mData, unsigned int numPoints, float amplitude, unsigned int frequency, float phase );
	unsigned int SpirographCurve( float* xData, float* yData, unsigned char* mData, unsigned int numPoints, unsigned int curveType, unsigned int waveType, float amplitude, int p1, int p2, int p3 );
	unsigned int LissajousCurve( float* xData, float* yData, unsigned char* mData, unsigned int numPoints, unsigned int waveType, float amplitude, unsigned int xFreq, unsigned int yFreq, float phase,	unsigned int modType, float modAmplitude, unsigned int modFreq, float modPhase );
	unsigned int LissajousCurve( float* xData, float* yData, unsigned char* mData, unsigned int numPoints, unsigned int waveType, float xAmplitude, float yAmplitude, unsigned int xFreq, unsigned int yFreq, float phase, unsigned int modType, float modAmplitude, unsigned int modFreq, float modPhase );
	unsigned int AnalogClockCurve( float* xData, float* yData, unsigned char* mData, unsigned int hrs, unsigned int min, unsigned int sec, float amplitude );
	unsigned int DigitalClockCurve( float* xData, float* yData, unsigned char* mData, unsigned int hrs, unsigned int min, unsigned int sec, unsigned int fontIndex, float amplitude, float theta, bool retrace );
	unsigned int DynamicsCurve( float* xData, float* yData, unsigned char* mData, unsigned int numPoints, float x0, float y0, float xVel, float yVel, float xAcc, float yAcc, float fRatio );
	//@}

	/** @name ILDA Files
	*  Tools to load and display ILDA files
	*/
	//@{
	/**
	 * @brief Clears ILDA data arrays to prepare for new data.
	 *
	 * This function deletes all current ILDA data arrays to clean up memory
	 * and to prepare for loading new data.
	 */
	void ClearIldaData();
	
	/**
	 * @brief Loads ILDA file.
	 *
	 * This function opens an ILDA file, clears any previous ILDA data, and then
	 * loads data from the file into corresponding arrays. It reads data in various
	 * formats, then processes and stores the data for use in subsequent functions.
	 *
	 * @param[in] filename The name of the ILDA file to load.
	 */
	void LoadIldaFile( const char* filename );

	/**
	 * @brief Retrieves the Frame IDs of the loaded ILDA file.
	 *
	 * This function returns a pointer to the array of frame IDs read from the ILDA file.
	 *
	 * @return A pointer to the array of frame IDs.
	 */
	unsigned int* IldaFrameIds();
	
	/**
	 * @brief Retrieves the Frame Type of the loaded ILDA file.
	 *
	 * This function returns the frame type read from the ILDA file.
	 *
	 * @return The frame type of the ILDA file.
	 */
	unsigned int IldaFrameType();
	
	/**
	 * @brief Retrieves the number of Frames in the loaded ILDA file.
	 *
	 * This function returns the total number of frames read from the ILDA file.
	 *
	 * @return The number of frames in the ILDA file.
	 */
	unsigned int IldaNumFrames();

	/**
	 * @brief Retrieves the maximum number of points in any frame of the loaded ILDA file.
	 *
	 * This function returns the maximum number of points found in any frame of the ILDA file.
	 *
	 * @return The maximum number of points in any frame of the ILDA file.
	 */
	unsigned int IldaNumMaxPoints();

	/**
	 * @brief Retrieves the total number of points across all frames in the loaded ILDA file.
	 *
	 * This function returns the total number of points found across all frames of the ILDA file.
	 *
	 * @return The total number of points in the ILDA file.
	 */
	unsigned int IldaNumTotalPoints();

	/**
	 * @brief Retrieves the number of points in a specific frame of the loaded ILDA file.
	 *
	 * This function returns the number of points in the specified frame.
	 *
	 * @param[in] frameId The ID of the frame for which to retrieve the number of points.
	 * 
	 * @return The number of points in the specified frame.
	 */
	unsigned int IldaFrameSize( unsigned int frameId );

	/**
	 * @brief Calculates the total number of points in a subset of frames from the loaded ILDA file.
	 *
	 * This function sums the number of points across a specified subset of frames.
	 *
	 * @param[in] frameIds An array of frame IDs to include in the total.
	 * @param[in] numFrames The number of frames specified in the frameIds array.
	 * 
	 * @return The total number of points in the specified frames.
	 */
	unsigned int IldaAnimSize( unsigned int* frameIds, unsigned int numFrames );

	/**
	 * @brief Generates a data stream from ILDA file data without color information. 
	 
	 * IldaDataStream retrieves the frames specified by frameIds from the ILDA file and transforms them based on the given parameters. 
	 * It also adjusts the output arrays xData, yData and mData with the data of each point in the frames. 
	 
	 * It applies a 3D transformation using the provided theta, theta1 and theta2 parameters. Retrace parameter allows users to close 
	 * open curves in the data for retracing. If retrace is true, it will close any open curves in the data to make them suitable for retracing.
	 *
	 * @param[in,out] xData Pointer to a float array where x coordinate data will be stored. Must be pre-allocated.
	 * @param[in,out] yData Pointer to a float array where y coordinate data will be stored. Must be pre-allocated.
	 * @param[in,out] mData Pointer to an unsigned char array where Digital Output data will be stored. Must be pre-allocated.
	 * @param[in] frameIds An array of frame IDs to include in the data stream.
	 * @param[in] numFrames The number of frames specified in the frameIds array.
	 * @param[in] amplitude The amplitude of the laser scan.
	 * @param[in] theta The tilt angle of the laser scan.
	 * @param[in] theta1 The first rotation angle used in 3D transformation.
	 * @param[in] theta2 The second rotation angle used in 3D transformation.
	 * @param[in] retrace If true, open curves in the data will be closed for retrace.
	 * 
	 * @return Returns the number of points in the generated data stream.
	 */
	unsigned int IldaDataStream( float* xData, float* yData, unsigned char* mData, unsigned int* frameIds, unsigned int numFrames, float amplitude, float theta, float theta1, float theta2, bool retrace );
	
	/**
	 * @brief Generates a data stream from ILDA file data with color information. Intended for use with RGB Playzers.
	 *
	 * @param[in,out] xData Pointer to a float array to store x coordinate data.
	 * @param[in,out] yData Pointer to a float array to store y coordinate data.
	 * @param[in,out] mData Pointer to an unsigned char array to store 8-bit Digital Output data.
	 * @param[in,out] rgbData Pointer to an unsigned int array to store color data. If NULL, no color data is included.
	 * @param[in] frameIds An array of frame IDs to include in the data stream.
	 * @param[in] numFrames The number of frames specified in the frameIds array.
	 * @param[in] amplitude The amplitude of the laser scan.
	 * @param[in] theta The tilt angle of the laser scan.
	 * @param[in] theta1 The first rotation angle in 3D transformation.
	 * @param[in] theta2 The second rotation angle in 3D transformation.
	 * @param[in] retrace If true, open curves in the data will be closed for retrace.
	 * @return Returns the number of points in the generated data stream.
	 */
	unsigned int IldaDataStream( float* xData, float* yData, unsigned char* mData, unsigned int* rgbData, unsigned int* frameIds, unsigned int numFrames, float amplitude, float theta, float theta1, float theta2, bool retrace );

	/**
	 * @brief Generates and animates keypoints into samples from information loaded from an ILDA file. This overload does not include RGB data.
	 *
	 * This function interpolates and optionally animates keypoint data loaded from an ILDA file to create the sample data that
	 * can be sent to the USB MEMS Controller.
	 * 
	 * If numFrames > 1, GenerateIldaSampleData will create an animation according to the atime parameter.
	 * Otherwise, it will only interpolate keypoints to samples.
	 * 
	 * When animating, numSamples will be discarded and the function will adhere to the refresh rate (rr),
	 * animation time (atime), and number of frames (numFrames) constraints. When not animating, numSamples
	 * determines the target number of samples in the interpolation.
	 *
	 * @param[in] xKey Pointer to array of X position keypoints
	 * @param[in] yKey Pointer to array of Y position keypoints
	 * @param[in] mKey Pointer to array of M digital output keypoints
	 * @param[out] xSample Pointer to output array for X position samples, should be pre-allocated.
	 * @param[out] ySample Pointer to output array for Y position samples, should be pre-allocated.
	 * @param[out] mSample Pointer to output array for M digital output samples, should be pre-allocated.
	 * @param[in] numKeyPoints Number of keypoints provided in each input array
	 * @param[in] numSamples Desired number of samples to be generated per frame
	 * @param[in] numFrames Number of frames to generate in the animation
	 * @param[in] rr Frame rate to adhere to during animation
	 * @param[in] interpIlda If true, interpolate ILDA data, if false treat key points directly as samples
	 * @param[in] atime Animation time in seconds for the full animation.
	 * @param[in] retrace If true, enable retracing (MEMS retraces the path to the first sample)
	 * 
	 * @return Number of points in the generated samples
	 */
	unsigned int GenerateIldaSampleData( float* xKey, float* yKey, unsigned char* mKey, float* xSample, float* ySample, unsigned char* mSample, unsigned int numKeyPoints, unsigned int numSamples, unsigned int numFrames, float rr, bool interpIlda, float atime, bool retrace );
	unsigned int GenerateIldaSampleData( float* xKey, float* yKey, unsigned char* mKey, unsigned int* rgbKey, float* xSample, float* ySample, unsigned char* mSample, unsigned int* rgbSample, unsigned int numKeyPoints, unsigned int numSamples, unsigned int numFrames, float rr, bool interpIlda, float atime, bool retrace );

	unsigned int CurvesDataSize( unsigned int curveType, unsigned int sps, float rr, unsigned int animType, float atime, bool retrace, unsigned int fileType, unsigned int m1, unsigned int m2, float p2, float p3, float p4, const char* txt, unsigned int numChars );
	//@}

	/** @name Hershey Text
	*  Tools to generate Hershey text
	*/
	//@{
	void ClearHersheyData();

	/**
	 * @brief Loads data from a Hershey file into the MTIDataGenerator object.
	 *
	 * This function reads data from the provided Hershey file, such as the number of characters, fonts, mathematical symbols, etc.
	 * The data is then stored into the object's member variables and arrays.
	 * 
	 * Typically used with the provided hershey.dat file.
	 *
	 * @param[in] filename The path to the Hershey file to be loaded.
	 */
	void LoadHersheyFile( const char* filename );

	/**
	 * @brief Retrieves the number of mathematical symbols in the loaded Hershey file.
	 *
	 * @return The number of mathematical symbols.
	 */
	unsigned int HersheyNumMath();

	/**
	 * @brief Retrieves the number of symbols in the loaded Hershey file.
	 *
	 * @return The number of symbols.
	 */
	unsigned int HersheyNumSym();

	/**
	 * @brief Retrieves the number of original representation characters in the loaded Hershey file.
	 *
	 * @return The number of original representation characters.
	 */
	unsigned int HersheyNumOR();

	/**
	 * @brief Retrieves the number of orthogonal coordinates characters in the loaded Hershey file.
	 *
	 * @return The number of orthogonal coordinates characters.
	 */
	unsigned int HersheyNumOC();

	/**
	 * @brief Retrieves the index of a mathematical symbol in the loaded Hershey file.
	 *
	 * @param[in] i Index of the mathematical symbol to retrieve.
	 * @return The index of the mathematical symbol. If the index is out of range, return (unsigned int)-1.
	 */
	unsigned int HersheyIndexMath( unsigned int i );

	/**
	 * @brief Retrieves the index of a symbol in the loaded Hershey file.
	 *
	 * @param[in] i Index of the symbol to retrieve.
	 * @return The index of the symbol. If the index is out of range, return (unsigned int)-1.
	 */
	unsigned int HersheyIndexSym( unsigned int i );

	/**
	 * @brief Retrieves the index of a font in the loaded Hershey file.
	 *
	 * @param[in] i Index of the font to retrieve.
	 * @param[in] j Index of the character within the font.
	 * @return The index of the font. If the index is out of range, return (unsigned int)-1.
	 */
	unsigned int HersheyIndexFont( unsigned int i, unsigned int j );

	/**
	 * @brief Calculates the total size of Hershey data based on provided character ids.
	 *
	 * This function calculates the total size of Hershey data points based on the provided character ids and mode.
	 * The ocMode determines whether to use orthogonal coordinates (true) or original representation (false).
	 *
	 * @param[in] charIds Pointer to an array of character ids.
	 * @param[in] numChars Number of character ids provided.
	 * @param[in] ocMode If true, use orthogonal coordinates mode. If false, use original representation.
	 * @return The total number of data points.
	 */
	unsigned int HersheyDataSize( unsigned int* charIds, unsigned int numChars, bool ocMode );

	/**
	 * @brief Calculates the total size of Hershey data based on provided text.
	 *
	 * This function calculates the total size of Hershey data points based on the provided text and font index.
	 * The text is mapped to character ids using the HersheyIndexFont function. The function currently uses the orthogonal coordinates mode.
	 *
	 * @param[in] text Pointer to the text string.
	 * @param[in] numChars Number of characters in the text.
	 * @param[in] fontIndex Index of the font to be used.
	 * @return The total number of data points.
	 */
	unsigned int HersheyDataSize( const char* text, unsigned int numChars, unsigned int fontIndex );

	/**
	 * @brief Generates a data stream representing a sequence of Hershey characters.
	 *
	 * This function creates a sequence of Hershey characters given their IDs and streams the generated data into the provided arrays.
	 * The function also provides the option to normalize and close the data curve. Special handling is provided for the characters "O", "o", and "0".
	 *
	 * @param[out] xData Pointer to the array where the X data points will be stored.
	 * @param[out] yData Pointer to the array where the Y data points will be stored.
	 * @param[out] mData Pointer to the array where the M data points will be stored.
	 * @param[in]  charIds Pointer to an array of character ids.
	 * @param[in]  numChars Number of characters to generate.
	 * @param[in]  ocMode If true, use orthogonal coordinates mode. If false, use original representation.
	 * @param[in]  amplitude The amplitude for the normalization of the data.
	 * @param[in]  theta The rotation angle (in radians) for the normalization of the data.
	 * @param[in]  retrace If true, the curve will be closed by retracing it in reverse.
	 * @return The total number of data points generated.
	 */
	unsigned int HersheyDataStream( float* xData, float* yData, unsigned char* mData, unsigned int* charIds, unsigned int numChars, bool ocMode, float amplitude, float theta, bool retrace );
	
	/**
	 * @brief Generates a data stream representing a sequence of Hershey characters from a text string.
	 *
	 * This function creates a sequence of Hershey characters given a text string and streams the generated data into the provided arrays.
	 * The function also provides the option to normalize and close the data curve. It always uses orthogonal coordinates mode.
	 *
	 * @param[out] xData Pointer to the array where the X data points will be stored.
	 * @param[out] yData Pointer to the array where the Y data points will be stored.
	 * @param[out] mData Pointer to the array where the M data points will be stored.
	 * @param[in]  text The text string to generate Hershey data for.
	 * @param[in]  numChars Number of characters in the text string.
	 * @param[in]  fontIndex Index of the font to be used.
	 * @param[in]  amplitude The amplitude for the normalization of the data.
	 * @param[in]  theta The rotation angle (in radians) for the normalization of the data.
	 * @param[in]  retrace If true, the curve will be closed by retracing it in reverse.
	 * @return The total number of data points generated.
	 */
	unsigned int HersheyDataStream( float* xData, float* yData, unsigned char* mData, const char* text, unsigned int numChars, unsigned int fontIndex, float amplitude, float theta, bool retrace );
	//@}


	/** @name Point Files
	*  Point files have 3 columns, for X data (-1 to 1), Y data (-1 to 1), and M data (0-255 for digital output)
	*  File may specify sample per second rate SPS in the first line
	*/
	//@{

	/**
	 * @brief Deletes and clears any point data currently loaded into the file buffers.
	 *
	 * This function removes all point data that was previously loaded from a point file.
	 */
	void ClearPointFileData();

	/**
	 * @brief Allocates space for point file data.
	 *
	 * This function creates new arrays for the X, Y, and M data of the point file if the specified number
	 * of points is greater than the currently allocated space. The number of points is then updated.
	 *
	 * @param[in] numPoints The number of points to allocate space for.
	 */
	void AllocPointFileData( unsigned int numPoints );

	/**
	 * @brief Loads point data from a specified file.
	 *
	 * This function opens a file, reads its lines, and loads the point data into internal (object) X, Y, and M arrays.
	 * If the file specifies a samples per second rate (SPS) in first line, this is also internally recorded.
	 * The number of points read from the file is returned.
	 *
	 * @param[in] filename The name of the file to load.
	 * @return The number of points loaded from the file.
	 */
	unsigned int LoadPointFile( const char* filename );

	/**
	 * @brief Copies point data loaded from file to the specified output buffers.
	 *
	 * The function copies X, Y, and M data from the internal (object) arrays to the specified output arrays.
	 *
	 * @param[out] xData The X data output buffer.
	 * @param[out] yData The Y data output buffer.
	 * @param[out] mData The M data output buffer.
	 * @return The number of points copied to the output buffers.
	 */
	unsigned int PointFileDataStream( float* xData, float* yData, unsigned char* mData );

	/**
	 * @brief Retrieves the size of the loaded point file.
	 *
	 * This function returns the number of points loaded from the most recent file.
	 *
	 * @return The number of points in the loaded point file.
	 */
	unsigned int GetPointFileSize();

	/**
	 * @brief Retrieves the samples per second rate of the loaded point file.
	 *
	 * This function returns the samples per second rate specified by the most recent file.
	 * If loaded point file did not specify sample rate (SPS or sps) value is 0
	 *
	 * @return The samples per second rate of the loaded point file.
	 */
	unsigned int GetPointFileSps();

	/**
	 * @brief Writes a data stream to a specified file.
	 *
	 * This function opens a file and writes the specified X, Y, and M data, along with the sampling rate
	 * in samples per second (if specified > 0).
	 *
	 * @param[in] filename The name of the file to write to.
	 * @param[in] xData The X data.
	 * @param[in] yData The Y data.
	 * @param[in] mData The M data.
	 * @param[in] numPoints The number of points to write.
	 * @param[in] sps The samples per second rate.
	 */
	void ExportFile( const char* filename, float* xData, float* yData, unsigned char* mData, unsigned int numPoints, unsigned int sps = 0 );

	/**
	 * @brief Reads a specified parameter from a file.
	 *
	 * This function opens a file, searches for the specified parameter, and returns its value.
	 * If the parameter is not found, the default value is returned.
	 *
	 * @param[in] filename The name of the file to read from.
	 * @param[in] paramName The name of the parameter to read.
	 * @param[in] paramValue The default value to return if the parameter is not found.
	 * @return The value of the parameter read from the file, or the default value if not found.
	 */
	static float ReadFileParameter( const char* filename, const char *paramName, float paramValue = 0.f );
	//@}

	/** @name Point-Time Files - Files
	*  Point-Time files have 4 columns, for X data (-1 to 1 float), Y data (-1 to 1 float), M data (0-255 for digital output), and T data (0 to 1 float) for time duration at specified XYM values
	*  File may specify sample per second rate SPS in the first line
	*/
	//@{
	void ClearPointTimeFileData();
	void AllocPointTimeFileData( unsigned int numPoints );

	/**
	 * @brief Loads point (keypoint) and time data from a specified file.
	 *
	 * This function opens a file, reads its lines, and loads the point and time data into internal (object) X, Y, M,
	 * and T arrays. If the file specifies a samples per second rate (SPS), this is also internally recorded.
	 * The number of points read from the file is returned.
	 *
	 * @param[in] filename The name of the file to load.
	 * @return The number of points loaded from the file.
	 */
	unsigned int LoadPointTimeFile( const char* filename );

	/**
	 * @brief Copies point and time data loaded from file to the specified output buffers.
	 *
	 * The function copies X, Y, M, and T data from the internal (object) arrays to the specified output arrays.
	 *
	 * @param[out] xData The X data output buffer.
	 * @param[out] yData The Y data output buffer.
	 * @param[out] mData The M data output buffer.
	 * @param[out] tData The T (time) data output buffer.
	 * @return The number of points copied to the output buffers.
	 */
	unsigned int PointTimeFileDataStream( float* xData, float* yData, unsigned char* mData, float* tData );

	/**
	 * @brief Retrieves the size of the loaded Point-Time file.
	 *
	 * This function returns the number of points loaded from the most recent file.
	 *
	 * @return The number of points in the loaded Point-Time file.
	 */
	unsigned int GetPointTimeFileSize();

	 /**
	 * @brief Retrieves the samples per second rate of the loaded Point-Time file.
	 *
	 * This function returns the samples per second rate specified by the most recent file.
	 * If loaded point file did not specify sample rate (SPS or sps) value is 0
	 *
	 * @return The samples per second rate of the loaded point time file.
	 */
	unsigned int GetPointTimeFileSps();
	//@}

	/** @name Interpolation
	*  Functions for interpolating data using various interpolation methodologies
	*/
	//@{

	/**
	 * @brief Clears interpolation data by deallocating the memory used by the array `m_fDs`.
	 */
	void ClearInterpData();

	/**
	 * @brief Allocates memory for the interpolation data array (m_fDs) with the specified number of key points.
	 *
	 * The number of key points is rounded up to the nearest multiple of 10,000 before allocation. If the newly calculated
	 * number of key points exceeds the maximum number of key points currently allocated (m_iMaxKeyPoints), the existing
	 * memory for `m_fDs` is deallocated and reallocated with the new size.
	 * 
	 * Note: Users typically will not need to use this function when calling the InterpolateData function family,
	 * as these functions make calls to `AllocInterpData` on their own.
	 *
	 * @param[in] numKeyPoints The initial number of key points.
	 * @param[in] numSamplePoints The number of sample points (currently unused).
	 */
	void AllocInterpData( unsigned int numKeyPoints, unsigned int numSamplePoints );

	/**
	 * @brief Interpolates keypoint data using either linear or trapezoidal interpolation methodologies
	 *
	 * This function interpolates keypoint data to create sample data. The interpolation methodology is selected based on the
	 * provided `ooRatio`, `onFrac`, and `offFrac` parameters.
	 * 
	 * If `onFrac` and `offFrac` are 0, then `InterpolateDataLinear()` is used.
	 * Otherwise, `InterpolateDataTrapezoidal()` is used.
	 *
	 * @param[in] xKey Pointer to array of X position keypoints
	 * @param[in] yKey Pointer to array of Y position keypoints
	 * @param[in] mKey Pointer to array of M digital output keypoints
	 * @param[out] xSample Pointer to output array for X position samples, should be pre-allocated.
	 * @param[out] ySample Pointer to output array for Y position samples, should be pre-allocated.
	 * @param[out] mSample Pointer to output array for M digital output samples, should be pre-allocated.
	 * @param[in] numKeyPoints Number of keypoints provided in each input array
	 * @param[in] numSamplePoints Desired number of samples to be generated
	 * @param[in] ooRatio On-off ratio used to adjust keypoint interpolation
	 * @param[in] onFrac Proportional acceleration distance for laser-on segments: 0.05 to 0.15 are recommended
	 * @param[in] offFrac Proportional acceleration distance for laser-off segments: best results with 0.5
	 *	Values >0.5 will break the function's behavior as you need at least as much time to decelerate as accelerate
	 *
	 * @return Number of points in the generated samples data stream
	 */
	unsigned int InterpolateData( float* xKey, float* yKey, unsigned char* mKey, float* xSample, float* ySample, unsigned char* mSample, unsigned int numKeyPoints, unsigned int numSamplePoints, float ooRatio = 2.5f, float onFrac = 0.1f, float offFrac = 0.5f );

	/**
	 * @brief Performs linear interpolation of keypoint data to generate sample data
	 *
	 * This function interpolates keypoint data to create sample data using linear interpolation methodology.
	 *
	 * @param[in] xKey Pointer to array of X position keypoints
	 * @param[in] yKey Pointer to array of Y position keypoints
	 * @param[in] mKey Pointer to array of M digital output keypoints
	 * @param[out] xSample Pointer to output array for X position samples, should be pre-allocated.
	 * @param[out] ySample Pointer to output array for Y position samples, should be pre-allocated.
	 * @param[out] mSample Pointer to output array for M digital output samples, should be pre-allocated.
	 * @param[in] numKeyPoints Number of keypoints provided in each input array
	 * @param[in] numSamplePoints Desired number of samples to be generated
	 * @param[in] ooRatio On-off ratio used to adjust keypoint interpolation
	 *
	 * @return Number of points in the generated samples data stream
	 */
	unsigned int InterpolateDataLinear( float* xKey, float* yKey, unsigned char* mKey, float* xSample, float* ySample, unsigned char* mSample, unsigned int numKeyPoints, unsigned int numSamplePoints, float ooRatio = 2.5f );

	/**
	 * @brief Interpolates keypoint data into sample data using trapezoidal interpolation.
	 *
	 * Trapezoidal interpolation is especially useful in this context where the MEMS mirror's velocity needs to be controlled.
	 * `ooRatio`, `onFrac`, and `offFrac` are the key parameters used to tune the velocity profile of the MEMS as the position
	 * samples are sampled to the device at a constant sample rate.
	 *
	 * @param[in] xKey Pointer to array of X position keypoints
	 * @param[in] yKey Pointer to array of Y position keypoints
	 * @param[in] mKey Pointer to array of M digital output keypoints
	 * @param[out] xSample Pointer to output array for X position samples, should be pre-allocated.
	 * @param[out] ySample Pointer to output array for Y position samples, should be pre-allocated.
	 * @param[out] mSample Pointer to output array for M digital output samples, should be pre-allocated.
	 * @param[in] numKeyPoints Number of keypoints provided in each input array
	 * @param[in] numSamplePoints Desired number of samples to be generated (the target length of the output data stream).
	 * @param[in] ooRatio On-off ratio used in linear interpolation. 
		Function clamps value to [1,10].
	 * @param[in] onFrac Proportional acceleration distance for laser-on segments: 0.05 to 0.15 are recommended.
	 *	Function clamps value [0,0.5].
	 * @param[in] offFrac Proportional acceleration distance for laser-off segments: best results with 0.5 
	 *	Values >0.5 will break the function's behavior as you need at least as much time to decelerate as accelerate
	 *	Therefore the function clamps value [0,0.5].
	 *
	 * @return Number of points in the generated samples
	 */
	unsigned int InterpolateDataTrapezoidal( float* xKey, float* yKey, unsigned char* mKey, float* xSample, float* ySample, unsigned char* mSample, unsigned int numKeyPoints, unsigned int numSamplePoints, float ooRatio = 2.5f, float onFrac = 0.1f, float offFrac = 0.5f );

	/**
	 * @brief Interpolates data points from the input keypoints, including RGB data, using an optimized interpolation algorithm.
	 *
	 * This function is an overload of `InterpolateDataOptimized()` function and also includes RGB data interpolation.
	 * It uses a series of data optimizations to interpolate from keypoints to sample data points.
	 * It performs checks and adjustments on the data to ensure effective and safe interpolation.
	 * These include handling corner cases, adjusting for short segments, assigning acceleration, uniform velocity,
	 * and deceleration points for sufficient sample points, and applying linear interpolation when there are too few points per segment.
	 * If RGB data is provided (i.e., rgbKey and rgbSample are not NULL), it is also interpolated.
	 * 
	 * Typically preceded by a call to `OptimizeKeypoints()`
	 *
	 * @param[in] xKey Pointer to array of X position keypoints
	 * @param[in] yKey Pointer to array of Y position keypoints
	 * @param[in] mKey Pointer to array of M digital output keypoints
	 * @param[in] rgbKey Pointer to array of RGB color keypoints
	 * @param[out] xSample Pointer to output array for X position samples, should be pre-allocated.
	 * @param[out] ySample Pointer to output array for Y position samples, should be pre-allocated.
	 * @param[out] mSample Pointer to output array for M digital output samples, should be pre-allocated.
	 * @param[out] rgbSample Pointer to output array for RGB color samples, should be pre-allocated.
	 * @param[in] numKeyPoints Number of keypoints provided in each input array
	 * @param[in] numSamplePoints Desired number of samples to be generated
	 *
	 * @return Number of points in the interpolated samples
	 */
	unsigned int InterpolateDataOptimized( float* xKey, float* yKey, unsigned char* mKey, unsigned int* rgbKey, float* xSample, float* ySample, unsigned char* mSample, unsigned int* rgbSample, unsigned int numKeyPoints, unsigned int numSamplePoints );

	/**
	 * @brief Interpolates data points from the input keypoints, using an optimized interpolation algorithm.
	 *
	 * This function uses a series of data optimizations to interpolate from keypoints to sample data points.
	 * It performs checks and adjustments on the data to ensure effective and safe interpolation.
	 * These include handling corner cases, adjusting for short segments, assigning acceleration, uniform velocity,
	 * and deceleration points for sufficient sample points, and applying linear interpolation when there are too few points per segment.
	 * RGB data is not included in this overload.
	 * 
	 * Typically preceded by a call to OptimizeKeypoints()`
	 *
	 * @param[in] xKey Pointer to array of X position keypoints
	 * @param[in] yKey Pointer to array of Y position keypoints
	 * @param[in] mKey Pointer to array of M digital output keypoints
	 * @param[out] xSample Pointer to output array for X position samples, should be pre-allocated.
	 * @param[out] ySample Pointer to output array for Y position samples, should be pre-allocated.
	 * @param[out] mSample Pointer to output array for M digital output samples, should be pre-allocated.
	 * @param[in] numKeyPoints Number of keypoints provided in each input array
	 * @param[in] numSamplePoints Desired number of samples to be generated
	 *
	 * @return Number of points in the interpolated samples
	 */
	unsigned int InterpolateDataOptimized( float* xKey, float* yKey, unsigned char* mKey, float* xSample, float* ySample, unsigned char* mSample, unsigned int numKeyPoints, unsigned int numSamplePoints );

	/**
	 * @brief Optimizes the given arrays of keypoints, targeting the requested number of keypoints post-optimization.
	 *
	 * This function aims to enhance the distribution and minimize the redundancy of keypoints
	 * in a given dataset. It primarily addresses the removal of repeated identical keypoints,
	 * multiple blanking keypoints, and non-essential keypoints where the angle of adjacent vectors is equal.
	 * 
	 * Typically followed by a call to `InterpolateDataOptimized()`.
	 *
	 * @param[in, out] xKey Pointer to array of X position keypoints, this array will be updated based on optimizations.
	 * @param[in, out] yKey Pointer to array of Y position keypoints, this array will be updated based on optimizations.
	 * @param[in, out] mKey Pointer to array of M digital output keypoints, this array will be updated based on optimizations.
	 * @param[in] numKeyPoints Initial number of keypoints provided in each input array.
	 * @param[in] targetNumKeyPoints Desired number of keypoints after optimization.
	 *
	 * @return Number of keypoints after the optimization process.
	 */
	unsigned int OptimizeKeypoints( float* xKey, float* yKey, unsigned char* mKey, unsigned int numKeyPoints, unsigned int targetNumKeyPoints = 0 );

	/**
	 * @brief Optimizes the keypoints of given data arrays, including consideration of RGB data.
	 *
	 * This function aims to enhance the distribution and minimize the redundancy of keypoints
	 * in a given dataset, also taking into account RGB data. It addresses the removal of repeated identical keypoints,
	 * multiple blanking keypoints, and non-essential keypoints where the angle of adjacent vectors is equal.
	 * 
	 * Typically followed by a call to `InterpolateDataOptimized()`.
	 *
	 * @param[in, out] xKey Pointer to array of X position keypoints, this array will be updated based on optimizations.
	 * @param[in, out] yKey Pointer to array of Y position keypoints, this array will be updated based on optimizations.
	 * @param[in, out] mKey Pointer to array of M digital output keypoints, this array will be updated based on optimizations.
	 * @param[in, out] rgbKey Pointer to array of RGB data keypoints, this array will be updated based on optimizations.
	 * @param[in] numKeyPoints Initial number of keypoints provided in each input array.
	 * @param[in] targetNumKeyPoints Desired number of keypoints after optimization.
	 *
	 * @return Number of keypoints after the optimization process.
	 */
	unsigned int OptimizeKeypoints( float* xKey, float* yKey, unsigned char* mKey, unsigned int* rgbKey, unsigned int numKeyPoints, unsigned int targetNumKeyPoints = 0 );
	//@}

	/** @name Filtering
	*  Functions for filtering data
	*/
	//@{
	void LoadIIRFilterParams( const char* filename );
	unsigned int GetIIRFilterOrder();
	unsigned int GetIIRFilterSps();
	void SetupSoftwareFilter( unsigned int type, unsigned int order, float cutoffFreq, float sampleFreq, bool twoChannel = true );
	void FilterData( float* xData, float* yData, float *xFilt, float *yFilt, unsigned int numPoints, bool zeroPhase = true );
	void FilterData( float* xData, float *xFilt, unsigned int numPoints, bool zeroPhase = true );
	//@}

	/** @name Scan Patterns
	*  Functions to generate various scan patterns
	*/
	//@{

	//! LinearRasterPattern constructs a linear raster scan pattern based on the given parameters.
	//!
	//! @param[in,out]	xData	X position output container with pre-allocated length (recommended 1,000,000 points for large waveform streaming)
	//! @param[in,out]	yData	Y position output container with pre-allocated length (recommended 1,000,000 points for large waveform streaming)
	//! @param[in,out]	mData	8-bit Digital Output number output container with pre-allocated length (recommended 1,000,000 points for large waveform streaming)
	//!		- Bit 0: On during the "active" portion of line
	//!		- Bit 1: On during the pixel portion of line
	//!		- Bit 2: Pulsed at the beginning of the frame
	//!		- Bit 3: Indicates Y-Axis direction (if implemented). High for increasing Y position, low for decreasing Y position.
	//!		- Bit 4: On during the "active portion of line" (same as Bit 0)
	//!		- Bit 5: On during the "active portion of line" (same as Bit 0)
	//!		- Bit 6: On during the "active portion of line" (same as Bit 0)
	//!		- Bit 7: On during the "active portion of line" (same as Bit 0)
	//!
	//! @param[in]	xAmp	Amplitude of X-axis of the raster scan (normalized from 0.0 to 1.0)
	//! @param[in]	yAmp	Amplitude of Y-axis of the raster scan (normalized from 0.0 to 1.0)
	//!
	//! @param[in]	numLines	Total number of raster lines. During each line scan there is a high output on DOut0, and
	//!		during each turn-around there is a low output on DOut0.
	//!
	//!	@param[in]	numPixels	When ppMode = true, numPixels corresponds to the number of stop-and-go locations ("pixels")
	//!		along a line. In both ppMode = false and ppMode = true, numPixels corresponds to the number of equally spaced
	//!		Digital Output pulses along a line, contained in Bit 1 of mData.
	//!
	//! @param[in]	lineTime	Duration of time allocated to each raster line in seconds. The time is specifically allocated
	//!		to the 'active' portion of each line. The actual scan along one line will include a small additional percentage of
	//!		time for turn-around.
	//! 
	//! @param[in]	ppMode	ppMode = 1 corresponds to point-to-point raster with near stopping at each point. ppMode = 0 corresponds
	//!		to a uniform velocity line raster.
	//!
	//!	@param[in]	retrace	Toggles between bi-directional writing on (1) and off (0)
	//!
	//! @param[in]	triggerShift	Number of samples the synchronous digital output should be rotated with respect to the MEMS
	//!		Driver X, Y output (can be negative or positive). User may want to differently align the locations of the TTL pulses on
	//!		DOut0-3 and can therefore rotate forward or backward a number of samples.
	//!
	//! @param[in]	theta	Rotation angle for the raster pattern (looking toward display surface) in radians. Passing 0 results in vertical lines.
	//!
	//! @param[in, out]	sps	Sample rate of raster scan. The user can pre-set this sample rate, however the function will likely override
	//!		and return (in-place) the rate based on the numLines and numPixels input to best match the desired scan
	//!
	//!	@param[in]	spsMin	Sets the lower limit for samples-per-second rate generated internally by the function. 
	//!		Typically this is the controller's lower sample rate limit (MTIDeviceLimits::SampleRate_Min).
	//!	@param[in]	spsMax	Sets the upper limit for samples-per-second rate generated internally by the function. 
	//!		Typically this is the controller's upper sample rate limit (MTIDeviceLimits::SampleRate_Max).
	//!
	//! @param[in] dRatio Proportional acceleration distance for active segments : 0.05 to 0.15 are recommended
	//!		Typically a larger value is needed for very fast waveforms (compared to available bandwidth) with shot lineTime
	//!
	//! @return	Returns the number of points in the constructed scan pattern.
	unsigned int LinearRasterPattern( float* xData, float* yData, unsigned char* mData, float xAmp, float yAmp, unsigned int numLines, unsigned int numPixels, float lineTime, bool ppMode, bool retrace, int triggerShift, float theta, unsigned int& sps, int spsMin = 1000, int spsMax = 100000, float dRatio = 0.08f );

	unsigned int LinearRasterPatternImaging( float* xData, float* yData, unsigned char* mData, float xAmp, float yAmp, unsigned int numLines, unsigned int numPixels, float lineTime, bool ppMode, bool retrace, int triggerShift, float theta, unsigned int& sps, int spsMin = 1000, int spsMax = 100000, float dRatio = 0.08f );

	//! PointToPointPattern constructs a scan pattern that traces a path between specified points, spending a specified amount of time at each point, and 
	//! attempting to step between points with the specified step time. 
	//!
	//! @param[in,out]	xData	X position output container with pre-allocated length (recommended 100,000 points)
	//! @param[in,out]	yData	Y position output container with pre-allocated length (recommended 100,000 points)
	//! @param[in,out]	mData	8-bit Digital Output number output container with pre-allocated length (recommended 100,000 points)
	//! 
	//! @param[in]	xPoint		Array of X position keypoints specifying the path for the function to trace
	//! @param[in]	yPoint		Array of Y position keypoints specifying the path for the function to trace
	//! @param[in]	mPoint		Array of M 8-bit Digital Output number keypoints specifying the digital output number at each position on the path. 
	//!		Note that the M value for step segments between points is forced to 0 by the function, so the specified M value is only sampled by the Controller 
	//!		at the corresponding X/Y position for the amount of time tPoint[position] - stepTime.
	//! @param[in]	tPoint 	Array of time keypoints specifying the time spent at each position on the path (in milliseconds)
	//!		Note that the time allocated with tPoint to a given point is a total of time dwelling at position plus the stepTime to next position
	//!		therefore dwell time at position is tPoint[position]-stepTime unless stepTime is longer than allocated time
	//!
	//! @param[in]	nPoints		Number of keypoints in the xPoint, yPoint, mPoint, and tPoint arrays to be used in the pattern
	//! 
	//! @param[in]	stepTime	Desired time spent on the step/transition between points (in milliseconds)
	//!		Note that if stepTime is 0 or too small, function will assign a minimum number of points to the step (usually 6 samples)
	//!		stepTime should be less than any tPoint since dwellTime[position] = tPoint[position]-stepTime
	//!
	//! @param[in, out]	sps		Sample rate of the point-to-point scan. If the sample rate is set to 0, the function will calculate the optimal sample rate for the given
	//!		path parameters and populate the given reference. For a non-zero sample rate, the function will attempt to use the given sample rate.
	//!		Note that user must use the sample rate sps when sampling this waveform to have the resulting scan follow prescribed point-to-point timing
	//!
	//!	@param[in]	spsMin		Sets the lower limit for samples-per-second rate generated internally by the function. 
	//!		Typically this is the controller's lower sample rate limit (MTIDeviceLimits::SampleRate_Min).
	//!	@param[in]	spsMax		Sets the upper limit for samples-per-second rate generated internally by the function. 
	//!		Typically this is the controller's upper sample rate limit (MTIDeviceLimits::SampleRate_Max).
	//!
	//! @return	Returns the number of points in the constructed point-to-point pattern.
	unsigned int PointToPointPattern( float* xData, float* yData, unsigned char* mData, float* xPoint, float* yPoint, unsigned char* mPoint, float* tPoint, int nPoints, float stepTime, unsigned int& sps, int spsMin, int spsMax );

	void LaserWriterInterpData( int inCount, int outCount, unsigned char* inM, float* inX, float* inY,unsigned char* outM, float* outX, float* outY, float DON, float DOFF, int closed_trajectory );
	void LaserWriterInterpData1Seg( int inCount, int outCount, float* inX, float* outX, float DON );
#ifdef _MTIOPENCV
#ifdef MTI_WINDOWS
	unsigned int LaserWriterDataStream( float* x, float* y, unsigned char* m, unsigned char* stream, int* lastOnPixelList, unsigned int lines, unsigned int numPixels, float lineTime, bool compressLine, unsigned int& sps, int spsMin, int spsMax, float xMin, float xMax, float yMin, float yMax );
	unsigned int ImageRasterPattern( float* xData, float* yData, unsigned char* mData, float xAmplitude, float yAmplitude, unsigned int numLines, unsigned int numPixels, IplImage* srcImage, bool bwMode, int bwThreshold, float lineTime, bool compressLine, unsigned int& sps, int spsMin, int spsMax );
#endif
#endif
	//@}

	/** @name Plotting
	*  Functions to generate plots of user data in windows and close those windows
	*/
	//@{

	//!	This function creates plot windows with its user defined title
	//! 
	//!	Returns	plotID for the plot created so content can be resent to same plot window
	//!
	//!	@param[in] windowTitle	Allows you to specify a custom title for the plot window. If no title is provided, the function will automatically generate a default title in the format "Plot + (plot number)".
	//!
	unsigned int CreatePlot( const char* windowTitle = "" );

	//!	This function generates plots of user data with respect to the plot type
	//!
	//!	Returns unique plotID. plotID values are auto-assigned starting from 1.
	//!
	//!	@param[in]	plotID			The plot's unique ID, provide 0 to create a new plot and generate a new plotID.
	//!	@param[in]	x				Array of X position data to be plotted.
	//!	@param[in]	y				Array of Y position data to be plotted.
	//!	@param[in]	m				Array of 8-bit Digital Output data to be plotted.
	//!	@param[in]	npts			The number of points in the arrays x and y
	//!	@param[in]	sps				The sample rate to use to determine the time axis of the plots
	//!	@param[in]	windowWidth		Adjust the plot window width in pixels
	//!	@param[in]	windowHeight	Adjust the plot window height in pixels
	//!	@param[in]	plotType		The ::PlotType to use when drawing the plot axes and content
	//!
	unsigned int PlotData( unsigned int plotID, float* x, float* y, unsigned char* m, unsigned int npts, unsigned int sps = 1, unsigned int windowWidth = 900, unsigned int  windowHeight = 800, PlotType plotType = PlotType::TimeVsXYData );
	
	//!	Function that first creates a plot window and then generates plots of the user data in it with respect to the plot type
	//!
	//!	Returns unique plotID. plotID values are auto-assigned starting from 1.
	//!
	//!	@param[in]	x				Array of X position data to be plotted.
	//!	@param[in]	y				Array of Y position data to be plotted.
	//!	@param[in]	m				Array of 8-bit Digital Output data to be plotted.
	//!	@param[in]	npts			The number of points in the arrays x and y
	//!	@param[in]	sps				The sample rate to use to determine the time axis of the plots
	//!	@param[in]	windowWidth		Adjust the plot window width in pixels
	//!	@param[in]	windowHeight	Adjust the plot window height in pixels
	//!	@param[in]	plotType		The ::PlotType to use when drawing the plot axes and content
	//!
	unsigned int PlotData( float* x, float* y, unsigned char* m, unsigned int npts, unsigned int sps = 1, unsigned int windowWidth = 900, unsigned int  windowHeight = 800, PlotType plotType = PlotType::TimeVsXYData );
	
	//!	Legacy version of the function with only 2 plot types.
	//!
	//!	This function first creates a plot window and then generates two types of plots for user data: X Data Vs. Y Data and Time Vs. X Data and Y Data
	//!
	//!	Returns unique plotID. plotID values are auto-assigned starting from 1.
	//!
	//!	@param[in]	x				Array of X position data to be plotted.
	//!	@param[in]	y				Array of Y position data to be plotted.
	//!	@param[in]	m				Array of 8-bit Digital Output data to be plotted.
	//!	@param[in]	npts			The number of points in the arrays x and y
	//!	@param[in]	xyplot			Plot type, True = X Data Vs. Y Data, False = Time Vs. X Data and Y Data
	//!	@param[in]	sps				The sample rate to use to determine the time axis of the plots
	//!	@param[in]	windowWidth		Adjust the plot window width in pixels
	//!	@param[in]	windowHeight	Adjust the plot window height in pixels
	//!
	unsigned int PlotData( float* x, float* y, unsigned char* m, unsigned int npts, bool xyplot = false, unsigned int sps = 1, unsigned int windowWidth = 900, unsigned int  windowHeight = 800 );
	
	//!	This function closes all plot windows
	//!
	void ClosePlots();

	//@}


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	std::unique_ptr<MTIDataGeneratorImpl> m_pImpl;
};

#endif