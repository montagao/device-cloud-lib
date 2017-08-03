/**
 * @file
 * @brief source file for the tr50 (telit) plugin
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "../../shared/iot_base64.h"
#include "../../shared/iot_defs.h"
#include <iot_json.h>
#include <iot_mqtt.h>
#include <iot_plugin.h>
#include <os.h>

#include "../../shared/iot_types.h"

/** @brief Maximum length for a "thingkey" */
#define TR50_THING_KEY_MAX_LEN         ( IOT_ID_MAX_LEN * 2 ) + 1u
/** @brief default QOS level */
#define TR50_MQTT_QOS                  1
/** @brief number of seconds to show "Connection loss message" */
#define TR50_SHOW_CONNECTION_LOSS_MSG  20u

/** @brief internal data required for the plug-in */
struct tr50_data
{
	/** @brief library handle */
	iot_t *lib;
	/** @brief sequential message id */
	unsigned int msg_id;
	/** @brief pointer to the mqtt connection to the cloud */
	iot_mqtt_t *mqtt;
	/** @brief the key of the thing */
	char thing_key[ TR50_THING_KEY_MAX_LEN + 1u ];
	/** @brief time_stamp of when connection loss is reported */
	iot_timestamp_t time_stamp_connetion_loss_reported;
};

/**
 * @brief function called to respond to the cloud on an action complete
 *
 * @param[in]      data                plug-in specific data
 * @param[in]      action              action being executed
 * @param[in]      request             action request
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t tr50_action_complete(
	struct tr50_data *data,
	const iot_action_t *action,
	const iot_action_request_t *request );

/**
 * @brief appends location information to json structure
 *
 * @param[in,out]  json                structure to append to
 * @param[in]      key                 (optional) key enclouse output in
 * @param[in]      location            location to append
 */
static IOT_SECTION void tr50_append_location(
	iot_json_encoder_t *json,
	const char *key,
	const iot_location_t *location );

/**
 * @brief appends a raw data value to to json structure
 *
 * @param[in,out]  json                structure to append to
 * @param[in]      key                 key to assoicate with object
 * @param[in]      value               value to append
 * @param[in]      len                 length of value to append
 */
static IOT_SECTION void tr50_append_value_raw(
	iot_json_encoder_t *json,
	const char *key,
	const void *value,
	size_t len );

/**
 * @brief Sends the message to check the mailbox for any cloud requests
 *
 * @param[in]      data                plug-in specific data
 *
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter passed to the function
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t tr50_check_mailbox(
	struct tr50_data *data );

/**
 * @brief helper function for tr50 to connect to the cloud
 *
 * @param[in]      lib                 loaded iot library
 * @param[in]      data                plug-in specific data
 * @param[in]      max_time_out        maximum time to wait
 *                                     (0 = wait indefinitely)
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t tr50_connect(
	iot_t *lib,
	struct tr50_data *data,
	iot_millisecond_t max_time_out );

/**
 * @brief helper function for tr50 to check connection to the cloud
 *
 * @param[in]      lib                 loaded iot library
 * @param[in]      data                plug-in specific data
 * @param[in]      max_time_out        maximum time to wait
 *                                     (0 = wait indefinitely)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t tr50_connect_check(
	iot_t *lib,
	struct tr50_data *data,
	iot_millisecond_t max_time_out );

/**
 * @brief plug-in function called to disable the plug-in
 *
 * @param[in]      lib                 loaded iot library
 * @param[in]      plugin_data         plugin specific data
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see tr50_enable
 * @see tr50_terminate
 */
iot_status_t tr50_disable(
	iot_t *lib,
	void* plugin_data,
	iot_bool_t force );

/**
 * @brief helper function for tr50 to disconnect from the cloud
 *
 * @param[in]      lib                 loaded iot library
 * @param[in]      data                plug-in specific data
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t tr50_disconnect(
	iot_t *lib,
	struct tr50_data *data );

/**
 * @brief plug-in function called to enable the plug-in
 *
 * @param[in]      lib                 loaded iot library
 * @param[in]      plugin_data         plugin specific data
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see tr50_disable
 * @see tr50_execute
 * @see tr50_initialize
 */
iot_status_t tr50_enable(
	iot_t *lib,
	void* plugin_data );

/**
 * @brief plug-in function called to perform work in the plug-in
 *
 * @param[in]      lib                 loaded iot library
 * @param[in]      plugin_data         plugin specific data
 * @param[in]      op                  operation to perform
 * @param[in]      max_time_out        maximum time to perform operation
 *                                     (0 = indefinite)
 * @param[in]      step                pointer to the operation step
 * @param[in]      item                item being modified in the operation
 * @param[in]      value               new value for the item
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see tr50_disable
 * @see tr50_enable
 * @see tr50_execute
 */
