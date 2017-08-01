/**
 * @file
 * @brief Header file for internal structures within the Wind River Agent library
 *
 * @copyright Copyright (C) 2015-2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software may
 * be licensed only pursuant to the terms of an applicable Wind River license
 * agreement.  No license to Wind River intellectual property rights is granted
 * herein.  All rights not licensed by Wind River are reserved by Wind River.
 */

#ifndef WRA_INTERNAL_H
#define WRA_INTERNAL_H

#include "iot_types.h"                 /* for UNUSED define, iot_action_request */
#include "../public/wra_types.h"       /* for wra_timestamp_t */

#include <limits.h>                    /* for PATH_MAX */

#ifdef _WIN32
typedef int mqd_t;                     /**< @brief message queue id */
typedef int gid_t;                     /**< @brief group id */
typedef int uid_t;                     /**< @brief user id */
#define O_WRONLY 1                     /**< @brief write only */
#define O_RDONLY 2                     /**< @brief read only */
#endif

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/* defines */
/** @brief Flag indicating whether 'accuracy' field is set */
#define WRA_LOCATION_FLAG_ACCURACY               (0x01)
/** @brief Flag indicating whether 'altitude' field is set */
#define WRA_LOCATION_FLAG_ALTITUDE               (0x02)
/** @brief Flag indicating whether 'altitude_accuracy' field is set */
#define WRA_LOCATION_FLAG_ALTITUDE_ACCURACY      (0x04)
/** @brief Flag indicating whether 'heading' field is set */
#define WRA_LOCATION_FLAG_HEADING                (0x08)
/** @brief Flag indicating whether 'source_type' field is set */
#define WRA_LOCATION_FLAG_SOURCE                 (0x10)
/** @brief Flag indicating whether 'speed' field is set */
#define WRA_LOCATION_FLAG_SPEED                  (0x20)

/** @brief Maximum length of a version */
#define VERSION_MAX_LEN  16u

/** @brief Enumeration defining the index of queues between a client and agent */
enum
{
	QUEUE_INDEX_COMMAND = 0,                        /**< @brief Command queue index */
	QUEUE_INDEX_METRIC,                             /**< @brief Metric queue index */
	QUEUE_INDEX_NOTIFICATION,                       /**< @brief Notification queue index */
	QUEUE_INDEX_REQUEST,                            /**< @brief Request queue index */

	/* This should be the last item */
	QUEUE_INDEX_LAST                                /**< @brief Last queue index (not a real queue) */
};

/** @brief Structure for passing information to agent */
struct wra_connection
{
	struct wra* parent;                             /**< @brief Parent connection */
	uint8_t* buffer;                                /**< @brief Internal buffer used to rx & tx */
	size_t buffer_len;                              /**< @brief Size of the internal buffer */
	wra_bool_t created_q;                           /**< @brief Whether this device owns the queue */
	char topic[PATH_MAX];                           /**< @brief Topic name */
	int  topic_permission;                          /**< @brief Topic permission */
};

/** @brief Define to rename the the new type */
#define wra_raw_data iot_data_raw

/** @brief Defines the type for raw data */
typedef struct wra_raw_data wra_raw_data_t;


/** @brief Defines data stored within the library */
#define wra_data iot_data

/** @brief Defines a parameter to a command */
#define wra_parameter iot_action_parameter

/**
 * @brief Structure for calling the old callback with new API
 */
struct wra_command_callback_data
{
	wra_command_callback_t* callback;               /**< @brief Callback to run */
	void* user_data;                                /**< @brief User data to pass to callback */
};

