/**
 * @file
 * @brief Header file for using the Internet of Things library
 *
 * @copyright Copyright (C) 2016-2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */
#ifndef IOT_H
#define IOT_H

#ifdef _MSC_VER
#	pragma warning( push, 1 )
#endif /* ifdef _MSC_VER */
#include <stddef.h> /* for size_t */
#ifdef _MSC_VER
#	pragma warning( pop )
#endif /* ifdef _MSC_VER */

#include <stdint.h> /* for uint8_t, uint16_t, uint32_t, etc. */

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/* Compiler defines */
#ifndef __GNUC__
/**
 * @brief Removes the GCC specific __attribute__ flag from non-GCC compilers
 */
#	define __attribute__(x)
#endif /* ifndef __GNUC__ */

/**
 * @def IOT_SECTION
 * @brief Macro to add compiler specifications for hints where to store library
 * functions in memory
 */
#ifdef ARDUINO
#	define IOT_SECTION __attribute__((section(".irom0.text")))
#else
#	define IOT_SECTION
#endif /* ifdef ARDUINO */

/**
 * @def IOT_API
 * @brief Macro to add compiler specifications for external/internal functions
 */
#ifdef IOT_STATIC
#	define IOT_API
#else /* ifdef IOT_STATIC */
#	if defined _WIN32 || defined __CYGWIN__
#		if iot_EXPORTS
#			ifdef __GNUC__
#				define IOT_API __attribute__((dllexport))
#			else
#				define IOT_API __declspec(dllexport)
#			endif
#		else /* if iot_EXPORTS */
#			ifdef __GNUC__
#				define IOT_API __attribute__((dllimport))
#			else
#				define IOT_API __declspec(dllimport)
#			endif
#		endif /* if iot_EXPORTS */
#	else /* if defined _WIN32 || defined __CYGWIN__ */
#		if __GNUC__ >= 4
#			define IOT_API __attribute__((visibility("default")))
#		else
#			define IOT_API
#		endif
#	endif /* if defined _WIN32 || defined __CYGWIN__ */
#endif /* ifdef IOT_STATIC */

/* types */
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-macros"
#endif /* ifdef __clang__ */

/* log support  */
/**
 * @def IOT_FUNC
 * @brief Contains the name of the current function
 */
#ifdef __GNUC__
#	define IOT_FUNC __func__
#else
#	define IOT_FUNC __FUNCTION__
#endif /* __GNUC__ */

/**
 * @def IOT_LOG
 * @brief Macro to quickly write a log message
 */
#ifdef IOT_OS_MICRO
#define IOT_LOG( lib, level, fmt, ... ) while ( 0 )
#else
#define IOT_LOG( lib, level, fmt, ... ) \
	(void) iot_log( lib, level, IOT_FUNC, __FILE__, __LINE__, fmt, __VA_ARGS__ )
#endif /* IOT_OS_MICRO */

/** @brief False */
#define IOT_FALSE                                (iot_bool_t)(0 == 1)
/** @brief True */
#define IOT_TRUE                                 (iot_bool_t)(1 == 1)

#ifdef __clang__
#pragma clang diagnostic pop
#endif /* ifdef __clang__ */

/**
 * @brief Possible location sources
 */
enum iot_location_source {
	/** @brief Unknown location source */
	IOT_LOCATION_SOURCE_UNKNOWN = 0,
	/** @brief Fixed location */
	IOT_LOCATION_SOURCE_FIXED,
	/** @brief Global positioning system source */
	IOT_LOCATION_SOURCE_GPS,
	/** @brief Wireless position system source */
	IOT_LOCATION_SOURCE_WIFI
};

/**
 * @defgroup iot_property_flags Bitmask values for property flags
 * @{
 */
/** @brief Property is read only by the cloud */
#define IOT_PROPERTY_FLAG_CLOUD_READ_ONLY   0x1
/** @brief Property is read only by other client applications */
#define IOT_PROPERTY_FLAG_DEVICE_READ_ONLY  0x4
/** @brief Property is read-only by every body else */
#define IOT_PROPERTY_FLAG_READ_ONLY         \
	(IOT_PROPERTY_FLAG_CLOUD_READ_ONLY | IOT_PROPERTY_FLAG_DEVICE_READ_ONLY )
/**
 * @}
 */

/**
 * @defgroup iot_property_method Bitmask values for property publish method
 * @{
 */
/** @brief only add if property is not already defined */
#define IOT_PROPERTY_METHOD_ADD        0x1
/** @brief only modify the property if it exists */
#define IOT_PROPERTY_METHOD_MODIFY     0x2
/** @brief add if property not defined, modify if it exists */
#define IOT_PROPERTY_METHOD_UPDATE     \
	(IOT_PROPERTY_METHOD_ADD | IOT_PROPERTY_METHOD_MODIFY)
/** @brief dete a property if is defined */
#define IOT_PROPERTY_METHOD_DELETE     0x4
/**
 * @}
 */

/**
 * @defgroup iot_property_reason Bitmask values for calling property callback
 * @{
 */
/** @brief Property callback called due to change in flags (due to synchronziation) */
#define IOT_PROPERTY_REASON_FLAGS      0x1
/** @brief Property callback called due to change in owner (due to synchronization) */
#define IOT_PROPERTY_REASON_OWNER      0x2
/** @brief Property callback called due to change in source */
#define IOT_PROPERTY_REASON_SOURCE     0x3
/** @brief Property callback called due to change in value */
#define IOT_PROPERTY_REASON_VALUE      0x4
/**
 * @}
 */

/**
 * @defgroup iot_types List of basic IoT types
 * @{
 */
/** @brief IoT boolean (true or false) */
typedef int                                      iot_bool_t;
/** @brief IoT 32-bit floating-point */
typedef float                                    iot_float32_t;
/** @brief IoT 64-bit floating-point */
typedef double                                   iot_float64_t;
/** @brief IoT 8-bit signed integer */
typedef int8_t                                   iot_int8_t;
/** @brief IoT 16-bit signed integer */
typedef int16_t                                  iot_int16_t;
/** @brief IoT 32-bit signed integer */
typedef int32_t                                  iot_int32_t;
/** @brief IoT 64-bit signed integer */
typedef int64_t                                  iot_int64_t;
/** @brief IoT 8-bit unsigned integer */
typedef uint8_t                                  iot_uint8_t;
/** @brief IoT 16-bit unsigned integer */
typedef uint16_t                                 iot_uint16_t;
/** @brief IoT 32-bit unsigned integer */
typedef uint32_t                                 iot_uint32_t;
/** @brief IoT 64-bit unsigned integer */
typedef uint64_t                                 iot_uint64_t;
/** @brief IoT time interval in milliseconds */
typedef uint32_t                                 iot_millisecond_t;
/** @brief IoT time stamp in milliseconds */
typedef uint64_t                                 iot_timestamp_t;