iot_status_t tr50_execute(
	iot_t *lib,
	void* plugin_data,
	iot_operation_t op,
	iot_millisecond_t max_time_out,
	iot_step_t *step,
	const void *item,
	const void *value );

/**
 * @brief plug-in function called to initialize the plug-in
 *
 * @param[in]      lib                 library handle
 * @param[out]     plugin_data         plugin specific data
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see tr50_enable
 * @see tr50_terminate
 */
iot_status_t tr50_initialize(
	iot_t *lib,
	void **plugin_data );

/**
 * @brief callback function that is called when tr50 receives a message from the
 *        cloud
 *
 * @param[in]      user_data           user specific data
 * @param[in]      topic               topic the message was received on
 * @param[in]      payload             payload that was received
 * @param[in]      payload_len         length of the received payload
 * @param[in]      qos                 mqtt quality of service level
 * @param[in]      retain              whether the message is to be retained
 */
static IOT_SECTION void tr50_on_message(
	void *user_data,
	const char *topic,
	void *payload,
	size_t payload_len,
	int qos,
	iot_bool_t retain );

/**
 * @brief convert a timestamp to a formatted time as in RFC3339
 *
 * @param[in]      ts                  time stamp to convert
 * @param[in,out]  out                 output buffer
 * @param[in]      len                 size of the output buffer
 *
 * @return a pointer to the output buffer
 */
static IOT_SECTION char *tr50_strtime(
	iot_timestamp_t ts,
	char *out,
	size_t len );

/**
 * @brief publishes a piece of iot telemetry to the cloud
 *
 * @param[in]      data                plug-in specific data
 * @param[in]      t                   telemetry object to publish
 * @param[in]      d                   data for telemetry object to publish
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t tr50_telemetry_publish(
	struct tr50_data *data,
	const iot_telemetry_t *t,
	const struct iot_data *d );

/**
 * @brief plug-in function called to terminate the plug-in
 *
 * @param[in]      lib                 library handle
 * @param[in]      plugin_data         plugin specific data
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see tr50_disable
 * @see tr50_initialize
 */
iot_status_t tr50_terminate(
	iot_t *lib,
	void *plugin_data );


