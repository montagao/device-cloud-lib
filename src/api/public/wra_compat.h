/**
 * @file
 * @brief Header file for Wind River Agent library backwards compatibility
 *
 * @copyright Copyright (C) 2015-2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software may
 * be licensed only pursuant to the terms of an applicable Wind River license
 * agreement.  No license to Wind River intellectual property rights is granted
 * herein.  All rights not licensed by Wind River are reserved by Wind River.
 */
#ifndef WRA_COMPAT_H
#define WRA_COMPAT_H

#include "wra_types.h"

#include <assert.h> /* for backwards compatibility */
#include <time.h>   /* for timespec */

#if defined( __GNUC__ )
#	include <stdbool.h>
#else
	typedef int bool;
#	define  __attribute__(x) /* gcc specific */
#endif /* defined( __GNUC__ ) */

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/**
 * @brief Enumeration defining telemetry data types
 */
enum {
	WRA_TM_DATATYPE_DOUBLE = 0,                     /**< @brief Double precision float-point number */
	WRA_TM_DATATYPE_BOOL,                           /**< @brief Boolean */
	WRA_TM_DATATYPE_STRING                          /**< @brief String */
};

/**
 * @brief Enumeration defining prority levels
 */
enum {
	WRA_TM_PRIO_LOW = 0,                            /**< @brief Low priority */
	WRA_TM_PRIO_MEDIUM,                             /**< @brief Medium priority */
	WRA_TM_PRIO_HIGH                                /**< @brief High priority */
};

/**
 * @brief Backwards-compatible time stamp
 */
struct wra_timestamp_s
{
	long tv_sec;     /**< @brief Seconds */
	long tv_usec;    /**< @brief Microseconds */
};

/** @brief Time stamp */
typedef struct wra_timestamp_s wra_timestamp;
/** @brief Notification handle */
typedef enum wra_status* wra_notification_handle_t;
/** @brief Type holding the connection to the telemetry sensor */
typedef struct wra_telemetry wra_telemetry_t;

/* defines */
/**
 * @brief For assertions within the library
 * @deprecated Assertions are no longer used
 */
#define wra_assert(s) assert(s)
/**
 * @brief Defines the type for a callback for a custom command
 * @deprecated use wra_command_callback_t instead
 */
#define wra_app_ex_handler wra_app_ex_handler_t
/**
 * @brief Defines a handle to the library
 * @deprecated use wra_t instead
 */
typedef wra_t* wra_handle __attribute__((deprecated));
/**
 * @brief Defiens a handle to a service
 * @deprecated use wra_service_t instead
 */
typedef wra_service_t* wra_service_handle __attribute__((deprecated));
/**
 * @brief The return value from functions within the library
 * @deprecated use wra_status_t instead
 */
#define wra_status wra_status_t
/**
 * @brief Defines the handle to telemetry data within the library
 * @deprecated use wra_telemetry_t instead
 */
typedef wra_telemetry_t* wra_tm_handle __attribute__((deprecated));

/**
 * @brief Data telemetry
 */
#define WRA_TM_ALARMTM "alarm-tm"
/**
 * @brief Alarm telemetry
 */
#define WRA_TM_DATATM  "data-tm"
/**
 * @brief Event telemetry
 */
#define WRA_TM_EVENTTM "event-tm"

/**
 * @brief Acknowledge attribute
 */
#define WRA_TM_ATTR_ACK       "ack"
/**
 * @brief Active attribute
 */
#define WRA_TM_ATTR_ACTIVE    "active"
/**
 * @brief Condition attribute
 */
#define WRA_TM_ATTR_CONDITION "condition"
/**
 * @brief Data attribute
 */
#define WRA_TM_ATTR_DATA      "data"
/**
 * @brief Auxiliary data attribute
 */
#define WRA_TM_ATTR_DATATM    "aux-data-tm"
/**
 * @brief Data type attribute
 */
#define WRA_TM_ATTR_DATATYPE  "datatype"
/**
 * @brief Description attribute
 */
#define WRA_TM_ATTR_DESC      "description"
/**
 * @brief Name attribute
 */
#define WRA_TM_ATTR_NAME      "name"
/**
 * @brief Priority attribute
 */
#define WRA_TM_ATTR_PRIORITY  "priority"
/**
 * @brief Severity attribute
 */
#define WRA_TM_ATTR_SEVERITY  "severity"

/**
 * @brief Major version of telemetry supported
 */
#define WRA_TM_MAJOR_VER 1
/**
 * @brief Minor version of telemetry supported
 */
#define WRA_TM_MINOR_VER 2