/** @brief Type representing a connection to between the client and agent */
typedef struct iot                               iot_t;
/** @brief Type representing an action that can be triggered from the cloud */
typedef struct iot_action                        iot_action_t;
/** @brief Type representing an action request from the cloud */
typedef struct iot_action_request                iot_action_request_t;
/** @brief Type representing a location sample */
typedef struct iot_location                      iot_location_t;
/** @brief Type representing a location source type within the system */
typedef enum iot_location_source                 iot_location_source_t;
/** @brief Type representing a custom property */
typedef struct iot_property                      iot_property_t;
/** @brief Type representing flags for a property */
typedef uint32_t                                 iot_property_flags_t;
/** @brief Type defining method for publishing a property */
typedef uint32_t                                 iot_property_method_t;
/** @brief Reason for triggering the property callback */
typedef uint32_t                                 iot_property_reason_t;
/** @brief Type representing a telemetry data */
typedef struct iot_telemetry                     iot_telemetry_t;
/** @brief Type representing communication between client and agent */
typedef struct iot_transaction                   iot_transaction_t;
/** @brief Type containing verison information for the library */
typedef iot_uint32_t                             iot_version_t;
/** @brief Type containing information required for file transfer */
typedef struct iot_file_transfer                 iot_file_transfer_t;
/** @brief Type containing information to pass to file transfer callback */
typedef struct iot_file_progress                 iot_file_progress_t;

/**
 * @}
 */


/** @brief indicates data type */
typedef enum iot_type
{
	/** @brief Undefined type */
	IOT_TYPE_NULL = 0,
	/** @brief Boolean type */
	IOT_TYPE_BOOL,
	/** @brief 32-bit floating point */
	IOT_TYPE_FLOAT32,
	/** @brief 64-bit floating point */
	IOT_TYPE_FLOAT64,
	/** @brief 8-bit signed integer */
	IOT_TYPE_INT8,
	/** @brief 16-bit signed integer */
	IOT_TYPE_INT16,
	/** @brief 32-bit signed integer */
	IOT_TYPE_INT32,
	/** @brief 64-bit signed integer */
	IOT_TYPE_INT64,
	/** @brief location object */
	IOT_TYPE_LOCATION,
	/** @brief Raw binary data */
	IOT_TYPE_RAW,
	/** @brief UTF-8 String data */
	IOT_TYPE_STRING,
	/** @brief 8-bit unsigned integer */
	IOT_TYPE_UINT8,
	/** @brief 16-bit unsigned integer */
	IOT_TYPE_UINT16,
	/** @brief 32-bit unsigned integer */
	IOT_TYPE_UINT32,
	/** @brief 64-bit unsigned integer */
	IOT_TYPE_UINT64
} iot_type_t;

/**
 * @brief Possible return code values
 */
typedef enum iot_status
{
	/** @brief Success */
	IOT_STATUS_SUCCESS = 0,
	/** @brief Action successfully invoked (fire & forget) */
	IOT_STATUS_INVOKED,
	/** @brief Invalid parameter passed */
	IOT_STATUS_BAD_PARAMETER,
	/** @brief Bad request received */
	IOT_STATUS_BAD_REQUEST,
	/** @brief Error executing the requested action */
	IOT_STATUS_EXECUTION_ERROR,
	/** @brief Already exists */
	IOT_STATUS_EXISTS,
	/** @brief File open failed */
	IOT_STATUS_FILE_OPEN_FAILED,
	/** @brief Full storage */
	IOT_STATUS_FULL,
	/** @brief Input/output error */
	IOT_STATUS_IO_ERROR,
	/** @brief No memory */
	IOT_STATUS_NO_MEMORY,
	/** @brief No permission */
	IOT_STATUS_NO_PERMISSION,
	/** @brief Not executable */
	IOT_STATUS_NOT_EXECUTABLE,
	/** @brief Not found */
	IOT_STATUS_NOT_FOUND,
	/** @brief Not Initialized */
	IOT_STATUS_NOT_INITIALIZED,
	/** @brief Parameter out of range */
	IOT_STATUS_OUT_OF_RANGE,
	/** @brief Failed to parse a message */
	IOT_STATUS_PARSE_ERROR,
	/** @brief Timed out */
	IOT_STATUS_TIMED_OUT,
	/** @brief Try again */
	IOT_STATUS_TRY_AGAIN,
	/** @brief Not supported in this version of the api */
	IOT_STATUS_NOT_SUPPORTED,

	/**
	 * @brief General failure
	 * @note This must be the last state
	 */
	IOT_STATUS_FAILURE
} iot_status_t;

/**
 * @defgroup iot_parameter_flags IoT parameter flags
 *
 * The following bits corrospond to the following meanings:
 * - 0001 (1): IN parameter only, not required
 * - 0010 (2): (the same as 0011)
 * - 0011 (3): IN parameter only, required
 * - 0100 (4): OUT parameter only, not required
 * - 0101 (5): IN/OUT parameter only, not required
 * - 0110 (6): (the same as 0111)
 * - 0111 (7): IN parameter required, OUT parameter not required
 * - 1000 (8): (the same as 1100)
 * - 1001 (9): (the same as 1101)
 * - 1010 (A): (the same as 1111)
 * - 1011 (B): (the same as 1111)
 * - 1100 (C): OUT parameter required
 * - 1101 (D): OUT parameter required, IN parameter not required
 * - 1110 (E): (the same as 1111)
 * - 1111 (F): IN/OUT parameter, both required
 *
 * @{
 */
/** @brief Parameter type */
typedef iot_uint8_t iot_parameter_type_t;
/** @brief IN parameter */
#define IOT_PARAMETER_IN                         0x1
/** @brief IN required parameter */
#define IOT_PARAMETER_IN_REQUIRED                0x2
/** @brief OUT parameter */
#define IOT_PARAMETER_OUT                        0x4
/** @brief OUT required parameter */
#define IOT_PARAMETER_OUT_REQUIRED               0x8
/** @brief IN/OUT parameter */
#define IOT_PARAMETER_IN_OUT                     (IOT_PARAMETER_IN|IOT_PARAMETER_OUT)
/** @brief IN/OUT required parameter */
#define IOT_PARAMETER_IN_OUT_REQUIRED            (IOT_PARAMETER_IN_REQUIRED|IOT_PARAMETER_OUT_REQUIRED)
/**
 * @}
 */
/**
 * @brief type representing a object used to iterate through action request
 *        parameters
 */
typedef iot_uint32_t iot_action_request_parameter_iterator_t;

/**
 * @brief log message severity levels
 */
typedef enum iot_log_level
{
	/** @brief Fatal (unrecoverable error) */
	IOT_LOG_FATAL = 0,
	/** @brief Alert (most likely unrecoverable error) */
	IOT_LOG_ALERT,
	/** @brief Critical (recoverable, but not good) */
	IOT_LOG_CRITICAL,
	/** @brief Error (recoverable error) */
	IOT_LOG_ERROR,
	/** @brief Warning (something needs attention) */
	IOT_LOG_WARNING,
	/** @brief Notice (something could be correct or incorrect) */
	IOT_LOG_NOTICE,
	/** @brief Information (helpful information) */
	IOT_LOG_INFO,
	/** @brief Debug (useful for debugging purposes) */
	IOT_LOG_DEBUG,
	/** @brief Tracing execution (tracing the execution path) */
	IOT_LOG_TRACE,
	/**
	 * @brief None (useful for compatible purposes)
	 * @note This must be the last state
	 * */
	IOT_LOG_ALL
} iot_log_level_t;