/** @brief Commands registered with the agent */
struct wra_command
{
	struct wra_service* parent;                     /**< @brief Handle to the parent service */
	char command_name[IOT_NAME_MAX_LEN];            /**< @brief Name of the command */
	struct wra_command_callback_data callback_data; /**< @brief Information for calling the callback */
	char script[PATH_MAX];                          /**< @brief Script to execute */
	size_t parameter_count;                         /**< @brief Number of parameters */
	struct wra_parameter parameter[ IOT_PARAMETER_MAX ];      /**< @brief Parameters for the command */
	void* compat_data;                              /**< @brief Heap allocated data for backwards compatibility */
	struct wra_command* command_next;               /**< @brief Next command */
	struct wra_command* command_prev;               /**< @brief Previous command */
	struct iot_action* action;                      /**< @brief Pointer to action object */
};

/** @brief Location information */
struct wra_location
{
	wra_timestamp_t time_stamp;                     /**< @brief Time data was produced */
	struct iot_location *location;                  /**< @brief Pointer to location object */
};

/** @brief Holds information about a possible service to run */
struct wra_service
{
	struct wra* lib_handle;                         /**< @brief Handle to the library */
	char service_name[IOT_NAME_MAX_LEN];            /**< @brief Name of the service */
	char service_version[VERSION_MAX_LEN];          /**< @brief Version of the service */
	struct wra_command* command_first;              /**< @brief First registered command */
	struct wra_command* command_last;               /**< @brief Last registered command */
	struct wra_service* service_next;               /**< @brief Next service */
	struct wra_service* service_prev;               /**< @brief Previous service */
};

/** @brief Holds information about sources provided by a client */
struct wra_source
{
	struct wra* lib_handle;                         /**< @brief Handle to the library */
	char source_name[IOT_NAME_MAX_LEN];             /**< @brief Name of the source */
	char source_version[VERSION_MAX_LEN];           /**< @brief Version of the source */
	struct wra_metric* metric_first;                /**< @brief First reigstered metric */
	struct wra_metric* metric_last;                 /**< @brief Last registered metric */
	struct wra_source* source_next;                 /**< @brief Next source */
	struct wra_source* source_prev;                 /**< @brief Previous source */
};

/** @brief Holds information about a metric provided by a source of a client */
struct wra_metric
{
	struct wra_source* parent;                      /**< @brief Handle to the parent source */
	char metric_name[IOT_NAME_MAX_LEN];             /**< @brief Name of the metric */
	char metric_version[VERSION_MAX_LEN];           /**< @brief Version of the metric */
	char metric_units[IOT_NAME_MAX_LEN];            /**< @brief Units for the metric */
	enum wra_type metric_type;                      /**< @brief Metric data type */
	uint8_t samples_max;                            /**< @brief Maximum number of samples to publish */
	uint8_t samples_min;                            /**< @brief Minimum number of samples to publish */
	struct wra_metric* metric_next;                 /**< @brief Next metric */
	struct wra_metric* metric_prev;                 /**< @brief Previous metric */
	struct iot_telemetry* telemetry;                /**< @brief Pointer to telemetry object */
};

/** @brief Type of notification */
enum wra_notification_type
{
	WRA_NOTIFICATION_UNKNOWN = 0,                   /**< @brief Default notification type */
	WRA_NOTIFICATION_AGENT_AUDIT,                   /**< @brief Agent audit notification */
	WRA_NOTIFICATION_CLIENT_HEARTBEAT,              /**< @brief Client heartbeat notification */
	WRA_NOTIFICATION_CLIENT_REGISTERED,             /**< @brief Client registered notification */
	WRA_NOTIFICATION_CLIENT_DEREGISTERED,           /**< @brief Client deregistered notification */
	WRA_NOTIFICATION_COMMAND_REGISTERED,            /**< @brief Command registered notification */
	WRA_NOTIFICATION_STATE_CHANGED                  /**< @brief State change notification */
};

/** @brief Type of agent audit sub-type */
enum wra_audit_sub_type
{
	WRA_AUDIT_REQUEST,                              /**< @brief Audit request */
	WRA_AUDIT_RESPONSE,                             /**< @brief Audit response */
	WRA_AUDIT_UPDATE,                               /**< @brief Audit update */
	WRA_AUDIT_RESET                                 /**< @brief Audit reset request */
};