/**
 * @brief Bad parameter (backwards compatibility)
 * @deprecated use WRA_STATUS_BAD_PARAMETER
 */
#define WRA_ERR_BAD_PARAM    WRA_STATUS_BAD_PARAMETER
/**
 * @brief Try again (backwards compatibility)
 * @deprecated use WRA_STATUS_TRY_AGAIN
 */
#define WRA_ERR_EAGAIN       WRA_STATUS_TRY_AGAIN
/**
 * @brief Timed out (backwards compatibility)
 * @deprecated use WRA_STATUS_TIMED_OUT
 */
#define WRA_ERR_ETIMEDOUT    WRA_STATUS_TIMED_OUT
/**
 * @brief Already exists (backwards compatibility)
 * @deprecated use WRA_STATUS_EXISTS
 */
#define WRA_ERR_EXISTS       WRA_STATUS_EXISTS
/**
 * @brief Failed (backwards compatibility)
 * @deprecated use WRA_STATUS_FAILURE
 */
#define WRA_ERR_FAILED       WRA_STATUS_FAILURE
/**
 * @brief Full (backwards compatibility)
 * @deprecated use WRA_STATUS_FULL
 */
#define WRA_ERR_FULL         WRA_STATUS_FULL
/**
 * @brief Out of memory (backwards compatibility)
 * @deprecated use WRA_STATUS_NO_MEMORY
 */
#define WRA_ERR_NO_MEMORY    WRA_STATUS_NO_MEMORY
/**
 * @brief Not initialized (backwards compatibility)
 * @deprecated use WRA_STATUS_NOT_INITIALIZED
 */
#define WRA_ERR_NOT_INIT     WRA_STATUS_NOT_INITIALIZED
/**
 * @brief Success (backwards compatibility)
 * @deprecated use WRA_STATUS_SUCCESS
 */
#define WRA_SUCCESS          WRA_STATUS_SUCCESS
/** @brief NULL */
#define WRA_NULL             NULL

/** @brief application execution handler type */
typedef int (*wra_app_ex_handler_t)( void *dev_h, const char *appname, const char *args ) __attribute__((deprecated));

/**
 * @brief Subscribes an action from the cloud
 * @deprecated use wra_command_register instead
 *
 * @param[in,out]  wra_h                         A valid handle to the WR Agent
 * @param[in]      app_ex_handler                callback to be called when action is encountered
 * @param[in]      action_name                   name of action to unregister
 *
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_NO_MEMORY                  not enough system memory
 * @retval WRA_STATUS_NOT_INITIALIZED            library not initialized with the cloud
 *
 * @see wra_command_register
 */

WRA_API wra_status_t wra_reg_app_ex_handler( wra_handle wra_h, wra_app_ex_handler_t app_ex_handler, const char* action_name ) __attribute__((deprecated));

/**
 * @brief Subscribes an action from the cloud
 * @deprecated use wra_command_register instead
 *
 * @param[in,out]  wra_h                         A valid handle to the WR Agent
 * @param[in]      app_ex_handler                callback to be called when action is encountered
 * @param[in]      action_name                   name of action to unregister
 *
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_NO_MEMORY                  not enough system memory
 * @retval WRA_STATUS_NOT_INITIALIZED            library not initialized with the cloud
 *
 * @see wra_command_register
 */
WRA_API wra_status_t wra_action_subscribe( wra_handle wra_h, wra_app_ex_handler_t app_ex_handler, const char* action_name ) __attribute__((deprecated));

/**
 * @brief Unsubscribes an action from the cloud
 * @deprecated use wra_command_deregister instead
 *
 * @param[in,out]  wra_h                         A valid handle to the WR Agent
 * @param[in]      action_name                   name of action to unregister
 *
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_NO_MEMORY                  not enough system memory
 * @retval WRA_STATUS_NOT_FOUND                  specified action name was not found
 * @retval WRA_STATUS_NOT_INITIALIZED            library not initialized with the cloud
 *
 * @see wra_command_deregister
 */
WRA_API wra_status_t wra_action_unsubscribe( wra_handle wra_h, const char* action_name ) __attribute__((deprecated));

/**
 * @brief Waits up to the specified time out for an incoming command to occur
 *
 * @param[in,out]  wra_h                         A valid handle to the WR Agent
 * @param[in]      time_out                      maximum amount of time to wait
 *
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                received an invalid request from agent
 * @retval WRA_STATUS_NOT_EXECUTABLE             specified script not executable
 * @retval WRA_STATUS_NOT_FOUND                  command request received for command that isn't registered
 * @retval WRA_STATUS_NOT_INITIALIZED            command was not initialized with the library
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for next request
 * @retval WRA_STATUS_FAILURE                    communication with agent failed
 */
