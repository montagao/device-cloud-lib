/**
 * @file
 * @brief Source file for commands within the Wind River Internet of Things (IoT) library
 *
 * @copyright Copyright (C) 2015-2016 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software may
 * be licensed only pursuant to the terms of an applicable Wind River license
 * agreement.  No license to Wind River intellectual properly rights is granted
 * herein.  All rights not licensed by Wind River are reserved by Wind River.
 */

#include "public/wra_command.h"

#include "os.h"                        /* for os_malloc */
#include "public/wra_common.h"
#include "public/wra_service.h"        /* for wra_service_find */
#include "shared/wra_internal.h"       /* for struct wra_command */


/** @brief Default Sleep in ms for wra_command_wait API */
#ifndef IOT_NO_THREAD_SUPPORT
#define WRA_COMMAND_WAIT_DEFAULT 20u
#else
#define WRA_COMMAND_WAIT_DEFAULT 1000u
#endif /* ifndef IOT_NO_THREAD_SUPPORT */

/**
 * @brief Internal function called to convert incoming command to an action
 *
 * @param[in,out]  request                       incoming request to be handled
 * @param[in]      user_data                     user specific data
 */
static IOT_SECTION iot_status_t wra_command_compatible_callback(
	iot_action_request_t *request, void *user_data );
/**
 * @brief Converts an older "wra type" to the equivalent new "iot type"
 *
 * @param[in]      type                          type to convert
 *
 * @retval the equivalent "iot type"
 */
static IOT_SECTION iot_type_t wra_type_to_iot_type( wra_type_t type );


/* functions */
wra_command_t* wra_command_allocate( const char* name )
{
	struct wra_command* result = NULL;
	if ( name )
	{
		result = os_malloc( sizeof( struct wra_command ) );
		if( result )
		{
			os_memzero( result, sizeof( struct wra_command ) );
			os_strncpy( result->command_name, name, IOT_NAME_MAX_LEN - 1u );
		}
	}
	return result;
}

iot_status_t wra_command_compatible_callback(
	iot_action_request_t *request, void *user_data )
{
	iot_status_t result = IOT_STATUS_PARSE_ERROR;
	if ( request && user_data )
	{
		struct wra_command_callback_data *callback_data =
			(struct wra_command_callback_data*)user_data;
		result = (iot_status_t)
			callback_data->callback( request,
				callback_data->user_data );
	}
	return result;
}

wra_status_t wra_command_deregister( wra_command_t* command, wra_millisecond_t max_time_out )
{
	wra_timestamp_t  end_time;
	wra_timestamp_t* end_time_ptr;

	/* determine the end time */
	end_time_ptr = wra_common_time_relative_to_absolute( &end_time, max_time_out );

	return wra_command_deregister_implementation( command, end_time_ptr );
}

wra_status_t wra_command_deregister_implementation( wra_command_t* command,
	const wra_timestamp_t* abs_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( command )
	{
		const struct wra_service* service = command->parent;
		result = WRA_STATUS_NOT_INITIALIZED;
		if ( service )
		{
			const wra_t* const lib_handle = service->lib_handle;
			result = wra_command_register_implementation( NULL, command, abs_time_out );
			if ( lib_handle )
				IOT_LOG( lib_handle->iot, IOT_LOG_DEBUG,
				"Deregistration of command \"%s\" returned: %s",
				command->command_name, wra_error( result ) );
		}
	}
	return result;
}

wra_status_t wra_command_free( wra_command_t* command, wra_millisecond_t max_time_out )
{
	wra_timestamp_t  end_time;
	wra_timestamp_t* end_time_ptr;

	/* determine the end time */
	end_time_ptr = wra_common_time_relative_to_absolute( &end_time, max_time_out );

	return wra_command_free_implementation( command, end_time_ptr );
}

wra_status_t wra_command_free_implementation( wra_command_t* command,
	const wra_timestamp_t* abs_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( command )
	{
		result = WRA_STATUS_SUCCESS;
		if ( command->parent )
			result = wra_command_deregister_implementation( command, abs_time_out );
		if ( result == WRA_STATUS_SUCCESS )
		{
			/* clear memory to help in cases of use after free */
			os_memzero( command, sizeof( struct wra_command ) );
			os_free_null( (void**)&command );
		}
	}
	return result;
}