/** @brief Contains information about where a log message was generated */
typedef struct iot_log_source
{
	/** @brief Name of source file that generated log message */
	const char *file_name;
	/** @brief Name of function that generated log message */
	const char *function_name;
	/** @brief Line number that generated log message */
	unsigned int line_number;
} iot_log_source_t;

/**
 * @defgroup state Possible states of the system
 * @{
 */
/** @brief Not connected to cloud */
#define IOT_STATE_NOT_CONNECTED                  0
/** @brief Connected to cloud */
#define IOT_STATE_CONNECTED                      1

/**
 * @brief Last state (not a true state
 * @note This must be the last state (i.e. highest number)
 */
#define IOT_STATE_LAST                          (IOT_STATE_CONNECTED)
/** @brief State of client */
typedef int iot_state_t;
/**
 * @}
 */

/**
 * @brief Type for a callback function called when an internal action is
 * requested
 *
 * @param[in]      request                       information about the request
 *                                               that invoked the callback
 * @param[in]      user_data                     pointer to user specific data
 *
 * @return a return code indicating if action was handled
 */
typedef iot_status_t (iot_action_callback_t)(
	iot_action_request_t *request,
	void *user_data );

/**
 * @brief Type for a callback function called during file transfer
 *        to give progress update
 * requested
 *
 * @param[in]      progress                      information about the
 *                                               file transfer's progress
 * @param[in]      user_data                     pointer to user specific data
 *
 * @return a return code indicating if action was handled
 */
typedef void (iot_file_progress_callback_t)(
	iot_file_progress_t *progress,
	void *user_data );

/**
 * @brief Type for a callback function called when log information is produced
 *
 * @param[in]      log_level                     log level of the message
 * @param[in]      log_source                    source of log message
 * @param[in]      message                       log message
 * @param[in]      user_data                     pointer to user specific data
 */
typedef void (iot_log_callback_t)(
	iot_log_level_t log_level,
	iot_log_source_t *log_source,
	const char *message,
	void *user_data );

/**
 * @brief Type for a callback function called when a property is updated
 *
 * @param[in]      property            property that was updated
 * @param[in]      reason              bitmask of the reason to trigger callback
 * @param[in]      user_data           pointer to user specific data
 *
 * @see iot_property_reason
 */
typedef void (iot_property_callback_t)(
	const iot_property_t *property,
	iot_property_reason_t reason,
	void *user_data );

/* common */
/**
 * @brief Connects to an agent
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          on success
 * @retval IOT_STATUS_TIMED_OUT        timed out while waiting for connection
 *
 * @see iot_disconnect
 */
IOT_API IOT_SECTION iot_status_t iot_connect(
	iot_t *lib,
	iot_millisecond_t max_time_out );

/**
 * @brief Disconnects from an agent
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          on success
 * @retval IOT_STATUS_TIMED_OUT        timed out while waiting for disconnection
 *
 * @see iot_connect
 */
IOT_API IOT_SECTION iot_status_t iot_disconnect(
	iot_t *lib,
	iot_millisecond_t max_time_out );

/**
 * @brief Translates an error number into an error message
 *
 * @param[in]      code                id of the error to return the message for
 *
 * @return the string corresponding to the error identifier specified
 */
IOT_API IOT_SECTION const char *iot_error(
	iot_status_t code );

/**
 * @brief returns the client id for the library
 *
 * @return the client id
 * @see iot_initialize
 */
IOT_API IOT_SECTION const char *iot_id(
	const iot_t *lib );

/**
 * @brief Initialize the library
 *
 * @param[in]      id                  unique token identifying device
 * @param[in]      cfg_path            path to a configuration file (optional)
 * @param[in]      flags               initialization flags
 *
 * @return A reference handle to the library
 *
 * @see iot_terminate
 */
IOT_API IOT_SECTION iot_t *iot_initialize(
	const char *id,
	const char *cfg_path,
	unsigned int flags );

/**
 * @brief Internal function to log data
 *
 * @param[in]      lib                 library handle
 * @param[in]      log_level           log level of the message
 * @param[in]      function_name       name of function that generated the log
 *                                     message
 * @param[in]      file_name           name of source file that generated the
 *                                     log message
 * @param[in]      line_number         line number that generated the message
 * @param[in]      log_msg_fmt         message to log in printf format
 * @param[in]      ...                 replacement values as specified by the
 *                                     format
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_log_callback_set
 * @see IOT_LOG
 */
IOT_API IOT_SECTION iot_status_t iot_log(
	iot_t *lib,
	iot_log_level_t log_level,
	const char *function_name,
	const char *file_name,
	unsigned int line_number,
	const char *log_msg_fmt, ... )
	__attribute__((format(printf,6,7)));

/**
 * @brief Sets a callback to be called when a log message is available to be
 *        published
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      log_callback        callback to be called on a log message
 * @param[in]      user_data           user data pointer to pass
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_log
 * @see IOT_LOG
 */
IOT_API IOT_SECTION iot_status_t iot_log_callback_set(
	iot_t *lib,
	iot_log_callback_t *log_callback,
	void *user_data );

/**
 * @brief Sets a log level for the service
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      level               log level filter for the service
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_log
 * @see IOT_LOG
 */
IOT_API IOT_SECTION iot_status_t iot_log_level_set(
	iot_t *lib,
	iot_log_level_t level );

/**
 * @brief Destroys memory associated with the library
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          on success
 * @retval IOT_STATUS_TIMED_OUT        timed out while waiting for termination
 *
 * @see iot_initialize
 */
IOT_API IOT_SECTION iot_status_t iot_terminate(
	iot_t *lib,
	iot_millisecond_t max_time_out );

/* actions */
/**
 * @brief Allocates memory for a new action that can be registered
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      name                name of the action
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          pointer was successfully set
 *
 * @see iot_action_free
 */
IOT_API IOT_SECTION iot_action_t *iot_action_allocate(
	iot_t *lib,
	const char *name );

/**
 * @brief Sets an attribute value for a action object
 *
 * @param[in,out]  action              action object to set
 * @param[in]      name                attibute name
 * @param[in]      type                type of attribute data
 * @param[in]      ...                 value of attribute data in the
 *                                     type specified
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FULL             maximum number of attributes
 *                                     reached
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_action_attribute_set_raw
 */
IOT_API IOT_SECTION iot_status_t iot_action_attribute_set(
	iot_action_t *action,
	const char *name,
	iot_type_t type,
	... );

/**
 * @brief Sets a raw attribute value for a action object
 *
 * @param[in,out]  action              action object to set
 * @param[in]      name                attibute name
 * @param[in]      length              length of attribute data
 * @param[in]      ptr                 pointer to attribute data
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FULL             maximum number of attributes reached
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_action_attribute_set
 */
IOT_API IOT_SECTION iot_status_t iot_action_attribute_set_raw(
	iot_action_t *action,
	const char *name,
	size_t length,
	const void *ptr );