/** @brief Type of heartbeat response */
enum wra_heartbeat_response_type
{
	WRA_RESPONSE_CLIENT_HEARTBEAT                   /**< @brief Client heartbeat response */
};

/** @brief Holds information about a sample published under a metric */
struct wra_sample
{
	struct wra_metric* parent;                      /**< @brief Handle to the parent metric */
	wra_timestamp_t time_stamp;                     /**< @brief Time stamp sample was published at */
	struct wra_data data;                           /**< @brief Data value for the sample */
};

/** @brief Enumeration for the type of telemetry */
enum wra_telemetry_type
{
	WRA_TELEMETRY_ALARM = 1u,                       /**< @brief Telemetry contains alarm object */
	WRA_TELEMETRY_EVENT,                            /**< @brief Telemetry contains event object */
	WRA_TELEMETRY_METRIC,                           /**< @brief Telemetry contains metric object */
};

/** @brief Structure containing information about a telemetry object */
struct wra_telemetry
{
	enum wra_telemetry_type type;                   /**< @brief Type of telemetry */
	struct wra_data data;                           /**< @brief Temporary data of the telemetry */
	/** @brief Backwards-compatible object for holding information */
	union
	{
		wra_metric_t* metric;                          /**< @brief Metric object this telemetry represents */
	} object;
	wra_bool_t      is_active;                      /**< @brief Telemetry is active (alarms and events) */
	wra_bool_t      time_stamp_has_value;           /**< @brief Whether a value has been set for the time stamp */
	wra_timestamp_t time_stamp;                     /**< @brief Time stamp of the telemetry */
};

/** @brief Internal information for the client and connection data */
struct wra
{
	/** @brief Current state of the agent */
	wra_state_t agent_state;
	/** @brief Comon queue for registering/deregistering the client */
	struct wra_connection client_queue;
	/** @brief Various queues unique for each client */
	struct wra_connection data_queue[QUEUE_INDEX_LAST];
	/** @brief Initialization topic for client getting reponse from agent */
	char wra_init_topic[ IOT_ID_MAX_LEN];
	/** @brief Identifier of the client */
	char id[ IOT_ID_MAX_LEN];
	/** @brief Function to call on log message */
	wra_log_callback_t* logger;
	/** @brief User data to pass to the log function */
	void* logger_user_data;
	/** @brief Condition variable signalling when a notification arrives */
	os_thread_condition_t notification_cond;
	/** @brief Mutex to lock notification condition variable */
	os_thread_mutex_t notification_mutex;
	/** @brief Notification thread */
	os_thread_t notification_thread;
	/** @brief Result of last notification message */
	wra_status_t notification_status;
	/** @brief Type of notification that was received */
	enum wra_notification_type notification_type;
	/** @brief Function to call on state change */
	wra_state_callback_t* state_callback;
	/** @brief User data to pass to state change function */
	void* state_user_data;

	/** @brief Pointer to new version of library */
	iot_t *iot;

	/** @brief First registered service */
	struct wra_service* service_first;
	/** @brief Last registered service */
	struct wra_service* service_last;
	/** @brief First registered source */
	struct wra_source* source_first;
	/** @brief Last registered source */
	struct wra_source* source_last;
};

/** @brief Type for a structure containing variable data */
typedef struct wra_data wra_data_t;
/** @brief Type defining the notification types */
typedef enum wra_notification_type wra_notification_type_t;
/** @brief Type defining the response types */
typedef enum wra_heartbeat_response_type wra_heartbeat_response_type_t;
/** @brief Type for a parameter */
typedef struct wra_parameter wra_parameter_t;

