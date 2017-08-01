/**
 * @file
 * @brief Source file for Wind River Sensor library backwards compatibility
 *
 * @copyright Copyright (C) 2015-2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software may
 * be licensed only pursuant to the terms of an applicable Wind River license
 * agreement.  No license to Wind River intellectual properly rights is granted
 * herein.  All rights not licensed by Wind River are reserved by Wind River.
 */

#include "public/wra_compat.h"

#include "public/wra_command.h"      /* for wra_command* functions */
#include "public/wra_common.h"       /* for wra_initialize, wra_terminate */
#include "public/wra_metric.h"       /* for wra_metric* functions */
#include "public/wra_service.h"      /* for wra_service* functions */
#include "public/wra_source.h"       /* for wra_source* functions */
#include "shared/wra_internal.h"     /* internal support */

/** @brief Maximum length of a backwards-compatible command parameter */
#define WRA_COMPAT_COMMAND_PARAM_MAX_LEN 256u
/** @brief Name for the arguments to a backwards-compatible command */
#define WRA_COMPAT_COMMAND_PARAM_NAME "args"
/** @brief Version for new metrics for backwards-compatibility */
#define WRA_COMPAT_METRIC_VERSION NULL
/** @brief Name for service to put backwards-compatible commands under */
#define WRA_COMPAT_SERVICE_NAME "default service"
/** @brief Version for the service to put backwards-compatible commands under */
#define WRA_COMPAT_SERVICE_VERSION NULL
/** @brief Name for source to put backward-compatible telemetry under */
#define WRA_COMPAT_SOURCE_NAME "default source"
/** @brief Version for the source to put backwards-compatible telemetry under */
#define WRA_COMPAT_SOURCE_VERSION  NULL

/** @brief Structure for holding registered command */
struct compat_command
{
	wra_app_ex_handler_t handler;   /**< @brief Backwards-compatible handler */
};

/** @brief Defines type for holding registered commands */
typedef struct compat_command compat_command_t;

/**
 * @brief Helper function to free dynamic data, if a data object contains some
 *
 * @param[in,out] data                           object to remove data for
 */
static void wra_compat_free_dynamic_data( wra_data_t* data );

/**
 * @brief Action handler for backwards compatible command
 *
 * @param[in]     request                        request received from the cloud
 * @param[in]     user_data                      pointer to the backwards-compatible callback to call
 *
 * @retval WRA_COMMAND_STATUS_COMPLETE           callback successfully invoked
 * @retval WRA_COMMAND_STATUS_EXECUTION_ERROR    callback unsuccessfully invoked
 */
static wra_command_status_t wra_compat_handle_command( wra_command_request_t* request, void* user_data );

void wra_compat_free_dynamic_data( wra_data_t* data )
{
	if ( data && data->has_value == WRA_TRUE )
	{
		os_free_null( (void **)&data->heap_storage );
		os_memzero( data, sizeof( struct wra_data ) );
		data->has_value = WRA_FALSE;
	}
}

wra_command_status_t wra_compat_handle_command( wra_command_request_t* request, void* user_data )
{
	wra_command_status_t result = WRA_COMMAND_EXECUTION_ERROR;
	const compat_command_t* compat_command = (compat_command_t*)user_data;
	if ( compat_command )
	{
		const wra_app_ex_handler_t compat_cb = compat_command->handler;
		if ( compat_cb )
		{
			int cb_result;
			char param_value[WRA_COMPAT_COMMAND_PARAM_MAX_LEN];
			const char *command_name;
			os_memzero( param_value,
				WRA_COMPAT_COMMAND_PARAM_MAX_LEN );
			wra_command_parameter_string_get( request,
				WRA_COMPAT_COMMAND_PARAM_NAME, param_value,
				WRA_COMPAT_COMMAND_PARAM_MAX_LEN );
			command_name = os_strrchr( request->name, '/' );
			if ( command_name )
				++command_name;
			else
				command_name = request->name;
			cb_result = (*compat_cb)( NULL, command_name, param_value );
			if( cb_result )
				result = WRA_COMMAND_COMPLETE;
		}
	}

	return result;
}

