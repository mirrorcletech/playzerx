/**
 * \file PlayzerXDefinitions.h
 * \brief Defines global constants and error enumerations for the PlayzerX library.
 * \version 2.0.1.0
 *
 * Core error codes related to PlayzerX devices
 */

#ifndef PLAYZERX_DEFINITIONS_H
#define PLAYZERX_DEFINITIONS_H

#include "MTIDefinitions.h"	 // Required for MTI-specific definitions

#ifdef MTI_UNIX
#include <cstring>
#include <cmath>
#endif

#ifndef M_PI
/**
 * \def M_PI
 * \brief Defines the value of pi if not already defined.
 */
#define M_PI 3.14159265358979323846
#endif

namespace playzerx
{

/**
 * \enum PlayzerXError
 * \brief Enumerates possible error conditions within the PlayzerX library.
 *
 */
enum struct PlayzerXError
{
	/**
	 * \brief Operation completed successfully without error.
	 */
	SUCCESS = 0,

	/**
	 * \brief A generic error has occurred.
	 */
	ERROR_GENERAL,

	/**
	 * \brief Unable to establish or maintain a connection.
	 */
	ERROR_CONNECTION,

	/**
	 * \brief Invalid or unsupported device type was encountered.
	 */
	ERROR_INVALID_DEVICE_TYPE,

	/**
	 * \brief Connection was closed or lost unexpectedly.
	 */
	ERROR_DISCONNECT,

	/**
	 * \brief Device is in a running or busy state that prevents the operation.
	 */
	ERROR_PLAYZERX_RUNNING_STATE,

	/**
	 * \brief One or more function parameters are invalid.
	 */
	ERROR_INVALID_PARAM,

	/**
	 * \brief Operation timed out while waiting for a response or condition.
	 */
	ERROR_FUNCTION_TIMEOUT,

	/**
	 * \brief The requested scan or data stream is not configured.
	 */
	ERROR_SCAN_NOT_SET
};

}  // namespace playzerx

#endif	// PLAYZERX_DEFINITIONS_H