/**
 * @brief Deregisters an action from an agent
 *
 * @param[in,out]  action              action to deregister
 * @param[out]     txn                 transaction status (optional)
 * @param[in]      max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_FAILURE          internal system failure
`* @retval IOT_STATUS_NOT_INITIALIZED  action was not initialized
 * @retval IOT_STATUS_SUCCESS          pointer was successfully set
 * @retval IOT_STATUS_TIMED_OUT        timed out while waiting for
 *                                     deregistration
 *
 * @see iot_action_register_callback
 * @see iot_action_register_command
 */
IOT_API IOT_SECTION iot_status_t iot_action_deregister(
	iot_action_t *action,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out );

/**
 * @brief Frees memory for an allocated action
 *
 * @param[in,out]  action              action to deregister and free
 * @param[in]      max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          pointer was successfully set
 *
 * @see iot_action_allocate
 */
IOT_API IOT_SECTION iot_status_t iot_action_free(
	iot_action_t *action,
	iot_millisecond_t max_time_out );

/**
 * @brief Adds a parameter to an action
 *
 * @param[in,out]  action              action to add the parameter to
 * @param[in]      name                name of the parameter
 * @param[in]      param_type          type of the parameter (in and/or out)
 * @param[in]      data_type           data type of the parameter
 * @param[in]      max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      parameter name already exists
 * @retval IOT_STATUS_FULL             maximum number of parameters reached
 * @retval IOT_STATUS_SUCCESS          on success
 * @retval IOT_STATUS_TIMED_OUT        timed out while waiting for parameter
 *                                     addition
 */
IOT_API IOT_SECTION iot_status_t iot_action_parameter_add(
	iot_action_t *action,
	const char *name,
	iot_parameter_type_t param_type,
	iot_type_t data_type,
	iot_millisecond_t max_time_out );

/**
 * @brief Returns the value of a parameter
 *
 * @param[in]      request             action request
 * @param[in]      name                parameter name
 * @param[in]      convert             convert to type, if possible
 * @param[in]      type                type of data to return
 * @param[in,out]  ...                 pointer to a variable of the type
 *                                     specified
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      type doesn't match parameter
 *                                     (and not convertible, if set)
 * @retval IOT_STATUS_NOT_FOUND        parameter name not found
 * @retval IOT_STATUS_NOT_INITIALIZED  no value exists for parameter
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_action_parameter_get_raw
 * @see iot_action_parameter_set
 */
IOT_API IOT_SECTION iot_status_t iot_action_parameter_get(
	const iot_action_request_t *request,
	const char *name,
	iot_bool_t convert,
	iot_type_t type,
	... );

/**
 * @brief Returns the raw value of a parameter
 *
 * @param[in]      request             action request
 * @param[in]      name                parameter name
 * @param[in]      convert             convert to raw, if possible
 * @param[out]     length              amount of raw data
 * @param[in,out]  data                pointer to the raw data
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      type doesn't match parameter
 *                                     (and not convertible, if set)
 * @retval IOT_STATUS_NOT_FOUND        parameter name not found
 * @retval IOT_STATUS_NOT_INITIALIZED  no value exists for parameter
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_action_parameter_get
 * @see iot_action_parameter_set_raw
 */
IOT_API IOT_SECTION iot_status_t iot_action_parameter_get_raw(
	const iot_action_request_t *request,
	const char *name,
	iot_bool_t convert,
	size_t *length,
	const void **data );

/**
 * @brief Sets the value of a parameter
 *
 * @param[in]      request             action request
 * @param[in]      name                parameter name
 * @param[in]      type                type of data
 * @param[in]      ...                 value of the specified type
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      type doesn't match parameter
 * @retval IOT_STATUS_FULL             no space to store new parameter
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_action_parameter_get
 * @see iot_action_parameter_set_raw
 */
IOT_API IOT_SECTION iot_status_t iot_action_parameter_set(
	iot_action_request_t *request,
	const char *name,
	iot_type_t type,
	... );

/**
 * @brief Sets the raw value of a parameter
 *
 * @param[in]      request             action request
 * @param[in]      name                parameter name
 * @param[out]     length              amount of raw data
 * @param[in,out]  data                pointer to the raw data
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      type doesn't match parameter
 * @retval IOT_STATUS_FULL             no space to store new parameter
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_action_parameter_get_raw
 * @see iot_action_parameter_set
 */
IOT_API IOT_SECTION iot_status_t iot_action_parameter_set_raw(
	iot_action_request_t *request,
	const char *name,
	size_t length,
	const void *data );

/**
 * @brief Registers an action with a callback
 *
 * @param[in,out]  action              action to register
 * @param[in]      func                function pointer to be called
 * @param[in]      user_data           user data to pass to function
 *                                     (optional)
 * @param[out]     txn                 transaction status (optional)
 * @param[in]      max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          pointer was successfully set
 * @retval IOT_STATUS_TIMED_OUT        timed out while waiting for
 *                                     registration
 *
 * @see iot_action_deregister
 * @see iot_action_register_command
 */
IOT_API IOT_SECTION iot_status_t iot_action_register_callback(
	iot_action_t *action,
	iot_action_callback_t *func,
	void *user_data,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out );

/**
 * @brief Registers an action with a system command
 *
 * @param[in,out]  action              action to register
 * @param[in]      command             command to be called
 * @param[out]     txn                 transaction status (optional)
 * @param[in]      max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          pointer was successfully set
 * @retval IOT_STATUS_TIMED_OUT        timed out while waiting for
 *                                     registration
 *
 * @see iot_action_deregister
 * @see iot_action_register_command
 */
IOT_API IOT_SECTION iot_status_t iot_action_register_command(
	iot_action_t *action,
	const char *command,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out );

/**
 * @brief allocates a new action request object
 *
 * @param[in]      lib                 library handle
 * @param[in]      name                name of the action
 * @param[in]      source              source of the request (optional)
 *
 * @retval NULL                        no more space or action not found
 * @retval !NULL                       request succesfully allocated
 *
 * @see iot_action_request_execute
 * @see iot_action_request_free
 * @see iot_action_request_parameter_set
 */
IOT_API IOT_SECTION iot_action_request_t *iot_action_request_allocate(
	iot_t *lib,
	const char *name,
	const char *source );

/**
 * @brief Returns the value of a action request attribute
 *
 * @param[in,out]  request             action request object to get value from
 * @param[in]      name                attribute name
 * @param[in]      convert             convert to type, if possible
 * @param[in]      type                type of data to return
 * @param[in,out]  ...                 pointer to a variable of the type
 *                                     specified
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      type doesn't match parameter
 *                                     (and not convertible, if set)
 * @retval IOT_STATUS_NOT_FOUND        attribute not found
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_action_attribute_request_set_raw
 * @see iot_action_attribute_request_set
 */
IOT_API IOT_SECTION iot_status_t iot_action_request_attribute_get(
	const iot_action_request_t *request,
	const char *name,
	iot_bool_t convert,
	iot_type_t type, ... );

/**
 * @brief Sets an attribute value for a action request object
 *
 * @param[in,out]  request             action request object to set
 * @param[in]      name                attibute name
 * @param[in]      type                type of attribute data
 * @param[in]      ...                 value of attribute data in the
 *                                     type specified
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FULL             maximum number of attributes
 *                                     reached
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_action_request_attribute_get
 * @see iot_action_request_attribute_set_raw
 */