wra_status_t wra_reg_app_ex_handler( wra_handle wra_h, wra_app_ex_handler_t app_ex_handler, const char* name )
{
	if ( wra_h )
		IOT_LOG( wra_h->iot, IOT_LOG_WARNING,
			"The function %s is deprecated in this version of the Wind River Agent library.", IOT_FUNC );
	return wra_action_subscribe( wra_h, app_ex_handler, name );
}

wra_status_t wra_action_subscribe( wra_handle wra_h, wra_app_ex_handler_t app_ex_handler, const char* action_name )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( wra_h )
		IOT_LOG( wra_h->iot, IOT_LOG_WARNING,
			"The function %s is deprecated in this version of the Wind River Agent library.", IOT_FUNC );
	if ( wra_h && app_ex_handler && action_name )
	{
		wra_service_t* service = NULL;
		result = wra_service_find( wra_h, WRA_COMPAT_SERVICE_NAME, WRA_COMPAT_SERVICE_VERSION, &service );
		if ( result == WRA_STATUS_NOT_FOUND )
		{
			service = wra_service_allocate( WRA_COMPAT_SERVICE_NAME, WRA_COMPAT_SERVICE_VERSION );
			result = wra_service_register( wra_h, service, 0u );
		}

		if ( service )
		{
			wra_command_t* new_command = wra_command_allocate( action_name );
			result = WRA_STATUS_NO_MEMORY;
			if ( new_command )
			{
				compat_command_t* new_compat_command =
					(compat_command_t*)os_malloc( sizeof( compat_command_t ) );
				if ( new_compat_command )
				{
					wra_command_parameter_add( new_command, WRA_COMPAT_COMMAND_PARAM_NAME, WRA_TYPE_STRING, 0u );

					os_memzero( new_compat_command,
						sizeof( compat_command_t ) );
					new_compat_command->handler = app_ex_handler;

					/* register callback to handle compat actions */
					new_command->compat_data = new_compat_command;
					result = wra_command_register_callback( new_command, &wra_compat_handle_command, new_compat_command );
					if ( result == WRA_STATUS_SUCCESS )
						result = wra_command_register( service, new_command, 0u );
				}
				else
					wra_command_free( new_command, 0u );
			}
		}
	}
	return result;
}

wra_status_t wra_action_unsubscribe( wra_handle wra_h, const char* action_name )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( wra_h )
		IOT_LOG( wra_h->iot, IOT_LOG_WARNING,
			"The function %s is deprecated in this version of the Wind River Agent library.", IOT_FUNC );
	if ( wra_h && action_name )
	{
		struct wra_service* service = wra_h->service_first;
		result = WRA_STATUS_NOT_FOUND;
		while ( service )
		{
			struct wra_command* command = service->command_first;
			struct wra_command* prev_command = NULL;
			while( command )
			{
				if ( os_strncmp( command->command_name, action_name, IOT_NAME_MAX_LEN - 1u ) == 0 )
					result = wra_command_deregister( command, 0u );
				else
					prev_command = command;
				if ( prev_command )
					command = prev_command->command_next;
				else
					command = NULL;
			}
			service = service->service_next;
		}

	}
	return result;
}

int wra_action_wait( wra_handle wra_h, const wra_timestamp* time_out )
{
	wra_status_t result;
	wra_millisecond_t time_to_wait = 0u;
	if ( wra_h )
		IOT_LOG( wra_h->iot, IOT_LOG_WARNING,
			"The function %s is deprecated in this version of the Wind River Agent library.", IOT_FUNC );
	if ( time_out )
	{
		time_to_wait = ((wra_millisecond_t)
			( time_out->tv_sec * IOT_MILLISECONDS_IN_SECOND )) +
			((wra_millisecond_t)( time_out->tv_usec / IOT_MICROSECONDS_IN_MILLISECOND ));
	}
	result = wra_command_wait( wra_h, time_to_wait );
	return (int)result;
}

wra_status_t wra_delete_handle( wra_handle wra_h )
{
	if ( wra_h )
		IOT_LOG( wra_h->iot, IOT_LOG_WARNING,
			"The function %s is deprecated in this version of the Wind River Agent library.", IOT_FUNC );
	return wra_terminate( wra_h, 0u );
}