wra_status_t wra_command_parameter_add( wra_command_t* command,
	const char* name, wra_type_t type, wra_millisecond_t max_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( command && name )
	{
		/* add commands to support non-registered actions */
		result = WRA_STATUS_BAD_REQUEST;
		if ( type != WRA_TYPE_NULL )
		{
			result = WRA_STATUS_FULL;
			if ( command->parameter_count < IOT_PARAMETER_MAX )
			{
				struct wra_parameter* parameter =
					&command->parameter[ command->parameter_count ];
				os_strncpy( parameter->name, name,
					IOT_NAME_MAX_LEN - 1u );
				parameter->type = IOT_PARAMETER_IN_REQUIRED;
				parameter->data.type =
					wra_type_to_iot_type( type );
				++command->parameter_count;
				result = WRA_STATUS_SUCCESS;
			}
		}

		/* if command is already registered, we must update it */
		if ( result == WRA_STATUS_SUCCESS && command->action )
		{
			result = (wra_status_t)iot_action_parameter_add(
				command->action, name,
				IOT_PARAMETER_IN_REQUIRED,
				wra_type_to_iot_type( type ), max_time_out );

			if ( result != WRA_STATUS_SUCCESS )
				--command->parameter_count;
		}
	}
	return result;
}

wra_status_t wra_command_parameter_boolean_get( const wra_command_request_t* request, const char* name, wra_bool_t* value )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( request && value )
	{
		iot_bool_t new_value = IOT_FALSE;
		result = (wra_status_t)iot_action_parameter_get(
			request, name, IOT_FALSE, IOT_TYPE_BOOL, &new_value );
		*value = new_value;
	}
	return result;
}

wra_status_t wra_command_parameter_boolean_set( wra_command_request_t* request, const char* name, wra_bool_t value )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( request && name )
	{
		size_t i;
		result = WRA_STATUS_NOT_FOUND;
		for( i = 0u; i < request->parameter_count && result == WRA_STATUS_NOT_FOUND; ++i )
		{
			if ( os_strncmp( request->parameter[i].name, name, IOT_NAME_MAX_LEN - 1u ) == 0 )
			{
				result = WRA_STATUS_BAD_REQUEST;
				if ( request->parameter[i].data.type == IOT_TYPE_BOOL )
				{
					request->parameter[i].data.value.boolean = value;
					request->parameter[i].data.has_value = WRA_TRUE;
					result = WRA_STATUS_SUCCESS;
				}
			}
		}
	}
	return result;
}

wra_status_t wra_command_parameter_integer_get( const wra_command_request_t* request, const char* name, int32_t* value )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( request && value )
	{
		iot_int32_t new_value = 0;
		result = (wra_status_t)iot_action_parameter_get(
			request, name, IOT_FALSE, IOT_TYPE_INT32, &new_value );
		*value = new_value;
	}
	return result;
}

wra_status_t wra_command_parameter_integer_set( wra_command_request_t* request, const char* name, int32_t value )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( request && name )
	{
		result = (wra_status_t)iot_action_parameter_set(
			request, name, IOT_TYPE_INT32, (iot_int32_t)value );
	}
	return result;
}

wra_status_t wra_command_parameter_float_get( const wra_command_request_t* request, const char* name, double* value )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( request && value )
	{
		iot_float64_t new_value = 0.0;
		result = (wra_status_t)iot_action_parameter_get(
			request, name, IOT_FALSE, IOT_TYPE_FLOAT64, &new_value );
		*value = new_value;
	}
	return result;
}

wra_status_t wra_command_parameter_float_set( wra_command_request_t* request, const char* name, double value )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( request && name )
	{
		result = (wra_status_t)iot_action_parameter_set(
			request, name, IOT_TYPE_FLOAT64, (iot_float64_t)value );
	}
	return result;
}

wra_status_t wra_command_parameter_raw_get( const wra_command_request_t* request, const char* name, void* dest, size_t len )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( request && name && dest )
	{
		const void *internal_dest = NULL;
		size_t internal_len = 0u;
		result = (wra_status_t)iot_action_parameter_get_raw(
			request, name, IOT_TRUE, &internal_len, &internal_dest );
		if ( result == WRA_STATUS_SUCCESS )
		{
			os_memzero( dest, len );
			if ( internal_len < len && internal_dest )
				os_memcpy( dest, internal_dest, internal_len );
			else
				result = WRA_STATUS_NO_MEMORY;
		}
	}
	return result;
}

