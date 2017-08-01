/**
 * @file
 * @brief Source file for sources within the Wind River Internet of Things (IoT) library
 *
 * @copyright Copyright (C) 2015-2016 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software may
 * be licensed only pursuant to the terms of an applicable Wind River license
 * agreement.  No license to Wind River intellectual properly rights is granted
 * herein.  All rights not licensed by Wind River are reserved by Wind River.
 */

#include "public/wra_source.h"

#include "public/wra_metric.h"
#include "shared/wra_internal.h"

/**
 * @brief Implementation of deregistering a source from the cloud
 *
 * @param[in]      source                        source to deregister
 * @param[in]      abs_time_out                  absolute maximum time to wait for
 *                                               deregistration (optional)
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_NO_MEMORY                  not enough system memory
 * @retval WRA_STATUS_NOT_INITIALIZED            library not initialized with the cloud
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for deregistration
 */
static wra_status_t wra_source_deregister_implementation( wra_source_t* source,
	const wra_timestamp_t* abs_time_out );

wra_source_t* wra_source_allocate( const char* name, const char* version )
{
	struct wra_source* result = NULL;
	if ( name )
	{
		result = (struct wra_source*)os_malloc( sizeof( struct wra_source ) );
		if ( result )
		{
			os_memzero( result, sizeof( struct wra_source ) );
			os_strncpy( result->source_name, name, IOT_NAME_MAX_LEN - 1u );
			if ( version )
				os_strncpy( result->source_version, version, VERSION_MAX_LEN - 1u );
		}
	}
	return result;
}

wra_status_t wra_source_deregister( wra_source_t* source, wra_millisecond_t max_time_out )
{
	wra_timestamp_t  end_time;
	wra_timestamp_t* end_time_ptr;

	/* determine the end time */
	end_time_ptr = wra_common_time_relative_to_absolute( &end_time, max_time_out );
	return wra_source_deregister_implementation( source, end_time_ptr );
}

wra_status_t wra_source_deregister_implementation( wra_source_t* source, const wra_timestamp_t* abs_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( source )
	{
		struct wra* const lib_handle = source->lib_handle;

		/* ensure already registered metric are registered with the agent */
		result = WRA_STATUS_NOT_INITIALIZED;
		if ( lib_handle )
		{
			struct wra_metric* cur_metric = source->metric_first;
			result = WRA_STATUS_SUCCESS;
			while ( cur_metric && result != WRA_STATUS_TIMED_OUT )
			{
				struct wra_metric* const next_metric = cur_metric->metric_next;
				result = wra_metric_free_implementation( cur_metric, abs_time_out );
				cur_metric = next_metric;
			}

			if ( lib_handle->source_first == source )
				lib_handle->source_first = source->source_next;
			if ( lib_handle->source_last == source )
				lib_handle->source_last = source->source_prev;
			source->lib_handle = NULL;
		}

		if ( result != WRA_STATUS_TIMED_OUT )
		{
			if ( source->source_prev )
				source->source_prev->source_next = source->source_next;
			if ( source->source_next )
				source->source_next->source_prev = source->source_prev;
			source->source_prev = NULL;
			source->source_next = NULL;
		}
	}
	return result;
}

wra_status_t wra_source_find( const wra_t* lib_handle, const char* name, const char* version, wra_source_t** match )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( lib_handle && name )
	{
		wra_bool_t exact_match = WRA_FALSE;
		struct wra_source* match_found = NULL;
		struct wra_source* cur_source = lib_handle->source_first;
		result = WRA_STATUS_NOT_FOUND;
		while ( cur_source && !exact_match )
		{
			if ( os_strncmp( cur_source->source_name, name, IOT_NAME_MAX_LEN - 1u ) == 0 )
			{
				int compare_result = 0;
				if ( version != NULL )
					compare_result = os_strncmp(
						cur_source->source_version, version, VERSION_MAX_LEN - 1u );

				if ( compare_result == 0 )
				{
					match_found = cur_source;
					result = WRA_STATUS_SUCCESS;
					if ( version )
						exact_match = WRA_TRUE;
				}
			}
			cur_source = cur_source->source_next;
		}

		if ( match )
			*match = match_found;
	}
	return result;
}

wra_status_t wra_source_free( wra_source_t* source, wra_millisecond_t max_time_out  )
{
	wra_timestamp_t  end_time;
	wra_timestamp_t* end_time_ptr;

	/* determine the end time */
	end_time_ptr = wra_common_time_relative_to_absolute( &end_time, max_time_out );

	return wra_source_free_implementation( source, end_time_ptr );
}

wra_status_t wra_source_free_implementation( wra_source_t* source, const wra_timestamp_t* abs_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( source )
	{
		result = WRA_STATUS_SUCCESS;
		if ( source->lib_handle )
			result = wra_source_deregister_implementation( source, abs_time_out );
		if ( result == WRA_STATUS_SUCCESS )
		{
			/* clear memory to help in cases of use after free */
			os_memzero( source, sizeof( struct wra_source ) );
			os_free_null( (void **)&source );
		}
	}
	return result;
}

wra_status_t wra_source_register( wra_t* lib_handle, wra_source_t* source, wra_millisecond_t max_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( lib_handle && source )
	{
		wra_timestamp_t  end_time;
		wra_timestamp_t* end_time_ptr;

		/* determine the end time */
		end_time_ptr = wra_common_time_relative_to_absolute( &end_time, max_time_out );

		result = WRA_STATUS_EXISTS;
		if ( source->lib_handle != lib_handle )
		{
			struct wra_metric* cur_metric;
			if ( lib_handle->source_last )
				lib_handle->source_last->source_next = source;
			if ( !lib_handle->source_first )
				lib_handle->source_first = source;
			source->source_prev = lib_handle->source_last;
			lib_handle->source_last = source;
			source->lib_handle = lib_handle;

			/* ensure already registered metric are registered with the agent */
			cur_metric = source->metric_first;

			/* remove metric from list */
			source->metric_first = NULL;
			source->metric_last = NULL;
			result = WRA_STATUS_SUCCESS;
			while ( cur_metric && result == WRA_STATUS_SUCCESS )
			{
				cur_metric->parent = NULL;
				result = wra_metric_register_implementation( source, cur_metric, end_time_ptr );
				cur_metric = cur_metric->metric_next;
			}
		}
	}
	return result;
}