wra_handle wra_gethandle( void )
{
	wra_t* lib = wra_initialize( NULL );
	if ( wra_connect( lib, 0u ) != WRA_STATUS_SUCCESS )
		lib = NULL;
	return lib;
}

wra_tm_handle wra_tm_create( const char* type, const char* name )
{
	struct wra_telemetry* result = WRA_NULL;
	if ( type && name )
	{
		wra_bool_t is_valid = WRA_FALSE;
		result = (struct wra_telemetry*)os_malloc( sizeof( struct wra_telemetry ) );
		if ( result )
		{
			os_memzero( result, sizeof( struct wra_telemetry ) );
			result->is_active = WRA_TRUE;
			if ( os_strcmp( type, WRA_TM_ALARMTM ) == 0 )
			{
				/* ALARMS aren't currently supported */
				/* let's return success for backwards-compatibilty, though */
				result->type = WRA_TELEMETRY_ALARM;
				is_valid = WRA_TRUE;
			}
			else if ( os_strcmp( type, WRA_TM_EVENTTM ) == 0 )
			{
				/* EVENTS aren't currently supported */
				/* let's return success for backwards-compatibilty, though */
				result->type = WRA_TELEMETRY_EVENT;
				is_valid = WRA_TRUE;
			}
			else if ( os_strcmp( type, WRA_TM_DATATM ) == 0 )
			{
				wra_metric_t* metric = wra_metric_allocate( name, WRA_COMPAT_METRIC_VERSION );
				if ( metric )
				{
					result->object.metric = metric;
					result->type = WRA_TELEMETRY_METRIC;
					is_valid = WRA_TRUE;
				}
			}

			if ( is_valid == WRA_FALSE )
				os_free_null( (void **)&result );
		}
	}
	return result;
}

wra_status_t wra_tm_destroy( wra_tm_handle tm )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( tm )
	{
		result = WRA_STATUS_BAD_REQUEST;
		switch ( tm->type )
		{
		case WRA_TELEMETRY_ALARM:
		case WRA_TELEMETRY_EVENT:
			result = WRA_STATUS_SUCCESS;
			break;
		case WRA_TELEMETRY_METRIC:
			result = wra_metric_free( tm->object.metric, 0u );
		}

		wra_compat_free_dynamic_data( &tm->data );
		if ( result == WRA_STATUS_SUCCESS )
		{
			os_memzero( tm, sizeof( struct wra_telemetry ) );
			os_free_null( (void **)&tm );
		}
	}
	return result;
}

wra_status_t wra_tm_getaux( wra_tm_handle tm, const char* attr, wra_tm_handle aux )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( tm && attr && aux )
	{
		result = WRA_STATUS_FAILURE;
		if ( tm->type == WRA_TELEMETRY_METRIC )
			result = WRA_STATUS_BAD_REQUEST;
		else if ( tm->type == WRA_TELEMETRY_ALARM || tm->type == WRA_TELEMETRY_EVENT )
			result = WRA_STATUS_SUCCESS;
	}
	return result;
}

wra_status_t wra_tm_gettimestamp( wra_tm_handle tm, wra_timestamp* time_stamp )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( tm && time_stamp )
	{
		result = WRA_STATUS_NOT_INITIALIZED;
		if ( tm->time_stamp_has_value != WRA_FALSE )
		{
			os_memcpy( time_stamp, &tm->time_stamp, sizeof( wra_timestamp ) );
			result = WRA_STATUS_SUCCESS;
		}
	}
	return result;
}