IOT_API IOT_SECTION iot_status_t iot_action_request_attribute_set(
	iot_action_request_t *request,
	const char *name,
	iot_type_t type,
	... );

/**
 * @brief Sets a raw attribute value for a action request object
 *
 * @param[in,out]  request             action request object to set
 * @param[in]      name                attibute name
 * @param[in]      length              length of attribute data
 * @param[in]      ptr                 pointer to attribute data
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FULL             maximum number of attributes reached
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_action_request_attribute_get
 * @see iot_action_request_attribute_set
 */
IOT_API IOT_SECTION iot_status_t iot_action_request_attribute_set_raw(
	iot_action_request_t *request,
	const char *name,
	size_t length,
	const void *ptr );

/**
 * @brief executes the specified action request
 *
 * @note once this function is called the request ownership is handled by the
 *       library, no other functions should call methods on the request object
 *
 * @param[in]      request             request to execute
 * @param[in]      max_time_out        maximum time to wait (0=indefinite)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_action_request_allocate
 * @see iot_action_request_free
 * @see iot_action_request_parameter_set
 */
IOT_API IOT_SECTION iot_status_t iot_action_request_execute(
	iot_action_request_t *request,
	iot_millisecond_t max_time_out );

/**
 * @brief frees an action request
 *
 * @note this function should not be called if caller does not have ownership
 *
 * @param[in]      request             request to free
 *
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_action_request_allocate
 * @see iot_action_request_execute
 */
IOT_API IOT_SECTION iot_status_t iot_action_request_free(
	iot_action_request_t *request );

/**
 * @brief Creates an iterator to iterate through the parameters of the
 *        given action request
 *
 * @param[in]      request             action request to iterate through
 * @param[in]      type                type of parameter(s) to iterate through
 * @param[in,out]  iter                pointer to the iterator to move ahead
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_NOT_FOUND        no more items available
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_action_request_parameter_iterator_next
 */
IOT_API IOT_SECTION iot_status_t iot_action_request_parameter_iterator(
	const iot_action_request_t *request,
	iot_parameter_type_t type,
	iot_action_request_parameter_iterator_t *iter );

/**
 * @brief Retrives the data type of a parameter the iterator currently points at
 *
 * @param[in]      request             action request being iterated
 * @param[in]      iter                pointer to the iterator
 *
 * @return the type of data the iterator points at or IOT_TYPE_NULL if an
 *         invalid parameter is passed to the function or the iterator is
 *         invalid
 *
 * @see iot_action_request_parameter_iterator
 * @see iot_action_request_parameter_iterator_get
 * @see iot_action_request_parameter_iterator_get_raw
 * @see iot_action_request_parameter_iterator_name
 * @see iot_action_request_parameter_iterator_next
 */
IOT_API IOT_SECTION iot_type_t iot_action_request_parameter_iterator_data_type(
	const iot_action_request_t *request,
	const iot_action_request_parameter_iterator_t iter );

/**
 * @brief Retrives the value of a parameter the iterator currently points at
 *
 * @param[in]      request             action request being iterated
 * @param[in]      iter                pointer to the iterator
 * @param[in]      convert             convert to type, if possible
 * @param[in]      type                type of data to return
 * @param[in,out]  ...                 pointer to a variable of the type
 *                                     specified
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      type doesn't match parameter
 *                                     (and not convertible, if set)
 * @retval IOT_STATUS_NOT_FOUND        value not found or not set
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_action_request_parameter_iterator
 * @see iot_action_request_parameter_iterator_data_type
 * @see iot_action_request_parameter_iterator_get_raw
 * @see iot_action_request_parameter_iterator_name
 * @see iot_action_request_parameter_iterator_next
 */
IOT_API IOT_SECTION iot_status_t iot_action_request_parameter_iterator_get(
	const iot_action_request_t *request,
	const iot_action_request_parameter_iterator_t iter,
	iot_bool_t convert,
	iot_type_t type,
	... );

/**
 * @brief Returns the raw value of a parameter
 *
 * @param[in]      request             action request being iterated
 * @param[in]      iter                pointer to the iterator
 * @param[in]      convert             convert to type, if possible
 * @param[out]     length              amount of raw data
 * @param[in,out]  data                pointer to the raw data
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      type doesn't match parameter
 *                                     (and not convertible, if set)
 * @retval IOT_STATUS_NOT_FOUND        parameter name not found
 * @retval IOT_STATUS_NOT_INITIALIZED  no value exists for parameter
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_action_request_parameter_iterator
 * @see iot_action_request_parameter_iterator_get
 * @see iot_action_request_parameter_iterator_name
 * @see iot_action_request_parameter_iterator_next
 */
IOT_API IOT_SECTION iot_status_t iot_action_request_parameter_iterator_get_raw(
	const iot_action_request_t *request,
	const iot_action_request_parameter_iterator_t iter,
	iot_bool_t convert,
	size_t *length,
	const void **data );

/**
 * @brief Retrives the name of a parameter the iterator currently points at
 *
 * @param[in]      request             action request to iterate through
 * @param[in]      iter                pointer to the iterator
 *
 * @return the name of the parameter being pointed to by the iterator or NULL
 *         if the iterator or request is invalid
 *
 * @see iot_action_request_parameter_iterator
 * @see iot_action_request_parameter_iterator_get
 * @see iot_action_request_parameter_iterator_next
 */
IOT_API IOT_SECTION const char *iot_action_request_parameter_iterator_name(
	const iot_action_request_t *request,
	const iot_action_request_parameter_iterator_t iter );

/**
 * @brief Moves the iterator to the next item or NULL if end of list
 *
 * @param[in]      request             action request being iterated
 * @param[in,out]  iter                pointer to the iterator to move ahead
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_NOT_FOUND        no more items available
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_action_request_parameter_iterator
 * @see iot_action_request_parameter_iterator_get
 * @see iot_action_request_parameter_iterator_name
 */
IOT_API IOT_SECTION iot_status_t iot_action_request_parameter_iterator_next(
	const iot_action_request_t *request,
	iot_action_request_parameter_iterator_t *iter );

/**
 * @brief Sets a parameter value for an action request to be executed
 * @param[in,out]  request             action request
 * @param[in]      name                parameter name
 * @param[in]      type                data type
 * @param[in]      ...                 value of the specified type
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      type doesn't match parameter
 * @retval IOT_STATUS_FULL             no space to store new parameter
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_action_request_allocate
 * @see iot_action_request_execute
 */
IOT_API IOT_SECTION iot_status_t iot_action_request_parameter_set(
	iot_action_request_t *request,
	const char *name,
	iot_type_t type, ... );

/**
 * @brief returns the source of the action request
 *
 * @param[in]      request             request to retrieve source from
 *
 * @retval !NULL                       the request source
 * @retval NULL                        no source is set, or invalid parameter
 *
 * @see iot_action_request_allocate
 */
IOT_API IOT_SECTION const char *iot_action_request_source(
	const iot_action_request_t *request );