wra_status_t wra_command_parameter_raw_set( wra_command_request_t* request, const char* name, const void* value, size_t len )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( request && name )
	{
		result = (wra_status_t)iot_action_parameter_set_raw(
			request, name, len, value );
	}
	return result;
}

wra_status_t wra_command_parameter_string_get( const wra_command_request_t* request, const char* name, char* dest, size_t len )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( request && dest )
	{
		const char *str_ptr = NULL;
		result = (wra_status_t)iot_action_parameter_get(
			request, name, IOT_FALSE, IOT_TYPE_STRING, &str_ptr );
		if ( result == WRA_STATUS_SUCCESS && str_ptr )
			os_strncpy( dest, str_ptr, len );
		else
			result = WRA_STATUS_NO_MEMORY;
	}
	return result;
}

wra_status_t wra_command_parameter_string_set( wra_command_request_t* request, const char* name, const char* value )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( request )
	{
		result = (wra_status_t)iot_action_parameter_set(
			request, name, IOT_TYPE_STRING, value );
	}
	return result;
}

wra_status_t wra_command_register( wra_service_t* service, wra_command_t* command, wra_millisecond_t max_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( service && command )
	{
		const wra_t* const lib_handle = service->lib_handle;
		wra_timestamp_t  end_time;
		wra_timestamp_t* end_time_ptr;

		/* determine the end time */
		end_time_ptr = wra_common_time_relative_to_absolute( &end_time, max_time_out );

		result = wra_command_register_implementation( service, command, end_time_ptr );
		if ( lib_handle )
			IOT_LOG( lib_handle->iot, IOT_LOG_DEBUG,
				"Registration of command \"%s\" returned: %s",
				command->command_name, wra_error( result ) );
	}
	return result;
}

wra_status_t wra_command_register_implementation( struct wra_service* service, struct wra_command* command,
	const wra_timestamp_t* UNUSED(abs_time_out) )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( command )
	{
		size_t i;
		result = WRA_STATUS_SUCCESS;

		/* loop twice: once to deregister, once to register */
		for ( i = 0u; i < 2u && result == WRA_STATUS_SUCCESS; ++i )
		{
			struct wra_service* parent = NULL;

			/* deregister */
			if ( i == 0u && command->parent )
			{
				parent = command->parent;
				/* backwards support unsubscription */
				os_free_null( (void**)&command->compat_data );

				/* remove from list */
				if ( command->command_prev )
					command->command_prev->command_next = command->command_next;
				if ( command->command_next )
					command->command_next->command_prev = command->command_prev;
				if ( parent->command_first == command )
					parent->command_first = command->command_next;
				if ( parent->command_last == command )
					parent->command_last = command->command_prev;
				command->command_next = NULL;
				command->command_prev = NULL;
				command->parent = NULL;

				if ( command->action )
					result = (wra_status_t)iot_action_deregister(
						command->action, NULL, 0u );
			}
			else if ( i == 1u && service ) /* register */
			{
				iot_t *iot_lib = NULL;

				if ( service->lib_handle )
					iot_lib = service->lib_handle->iot;

				parent = service;
				if ( command->parent == NULL )
				{
					/* add to list */
					if ( !service->command_first )
						service->command_first = command;
					if( service->command_last )
					{
						service->command_last->command_next = command;
						command->command_prev = service->command_last;
					}
					service->command_last = command;
					command->parent = service;
				}

				if ( iot_lib )
				{
					char name[ IOT_NAME_MAX_LEN + 1u ];
					size_t name_len = 0u;
					size_t j;

					name[0u] = '\0';
					if ( *service->service_name != '\0' )
					{
						os_strncpy( name,
							service->service_name,
							IOT_NAME_MAX_LEN );
						name_len =
							os_strlen( name );
						if ( *service->service_version != '\0' )
						{
							os_snprintf( &name[name_len],
								IOT_NAME_MAX_LEN - name_len,
								"%s%s", IOT_SPLIT_VERSION,
								service->service_version );
							name_len +=
								os_strlen( &name[name_len] );
						}
						os_strncpy(
							&name[name_len],
							IOT_SPLIT_PARENT,
							IOT_NAME_MAX_LEN - name_len );
						name_len +=
							os_strlen( &name[name_len] );
					}

					os_strncpy( &name[name_len],
						command->command_name,
						IOT_NAME_MAX_LEN - name_len );

					command->action = iot_action_allocate(
						iot_lib, name );

					/* for backwards compatibility disable
					 * the time limit */
					if ( command->action )
					{
						command->action->flags |=
							IOT_ACTION_NO_TIME_LIMIT;
					}

					for ( j = 0u;
						j < command->parameter_count &&
						j < IOT_PARAMETER_MAX; ++j )
					{
						iot_action_parameter_add(
							command->action,
							command->parameter[j].name,
							command->parameter[j].type,
							command->parameter[j].data.type,
							0u );
					}
					if ( command->callback_data.callback )
						result = (wra_status_t)iot_action_register_callback(
							command->action,
							&wra_command_compatible_callback,
							&command->callback_data,
							NULL, 0u );
					else if ( *command->script != '\0' )
						result = (wra_status_t)iot_action_register_command(
							command->action,
							command->script,
							NULL, 0u );
				}
			}
		}
	}
	return result;
}