wra_status_t wra_tm_getvalue_bool( wra_tm_handle tm, const char* attr, bool *value )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( tm && attr && value )
	{
		result = WRA_STATUS_FAILURE;
		if ( tm->type == WRA_TELEMETRY_ALARM || tm->type == WRA_TELEMETRY_EVENT ||
		     tm->type == WRA_TELEMETRY_METRIC )
		{
			result = WRA_STATUS_BAD_REQUEST;
			if ( ( os_strcmp( attr, WRA_TM_ATTR_DATA ) == 0 && tm->type == WRA_TELEMETRY_METRIC ) ||
			     ( os_strcmp( attr, WRA_TM_ATTR_DESC ) == 0 && tm->type == WRA_TELEMETRY_ALARM ) ||
			     ( os_strcmp( attr, WRA_TM_ATTR_DESC ) == 0 && tm->type == WRA_TELEMETRY_EVENT ) )
			{
				result = WRA_STATUS_NOT_INITIALIZED;
				if ( tm->data.has_value != WRA_FALSE &&
					(wra_type_t)tm->data.type == WRA_TYPE_BOOLEAN )
				{
					*value = tm->data.value.boolean;
					result = WRA_STATUS_SUCCESS;
				}
			}
			else if ( os_strcmp( attr, WRA_TM_ATTR_ACTIVE ) == 0 )
			{
				if ( tm->type == WRA_TELEMETRY_ALARM || tm->type == WRA_TELEMETRY_EVENT )
				{
					*value = (bool)tm->is_active;
					result = WRA_STATUS_SUCCESS;
				}
			}
		}
	}
	return result;
}

wra_status_t wra_tm_getvalue_double( wra_tm_handle tm, const char* attr, double* value )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( tm && attr && value )
	{
		result = WRA_STATUS_FAILURE;
		if ( tm->type == WRA_TELEMETRY_ALARM || tm->type == WRA_TELEMETRY_EVENT ||
		     tm->type == WRA_TELEMETRY_METRIC )
		{
			result = WRA_STATUS_BAD_REQUEST;
			if ( ( os_strcmp( attr, WRA_TM_ATTR_DATA ) == 0 && tm->type == WRA_TELEMETRY_METRIC ) ||
			     ( os_strcmp( attr, WRA_TM_ATTR_DESC ) == 0 && tm->type == WRA_TELEMETRY_ALARM ) ||
			     ( os_strcmp( attr, WRA_TM_ATTR_DESC ) == 0 && tm->type == WRA_TELEMETRY_EVENT ) )
			{
				result = WRA_STATUS_NOT_INITIALIZED;
				if ( tm->data.has_value != WRA_FALSE &&
					(wra_type_t)tm->data.type == WRA_TYPE_FLOAT )
				{
					*value = tm->data.value.float64;
					result = WRA_STATUS_SUCCESS;
				}
			}
		}
	}
	return result;
}

wra_status_t wra_tm_getvalue_int( wra_tm_handle tm, const char *attr, int *value )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( tm && attr && value )
	{
		result = WRA_STATUS_FAILURE;
		if ( tm->type == WRA_TELEMETRY_ALARM || tm->type == WRA_TELEMETRY_EVENT ||
		     tm->type == WRA_TELEMETRY_METRIC )
		{
			result = WRA_STATUS_BAD_REQUEST;
			if ( ( os_strcmp( attr, WRA_TM_ATTR_DATA ) == 0 && tm->type == WRA_TELEMETRY_METRIC ) ||
			     ( os_strcmp( attr, WRA_TM_ATTR_DESC ) == 0 && tm->type == WRA_TELEMETRY_ALARM ) ||
			     ( os_strcmp( attr, WRA_TM_ATTR_DESC ) == 0 && tm->type == WRA_TELEMETRY_EVENT ) )
			{
				result = WRA_STATUS_NOT_INITIALIZED;
				if ( tm->data.has_value != WRA_FALSE &&
					(wra_type_t)tm->data.type == WRA_TYPE_INTEGER )
				{
					*value = (int)tm->data.value.int32;
					result = WRA_STATUS_SUCCESS;
				}
			}
			else if ( os_strcmp( attr, WRA_TM_ATTR_SEVERITY ) == 0 && tm->type == WRA_TELEMETRY_ALARM )
			{
				*value = 0; /* fake value */
				result = WRA_STATUS_SUCCESS;
			}
			else if ( os_strcmp( attr, WRA_TM_ATTR_PRIORITY ) == 0 )
			{
				*value = WRA_TM_PRIO_LOW; /* fake value */
				result = WRA_STATUS_SUCCESS;
			}
		}
	}
	return result;
}

