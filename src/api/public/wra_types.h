/**
 * @file
 * @brief Wind River Agent Internet of Things (IoT) Data Types
 *
 * @copyright Copyright (C) 2015-2016 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software may
 * be licensed only pursuant to the terms of an applicable Wind River license
 * agreement.  No license to Wind River intellectual property rights is granted
 * herein.  All rights not licensed by Wind River are reserved by Wind River.
 */

#ifndef WRA_TYPES_H
#define WRA_TYPES_H

#include  "iot.h"

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/** @brief Type representing milliseconds */
typedef iot_millisecond_t                        wra_millisecond_t;

/* global defines */
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-macros"
#endif /* ifdef __clang__ */

/** @brief Definition of a boolean for backwards compatibility */
typedef iot_bool_t wra_bool_t;
/** @brief False */
#define WRA_TRUE  IOT_TRUE
/** @brief True */
#define WRA_FALSE IOT_FALSE

/**
 * @brief Macro to include addional specifications for the linker
 */
#define WRA_API                                  IOT_API IOT_SECTION

#define wra_command_request iot_action_request

#ifdef __clang__
#pragma clang diagnostic pop
#endif /* ifdef __clang__ */

/* enumerations */
/**
 * @brief Possible return status when executing commands
 */
typedef enum wra_command_status {
	/** @brief Command successfully completed */
	WRA_COMMAND_COMPLETE = IOT_STATUS_SUCCESS,
	/** @brief Command successfully invoked (fire & forget) */
	WRA_COMMAND_INVOKED = IOT_STATUS_INVOKED,
	/** @brief Error executing the requested command */
	WRA_COMMAND_EXECUTION_ERROR =
		IOT_STATUS_EXECUTION_ERROR,
	/** @brief Internal error */
	WRA_COMMAND_INTERNAL_ERROR =
		IOT_STATUS_FAILURE,
	/** @brief Invalid parameter(s) passed */
	WRA_COMMAND_INVALID_PARAMETER =
		IOT_STATUS_BAD_PARAMETER,
	/** @brief Invalid request */
	WRA_COMMAND_INVALID_REQUEST =
		IOT_STATUS_BAD_REQUEST,
	/** @brief IO Error involving files or pipes while executing command */
	WRA_COMMAND_IO_ERROR =
		IOT_STATUS_IO_ERROR,
	/** @brief Not enough memory available to execute command */
	WRA_COMMAND_MEMORY_ERROR =
		IOT_STATUS_NO_MEMORY,
	/** @brief Requested command not registered */
	WRA_COMMAND_NOT_FOUND =
		IOT_STATUS_NOT_FOUND,
	/** @brief Error parsing request */
	WRA_COMMAND_PARSE_ERROR =
		IOT_STATUS_PARSE_ERROR
} wra_command_status_t;

/**
 * @brief Data types supported by the library
 */
typedef enum wra_type
{
	WRA_TYPE_NULL = 0,                         /**< @brief Unspecified data type */
	WRA_TYPE_BOOLEAN,                          /**< @brief Boolean values */
	WRA_TYPE_FLOAT,                            /**< @brief Floating point (decimal numbers) */
	WRA_TYPE_INTEGER,                          /**< @brief Integer values */
	WRA_TYPE_RAW,                              /**< @brief Raw data */
	WRA_TYPE_STRING                            /**< @brief String values */
} wra_type_t;

/* logging */
/**
 * @brief Enumeration of various log levels
 */
typedef enum wra_log_level
{
	WRA_LOG_FATAL = IOT_LOG_FATAL,                  /**< @brief Fatal (unrecoverable error) */
	WRA_LOG_ALERT = IOT_LOG_ALERT,                  /**< @brief Alert (most likely unrecoverable error) */
	WRA_LOG_CRITICAL = IOT_LOG_CRITICAL,            /**< @brief Critical (recoverable, but not good) */
	WRA_LOG_ERROR = IOT_LOG_ERROR,                  /**< @brief Error (recoverable error)  */
	WRA_LOG_WARNING = IOT_LOG_WARNING,              /**< @brief Warning (something needs attention) */
	WRA_LOG_NOTICE = IOT_LOG_NOTICE,                /**< @brief Notice (something could be correct or incorrect) */
	WRA_LOG_INFO = IOT_LOG_INFO,                    /**< @brief Information (helpful information) */
	WRA_LOG_DEBUG = IOT_LOG_DEBUG,                  /**< @brief Debug (useful for debugging purposes) */

	/* This must be the last state */
	WRA_LOG_TRACE = IOT_LOG_TRACE                   /**< @brief Tracing execution (tracing the execution path) */
} wra_log_level_t;

/** @brief Enumeration of possible running state of the agent */
enum wra_state
{
	WRA_STATE_NOT_CONNECTED =
		IOT_STATE_NOT_CONNECTED,                       /**< @brief Not connected to cloud */
	WRA_STATE_NORMAL
		= IOT_STATE_CONNECTED,                         /**< @brief Normal running state (enabled) */
	WRA_STATE_MAINTENANCE,                          /**< @brief Maintenance state (disabled), only cores services, no telemetry data */

	/* This must be the last state */
	WRA_STATE_LAST                                  /**< @brief Last state (not a true state) */
};

/**
 * @brief Possible status codes
 */