iot_status_t tr50_action_complete(
	struct tr50_data *data,
	const iot_action_t *UNUSED(action),
	const iot_action_request_t *request )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( data )
	{
		const char *source = iot_action_request_source( request );

		result = IOT_STATUS_SUCCESS;
		if ( os_strncmp( source, "tr50", 4 ) == 0 )
		{
			const char *req_id;
			result = iot_action_request_attribute_get(
				request, "id", IOT_FALSE, IOT_TYPE_STRING, &req_id );
			if ( data && result == IOT_STATUS_SUCCESS && req_id && *req_id )
			{
				char buf[ 512u ];
				iot_json_encoder_t *json;
				json = iot_json_encode_initialize( buf, 512u, 0 );
				result = IOT_STATUS_NO_MEMORY;
				if ( json )
				{
					char id[6u];
					const char *msg = NULL;
					iot_status_t status;
					iot_action_request_parameter_iterator_t  iter;

					os_snprintf( id, 5u, "%d", data->msg_id );
					id[5u] = '\0';

					status = iot_action_request_status( request, &msg );

					iot_json_encode_object_start( json, id );
					iot_json_encode_string( json, "command", "mailbox.ack" );
					iot_json_encode_object_start( json, "params" );
					iot_json_encode_string( json, "id", req_id );
					iot_json_encode_integer( json, "errorCode", (int)status );
					if ( status != IOT_STATUS_SUCCESS && msg )
						iot_json_encode_string( json,
							"errorMessage", msg );

					if ( iot_action_request_parameter_iterator(
						request, IOT_PARAMETER_OUT,
						&iter ) == IOT_STATUS_SUCCESS )
					{
						iot_status_t res;
						iot_json_encode_object_start( json, "params" );
						do
						{
							const char *name;
							iot_type_t type;
							name = iot_action_request_parameter_iterator_name( request, iter );
							type = iot_action_request_parameter_iterator_data_type( request, iter );
							switch ( type )
							{
							case IOT_TYPE_BOOL:
							{
								iot_bool_t value = IOT_FALSE;
								iot_action_request_parameter_iterator_get(
									request,
									iter,
									IOT_FALSE,
									IOT_TYPE_BOOL,
									&value );
								iot_json_encode_bool( json, name, value );
								break;
							}
							case IOT_TYPE_FLOAT32:
							case IOT_TYPE_FLOAT64:
							{
								iot_float64_t value = 0.0;
								iot_action_request_parameter_iterator_get(
									request,
									iter,
									IOT_TRUE,
									IOT_TYPE_FLOAT64,
									&value );
								iot_json_encode_real( json, name, value );
								break;
							}
							case IOT_TYPE_INT8:
							case IOT_TYPE_INT16:
							case IOT_TYPE_INT32:
							case IOT_TYPE_INT64:
							case IOT_TYPE_UINT8:
							case IOT_TYPE_UINT16:
							case IOT_TYPE_UINT32:
							case IOT_TYPE_UINT64:
							{
								iot_int64_t value = 0;
								iot_action_request_parameter_iterator_get(
									request,
									iter,
									IOT_TRUE,
									IOT_TYPE_INT64,
									&value );
								iot_json_encode_integer( json, name, value );
								break;
							}
							case IOT_TYPE_RAW:
							{
								size_t raw_len = 0u;
								const void *raw = NULL;
								iot_action_request_parameter_iterator_get_raw(
									request,
									iter,
									IOT_FALSE,
									&raw_len,
									&raw );
								tr50_append_value_raw( json, name,
									raw, raw_len );
								break;
							}
							case IOT_TYPE_STRING:
							{
								const char *value;
								iot_action_request_parameter_iterator_get(
									request,
									iter,
									IOT_FALSE,
									IOT_TYPE_STRING,
									&value );
								iot_json_encode_string( json, name, value );
								break;
							}
							case IOT_TYPE_LOCATION:
							default:
								break;
							}
							res = iot_action_request_parameter_iterator_next( request,
								&iter );
						} while ( res == IOT_STATUS_SUCCESS );
						iot_json_encode_object_end( json );
					}

					iot_json_encode_object_end( json );
					iot_json_encode_object_end( json );

					msg = iot_json_encode_dump( json );
					os_printf( "-->%s\n", msg );
					iot_mqtt_publish( data->mqtt, "api",
						msg, os_strlen( msg ), 0, IOT_FALSE, NULL );
					++data->msg_id;
					result = IOT_STATUS_SUCCESS;
				}
			}
		}
	}
	return result;
}

void tr50_append_location(
	iot_json_encoder_t *json,
	const char *key,
	const iot_location_t *location )
{
	if ( location )
	{
		if ( key )
			iot_json_encode_object_start( json, key );
		iot_json_encode_real( json, "lat", location->latitude );
		iot_json_encode_real( json, "lng", location->longitude );
		if ( location->flags & IOT_FLAG_LOCATION_HEADING )
			iot_json_encode_integer( json, "heading", location->heading );
		if ( location->flags & IOT_FLAG_LOCATION_ALTITUDE )
			iot_json_encode_integer( json, "altitude", location->altitude );
		if ( location->flags & IOT_FLAG_LOCATION_SPEED )
			iot_json_encode_integer( json, "speed", location->speed );
		if ( location->flags & IOT_FLAG_LOCATION_ACCURACY )
			iot_json_encode_integer( json, "fixAcc", location->accuracy );
		if ( location->flags & IOT_FLAG_LOCATION_SOURCE )
		{
			const char *source;
			switch ( location->source )
			{
				case IOT_LOCATION_SOURCE_FIXED:
					source = "manual";
					break;
				case IOT_LOCATION_SOURCE_GPS:
					source = "gps";
					break;
				case IOT_LOCATION_SOURCE_WIFI:
					source = "wifi";
					break;
				case IOT_LOCATION_SOURCE_UNKNOWN:
				default:
					source = "m2m-locate";
			}
			iot_json_encode_string( json, "fixType", source );
		}
		if ( location->flags & IOT_FLAG_LOCATION_TAG )
			iot_json_encode_string( json, "street", location->tag );

		if ( key )
			iot_json_encode_object_end( json );
	}
}

void tr50_append_value_raw(
	iot_json_encoder_t *json,
	const char *key,
	const void *value,
	size_t len )
{
	uint8_t *heap = NULL;
	if ( !value )
		value = "";
	else if ( len != (size_t)-1 )
	{
		size_t req_len = iot_base64_encode_size( len );
		heap = os_malloc( sizeof(uint8_t) * req_len + 1u );
		if ( heap )
		{
			iot_base64_encode( heap, req_len, value, len );
			heap[req_len] = '\0';
			value = heap;
		}
	}
	iot_json_encode_string( json, key, (const char *)value );
	if ( heap )
		os_free( heap );
}