wra_status_t wra_tm_getvalue_string( wra_tm_handle tm, const char *attr, char** value )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( tm && attr && value )
	{
		result = WRA_STATUS_FAILURE;
		if ( tm->type == WRA_TELEMETRY_ALARM || tm->type == WRA_TELEMETRY_EVENT ||
		     tm->type == WRA_TELEMETRY_METRIC )
		{
			const char* new_value = NULL;
			*value = NULL;
			result = WRA_STATUS_BAD_REQUEST;
			if ( ( os_strcmp( attr, WRA_TM_ATTR_DATA ) == 0 && tm->type == WRA_TELEMETRY_METRIC ) ||
			     ( os_strcmp( attr, WRA_TM_ATTR_DESC ) == 0 && tm->type == WRA_TELEMETRY_ALARM ) ||
			     ( os_strcmp( attr, WRA_TM_ATTR_DESC ) == 0 && tm->type == WRA_TELEMETRY_EVENT ) )
			{
				result = WRA_STATUS_NOT_INITIALIZED;
				if ( tm->data.has_value != WRA_FALSE &&
					(wra_type_t)tm->data.type == WRA_TYPE_STRING )
					new_value = tm->data.value.string;
			}
			else if ( os_strcmp( attr, WRA_TM_ATTR_DATATYPE ) == 0 )
			{
				switch ( (wra_type_t)tm->data.type )
				{
				case WRA_TYPE_BOOLEAN:
					new_value = "bool";
					break;
				case WRA_TYPE_FLOAT:
					new_value = "double";
					break;
				case WRA_TYPE_INTEGER:
					new_value = "int";
					break;
				case WRA_TYPE_RAW:
					new_value = "raw";
					break;
				case WRA_TYPE_STRING:
					new_value = "string";
					break;
				case WRA_TYPE_NULL:
					new_value = "null";
				}
			}
			else if ( os_strcmp( attr, WRA_TM_ATTR_NAME ) == 0 )
			{
				result = WRA_STATUS_FAILURE;
				if ( tm->type == WRA_TELEMETRY_METRIC )
				{
					if ( tm->object.metric )
						new_value = tm->object.metric->metric_name;
				}
				else if ( tm->type == WRA_TELEMETRY_ALARM || tm->type == WRA_TELEMETRY_EVENT )
					result = WRA_STATUS_SUCCESS;
			}

			if ( new_value )
			{
				const size_t str_len = os_strlen( new_value );
				*value = (char*)os_malloc( sizeof( char ) * str_len + 1u );
				result = WRA_STATUS_NO_MEMORY;
				if ( *value )
				{
					os_memzero( *value, str_len + 1u );
					os_strncpy( *value, new_value, str_len );
					result = WRA_STATUS_SUCCESS;
				}
			}
		}
	}
	return result;
}

