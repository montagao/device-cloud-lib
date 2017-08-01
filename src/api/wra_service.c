/**
 * @file
 * @brief Source file for services within the Wind River Internet of Things (IoT) library
 *
  @copyright Copyright (C) 2015-2016 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software may
 * be licensed only pursuant to the terms of an applicable Wind River license
 * agreement.  No license to Wind River intellectual properly rights is granted
 * herein.  All rights not licensed by Wind River are reserved by Wind River.
 */

#include "public/wra_service.h"

#include "public/wra_command.h"
#include "shared/wra_internal.h"

/**
 * @brief Implementation of deregistering a service from the cloud
 *
 * @param[in]      service                       service to deregister
 * @param[in]      abs_time_out                  absolute maximum time to wait for
 *                                               deregistration (optional)
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_NO_MEMORY                  not enough system memory
 * @retval WRA_STATUS_NOT_INITIALIZED            library not initialized with the cloud
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for deregistration
 */
static wra_status_t wra_service_deregister_implementation( wra_service_t* service,
	const wra_timestamp_t* abs_time_out );

wra_service_t* wra_service_allocate( const char* name, const char* version )
{
	struct wra_service* result = NULL;
	if ( name )
	{
		result = (struct wra_service *)os_malloc( sizeof( struct wra_service ) );
		if ( result )
		{
			os_memzero( result, sizeof( struct wra_service ) );
			os_strncpy( result->service_name, name, IOT_NAME_MAX_LEN - 1u );
			if ( version )
				os_strncpy( result->service_version, version, VERSION_MAX_LEN - 1u );
		}
	}
	return result;
}

wra_status_t wra_service_deregister( wra_service_t* service, wra_millisecond_t max_time_out )
{
	wra_timestamp_t  end_time;
	wra_timestamp_t* end_time_ptr;

	/* determine the end time */
	end_time_ptr = wra_common_time_relative_to_absolute( &end_time, max_time_out );

	return wra_service_deregister_implementation( service, end_time_ptr );
}

wra_status_t wra_service_deregister_implementation( wra_service_t* service,
	const wra_timestamp_t* abs_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( service )
	{
		struct wra* const lib_handle = service->lib_handle;

		result = WRA_STATUS_NOT_INITIALIZED;
		/* ensure already registered commands are deregistered from the agent */
		if ( lib_handle )
		{
			struct wra_command* cur_command = service->command_first;
			result = WRA_STATUS_SUCCESS;
			while ( cur_command && result != WRA_STATUS_TIMED_OUT )
			{
				struct wra_command* const next_command = cur_command->command_next;
				result = wra_command_free_implementation( cur_command, abs_time_out );
				cur_command = next_command;
			}

			if ( lib_handle->service_first == service )
				lib_handle->service_first = service->service_next;
			if ( lib_handle->service_last == service )
				lib_handle->service_last = service->service_prev;
			service->lib_handle = NULL;
		}

		if ( result != WRA_STATUS_TIMED_OUT )
		{
			if ( service->service_prev )
				service->service_prev->service_next = service->service_next;
			if ( service->service_next )
				service->service_next->service_prev = service->service_prev;
			service->service_prev = NULL;
			service->service_next = NULL;
		}
	}
	return result;
}

wra_status_t wra_service_find( const wra_t* lib_handle, const char* name, const char* version, wra_service_t** match )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( lib_handle && name )
	{
		wra_bool_t exact_match = WRA_FALSE;
		struct wra_service* match_found = NULL;
		struct wra_service* cur_service = lib_handle->service_first;
		result = WRA_STATUS_NOT_FOUND;
		while ( cur_service && !exact_match )
		{
			if ( os_strncmp( cur_service->service_name, name, IOT_NAME_MAX_LEN - 1u ) == 0 )
			{
				int compare_result = 0;
				if ( version != NULL )
					compare_result = os_strncmp(
						cur_service->service_version, version, VERSION_MAX_LEN - 1u );
				if ( compare_result == 0 )
				{
					match_found = cur_service;
					result = WRA_STATUS_SUCCESS;
					if ( version )
						exact_match = WRA_TRUE;
				}
			}
			cur_service = cur_service->service_next;
		}

		if ( match )
			*match = match_found;
	}
	return result;
}

wra_status_t wra_service_free( wra_service_t* service, wra_millisecond_t max_time_out )
{
	wra_timestamp_t  end_time;
	wra_timestamp_t* end_time_ptr;

	/* determine the end time */
	end_time_ptr = wra_common_time_relative_to_absolute( &end_time, max_time_out );

	return wra_service_free_implementation( service, end_time_ptr );
}

wra_status_t wra_service_free_implementation( wra_service_t* service, const wra_timestamp_t* abs_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( service )
	{
		result = WRA_STATUS_SUCCESS;
		if ( service->lib_handle )
			result = wra_service_deregister_implementation( service, abs_time_out );
		if ( result == WRA_STATUS_SUCCESS )
		{
			/* clear memory to help in cases of use after free */
			os_memzero( service, sizeof( struct wra_service ) );
			os_free_null( (void **)&service );
		}
	}
	return result;
}

wra_status_t wra_service_register( wra_t* lib_handle, wra_service_t* service,
	const wra_millisecond_t max_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( lib_handle && service )
	{
		wra_timestamp_t  end_time;
		wra_timestamp_t* end_time_ptr;

		/* determine the end time */
		end_time_ptr = wra_common_time_relative_to_absolute( &end_time, max_time_out );

		result = WRA_STATUS_SUCCESS;
		if ( service->lib_handle != lib_handle )
		{
			struct wra_command* cur_command;
			service->lib_handle = lib_handle;

			/* ensure already registered commands are registered with the agent */
			cur_command = service->command_first;
			service->command_first = NULL;
			service->command_last = NULL;
			while ( cur_command && result == WRA_STATUS_SUCCESS )
			{
				cur_command->parent = NULL;
				result = wra_command_register_implementation( service, cur_command, end_time_ptr );
				cur_command = cur_command->command_next;
			}

			if ( result == WRA_STATUS_SUCCESS )
			{
				/* add service to end of list */
				if ( lib_handle->service_last )
					lib_handle->service_last->service_next = service;
				if ( !lib_handle->service_first )
					lib_handle->service_first = service;
				service->service_prev = lib_handle->service_last;
				lib_handle->service_last = service;
			}
			else
			{
				/* clean up commands registered prior to failure */
				cur_command = service->command_first;
				while ( cur_command )
				{
					wra_command_deregister_implementation( cur_command, end_time_ptr );
					cur_command = cur_command->command_next;
				}
				service->command_first = NULL;
				service->command_last = NULL;
				service->lib_handle = NULL;
			}
		}
	}
	return result;
}