iot_status_t tr50_check_mailbox(
	struct tr50_data *data )
{
	/* check for any outstanding messages on the cloud, when enabling plug-in */
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( data )
	{
		char id[6u];
		const char *msg;
		char req_buf[376u];
		iot_json_encoder_t *req_json;

		/* build the message: "{\"cmd\":{\"command\":\"mailbox.check\"}}" */
		os_snprintf( id, 5u, "%d", data->msg_id );
		id[5u] = '\0';
		req_json = iot_json_encode_initialize(
			req_buf, 376u, 0 );
		iot_json_encode_object_start( req_json, "cmd" );
		iot_json_encode_string( req_json, "command", "mailbox.check" );
		iot_json_encode_object_start( req_json, "params" );
		iot_json_encode_bool( req_json, "autoComplete", IOT_FALSE );
		iot_json_encode_object_end( req_json );
		iot_json_encode_object_end( req_json );
		msg = iot_json_encode_dump( req_json );
		if ( msg )
			iot_mqtt_publish( data->mqtt, "api", msg,
				os_strlen( msg ), 0, IOT_FALSE, NULL );
		else
			os_printf( "Error failed to generate mesage for checking mail\n" );
		iot_json_encode_terminate( req_json );
		++data->msg_id;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t tr50_connect(
	iot_t *lib,
	struct tr50_data *data,
	iot_millisecond_t max_time_out )
{
	iot_status_t result = IOT_STATUS_FAILURE;
	IOT_LOG( lib, IOT_LOG_TRACE, "tr50: %s", "connect" );
	if ( data )
	{
		const char *app_token = NULL;
		const char *ca_bundle = NULL;
		const char *host = NULL;
		iot_int64_t port = 0;
		iot_mqtt_ssl_t ssl_conf;
		iot_bool_t validate_cert = IOT_FALSE;

		iot_attribute_get( lib, "cloud.host", IOT_FALSE,
			IOT_TYPE_STRING, &host );
		iot_attribute_get( lib, "cloud.port", IOT_FALSE,
			IOT_TYPE_INT64, &port );
		iot_attribute_get( lib, "cloud.token", IOT_FALSE,
			IOT_TYPE_STRING, &app_token );
		iot_attribute_get( lib, "ca_bundle_file", IOT_FALSE,
			IOT_TYPE_STRING, &ca_bundle );
		iot_attribute_get( lib, "validate_cloud_cert", IOT_FALSE,
			IOT_TYPE_BOOL, &validate_cert );

		os_memzero( &ssl_conf, sizeof( iot_mqtt_ssl_t ) );
		ssl_conf.ca_path = ca_bundle;
		ssl_conf.insecure = !validate_cert;

		if ( app_token == NULL )
			IOT_LOG( lib, IOT_LOG_ERROR, "tr50: %s",
				"no application token provided" );

		os_snprintf( data->thing_key, TR50_THING_KEY_MAX_LEN,
			"%s-%s", lib->device_id, iot_id( lib ) );
		data->thing_key[ TR50_THING_KEY_MAX_LEN ] = '\0';
		data->mqtt = iot_mqtt_connect(
			iot_id( lib ),
			host,
			(iot_uint16_t)port,
			&ssl_conf,
			data->thing_key,
			app_token,
			max_time_out );

		if ( data->mqtt )
		{
			iot_mqtt_set_user_data( data->mqtt, data );
			iot_mqtt_set_message_callback( data->mqtt,
				tr50_on_message );
			iot_mqtt_subscribe( data->mqtt, "reply/#", TR50_MQTT_QOS );
			result = tr50_check_mailbox( data );
		}
		else
			IOT_LOG( lib, IOT_LOG_ERROR, "tr50: %s",
				"failed to connect" );
	}
	return result;
}

iot_status_t tr50_connect_check(
	iot_t *lib,
	struct tr50_data *data,
	iot_millisecond_t max_time_out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;

	if ( lib && data && data->mqtt )
	{
		iot_bool_t connected = IOT_TRUE;
		iot_bool_t connection_changed = IOT_TRUE;
		iot_timestamp_t time_stamp_current =
			iot_timestamp_now();
		iot_timestamp_t time_stamp_connection_changed;

		if( max_time_out == 0u )
			max_time_out = IOT_MILLISECONDS_IN_SECOND;

		result = IOT_STATUS_FAILURE;


		if( ( iot_mqtt_get_connection_status( data->mqtt,
			&connected, &connection_changed,
			&time_stamp_connection_changed ) ==
				IOT_STATUS_SUCCESS ) &&
			( connected == IOT_FALSE ||
			  connection_changed == IOT_TRUE ) )
		{
			const char *app_token = NULL;
			const char *ca_bundle = NULL;
			const char *host = NULL;
			iot_int64_t port = 0;
			iot_mqtt_ssl_t ssl_conf;
			iot_bool_t validate_cert = IOT_FALSE;

			iot_attribute_get( lib, "cloud.host", IOT_FALSE,
				IOT_TYPE_STRING, &host );
			iot_attribute_get( lib, "cloud.port", IOT_FALSE,
				IOT_TYPE_INT64, &port );
			iot_attribute_get( lib, "cloud.token", IOT_FALSE,
				IOT_TYPE_STRING, &app_token );
			iot_attribute_get( lib, "ca_bundle_file", IOT_FALSE,
				IOT_TYPE_STRING, &ca_bundle );
			iot_attribute_get( lib, "validate_cloud_cert", IOT_FALSE,
				IOT_TYPE_BOOL, &validate_cert );

			os_memzero( &ssl_conf, sizeof( iot_mqtt_ssl_t ) );
			ssl_conf.ca_path = ca_bundle;
			ssl_conf.insecure = !validate_cert;

			if ( app_token == NULL )
				IOT_LOG( lib, IOT_LOG_ERROR, "tr50 reconnect: %s",
					"no application token provided" );

			os_snprintf( data->thing_key, TR50_THING_KEY_MAX_LEN,
				"%s-%s", lib->device_id, iot_id( lib ) );
			data->thing_key[ TR50_THING_KEY_MAX_LEN ] = '\0';
			if( iot_mqtt_reconnect(
				data->mqtt,
				iot_id( lib ),
				host,
				(iot_uint16_t)port,
				&ssl_conf,
				data->thing_key,
				app_token,
				max_time_out ) == IOT_STATUS_SUCCESS )
			{
				iot_mqtt_subscribe( data->mqtt, "reply/#", TR50_MQTT_QOS );
				result = tr50_check_mailbox( data );
				IOT_LOG( lib, IOT_LOG_INFO, "tr50 reconnect: %s",
					"successfully");
				result = IOT_STATUS_SUCCESS;
			}
			else
			{
				iot_timestamp_t time_stamp_diff =
					time_stamp_current -
					time_stamp_connection_changed;
				iot_timestamp_t time_stamp_reported =
					time_stamp_current -
					data->time_stamp_connetion_loss_reported;

				time_stamp_diff /= IOT_MILLISECONDS_IN_SECOND;
				time_stamp_reported /= IOT_MILLISECONDS_IN_SECOND;

				if ( time_stamp_diff >= TR50_SHOW_CONNECTION_LOSS_MSG &&
				     time_stamp_reported >= TR50_SHOW_CONNECTION_LOSS_MSG )
				{
					IOT_LOG( lib, IOT_LOG_INFO,
						"tr50 connection loss for %d seconds",
						(int)time_stamp_diff );
					data->time_stamp_connetion_loss_reported =
						time_stamp_current;
				}
			}
		}
		else
			os_time_sleep( max_time_out, OS_TRUE );
	}
	return result;
}

iot_status_t tr50_disconnect(
	iot_t *lib,
	struct tr50_data *data )
{
	iot_status_t result = IOT_STATUS_FAILURE;
	IOT_LOG( lib, IOT_LOG_TRACE, "tr50: %s", "disconnect" );
	if ( data )
		result = iot_mqtt_disconnect( data->mqtt );
	return result;
}

iot_status_t tr50_disable(
	iot_t *lib,
	void *UNUSED(plugin_data),
	iot_bool_t UNUSED(force) )
{
	IOT_LOG( lib, IOT_LOG_TRACE, "tr50: %s", "disable" );
	/** @todo check if we are connected and call tr50_disconnect */
	/*
	if ( iot_connected( lib ) )
		tr50_disconnect( (struct tr50_data *)plugin_data );
	*/
	return IOT_STATUS_SUCCESS;
}

iot_status_t tr50_enable(
	iot_t *lib,
	void *UNUSED(plugin_data) )
{
	IOT_LOG( lib, IOT_LOG_TRACE, "tr50: %s", "enable" );
	/** @todo check if we are suppose to be connected an call: tr50_connect */
	/*
	if ( iot_connected( lib ) )
		tr50_connect( (struct tr50_data *)plugin_data );
	*/
	return IOT_STATUS_SUCCESS;
}

iot_status_t tr50_execute(
	iot_t *lib,
	void* plugin_data,
	iot_operation_t op,
	iot_millisecond_t max_time_out,
	iot_step_t *step,
	const void *item,
	const void *value )
{
	iot_status_t result = IOT_STATUS_SUCCESS;
	struct tr50_data *const data = plugin_data;
	if ( op != IOT_OPERATION_ITERATION )
		IOT_LOG( lib, IOT_LOG_TRACE, "tr50: %s %d.%d",
			"execute", (int)op, (int)*step );
	else
		tr50_connect_check( lib, data, max_time_out );

	if ( *step == IOT_STEP_DURING )
	{
		switch( op )
		{
			case IOT_OPERATION_CLIENT_CONNECT:
				result = tr50_connect( lib, data,
					max_time_out );
				break;
			case IOT_OPERATION_CLIENT_DISCONNECT:
				result = tr50_disconnect( lib, data );
				break;
			case IOT_OPERATION_TELEMETRY_PUBLISH:
				result = tr50_telemetry_publish( data,
					(const iot_telemetry_t*)item,
					(const struct iot_data*)value );
				break;
			case IOT_OPERATION_ACTION_COMPLETE:
				result = tr50_action_complete( data,
					(const iot_action_t*)item,
					(const iot_action_request_t*)value );
			default:
				/* unhandled operations */
				break;
		}
	}
	return result;
}

iot_status_t tr50_initialize(
	iot_t *lib,
	void **plugin_data )
{
	iot_status_t result = IOT_STATUS_NO_MEMORY;
	struct tr50_data *const data = os_malloc( sizeof( struct tr50_data ) );
	IOT_LOG( lib, IOT_LOG_TRACE, "tr50: %s", "initialize" );
	if ( data )
	{
		os_memzero( data, sizeof( struct tr50_data ) );
		data->lib = lib;
		*plugin_data = data;

		result = iot_mqtt_initialize();
	}
	return result;
}

iot_status_t tr50_terminate(
	iot_t *lib,
	void *plugin_data )
{
	iot_status_t result = IOT_STATUS_SUCCESS;
	struct tr50_data *data = plugin_data;
	IOT_LOG( lib, IOT_LOG_TRACE, "tr50: %s", "terminate" );
	os_free_null( (void**)&data );
	iot_mqtt_terminate();
	return result;
}

void tr50_on_message(
	void *user_data,
	const char *topic,
	void *payload,
	size_t payload_len,
	int UNUSED(qos),
	iot_bool_t UNUSED(retain) )
{
	char buf[1024u];
	struct tr50_data *const data = (struct tr50_data *)(user_data);
	iot_json_decoder_t *json;
	iot_json_item_t *root;

	if ( data )
		IOT_LOG( data->lib, IOT_LOG_TRACE,
			"tr50: received (%u bytes on %s): %.*s\n",
			(unsigned int)payload_len, topic,
			(int)payload_len, (const char *)payload );

	os_printf( "-->received: %.*s\n", (int)payload_len, (const char *)payload );

	json = iot_json_decode_initialize( buf, 1024u, 0u );
	if ( data && json &&
		iot_json_decode_parse( json, payload, payload_len, &root,
			NULL, 0u ) == IOT_STATUS_SUCCESS )
	{

		if ( os_strcmp( topic, "notify/mailbox_activity" ) == 0 )
		{
			iot_json_type_t type;
			iot_json_item_t *const j_thing_key =
				iot_json_decode_object_find( json, root,
					"thingKey" );
			type = iot_json_decode_type( json, j_thing_key );

			if ( type == IOT_JSON_TYPE_STRING )
			{
				const char *v = NULL;
				size_t v_len = 0u;
				iot_json_decode_string( json, j_thing_key,
					&v, &v_len );

				/* check if message is for us */
				if ( os_strncmp( v, data->thing_key, v_len ) == 0 )
					tr50_check_mailbox( data );
			}
		}
		else if ( os_strcmp( topic, "reply" ) == 0 )
		{
			iot_json_item_t *j_obj = iot_json_decode_object_find(
				json, root, "cmd" );
			if ( j_obj )
			{
				j_obj = iot_json_decode_object_find( json,
					j_obj, "params" );
				j_obj = iot_json_decode_object_find( json,
					j_obj, "messages" );
				if ( j_obj && iot_json_decode_type( json, j_obj )
					== IOT_JSON_TYPE_ARRAY )
				{
					const char *v = NULL;
					size_t v_len = 0u;
					size_t i;
					const size_t msg_count =
						iot_json_decode_array_size( json, j_obj );
					for ( i = 0u; i < msg_count; ++i )
					{
						iot_json_item_t *j_item;
						if ( iot_json_decode_array_at( json,
							j_obj, i, &j_item ) == IOT_STATUS_SUCCESS )
						{
							iot_json_item_t *j_id;
							iot_json_item_t *j_params;
							j_id = iot_json_decode_object_find(
								json, j_item, "id" );
							j_params = iot_json_decode_object_find(
								json, j_item, "params" );

							if ( j_id && j_params )
							{
								iot_json_item_t *j_method;
								iot_json_object_iterator_t *iter;
								iot_action_request_t *req = NULL;

								j_method = iot_json_decode_object_find(
									json, j_params, "method" );
								if ( j_method )
								{
									char id[ IOT_ID_MAX_LEN + 1u ];
									char name[ IOT_NAME_MAX_LEN + 1u ];
									*id = '\0';

									iot_json_decode_string( json, j_id, &v, &v_len );
									os_snprintf( id, IOT_ID_MAX_LEN, "%.*s", (int)v_len, v );
									id[ IOT_ID_MAX_LEN ] = '\0';

									iot_json_decode_string( json, j_method, &v, &v_len );
									os_snprintf( name, IOT_NAME_MAX_LEN, "%.*s", (int)v_len, v );
									name[ IOT_NAME_MAX_LEN ] = '\0';
									req = iot_action_request_allocate( data->lib, name, "tr50" );
									iot_action_request_attribute_set( req, "id", IOT_TYPE_STRING, id );
								}

								/* for each parameter */
								j_params = iot_json_decode_object_find(
									json, j_params, "params" );
								iter = iot_json_decode_object_iterator(
									json, j_params );
								while ( iter )
								{
									char name[ IOT_NAME_MAX_LEN + 1u ];
									iot_json_item_t *j_value = NULL;
									iot_json_decode_object_iterator_key(
										json, j_params, iter,
										&v, &v_len );
									iot_json_decode_object_iterator_value(
										json, j_params, iter,
										&j_value );
									os_snprintf( name, IOT_NAME_MAX_LEN, "%.*s", (int)v_len, v );
									name[ IOT_NAME_MAX_LEN ] = '\0';
									iter = iot_json_decode_object_iterator_next(
										json, j_params, iter );
									switch ( iot_json_decode_type( json,
										j_value ) )
									{
									case IOT_JSON_TYPE_BOOL:
										{
										iot_bool_t value;
										iot_json_decode_bool( json, j_value, &value );
										iot_action_request_parameter_set( req, name, IOT_TYPE_BOOL, value );
										}
									case IOT_JSON_TYPE_INTEGER:
										{
										iot_int64_t value;
										iot_json_decode_integer( json, j_value, &value );
										iot_action_request_parameter_set( req, name, IOT_TYPE_INT64, value );
										}
										break;
									case IOT_JSON_TYPE_REAL:
										{
										iot_float64_t value;
										iot_json_decode_real( json, j_value, &value );
										iot_action_request_parameter_set( req, name, IOT_TYPE_FLOAT64, value );
										}
										break;
									case IOT_JSON_TYPE_STRING:
										{
										char *value;
										iot_json_decode_string( json, j_value, &v, &v_len );
										value = os_malloc( v_len + 1u );
										if( value )
										{
											os_strncpy( value, v, v_len );
											value[v_len] = '\0';
											iot_action_request_parameter_set( req, name, IOT_TYPE_STRING, value );
											os_free( value );
										}
										}
									case IOT_JSON_TYPE_ARRAY:
									case IOT_JSON_TYPE_OBJECT:
									case IOT_JSON_TYPE_NULL:
									default:
										break;
									}
								}

								if ( req )
									iot_action_request_execute( req, 0u );
							}
						}
					}
				}
			}
		}
		else
			IOT_LOG( data->lib, IOT_LOG_TRACE, "tr50: %s",
				"message received on unknown topic" );
		iot_json_decode_terminate( json );
	}
	else if ( data )
		IOT_LOG( data->lib, IOT_LOG_ERROR, "tr50: %s",
			"failed to parse incoming message" );
}

char *tr50_strtime( iot_timestamp_t ts,
	char *out, size_t len )
{
	size_t out_len;

	/* TR50 format: "YYYY-MM-DDTHH:MM:SS.mmZ" */
	out_len = os_time_format( out, len,
		"%Y-%m-%dT%H:%M:%S", ts, OS_FALSE );
	if ( out_len > 0u )
	{
		/* add milliseconds, if there are any remaining */
		ts %= 1000u;
		if ( ts > 0u )
		{
			size_t out_req;
			out_req = os_snprintf( &out[out_len],
				len - out_len, ".%u", (unsigned int)ts );

			if ( out_req < len - out_len )
				out_len += out_req;
			else
				out_len = 0u;
		}

		/* add Zulu time indicator */
		if ( out_len > 0u && out_len < len - 1u )
			out[out_len++] = 'Z';
		else /* buffer too small */
			out_len = 0u;
	}

	/* ensure null-terminated */
	if ( out && len )
		out[out_len] = '\0';
	return out;
}

iot_status_t tr50_telemetry_publish(
	struct tr50_data *data,
	const iot_telemetry_t *t,
	const struct iot_data *d )
{
	iot_status_t result = IOT_STATUS_FAILURE;
	if ( d->has_value )
	{
		char buf[512u];
		const char *cmd;
		char id[6u];
		const char *msg;
		const char *const value_key = "value";
		iot_json_encoder_t *const json =
			iot_json_encode_initialize( buf, 512u, 0u);

		if ( d->type == IOT_TYPE_LOCATION )
			cmd = "location.publish";
		else if ( d->type == IOT_TYPE_STRING ||
			d->type == IOT_TYPE_RAW )
			cmd = "attribute.publish";
		else
			cmd = "property.publish";

		/* convert id to string */
		os_snprintf( id, 5u, "%d", data->msg_id );
		id[5u] = '\0';
		iot_json_encode_object_start( json, id );
		iot_json_encode_string( json, "command", cmd );
		iot_json_encode_object_start( json, "params" );
		iot_json_encode_string( json, "thingKey",
			data->thing_key );
		iot_json_encode_string( json, "key",
			iot_telemetry_name_get( t ) );
		switch ( d->type )
		{
		case IOT_TYPE_BOOL:
			iot_json_encode_real( json, value_key,
				(double)d->value.boolean );
			break;
		case IOT_TYPE_FLOAT32:
			iot_json_encode_real( json, value_key,
				(double)d->value.float32 );
			break;
		case IOT_TYPE_FLOAT64:
			iot_json_encode_real( json, value_key,
				(double)d->value.float64 );
			break;
		case IOT_TYPE_INT8:
			iot_json_encode_real( json, value_key,
				(double)d->value.int8 );
			break;
		case IOT_TYPE_INT16:
			iot_json_encode_real( json, value_key,
				(double)d->value.int16 );
			break;
		case IOT_TYPE_INT32:
			iot_json_encode_real( json, value_key,
				(double)d->value.int32 );
			break;
		case IOT_TYPE_INT64:
			iot_json_encode_real( json, value_key,
				(double)d->value.int64 );
			break;
		case IOT_TYPE_UINT8:
			iot_json_encode_real( json, value_key,
				(double)d->value.uint8 );
			break;
		case IOT_TYPE_UINT16:
			iot_json_encode_real( json, value_key,
				(double)d->value.uint16 );
			break;
		case IOT_TYPE_UINT32:
			iot_json_encode_real( json, value_key,
				(double)d->value.uint32 );
			break;
		case IOT_TYPE_UINT64:
			iot_json_encode_real( json, value_key,
				(double)d->value.uint64 );
			break;
		case IOT_TYPE_RAW:
			tr50_append_value_raw( json, value_key,
				d->value.raw.ptr, d->value.raw.length );
			break;
		case IOT_TYPE_STRING:
			tr50_append_value_raw( json, value_key,
				d->value.string, (size_t)-1 );
			break;
		case IOT_TYPE_LOCATION:
			tr50_append_location( json, NULL, d->value.location );
			break;
		default:
			break;
		}
		if ( t->time_stamp > 0u )
		{
			char ts_str[32u];
			tr50_strtime( t->time_stamp, ts_str, 25u );
			iot_json_encode_string( json, "ts", ts_str );
		}
		iot_json_encode_object_end( json );
		iot_json_encode_object_end( json );

		msg = iot_json_encode_dump( json );
		os_printf( "-->%s\n", msg );
		result = iot_mqtt_publish( data->mqtt, "api",
			msg, os_strlen( msg ), TR50_MQTT_QOS, IOT_FALSE, NULL );
		iot_json_encode_terminate( json );
		++data->msg_id;
	}
	return result;
}

IOT_PLUGIN( tr50, 10, iot_version_encode(1,0,0,0),
	iot_version_encode(2,3,0,0), 0 )