wra_status_t wra_tm_post( wra_handle wra_h, wra_tm_handle tm,
	wra_service_handle UNUSED(service), wra_notification_handle_t notification
)
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( wra_h )
		IOT_LOG( wra_h->iot, IOT_LOG_WARNING,
			"The function %s is deprecated in this version of the Wind River Agent library.", IOT_FUNC );
	if ( wra_h && tm )
	{
		result = WRA_STATUS_BAD_REQUEST;
		if ( tm->data.has_value == WRA_TRUE )
		{
			/* obtain timestamp */
			wra_timestamp_t* time_stamp = NULL;
			if ( tm->time_stamp_has_value == WRA_TRUE )
				time_stamp = &tm->time_stamp;

			/* handle publishing various telemetry types */
			switch ( tm->type )
			{
			case WRA_TELEMETRY_ALARM:
			case WRA_TELEMETRY_EVENT:
				/* not supported but let's pretend that the value was posted */
				result = WRA_STATUS_SUCCESS;
				break;
			case WRA_TELEMETRY_METRIC:
				if ( tm->object.metric )
				{
					/* register metric & source with agent */
					if ( !tm->object.metric->parent )
					{
						wra_source_t* source;
						result = wra_source_find( wra_h, WRA_COMPAT_SOURCE_NAME, WRA_COMPAT_SOURCE_VERSION, &source );
						if ( result != WRA_STATUS_SUCCESS || source == NULL )
						{
							source = wra_source_allocate( WRA_COMPAT_SOURCE_NAME, WRA_COMPAT_SOURCE_VERSION );
							result = wra_source_register( wra_h, source, 0u );
						}
						if ( result == WRA_STATUS_SUCCESS && source )
							result = wra_metric_register( source, tm->object.metric, 0u );
					}

					/* publish metric data */
					if ( (wra_type_t)tm->data.type == WRA_TYPE_BOOLEAN )
						result = wra_metric_publish_boolean( tm->object.metric, tm->data.value.boolean, time_stamp, 0u );
					else if ( (wra_type_t)tm->data.type == WRA_TYPE_FLOAT )
						result = wra_metric_publish_float( tm->object.metric, (double)tm->data.value.float64, time_stamp, 0u );
					else if ( (wra_type_t)tm->data.type == WRA_TYPE_INTEGER )
						result = wra_metric_publish_integer( tm->object.metric, (uint32_t)tm->data.value.int32, time_stamp, 0u );
					else if ( (wra_type_t)tm->data.type == WRA_TYPE_STRING )
						result = wra_metric_publish_string( tm->object.metric, tm->data.value.string, time_stamp, 0u );

					/* free memory for posted data */
					if ( result == WRA_STATUS_SUCCESS )
					{
						tm->time_stamp_has_value = WRA_FALSE;
						wra_compat_free_dynamic_data( &tm->data );
					}
				}
			}

			/* update notification handle */
			if ( notification )
				*notification = result;
		}
	}
	return result;
}

wra_tm_handle wra_tm_reset( wra_tm_handle tm, const char *name )
{
	wra_tm_handle result = WRA_NULL;
	if ( tm )
	{
		switch( tm->type )
		{
		case WRA_TELEMETRY_ALARM:
		case WRA_TELEMETRY_EVENT:
			/* not supported but let's pretend that operation was successful */
			result = tm;
			break;
		case WRA_TELEMETRY_METRIC:
			if ( tm->object.metric )
			{
				if ( name )
				{
					wra_source_t* source = tm->object.metric->parent;
					wra_metric_deregister( tm->object.metric, 0u );
					os_strncpy( tm->object.metric->metric_name, name, IOT_NAME_MAX_LEN - 1u );
					if ( source )
						wra_metric_register( source, tm->object.metric, 0u );
				}
				tm->time_stamp_has_value = WRA_FALSE;
				wra_compat_free_dynamic_data( &tm->data );
				result = tm;
			}
		}
	}
	return result;
}

wra_status_t wra_tm_setaux( wra_tm_handle tm, const char* attr, wra_tm_handle aux )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( tm && attr && aux )
	{
		result = WRA_STATUS_FAILURE;
		if ( tm->type == WRA_TELEMETRY_METRIC )
			result = WRA_STATUS_BAD_REQUEST;
		else if ( tm->type == WRA_TELEMETRY_ALARM || tm->type == WRA_TELEMETRY_EVENT )
			result = WRA_STATUS_SUCCESS;
	}
	return result;
}

wra_status_t wra_tm_settimestamp( wra_tm_handle tm, const wra_timestamp* time_stamp )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( tm )
	{
		if ( time_stamp )
		{
			os_memcpy( &tm->time_stamp, time_stamp, sizeof( wra_timestamp ) );
			tm->time_stamp_has_value = WRA_TRUE;
		}
		else
		{
			os_memzero( &tm->time_stamp, sizeof( wra_timestamp ) );
			tm->time_stamp_has_value = WRA_FALSE;
		}
		result = WRA_STATUS_SUCCESS;
	}
	return result;
}