/**
 * @brief Converts a wait time in milliseconds to an absolute time
 *
 * @param[out]     absolute_time                 pointer to a time stamp structure, this is updated
 *                                               with the current time plus the relative time
 * @param[in]      relative_time                 time relative to the current clock in milliseconds
 *
 * @retval NULL                                  if relative time is 0 or the absolute time is NULL
 * @retval !NULL                                 pointer to the absolute time updated with the
 *                                               relative time
 */
wra_timestamp_t* wra_common_time_relative_to_absolute( wra_timestamp_t* absolute_time,
	wra_millisecond_t relative_time );

/**
 * @brief Implementation for deregistering a command from the cloud
 *
 * @param[in]      command                       command to deregister
 * @param[in]      abs_time_out                  absolute maximum time to wait for
 *                                               deregistration (optional)
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_NO_MEMORY                  not enough system memory
 * @retval WRA_STATUS_NOT_INITIALIZED            library not initialized with the cloud
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for deregistration
 */
wra_status_t wra_command_deregister_implementation( wra_command_t* command,
	const wra_timestamp_t* abs_time_out );

/**
 * @brief Implementation for registering & deregisteing commands
 *
 * @param[in]      parent                        parent service (NULL to deregister)
 * @param[in]      command                       command to register or unregister
 * @param[in]      abs_time_out                  absolute maximum time to wait (optional)
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to the function
 * @retval WRA_STATUS_FAILURE                    failed to write the message
 * @retval WRA_STATUS_FULL                       the queue is full
 * @retval WRA_STATUS_NO_MEMORY                  not enough memory available to generate message
 * @retval WRA_STATUS_NOT_INITIALIZED            the connection is not initialized
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out before being able to register or deregister
 */
wra_status_t wra_command_register_implementation( struct wra_service* parent,
	struct wra_command* command, const wra_timestamp_t* abs_time_out );

/**
 * @brief Implementation for destroying a previously allocated command
 *
 * @param[in]      command                       command to destroy
 * @param[in]      abs_time_out                  absolute maximum time to wait (optional)
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for command deallocation
 */
wra_status_t wra_command_free_implementation( wra_command_t* command, const wra_timestamp_t* abs_time_out );

/**
 * @brief Implementation for destroying a previously allocated metric
 *
 * @param[in]      metric                        metric to destroy
 * @param[in]      abs_time_out                  absolute maximum time to wait (optional)
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for deallocation
 */
wra_status_t wra_metric_free_implementation( wra_metric_t* metric, const wra_timestamp_t* abs_time_out );

/**
 * @brief Implementation for registering & deregisteing metrics
 *
 * @param[in,out]  parent                        parent source (NULL to deregister)
 * @param[in,out]  metric                        metric to register or unregister
 * @param[in]      abs_time_out                  absolute maximum time to wait (optional)
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to the function
 * @retval WRA_STATUS_FAILURE                    failed to write the message
 * @retval WRA_STATUS_FULL                       the queue is full
 * @retval WRA_STATUS_NOT_INITIALIZED            the connection is not initialized
 * @retval WRA_STATUS_SUCCESS                    success
 * @retval WRA_STATUS_TIMED_OUT                  timed out before being able to send
 */
wra_status_t wra_metric_register_implementation( struct wra_source* parent, struct wra_metric* metric,
	const wra_timestamp_t* abs_time_out );

/**
 * @brief Implementation for destroying a previously allocated service
 *
 * @param[in,out]  service                       service to destroy
 * @param[in]      abs_time_out                  absolute maximum time to wait (optional)
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  time out value reached
 */
wra_status_t wra_service_free_implementation( wra_service_t* service, const wra_timestamp_t* abs_time_out );

/**
 * @brief Implementation for destroying a previously allocated source
 *
 * @param[in]      source                        source to destroy
 * @param[in]      abs_time_out                  absolute maximum time to wait (optional)
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  time out value reached
 */
wra_status_t wra_source_free_implementation( wra_source_t* source, const wra_timestamp_t* abs_time_out );

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef WRA_INTERNAL_H */