/**
 * @brief returns the current result of an action request
 *
 * @param[in]      request             request to retrieve result from
 * @param[out]     message             (optional) current status message, may
 *                                     return NULL on success
 *
 * @return         any return status result
 *
 * @see iot_action_request_allocate
 * @see iot_action_request_execute
 */
IOT_API IOT_SECTION iot_status_t iot_action_request_status(
	const iot_action_request_t *request,
	const char **message );

#ifndef iot_EXPORTS
#ifndef __clang__
/**
 * @brief Sets an attribute for a action request object
 *
 * @param[in,out]  request             object to set attribute for
 * @param[in]      name                attribute name
 * @param[in]      type                attribute type
 * @param[in]      data                attribute value
 */
#	define iot_action_request_attribute_set( request, name, type, data ) \
		iot_action_request_attribute_set( request, name, type, data )

/**
 * @brief Retrieve a parameter value for an action request being iterated
 * @param[in]      request             action request
 * @param[in]      iter                pointer to the iterator
 * @param[in]      convert             convert to type, if possible
 * @param[in]      type                data type
 * @param[out]     data_ptr            pointer to store value
 */
#	define iot_action_request_parameter_iterator_get( request, iter, convert, type, data ) \
		iot_action_request_parameter_iterator_get( request, iter, convert, type, data )
/**
 * @brief Sets a parameter value for an action request to be executed
 * @param[in,out]  request             action request
 * @param[in]      name                parameter name
 * @param[in]      type                data type
 * @param[in]      data                value to set
 */
#	define iot_action_request_parameter_set( request, name, type, data ) \
		iot_action_request_parameter_set( request, name, type, data )
#endif /* ifndef __clang__ */
#endif /* ifndef iot_EXPORTS */

/**
 * @brief Sets the maximum duration for an action after which time to report
 *        a failure
 *
 * @note This must be set before an action is registered
 *
 * @param[in]      action              action to set limit for
 * @param[in]      duration            duration limit in milliseconds
 *                                     (0 indicates no limit)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          pointer was successfully set
 */
IOT_API IOT_SECTION iot_status_t iot_action_time_limit_set(
	iot_action_t *action,
	iot_millisecond_t duration );

#ifndef iot_EXPORTS
#ifndef __clang__
/**
 * @brief Sets an attribute for a action object
 *
 * @param[in,out]  action              object to set attribute for
 * @param[in]      name                attribute name
 * @param[in]      type                attribute type
 * @param[in]      data                attribute value
 */
#	define iot_action_attribute_set( action, name, type, data ) \
		iot_action_attribute_set( action, name, type, data )

/**
 * @brief Retrieve a parameter value for an action being handled
 * @param[in]      request             action request
 * @param[in]      name                parameter name
 * @param[in]      convert             convert to type, if possible
 * @param[in]      type                data type
 * @param[out]     data_ptr            pointer to store value
 */
#	define iot_action_parameter_get( request, name, convert, type, data_ptr ) \
		iot_action_parameter_get( request, name, convert, type, data_ptr )

/**
 * @brief Sets a parameter value for an action being handled
 * @param[in,out]  request             action request
 * @param[in]      name                parameter name
 * @param[in]      type                data type
 * @param[in]      data                value to set
 */
#	define iot_action_parameter_set( request, name, type, data ) \
		iot_action_parameter_set( request, name, type, data )
#endif /* ifndef __clang__ */
#endif /* ifndef iot_EXPORTS */

/**
 * @brief Get a file from cloud
 *
 * @param[in]      handle              library handle
 * @param[out]     txn                 transaction status (optional)
 * @param[in]      max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely)
 * @param[in]      file_name           cloud's file name to get (optional)
 *                                     if file name is not given, local file
 *                                     name will be used
 * @param[in]      file_path           location of the local file.
 *                                     if path is not absolute, file will
 *                                     be saved relative to default
 *                                     download directory
 * @param[in]      func                callback function to give
 *                                     progress update (optional)
 *                                     if none is given, progress will
 *                                     be printed in the logs at
 *                                     a regular interval
 * @param[in]      user_data           user's specific data for progress
 *                                     callback (optional)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          operation successful
 */
IOT_API IOT_SECTION iot_status_t iot_file_receive(
	iot_t *handle,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	const char *file_name,
	const char *file_path,
	iot_file_progress_callback_t *func,
	void *user_data );

/**
 * @brief Send a file/directory to cloud
 *
 * @param[in]      handle              library handle
 * @param[out]     txn                 transaction status (optional)
 * @param[in]      max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely)
 * @param[in]      file_name           cloud's file name to send (optional)
 *                                     if file name is not given, local file
 *                                     name will be used, if it is a directory,
 *                                     the full path will be used where all
 *                                     the separators are replaced with dashes
 * @param[in]      file_path           location of the local file.
 *                                     if path is not absolute, it will
 *                                     be relative to default upload directory.
 *                                     if it is a directory instead of a file,
 *                                     all files within that directory will be
 *                                     bundled together.
 * @param[in]      func                callback function to give
 *                                     progress update (optional)
 *                                     if none is given, progress will
 *                                     be printed in the logs at
 *                                     a regular interval
 * @param[in]      user_data           user's specific data for progress
 *                                     callback (optional)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          operation successful
 */
IOT_API IOT_SECTION iot_status_t iot_file_send(
	iot_t *handle,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	const char *file_name,
	const char *file_path,
	iot_file_progress_callback_t *func,
	void *user_data  );

/**
 * @brief Get the percentage done of a file transfer
 *
 * @param[in]      progress            pointer containing various progress update data
 * @param[out]     percentage          percentage done of the file transfer
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          operation successful
 */
IOT_API IOT_SECTION iot_status_t iot_file_progress_percentage_get(
	iot_file_progress_t * progress,
	iot_float32_t *percentage );

/**
 * @brief Get the status of a file transfer
 *
 * @param[in]      progress            pointer containing various progress update data
 * @param[out]     status              status of the file transfer
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          operation successful
 */
IOT_API IOT_SECTION iot_status_t iot_file_progress_status_get(
	iot_file_progress_t *progress,
	iot_status_t *status );

/**
 * @brief Check whether a file transfer has been completed or not
 *
 * @param[in]      progress            pointer containing various progress update data
 * @param[out]     is_completed        true if file transfer is completed
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          operation successful
 */
IOT_API IOT_SECTION iot_status_t iot_file_progress_is_completed(
	iot_file_progress_t *progress,
	iot_bool_t *is_completed );

/* location */
/**
 * @brief Sets the accuracy for the location sample
 *
 * @param[in,out]  sample              object to modify
 * @param[in]      accuracy            accuracy for the location
 *                                     sample (in metres)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_location_allocate
 */
IOT_API IOT_SECTION iot_status_t iot_location_accuracy_set(
	iot_location_t *sample, iot_float64_t accuracy );

/**
 * @brief Allocates memory for a new location sample
 *
 * @param[in]      latitude            latitude for the new sample (in degrees)
 *                                     (range: -90.0 to 90.0)
 * @param[in]      longitude           longitude for the new sample (in degrees)
 *                                     (range: -180.0 to 180.0)
 *
 * @return a reference to the new location sample, NULL on failure
 *
 * @see iot_location_free
 */