wra_status wra_tm_setvalue_bool( wra_tm_handle tm, const char* attr, bool value )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( tm && attr && value )
	{
		result = WRA_STATUS_FAILURE;
		if ( tm->type == WRA_TELEMETRY_ALARM || tm->type == WRA_TELEMETRY_EVENT ||
		     tm->type == WRA_TELEMETRY_METRIC )
		{
			result = WRA_STATUS_BAD_REQUEST;
			if ( ( os_strcmp( attr, WRA_TM_ATTR_DATA ) == 0 && tm->type == WRA_TELEMETRY_METRIC ) ||
			     ( os_strcmp( attr, WRA_TM_ATTR_DESC ) == 0 && tm->type == WRA_TELEMETRY_ALARM ) ||
			     ( os_strcmp( attr, WRA_TM_ATTR_DESC ) == 0 && tm->type == WRA_TELEMETRY_EVENT ) )
			{
				wra_compat_free_dynamic_data( &tm->data );
				tm->data.has_value = WRA_TRUE;
				tm->data.type = (iot_type_t)WRA_TYPE_BOOLEAN;
				tm->data.value.boolean = value;
				result = WRA_STATUS_SUCCESS;
			}
			else if ( os_strcmp( attr, WRA_TM_ATTR_ACTIVE ) == 0 )
			{
				if ( tm->type == WRA_TELEMETRY_ALARM || tm->type == WRA_TELEMETRY_EVENT )
				{
					tm->is_active = (wra_bool_t)value;
					result = WRA_STATUS_SUCCESS;
				}
			}
		}
	}
	return result;
}

wra_status_t wra_tm_setvalue_double( wra_tm_handle tm, const char* attr, double value )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( tm && attr )
	{
		result = WRA_STATUS_FAILURE;
		if ( tm->type == WRA_TELEMETRY_ALARM || tm->type == WRA_TELEMETRY_EVENT ||
		     tm->type == WRA_TELEMETRY_METRIC )
		{
			result = WRA_STATUS_BAD_REQUEST;
			if ( ( os_strcmp( attr, WRA_TM_ATTR_DATA ) == 0 && tm->type == WRA_TELEMETRY_METRIC ) ||
			     ( os_strcmp( attr, WRA_TM_ATTR_DESC ) == 0 && tm->type == WRA_TELEMETRY_ALARM ) ||
			     ( os_strcmp( attr, WRA_TM_ATTR_DESC ) == 0 && tm->type == WRA_TELEMETRY_EVENT ) )
			{
				wra_compat_free_dynamic_data( &tm->data );
				tm->data.has_value = WRA_TRUE;
				tm->data.type = (iot_type_t)WRA_TYPE_FLOAT;
				tm->data.value.float64 = value;
				result = WRA_STATUS_SUCCESS;
			}
		}
	}
	return result;
}

wra_status_t wra_tm_setvalue_int( wra_tm_handle tm, const char *attr, int value )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( tm && attr )
	{
		result = WRA_STATUS_FAILURE;
		if ( tm->type == WRA_TELEMETRY_ALARM || tm->type == WRA_TELEMETRY_EVENT ||
		     tm->type == WRA_TELEMETRY_METRIC )
		{
			result = WRA_STATUS_BAD_REQUEST;
			if ( ( os_strcmp( attr, WRA_TM_ATTR_DATA ) == 0 && tm->type == WRA_TELEMETRY_METRIC ) ||
			     ( os_strcmp( attr, WRA_TM_ATTR_DESC ) == 0 && tm->type == WRA_TELEMETRY_ALARM ) ||
			     ( os_strcmp( attr, WRA_TM_ATTR_DESC ) == 0 && tm->type == WRA_TELEMETRY_EVENT ) )
			{
				wra_compat_free_dynamic_data( &tm->data );
				tm->data.has_value = WRA_TRUE;
				tm->data.type = (iot_type_t)WRA_TYPE_INTEGER;
				tm->data.value.int32 = (int32_t)value;
				result = WRA_STATUS_SUCCESS;
			}
			else if ( os_strcmp( attr, WRA_TM_ATTR_SEVERITY ) == 0 && tm->type == WRA_TELEMETRY_ALARM )
				result = WRA_STATUS_SUCCESS;
			else if ( os_strcmp( attr, WRA_TM_ATTR_PRIORITY ) == 0 )
				result = WRA_STATUS_SUCCESS;
		}
	}
	return result;
}