WRA_API int wra_action_wait( wra_handle wra_h, const wra_timestamp* time_out ) __attribute__((deprecated));

/**
 * @brief Destroys memory associated with the library
 * @deprecated use wra_terminate instead
 *
 * @param[in,out]  wra_h                         A valid handle to the WR Agent
 *
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 *
 * @see wra_terminate
 */
WRA_API wra_status_t wra_delete_handle( wra_handle wra_h ) __attribute__((deprecated));

/**
 * @brief Initialize the library
 * @deprecated use wra_initialize instead
 *
 * @return A reference handle to the library
 *
 * @see wra_initialize
 */
WRA_API wra_handle wra_gethandle( void ) __attribute__((deprecated));

/**
 * @brief Allocate a telemetry object and initializes it
 * @deprecated use wra_telemetry_allocate instead
 *
 * Allocate a telemetry object that can then be used in subsequent calls
 * to the wra_tm_* interface functions to set and get its attributes. Calling
 * his function allocates memory from the heap to store the object.
 *
 * @param[in]      type                          the type of telemetry
 * @param[in]      name                          the name of the telemetry object
 *
 * @return A reference to a new telemetry object
 *
 * @see wra_telemetry_allocate
 * @see wra_tm_destroy
 */
WRA_API wra_tm_handle wra_tm_create( const char* type, const char* name ) __attribute__((deprecated));

/**
 * @brief Free the contents of a telemetry object
 * @deprecated use wra_telemetry_free instead
 *
 * Free the contents of a telemetry object when it is no longer needed.
 *
 * @param[in,out]  tm                            A telemetry object whose contents to free
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                telemetry type not supported
 * @retval WRA_STATUS_SUCCESS                    if the operation was successful
 *
 * @see wra_telemetry_free
 * @see wra_tm_create
 * @see wra_tm_reset
 */
WRA_API wra_status_t wra_tm_destroy( wra_tm_handle tm ) __attribute__((deprecated));

/**
 * @brief Get the auxiliary telemetry information of a telemetry object attribute
 *
 * The calling function must provide an initialized value that is expected
 * by the attribute, or the operation will not complete successfully
 *
 * @param[in,out]  tm                            A valid, initialized telemetry object
 * @param[in]      attr                          The attribute to set
 * @param[out]     aux                           The attribute value
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                value on set, type is incorrect, or attribute doesn't support this
 * @retval WRA_STATUS_FAILURE                    if the operation was not successful
 * @retval WRA_STATUS_SUCCESS                    if the operation was successful
 * @see wra_tm_setaux
 */
WRA_API wra_status_t wra_tm_getaux( wra_tm_handle tm, const char* attr, wra_tm_handle aux ) __attribute__((deprecated));

/**
 * @brief Get the telemetry object time stamp
 *
 * Get the time stamp of the specified telemetry object
 *
 * @param[in,out]  tm                            A valid, initialized telemetry object
 * @param[out]     time_stamp                    A reference to the time stamp value
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                value on set, type is incorrect, or attribute doesn't support this
 * @retval WRA_STATUS_FAILURE                    if the operation was not successful
 * @retval WRA_STATUS_SUCCESS                    if the operation was successful
 *
 * @see wra_tm_settimestamp
 */
WRA_API wra_status_t wra_tm_gettimestamp( wra_tm_handle tm, wra_timestamp* time_stamp ) __attribute__((deprecated));

/**
 * @brief Get the boolean value of a telemetry object attribute
 *
 * Get the boolean value of the specified attribute from a telemetry object
 *
 * @param[in,out]  tm                            A valid, initialized telemetry object
 * @param[in]      attr                          The attribute to get
 * @param[out]     value                         A reference to the attribute value
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                value on set, type is incorrect, or attribute doesn't support this
 * @retval WRA_STATUS_FAILURE                    if the operation was not successful
 * @retval WRA_STATUS_SUCCESS                    if the operation was successful
 *
 * @see wra_tm_setvalue_bool
 */
WRA_API wra_status_t wra_tm_getvalue_bool( wra_tm_handle tm, const char* attr, bool *value ) __attribute__((deprecated));

/**
 * @brief Get the double value of a telemetry object attribute
 *
 * Get the double value of the specified attribute in a telemetry object
 *
 * @param[in,out]  tm                            A valid, initialized telemetry object
 * @param[in]      attr                          The attribute to set
 * @param[out]     value                         A reference to the attribute value
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                value on set, type is incorrect, or attribute doesn't support this
 * @retval WRA_STATUS_FAILURE                    if the operation was not successful
 * @retval WRA_STATUS_SUCCESS                    if the operation was successful
 *
 * @see wra_tm_setvalue_double
 */