enum wra_status {
	/**< @brief Success */
	WRA_STATUS_SUCCESS = IOT_STATUS_SUCCESS,
	/**< @brief Invalid Parameter */
	WRA_STATUS_BAD_PARAMETER =
		IOT_STATUS_BAD_PARAMETER,
	/**< @brief Bad request received */
	WRA_STATUS_BAD_REQUEST =
		IOT_STATUS_BAD_REQUEST,
	/**< @brief Already Exists */
	WRA_STATUS_EXISTS = IOT_STATUS_EXISTS,
	/**< @brief File open failed */
	WRA_STATUS_FILE_OPEN_FAILED =
		IOT_STATUS_FILE_OPEN_FAILED,
	/**< @brief Full storage */
	WRA_STATUS_FULL = IOT_STATUS_FULL,
	/**< @brief No memory */
	WRA_STATUS_NO_MEMORY =
		IOT_STATUS_NO_MEMORY,
	/**< @brief Not executable */
	WRA_STATUS_NOT_EXECUTABLE =
		IOT_STATUS_NOT_EXECUTABLE,
	/**< @brief Not found */
	WRA_STATUS_NOT_FOUND =
		IOT_STATUS_NOT_FOUND,
	/**< @brief Not Initialized */
	WRA_STATUS_NOT_INITIALIZED =
		IOT_STATUS_NOT_INITIALIZED,
	/**< @brief Value out of range */
	WRA_STATUS_OUT_OF_RANGE =
		IOT_STATUS_OUT_OF_RANGE,
	/**< @brief Timed out */
	WRA_STATUS_TIMED_OUT =
		IOT_STATUS_TIMED_OUT,
	/**< @brief Try again */
	WRA_STATUS_TRY_AGAIN =
		IOT_STATUS_TRY_AGAIN,

	/* This must be the last state */
	/**< @brief Failure */
	WRA_STATUS_FAILURE = IOT_STATUS_FAILURE
};

/**
 * @brief Possible location sources
 */
enum wra_location_source {
	WRA_LOCATION_SOURCE_UNKNOWN,                    /**< @brief Unknown location source */
	WRA_LOCATION_SOURCE_FIXED,                      /**< @brief Fixed location */
	WRA_LOCATION_SOURCE_GPS,                        /**< @brief Global positioning system source */
	WRA_LOCATION_SOURCE_WIFI                        /**< @brief Wireless position system source */
};

/* types */
/** @brief Type representing a connection to between the sensor and agent */
typedef struct wra wra_t;
/** @brief Type representing a command within the system */
typedef struct wra_command wra_command_t;
/** @brief Type representing a command request (from the cloud) within the system */
typedef struct wra_command_request wra_command_request_t;
/** @brief Type representing a location sample within the system */
typedef struct wra_location wra_location_t;
/** @brief Type representing a location source type within the system */
typedef enum wra_location_source wra_location_source_t;
/** @brief Type representing a metric within the system */
typedef struct wra_metric wra_metric_t;
/** @brief Type representing a sample within the system */
typedef struct wra_sample wra_sample_t;
/** @brief Type representing a service within the system */
typedef struct wra_service wra_service_t;
/** @brief Type representing a source within the system */
typedef struct wra_source wra_source_t;
/** @brief Type representing a state within the system */
typedef enum wra_state wra_state_t;
/** @brief Type representing a return code within the system */
typedef enum wra_status wra_status_t;
/** @brief Type representing a time within the system
 * @{
 */
#if !defined(_WIN32)
#	define HAVE_STRUCT_TIMESPEC 1
#endif /* if !defined(_WIN32) */

#if defined(_MSC_VER) && _MSC_VER >= 1900 && !defined(_CRT_NO_TIME_T)
#	define HAVE_STRUCT_TIMESPEC 1
#endif /* if defined(_MSC_VER) && _MSC_VER >= 1900 && !defined(_CRT_NO_TIME_T)*/

#ifndef HAVE_STRUCT_TIMESPEC
#define HAVE_STRUCT_TIMESPEC
struct timespec
{
	time_t tv_sec;                        /**< @brief seconds */
	long   tv_nsec;                       /**< @brief nanoseconds */
};
#endif /* ifndef HAVE_STRUCT_TIMESPEC */
typedef struct timespec wra_timestamp_t;
/**
 * @}
 */

/**
 * @brief Type for a callback function called when log information is produced
 * @param[in]      log_level                     log level of the message
 * @param[in]      msg                           log message
 * @param[in]      user_data                     pointer to user specific data to pass
 */
typedef void (wra_log_callback_t)( wra_log_level_t log_level, const char* msg, void* user_data );

/**
 * @brief Type for a callback function called when an internal command is requested
 * @param[in]      request                       information about the request that invoked the callback
 * @param[in]      user_data                     pointer to user specific data to pass
 * @return a return code indicating success or failure
 */
typedef wra_command_status_t (wra_command_callback_t)( wra_command_request_t* request, void* user_data );

/**
 * @brief Type for a callback function called when the agent state changes
 * @param[in]      new_state                     new state the agent just changed to
 * @param[in]      old_state                     previous state the agent was in
 * @param[in]      user_data                     pointer to user specific data to pass
 */
typedef void (wra_state_callback_t)( wra_state_t new_state, wra_state_t old_state, void* user_data );

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif  /* ifndef WRA_TYPES_H */

