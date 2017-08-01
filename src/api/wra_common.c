/**
 * @file
 * @brief Source file for common functionality the Wind River Internet of Things (IoT) library
 *
 * @copyright Copyright (C) 2014-2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software may
 * be licensed only pursuant to the terms of an applicable Wind River license
 * agreement.  No license to Wind River intellectual property rights is granted
 * herein.  All rights not licensed by Wind River are reserved by Wind River.
 */

#include "public/wra_common.h"

#include "public/wra_service.h"
#include "public/wra_source.h"
#include "shared/iot_types.h"
#include "shared/wra_internal.h"        /* for struct wra */

/**
 * @brief Function for backwards logging capability
 *
 * @param[in]      log_level           log level
 * @param[in]      source              details about where log message was
 *                                     generated from
 * @param[in]      log_msg             log message
 * @param[in]      user_data           user specified data
 */
static IOT_SECTION void wra_log_callback_compat( iot_log_level_t log_level,
	iot_log_source_t *source, const char *log_msg, void *user_data );


wra_status_t wra_connect( wra_t* lib_handle, wra_millisecond_t max_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( lib_handle )
	{
		result = (wra_status_t)iot_connect( lib_handle->iot, max_time_out );
		if ( result == WRA_STATUS_SUCCESS )
			lib_handle->agent_state = WRA_STATE_NORMAL;
	}
	return result;
}

wra_status_t wra_disconnect( wra_t* lib_handle, wra_millisecond_t max_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( lib_handle )
		result = (wra_status_t)iot_disconnect( lib_handle->iot, max_time_out );
	return result;
}

const char* wra_error( wra_status_t code )
{
	return iot_error( (iot_status_t)code );
}

wra_t* wra_initialize( const char* token )
{
	struct wra *plib_handle;
	char name[IOT_NAME_MAX_LEN];

	plib_handle = os_malloc( sizeof( struct wra ) );

#ifndef IOT_NO_THREAD_SUPPORT
	os_thread_condition_create( &plib_handle->notification_cond );
	os_thread_mutex_create( &plib_handle->notification_mutex );
#endif
	os_memzero( plib_handle, sizeof( struct wra ) );

	/* initially set the client id to the process id */
	os_snprintf( plib_handle->id, IOT_ID_MAX_LEN - 1u, "%u",
		(unsigned int)os_system_pid() );
	plib_handle->id[IOT_ID_MAX_LEN - 1u] = '\0';
	if ( token )
		os_strncpy( name, token, IOT_NAME_MAX_LEN );
	else
		os_strncpy( name, plib_handle->id, IOT_ID_MAX_LEN );
	name[IOT_NAME_MAX_LEN - 1u] = '\0';

	plib_handle->iot = iot_initialize( name, NULL, 0 );

	return plib_handle;
}

wra_status_t wra_log_callback( wra_t* lib_handle, wra_log_callback_t* log_callback, void* user_data )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( lib_handle )
	{
		lib_handle->logger = log_callback;
		lib_handle->logger_user_data = user_data;
		if ( iot_log_callback_set( lib_handle->iot,
			wra_log_callback_compat, lib_handle ) == IOT_STATUS_SUCCESS )
			result = WRA_STATUS_SUCCESS;
	}
	return result;
}

void wra_log_callback_compat( iot_log_level_t log_level,
	iot_log_source_t *UNUSED(source), const char *log_msg, void *user_data )
{
	wra_t *lib = (wra_t *)(user_data);
	if ( lib && lib->logger )
		(*lib->logger)( (wra_log_level_t)log_level, log_msg,
			lib->logger_user_data );
}

wra_status_t wra_state_callback( wra_t* lib_handle, wra_state_callback_t* state_callback, void* user_data )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( lib_handle )
	{
		lib_handle->state_callback = state_callback;
		lib_handle->state_user_data = user_data;
		result = WRA_STATUS_SUCCESS;
	}
	return result;
}

wra_status_t wra_state_get( wra_t* lib_handle, wra_state_t* state, wra_millisecond_t UNUSED(max_time_out) )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( lib_handle && state )
	{
		/** @todo create a message to query the agent for the status */
		result = WRA_STATUS_NOT_INITIALIZED;
		if ( lib_handle->id[0] != '\0' )
		{
			*state = lib_handle->agent_state;
			result = WRA_STATUS_SUCCESS;
		}
	}
	return result;
}

wra_status_t wra_terminate( wra_t* lib_handle, wra_millisecond_t max_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( lib_handle )
	{
		wra_disconnect( lib_handle, max_time_out );
		result = (wra_status_t)iot_terminate( lib_handle->iot,
			max_time_out );

#ifndef IOT_NO_THREAD_SUPPORT
		os_thread_mutex_destroy( &lib_handle->notification_mutex );
		os_thread_condition_destroy( &lib_handle->notification_cond );
#endif /* ifndef IOT_NO_THREAD_SUPPORT */
		os_free_null( (void **) &lib_handle );
	}
	return result;
}

wra_timestamp_t* wra_common_time_relative_to_absolute( wra_timestamp_t* absolute_time,
	wra_millisecond_t relative_time )
{
	wra_timestamp_t* result = NULL;
	if ( absolute_time )
	{
		iot_timestamp_t time_stamp;
		os_memzero( absolute_time, sizeof( wra_timestamp_t ) );
		os_time( &time_stamp, NULL );
		time_stamp += relative_time;
		absolute_time->tv_sec = time_stamp / IOT_MILLISECONDS_IN_SECOND;
		absolute_time->tv_nsec += time_stamp % IOT_MILLISECONDS_IN_SECOND;
		result = absolute_time;
	}
	return result;
}