WRA_API wra_status_t wra_tm_getvalue_double( wra_tm_handle tm, const char* attr, double* value ) __attribute__((deprecated));

/**
 * @brief Get the integer value of a telemetry object attribute
 *
 * Get the 32-bit unsigned value of the specified attribute from a telemetry
 * object
 *
 * @param[in,out]  tm                            A valid, initialized telemetry object
 * @param[in]      attr                          The attribute to get
 * @param[out]     value                         A reference to the attribute value
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                value on set, type is incorrect, or attribute doesn't support this
 * @retval WRA_STATUS_FAILURE                    if the operation was not successful
 * @retval WRA_STATUS_SUCCESS                    if the operation was successful
 *
 * @see wra_tm_setvalue_int
 */
WRA_API wra_status_t wra_tm_getvalue_int( wra_tm_handle tm, const char *attr, int *value ) __attribute__((deprecated));

/**
 * @brief Get the textual value of a telemetry object attribute
 *
 * Get the textual value of the specified attribute from a telemetry object.
 *
 * @param[in,out]  tm                            A valid, initialized telemetry object
 * @param[in]      attr                          The attribute to get
 * @param[out]     value                         A reference to the attribute value
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                value on set, type is incorrect, or attribute doesn't support this
 * @retval WRA_STATUS_FAILURE                    if the operation was not successful
 * @retval WRA_STATUS_NO_MEMORY                  if internal resources are exhausted
 * @retval WRA_STATUS_SUCCESS                    if the operation was successful
 *
 * @see wra_tm_setvalue_string
 */
WRA_API wra_status_t wra_tm_getvalue_string( wra_tm_handle tm, const char *attr, char** value ) __attribute__((deprecated));

/**
 * @brief Send a telemetry object to the server
 *
 * Transmit the telemetry data by way of the WR Agent
 *
 * @param[in,out]  wra_h                         A valid handle to the WR Agent
 * @param[in,out]  tm                            The address of the telemetry object
 * @param[in,out]  wra_service_h                 A valid service identification handle
 * @param[in]      notification                  A valid notification handle
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_FAILURE                    if the operation was not successful
 * @retval WRA_STATUS_FULL                       if the transmission queue is full
 * @retval WRA_STATUS_NO_MEMORY                  if internal resources are exhausted
 * @retval WRA_STATUS_TRY_AGAIN                  if the agent is not ready
 * @retval WRA_STATUS_SUCCESS                    if the operation was successful
 *
 * @see wra_tm_subscribe
 */
WRA_API wra_status_t wra_tm_post( wra_handle wra_h, wra_tm_handle tm,
	wra_service_handle wra_service_h, wra_notification_handle_t notification
) __attribute__((deprecated));

/**
 * @brief Send a telemetry object to the server
 *
 * Transmit the telemetry by way of the WR Agent
 *
 * @param[in]      wra_h                         A valid handle to the WR Agent
 * @param[in]      tm                            The address of the telemetry object
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_FAILURE                    if the operation was not successful
 * @retval WRA_STATUS_FULL                       if the transmission queue is full
 * @retval WRA_STATUS_NO_MEMORY                  if internal resources are exhausted
 * @retval WRA_STATUS_TRY_AGAIN                  if the agent is not ready
 * @retval WRA_STATUS_SUCCESS                    if the operation was successful
 */
#define wra_tm_post_default(wra_h,tm) wra_tm_post((wra_h),(tm),WRA_NULL,WRA_NULL)

/**
 * @brief Reset a telemetry object with default attributes
 *
 * Reset a telemetry object that can then be used in subsequent calls
 * to the wra_tm_* interface functions to set and get its attributes.
 *
 * @param[in,out] tm                             A telemetry object to be initialized
 * @param[in]     name                           The name of the telemetry object
 *
 * @return A valid handle to the reset telemetry object if successful. WRA_NULL otherwise
 *
 * @see wra_tm_destroy
 */
WRA_API wra_tm_handle wra_tm_reset( wra_tm_handle tm, const char *name ) __attribute__((deprecated));

/**
 * @brief Set the auxiliary telemetry information of a telemetry object attribute
 *
 * The value must resolve to a valid type expected by the
 * attribute, or the operation will not complete successfully
 *
 * @param[in,out]  tm                            A valid, initialized telemetry object
 * @param[in]      attr                          The attribute to set
 * @param[in]      aux                           A reference to the attribute value
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                attempt to modify non-supported attribute
 * @retval WRA_STATUS_FAILURE                    if the operation was not successful
 * @retval WRA_STATUS_SUCCESS                    if the operation was successful
 *
 * @see wra_tm_getaux
 */