wra_status_t wra_command_register_callback( wra_command_t* command, wra_command_callback_t* func, void* user_data )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( command )
	{
		command->script[0] = '\0';
		command->callback_data.callback = func;
		command->callback_data.user_data = user_data;
		result = WRA_STATUS_SUCCESS;
	}
	return result;
}

wra_status_t wra_command_register_script( wra_command_t* command, const char* script )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( command )
	{
		command->script[0] = '\0';
		if ( script )
		{
			os_strncpy( command->script, script, PATH_MAX - 1u );
			command->script[PATH_MAX - 1u] = '\0';
		}
		command->callback_data.callback = NULL;
		result = WRA_STATUS_SUCCESS;
	}
	return result;
}

wra_status_t wra_command_wait( wra_t* lib_handle, wra_millisecond_t max_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( lib_handle )
	{
#ifndef IOT_NO_THREAD_SUPPORT
		wra_millisecond_t time_out = WRA_COMMAND_WAIT_DEFAULT;

		if ( max_time_out == 0u || max_time_out > time_out )
			max_time_out = time_out;
		/* sleep in case application only running wra_command_wait
		 * in a while ( 1 ) loop
		 */
		os_time_sleep( max_time_out, IOT_TRUE );
		result = WRA_STATUS_SUCCESS;
#else
		if ( max_time_out == 0u )
			max_time_out = WRA_COMMAND_WAIT_DEFAULT;
		result = (wra_status_t)iot_loop_iteration( lib_handle->iot,
			max_time_out );
#endif /* ifndef IOT_NO_THREAD_SUPPORT */
	}
	return result;
}

wra_status_t wra_command_response_boolean_set( wra_command_request_t* request, const char* id, wra_bool_t data )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( request )
	{
		result = (wra_status_t)iot_action_parameter_set(
			request, id, IOT_TYPE_BOOL, (iot_bool_t)data );
	}
	return result;
}

wra_status_t wra_command_response_integer_set( wra_command_request_t* request, const char* id, int32_t data )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( request )
	{
		result = (wra_status_t)iot_action_parameter_set(
			request, id, IOT_TYPE_INT32, (iot_int32_t)data );
	}
	return result;
}

wra_status_t wra_command_response_float_set( wra_command_request_t* request, const char* id, double data )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( request )
	{
		result = (wra_status_t)iot_action_parameter_set(
			request, id, IOT_TYPE_FLOAT64, (iot_float64_t)data );
	}
	return result;
}

wra_status_t wra_command_response_string_set( wra_command_request_t* request, const char* id, const char* data )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( request )
	{
		result = (wra_status_t)iot_action_parameter_set(
			request, id, IOT_TYPE_STRING, data );
	}
	return result;
}

wra_status_t wra_command_response_raw_set( wra_command_request_t* request, const char* id, const void* data, size_t length )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( request )
	{
		result = (wra_status_t)iot_action_parameter_set_raw(
			request, id, length, data );
	}
	return result;
}

iot_type_t wra_type_to_iot_type( wra_type_t type )
{
	switch ( type )
	{
	case WRA_TYPE_BOOLEAN:
		return IOT_TYPE_BOOL;
	case WRA_TYPE_FLOAT:
		return IOT_TYPE_FLOAT64;
	case WRA_TYPE_INTEGER:
		return IOT_TYPE_INT32;
	case WRA_TYPE_RAW:
		return IOT_TYPE_RAW;
	case WRA_TYPE_STRING:
		return IOT_TYPE_STRING;
	case WRA_TYPE_NULL:
	default:
		return IOT_TYPE_NULL;
	}
}