IOT_API IOT_SECTION iot_location_t* iot_location_allocate(
	iot_float64_t latitude, iot_float64_t longitude );

/**
 * @brief Sets the accuracy of the altitude for the location sample
 *
 * @param[in,out]  sample              object to modify
 * @param[in]      accuracy            altitude accuracy for the
 *                                     location sample (in metres)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_location_allocate
 * @see iot_location_altitude_set
 */
IOT_API IOT_SECTION iot_status_t iot_location_altitude_accuracy_set(
	iot_location_t* sample, iot_float64_t accuracy );

/**
 * @brief Sets the altitude for the location sample
 *
 * @param[in,out]  sample              object to modify
 * @param[in]      altitude            altitude for the location
 *                                     sample (in metres)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_location_allocate
 * @see iot_location_altitude_accuracy_set
 */
IOT_API IOT_SECTION iot_status_t iot_location_altitude_set(
	iot_location_t* sample, iot_float64_t altitude );

/**
 * @brief Free memory associated with a location sample
 *
 * @param[in]      sample              sample to free
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_location_allocate
 */
IOT_API IOT_SECTION iot_status_t iot_location_free( iot_location_t* sample );

/**
 * @brief Sets the heading for the location sample
 *
 * @param[in,out]  sample              object to modify
 * @param[in]      heading             heading for the location
 *                                     sample (in degrees)
 *                                     (range: 0.0 to 360.0)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_location_allocate
 */
IOT_API IOT_SECTION iot_status_t iot_location_heading_set(
	iot_location_t* sample, iot_float64_t heading );

/**
 * @brief Sets the latitude and longitude for the location sample
 *
 * @param[in,out]  sample              object to modify
 * @param[in]      latitude            latitude for the sample
 * @param[in]      longitude           longitude for the sample
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_location_allocate
 */
IOT_API IOT_SECTION iot_status_t iot_location_set( iot_location_t *sample,
	iot_float64_t latitude, iot_float64_t longitude );

/**
 * @brief Sets the source for the location sample
 *
 * @param[in,out]  sample              object to modify
 * @param[in]      source              source generating the location
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_location_allocate
 */
IOT_API IOT_SECTION iot_status_t iot_location_source_set(
	iot_location_t* sample, iot_location_source_t source );

/**
 * @brief Sets the speed for the location sample
 *
 * @param[in,out]  sample              object to modify
 * @param[in]      speed               speed for the location sample in (m/s)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_location_allocate
 */
IOT_API IOT_SECTION iot_status_t iot_location_speed_set(
	iot_location_t* sample, iot_float64_t speed );

/**
 * @brief Sets the tag for the location sample
 *
 * @param[in,out]  sample              object to modify
 * @param[in]      tag                 tag for the location sample
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_location_allocate
 */
IOT_API IOT_SECTION iot_status_t iot_location_tag_set( iot_location_t* sample,
	const char* tag );


/* property */
/**
 * @brief Allocates a new property object
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      name                property name
 *
 * @return a new property object on success, NULL on failure
 *
 * @see iot_property_free
 */
IOT_API IOT_SECTION iot_property_t *iot_property_allocate(
	iot_t* lib,
	const char *name );

/**
 * @brief Registers a callback for when properties change
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      func                function pointer to be called
 * @param[in]      user_data           user data to pass to function (optional)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          pointer was successfully set
 *
 * @see iot_property_get_string
 */
IOT_API IOT_SECTION iot_status_t iot_property_callback_set(
	iot_t* lib,
	iot_property_callback_t *func,
	void *user_data );

/**
 * @brief Retrieves the flags set for the property
 *
 * @param[in]      property            property to retrieve flags for
 * @param[out]     flags               flags for the property
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_SUCCESS          pointer was successfully set
 *
 * @see iot_property_flags
 */
IOT_API IOT_SECTION iot_status_t iot_property_flags_get(
	const iot_property_t *property,
	iot_property_flags_t *flags );

/**
 * @brief Changes the flags for the property
 *
 * @param[in]      property            property to retrieve flags for
 * @param[out]     txn                 transaction status (optional)
 * @param[in]      max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely)
 * @param[in]      flags               flags to set for the property
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_NO_PERMISSION    permission denied
 * @retval IOT_STATUS_SUCCESS          pointer was successfully set
 */
IOT_API IOT_SECTION iot_status_t iot_property_flags_set(
	iot_property_t *property,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	iot_property_flags_t flags );

/**
 * @brief Frees memory associated with a property object
 *
 * @param[in,out]  property            property object to free
 * @param[in]      max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely)
 *
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_property_allocate
 */
IOT_API IOT_SECTION iot_status_t iot_property_free(
	iot_property_t *property,
	iot_millisecond_t max_time_out );

/**
 * @brief Returns a string value of a property
 *
 * @param[in]      property            property to get value for
 * @param[out]     value               pointer to receive the value
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_NOT_INITIALIZED  no value exists for parameter
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_property_callback_set
 * @see iot_property_publish_string
 */
IOT_API IOT_SECTION iot_status_t iot_property_get_string(
	const iot_property_t *property,
	const char **value );

/**
 * @brief Returns the name of the property
 *
 * @param[in]      property            property to get name for
 * @param[out]     name                pointer to receive the name
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_property_allocate
 * @see iot_property_callback_set
 */
IOT_API IOT_SECTION iot_status_t iot_property_name_get(
	const iot_property_t *property,
	const char **name );

/**
 * @brief Publishes a property update as a string to the cloud
 *
 * @param[in,out]  property            property object to publish
 * @param[out]     txn                 transaction status (optional)
 * @param[in]      max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely)
 * @param[in]      method              update method
 * @param[in]      value               value to publish
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      type does not match registered type
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_NO_MEMORY        no memory to store telemetry sample
 * @retval IOT_STATUS_NOT_INITIALIZED  telemetry object is not initialized
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_property_get_string
 */
IOT_API IOT_SECTION iot_status_t iot_property_publish_string(
	iot_property_t *property,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	iot_property_method_t method,
	const char *value );


/* telemetry */
/**
 * @brief Allocates a new telemetry object
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      name                telemetry name
 * @param[in]      type                type of telemetry data
 *
 * @return a new telemetry object on success, NULL on failure
 *
 * @see iot_telemetry_free
 */
IOT_API IOT_SECTION iot_telemetry_t *iot_telemetry_allocate(
	iot_t *lib,
	const char *name,
	iot_type_t type );

/**
 * @brief Sets an attribute value for a telemetry object
 *
 * @param[in,out]  telemetry           telemetry object to set
 * @param[in]      name                attibute name
 * @param[in]      type                type of attribute data
 * @param[in]      ...                 value of attribute data in the
 *                                     type specified
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FULL             maximum number of attributes reached
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_telemetry_attribute_set_raw
 */
IOT_API IOT_SECTION iot_status_t iot_telemetry_attribute_set(
	iot_telemetry_t *telemetry,
	const char *name,
	iot_type_t type,
	... );