WRA_API wra_status_t wra_tm_setaux( wra_tm_handle tm, const char* attr, wra_tm_handle aux ) __attribute__((deprecated));

/**
 * @brief Set the telemetry object time stamp
 *
 * Set the time stamp of the specified telemetry object
 *
 * @param[in,out]  tm                            A valid, initialized telemetry object
 * @param[in]      time_stamp                    Initialized time stamp (or NULL to unset)
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                attempt to modify non-supported attribute
 * @retval WRA_STATUS_FAILURE                    if the operation was not successful
 * @retval WRA_STATUS_SUCCESS                    if the operation was successful
 *
 * @see wra_tm_gettimestamp
 */
WRA_API wra_status_t wra_tm_settimestamp( wra_tm_handle tm, const wra_timestamp* time_stamp ) __attribute__((deprecated));

/**
 * @brief Set the boolean value of a telemetry object attribute
 *
 * Set the boolean value of the specified attribute in a telemetry object
 *
 * @param[in,out]  tm                            A valid, initialized telemetry object
 * @param[in]      attr                          The attribute to set
 * @param[in]      value                         The attribute value
 *
 * @retval WRA_STATUS_SUCCESS                    if the operation was successful
 * @retval WRA_STATUS_FAILURE                    otherwise
 *
 * @see wra_tm_getvalue_bool
 */
WRA_API wra_status wra_tm_setvalue_bool( wra_tm_handle tm, const char* attr, bool value ) __attribute__((deprecated));

/**
 * @brief Set the double value of a telemetry object attribute
 *
 * Set the double value of the specified attribute in a telemetry object
 *
 * @param[in,out]  tm                            A valid, initialized telemetry object
 * @param[in]      attr                          The attribute to set
 * @param[in]      value                         The attribute value
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                attempt to modify non-supported attribute
 * @retval WRA_STATUS_FAILURE                    if the operation was not successful
 * @retval WRA_STATUS_SUCCESS                    if the operation was successful
 *
 * @see wra_tm_getvalue_double
 */
WRA_API wra_status_t wra_tm_setvalue_double( wra_tm_handle tm, const char* attr, double value ) __attribute__((deprecated));

/**
 * @brief Set the integer value of a telemetry object attribute
 *
 * Set the 32-bit unsigned value of the specified attribute in a telemetry object.
 *
 * @param[in,out]  tm                            A valid, initialized telemetry object
 * @param[in]      attr                          The attribute to set
 * @param[in]      value                         The attribute value
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                attempt to modify non-supported attribute
 * @retval WRA_STATUS_FAILURE                    if the operation was not successful
 * @retval WRA_STATUS_SUCCESS                    if the operation was successful
 *
 * @see wra_tm_getvalue_int
 */
WRA_API wra_status_t wra_tm_setvalue_int( wra_tm_handle tm, const char *attr, int value ) __attribute__((deprecated));

/**
 * @brief Set the textual value of a telemetry object attribute
 *
 * Set the textual value of the specified attribute in a telemetry object.
 *
 * @param[in,out]  tm                            A valid, initialized telemetry object
 * @param[in]      attr                          The attribute to set
 * @param[in]      value                         The attribute value
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                attempt to modify non-supported attribute
 * @retval WRA_STATUS_FAILURE                    if the operation was not successful
 * @retval WRA_STATUS_NO_MEMORY                  unable to store memory to allocate value
 * @retval WRA_STATUS_SUCCESS                    if the operation was successful
 *
 * @see wra_tm_getvalue_string
 */
WRA_API wra_status_t wra_tm_setvalue_string( wra_tm_handle tm, const char* attr, const char* value ) __attribute__((deprecated));

/**
 * @brief Subscribe to data telemetry
 *
 * Register to receive incoming telemetry data from the server.
 *
 * @param[in,out]  wra_h                         A valid handle to the WR Agent
 * @param[in]      tm                            A valid, initialized handle to a telemetry object
 * @param[in]      name                          The name of the telemetry object
 * @param[in]      tmo                           A time out value to specify how long to wait without
 *                                               receiving a telemetry object. A NULL value specifies
 *                                               that the application will wait indefinitely
 *
 * @retval WRA_STATUS_FAILURE                    if the operation was not successful
 * @retval WRA_STATUS_SUCCESS                    if the operation was successful
 *
 * @see wra_tm_post
 */
WRA_API wra_status_t wra_tm_subscribe( wra_handle wra_h, wra_tm_handle tm, const char* name, struct timespec *tmo ) __attribute__((deprecated));

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* WRA_COMPAT_H */