wra_status_t wra_tm_setvalue_string( wra_tm_handle tm, const char* attr, const char* value )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( tm && attr && value )
	{
		result = WRA_STATUS_FAILURE;
		if ( tm->type == WRA_TELEMETRY_ALARM || tm->type == WRA_TELEMETRY_EVENT ||
		     tm->type == WRA_TELEMETRY_METRIC )
		{
			result = WRA_STATUS_BAD_REQUEST;
			if ( ( os_strcmp( attr, WRA_TM_ATTR_DATA ) == 0 && tm->type == WRA_TELEMETRY_METRIC ) ||
			     ( os_strcmp( attr, WRA_TM_ATTR_DESC ) == 0 && tm->type == WRA_TELEMETRY_ALARM ) ||
			     ( os_strcmp( attr, WRA_TM_ATTR_DESC ) == 0 && tm->type == WRA_TELEMETRY_EVENT ) )
			{
				char *heap_data;
				const size_t str_len = os_strlen( value );
				wra_compat_free_dynamic_data( &tm->data );
				result = WRA_STATUS_NO_MEMORY;
				tm->data.has_value = WRA_TRUE;
				tm->data.type = (iot_type_t)WRA_TYPE_STRING;
				heap_data = (char*)os_malloc( sizeof( char ) * ( str_len + 1u ) );
				if ( heap_data )
				{
					os_memzero( heap_data, str_len + 1u );
					os_strncpy( heap_data, value, str_len );
					tm->data.value.string = heap_data;
					result = WRA_STATUS_SUCCESS;
				}
			}
			else if ( os_strcmp( attr, WRA_TM_ATTR_DATATYPE ) == 0 )
			{
				wra_type_t new_type = WRA_TYPE_NULL;
				if ( os_strcmp( value, "bool" ) == 0 )
					new_type = WRA_TYPE_BOOLEAN;
				else if ( os_strcmp( value, "double" ) == 0 )
					new_type = WRA_TYPE_FLOAT;
				else if ( os_strcmp( value, "int" ) == 0 )
					new_type = WRA_TYPE_INTEGER;
				else if ( os_strcmp( value, "raw" ) == 0 )
					new_type = WRA_TYPE_RAW;
				else if ( os_strcmp( value, "string" ) == 0 )
					new_type = WRA_TYPE_STRING;

				if ( new_type != WRA_TYPE_NULL && tm->data.type != (iot_type_t)new_type )
				{
					wra_compat_free_dynamic_data( &tm->data );
					tm->data.type = (iot_type_t)new_type;
					result = WRA_STATUS_SUCCESS;
				}
			}
			else if ( os_strcmp( attr, WRA_TM_ATTR_NAME ) == 0 )
			{
				result = WRA_STATUS_FAILURE;
				if ( tm->type == WRA_TELEMETRY_METRIC )
				{
					result = WRA_STATUS_NOT_INITIALIZED;
					if ( tm->object.metric )
					{
						wra_source_t* source = tm->object.metric->parent;
						result = WRA_STATUS_SUCCESS;
						if ( source )
							result = wra_metric_deregister( tm->object.metric, 0u );
						if ( result == WRA_STATUS_SUCCESS )
							os_strncpy( tm->object.metric->metric_name, value, IOT_NAME_MAX_LEN - 1u );
						if ( source )
							result = wra_metric_register( source, tm->object.metric, 0u );
					}
				}
				else if ( tm->type == WRA_TELEMETRY_ALARM || tm->type == WRA_TELEMETRY_EVENT )
					result = WRA_STATUS_SUCCESS;
			}
		}
	}
	return result;
}

wra_status_t wra_tm_subscribe( wra_handle wra_h, wra_tm_handle tm, const char* name, struct timespec *UNUSED(tmo) )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( wra_h && tm && name )
	{
		IOT_LOG( wra_h->iot, IOT_LOG_WARNING,
			"The function %s is deprecated in this version of the Wind River Agent library.", IOT_FUNC );
		/* return success for backwards compatibility */
		result = WRA_STATUS_SUCCESS;
	}
	return result;
}