/**
 * @brief Sets a raw attribute value for a telemetry object
 *
 * @param[in,out]  telemetry          telemetry object to set
 * @param[in]      name               attibute name
 * @param[in]      length             length of attribute data
 * @param[in]      ptr                pointer to attribute data
 *
 * @retval IOT_STATUS_BAD_PARAMETER   invalid parameter passed to the function
 * @retval IOT_STATUS_FULL            maximum number of attributes reached
 * @retval IOT_STATUS_SUCCESS         on success
 *
 * @see iot_telemetry_attribute_set
 */
IOT_API IOT_SECTION iot_status_t iot_telemetry_attribute_set_raw(
	iot_telemetry_t *telemetry,
	const char *name,
	size_t length,
	const void *ptr );

/**
 * @brief Deregisters a telemetry object from the cloud
 *
 * @param[in,out]  telemetry           telemetry object to deregister
 * @param[out]     txn                 transaction status (optional)
 * @param[in]      max_time_out        maximum time to wait
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_NOT_INITIALIZED  telemetry was not initialized
 * @retval IOT_STATUS_SUCCESS          on success
 * @retval IOT_STATUS_TIMED_OUT        timed out while waiting for
 *                                     deregistration
 *
 * @see iot_telemetry_register
 */
IOT_API IOT_SECTION iot_status_t iot_telemetry_deregister(
	iot_telemetry_t *telemetry,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out );

/**
 * @brief Frees memory associated with a telemetry object
 *
 * @param[in,out]  telemetry           telemetry object to free
 * @param[in]      max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely)
 *
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_telemetry_allocate
 */
IOT_API IOT_SECTION iot_status_t iot_telemetry_free(
	iot_telemetry_t *telemetry,
	iot_millisecond_t max_time_out );

/**
 * @brief Registers a telemetry object with the cloud
 *
 * @param[in,out]  telemetry           telemetry object to register
 * @param[out]     txn                 transaction status (optional)
 * @param[in]      max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely)
 *
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_telemetry_deregister
 */
IOT_API IOT_SECTION iot_status_t iot_telemetry_register(
	iot_telemetry_t *telemetry,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out );

/**
 * @brief Publish a telemetry sample
 *
 * @param[in,out]  telemetry           telemetry object sample is for
 * @param[out]     txn                 transaction status (optional)
 * @param[in]      max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely)
 * @param[in]      type                type of data to publish
 * @param[in]      ...                 value of data to publish in the type
 *                                     specified
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      type does not match registered type
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_NO_MEMORY        no memory to store telemetry sample
 * @retval IOT_STATUS_NOT_INITIALIZED  telemetry object is not initialized
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_telemetry_publish_raw
 */
IOT_API IOT_SECTION iot_status_t iot_telemetry_publish(
	iot_telemetry_t *telemetry,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	iot_type_t type,
	... );

/**
 * @brief Publish a raw telemetry sample
 *
 * @param[in,out]  telemetry           telemetry object sample is for
 * @param[out]     txn                 transaction status (optional)
 * @param[in]      max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely)
 * @param[in]      length              length of data to publish
 * @param[in]      ptr                 pointer to data to publish
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      type does not match registered type
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_NO_MEMORY        no memory to store telemetry sample
 * @retval IOT_STATUS_NOT_INITIALIZED  telemetry object is not initialized
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_telemetry_publish
 */
IOT_API IOT_SECTION iot_status_t iot_telemetry_publish_raw(
	iot_telemetry_t *telemetry,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	size_t length,
	const void *ptr );

#ifndef iot_EXPORTS
#ifndef __clang__
/**
 * @brief Sets an attribute for a telemetry object
 *
 * @param[in,out]  telemetry           object to set attribute for
 * @param[in]      name                attribute name
 * @param[in]      type                attribute type
 * @param[in]      data                attribute value
 */
#	define iot_telemetry_attribute_set( telemetry, name, type, data ) \
		iot_telemetry_attribute_set( telemetry, name, type, data )

/**
 * @brief Publishs telemetry towards the cloud
 *
 * @param[in]      telemetry           object to publish for
 * @param[out]     txn                 transaction status (optional)
 * @param[in]      max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely)
 * @param[in]      type                type of data to publish
 * @param[in]      data                value of data to publish
 */
#	define iot_telemetry_publish( telemetry, txn, max_time_out, type, data ) \
		iot_telemetry_publish( telemetry, txn, max_time_out, type, data )
#endif /* ifndef __clang__ */
#endif /* ifndef iot_EXPORTS */

/**
 * @brief Explicitly sets the time stamp for a piece of telemetry data
 *
 * @param[in,out]  telemetry           object to set time stamp for
 * @param[in]      time_stamp          time stamp for telemetry to set
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_telemetry_publish
 * @see iot_telemetry_publish_raw
 * @see iot_timestamp_now
 */
IOT_API IOT_SECTION iot_status_t iot_telemetry_timestamp_set(
	iot_telemetry_t *telemetry,
	iot_timestamp_t time_stamp );

/* time */
/**
 * @brief Initializes a time stamp with the current time
 *
 * @retval 0u                          on failure, or not supported on
 *                                     operating system
 * @retval >0u                         current timestamp in milliseconds
 *
 * @see iot_telemetry_timestamp_set
 */
IOT_API IOT_SECTION iot_timestamp_t iot_timestamp_now( void );

/* version */
/**
 * @brief Returns the version of the library
 * @return Version of the library
 * @see iot_version_major
 * @see iot_version_minor
 * @see iot_version_patch
 * @see iot_version_tweak
 * @see iot_version_str
 */
IOT_API IOT_SECTION iot_version_t iot_version( void );

/**
 * @brief Returns the version of the library as a string
 * @return Version of the library as a string
 * @see iot_version
 */
IOT_API IOT_SECTION const char *iot_version_str( void );

/**
 * @brief Macro used to encode the library version
 *
 * @note This macro can be used for comparing library versions.  For example:
 @code
   if ( iot_version() >= iot_version_encode( 3, 0, 0, 0 ) )
 @endcode
 *
 * @param[in]      major               major portion of version number
 * @param[in]      minor               minor portion of version number
 * @param[in]      patch               patch portion of version number
 * @param[in]      tweak               tweak portion of version number
 */
#define iot_version_encode( major, minor, patch, tweak ) ( \
	((iot_version_t)major << 24u) | \
	((iot_version_t)minor << 16u) | \
	((iot_version_t)patch << 8u) | tweak)

/**
 * @brief Extracts the major verion portion from a version
 * @param[in]      v                   version to extract from
 * @return the major portion of the version number
 */
#define iot_version_major( v ) ((v >> 24u) & 0xFF)

/**
 * @brief Extracts the minor version portion from a version
 * @param[in]      v                   version to extract from
 * @return the minor portion of the version number
 */
#define iot_version_minor( v ) ((v >> 16u) & 0xFF)

/**
 * @brief Extracts the patch version portion from a version
 * @param[in]      v                   version to extract from
 * @return the patch portion of the version number
 */
#define iot_version_patch( v ) ((v >> 8u) & 0xFF)

/**
 * @brief Extracts the tweak version portion from a version
 * @param[in]      v                   version to extract from
 * @return the tweak portion of the version number
 */
#define iot_version_tweak( v ) (v & 0xFF)

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif

