/**
 * @file
 * @brief Source file for metrics within the Wind River Internet of Things (IoT) library
 *
 * @copyright Copyright (C) 2015-2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software may
 * be licensed only pursuant to the terms of an applicable Wind River license
 * agreement.  No license to Wind River intellectual properly rights is granted
 * herein.  All rights not licensed by Wind River are reserved by Wind River.
 */

#include "public/wra_metric.h"

#include "public/wra_common.h"
#include "shared/wra_internal.h"

/** @brief Flag indicating max samples in udmp protocol */
#define IOT_FLAG_UDMP_SAMPLES_MAX      "udmp:samples_max"
/** @brief Flag indicating min samples in udmp protocol */
#define IOT_FLAG_UDMP_SAMPLES_MIN      "udmp:samples_min"
/** @brief Flag indicating the units for a sample in udmp protocol */
#define IOT_FLAG_UDMP_SAMPLES_UNITS    "udmp:units"

/**
 * @brief Implementation of deregistering a metric from the cloud
 *
 * @param[in]      metric                        metric to deregister
 * @param[in]      abs_time_out                  absolute maximum time to wait for
 *                                               deregistration (optional)
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_NO_MEMORY                  not enough system memory
 * @retval WRA_STATUS_NOT_INITIALIZED            library not initialized with the cloud
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for deregistration
 *
 * @see wra_metric_register
 */
static wra_status_t wra_metric_deregister_implementation( wra_metric_t* metric,
	const wra_timestamp_t* abs_time_out );

/**
 * @brief Converts a time stamp from the old structure to the new iot time stamp
 *        format
 *
 * @param[in]      timestamp           timestamp to convert
 *
 * @return time stamp in iot format
 */
static iot_timestamp_t wra_timestamp_to_iot( const wra_timestamp_t *time_stamp );

wra_metric_t* wra_metric_allocate( const char* name, const char* version )
{
	struct wra_metric* result = NULL;
	if ( name )
	{
		result = (struct wra_metric*)os_malloc( sizeof( struct wra_metric ) );
		if ( result )
		{
			os_memzero( result, sizeof( struct wra_metric ) );
			os_strncpy( result->metric_name, name, IOT_NAME_MAX_LEN - 1u  );
			if ( version )
				os_strncpy( result->metric_version, version, VERSION_MAX_LEN - 1u );
		}
	}
	return result;
}

wra_status_t wra_metric_deregister( wra_metric_t* metric, wra_millisecond_t max_time_out )
{
	wra_timestamp_t  end_time;
	wra_timestamp_t* end_time_ptr;

	/* determine the end time */
	end_time_ptr = wra_common_time_relative_to_absolute( &end_time, max_time_out );
	return wra_metric_deregister_implementation( metric, end_time_ptr );
}

wra_status_t wra_metric_deregister_implementation( wra_metric_t* metric,
	const wra_timestamp_t* abs_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( metric )
	{
		const struct wra_source* const source = metric->parent;
		result = WRA_STATUS_NOT_INITIALIZED;
		if ( source )
			result = wra_metric_register_implementation( NULL, metric, abs_time_out );
	}
	return result;
}

wra_status_t wra_metric_free( wra_metric_t* metric, wra_millisecond_t max_time_out )
{
	wra_timestamp_t  end_time;
	wra_timestamp_t* end_time_ptr;

	/* determine the end time */
	end_time_ptr = wra_common_time_relative_to_absolute( &end_time, max_time_out );
	return wra_metric_free_implementation( metric, end_time_ptr );
}

wra_status_t wra_metric_free_implementation( wra_metric_t* metric,
	const wra_timestamp_t* abs_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( metric )
	{
		result = WRA_STATUS_SUCCESS;
		if ( metric->parent )
			result = wra_metric_deregister_implementation( metric, abs_time_out );
		if ( result == WRA_STATUS_SUCCESS )
		{
			/* clear memory to help in cases of use after free */
			os_memzero( metric, sizeof( struct wra_metric ) );
			os_free_null( (void **)&metric );
		}
	}
	return result;
}

wra_status_t wra_metric_type_set( wra_metric_t* metric, wra_type_t type )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( metric )
	{
		metric->metric_type = type;
		result = WRA_STATUS_SUCCESS;
	}
	return result;
}

wra_status_t wra_metric_sample_max_set( wra_metric_t* metric, uint8_t max )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( metric )
	{
		metric->samples_max = max;
		if ( max > 0u && metric->samples_min > max )
			metric->samples_min = max;
		result = WRA_STATUS_SUCCESS;
	}
	return result;
}

wra_status_t wra_metric_sample_min_set( wra_metric_t* metric, uint8_t min )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( metric )
	{
		metric->samples_min = min;
		if ( min > 0u && metric->samples_max < min )
			metric->samples_max = min;
		result = WRA_STATUS_SUCCESS;
	}
	return result;
}

wra_status_t wra_metric_publish_boolean( wra_metric_t* metric, wra_bool_t value,
	const wra_timestamp_t *time_stamp,
	wra_millisecond_t max_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( metric )
	{
		result = WRA_STATUS_BAD_REQUEST;
		if(  metric->metric_type == WRA_TYPE_NULL ||
			metric->metric_type == WRA_TYPE_BOOLEAN )
		{
			if ( time_stamp )
			{
				iot_telemetry_timestamp_set( metric->telemetry,
					wra_timestamp_to_iot( time_stamp ) );
			}

			result = (wra_status_t)iot_telemetry_publish(
				metric->telemetry, NULL, max_time_out,
				IOT_TYPE_BOOL, value );
		}
	}
	return result;
}

wra_status_t wra_metric_publish_float( wra_metric_t* metric, double value,
	const wra_timestamp_t* time_stamp,
	wra_millisecond_t max_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( metric )
	{
		result = WRA_STATUS_BAD_REQUEST;
		if(  metric->metric_type == WRA_TYPE_NULL ||
			metric->metric_type == WRA_TYPE_FLOAT )
		{
			if ( time_stamp )
			{
				iot_telemetry_timestamp_set( metric->telemetry,
					wra_timestamp_to_iot( time_stamp ) );
			}

			result = (wra_status_t)iot_telemetry_publish(
				metric->telemetry, NULL, max_time_out,
				IOT_TYPE_FLOAT64, (iot_float64_t)value );
		}
	}
	return result;
}

wra_status_t wra_metric_publish_integer( wra_metric_t* metric, uint32_t value,
	const wra_timestamp_t* time_stamp,
	wra_millisecond_t max_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( metric )
	{
		result = WRA_STATUS_BAD_REQUEST;
		if(  metric->metric_type == WRA_TYPE_NULL ||
			metric->metric_type == WRA_TYPE_INTEGER )
		{
			if ( time_stamp )
			{
				iot_telemetry_timestamp_set( metric->telemetry,
					wra_timestamp_to_iot( time_stamp ) );
			}

			result = (wra_status_t)iot_telemetry_publish(
				metric->telemetry, NULL, max_time_out,
				IOT_TYPE_INT32, (iot_int32_t)value );
		}
	}
	return result;
}

wra_status_t wra_metric_publish_string( wra_metric_t* metric, const char* value,
	const wra_timestamp_t* time_stamp,
	wra_millisecond_t max_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( metric )
	{
		result = WRA_STATUS_BAD_REQUEST;
		if(  metric->metric_type == WRA_TYPE_NULL ||
			metric->metric_type == WRA_TYPE_STRING )
		{
			if ( time_stamp )
			{
				iot_telemetry_timestamp_set( metric->telemetry,
					wra_timestamp_to_iot( time_stamp ) );
			}

			result = (wra_status_t)iot_telemetry_publish(
				metric->telemetry, NULL, max_time_out,
				IOT_TYPE_STRING, value );
		}
	}
	return result;
}

wra_status_t wra_metric_publish_raw( wra_metric_t* metric, const void* value,
	size_t len, const wra_timestamp_t* time_stamp,
	wra_millisecond_t max_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( metric )
	{
		result = WRA_STATUS_BAD_REQUEST;
		if(  metric->metric_type == WRA_TYPE_NULL ||
			metric->metric_type == WRA_TYPE_RAW )
		{
			if ( time_stamp )
			{
				iot_telemetry_timestamp_set( metric->telemetry,
					wra_timestamp_to_iot( time_stamp ) );
			}

			result = (wra_status_t)iot_telemetry_publish_raw(
				metric->telemetry, NULL, max_time_out,
				len, value );
		}
	}
	return result;
}

wra_status_t wra_metric_register( wra_source_t* source, wra_metric_t* metric,
	wra_millisecond_t max_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( source && metric )
	{
		wra_timestamp_t  end_time;
		wra_timestamp_t* end_time_ptr;

		/* determine the end time */
		end_time_ptr = wra_common_time_relative_to_absolute( &end_time, max_time_out );
		result = wra_metric_register_implementation( source, metric, end_time_ptr );
	}
	return result;
}

wra_status_t wra_metric_register_implementation( struct wra_source* source,
	struct wra_metric* metric, const wra_timestamp_t* UNUSED(abs_time_out) )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( metric )
	{
		size_t i;
		result = WRA_STATUS_SUCCESS;

		/* loop twice: once to deregister, once to register */
		for ( i = 0u; i < 2u && result == WRA_STATUS_SUCCESS; ++i )
		{
			/* deregister */
			if ( i == 0u && metric->parent )
			{
				struct wra_source* const parent =
					metric->parent;
				/* remove from list */
				if ( metric->metric_prev )
					metric->metric_prev->metric_next =
						metric->metric_next;
				if ( metric->metric_next )
					metric->metric_next->metric_prev =
						metric->metric_prev;
				if ( parent->metric_first == metric )
					parent->metric_first =
						metric->metric_next;
				if ( parent->metric_last == metric )
					parent->metric_last =
						metric->metric_prev;
				metric->parent = NULL;
				metric->metric_prev = NULL;
				metric->metric_next = NULL;

				if ( metric->telemetry )
					result = (wra_status_t)iot_telemetry_deregister(
						metric->telemetry, NULL, 0u );
			}
			else if ( i == 1u && source ) /* register */
			{
				iot_t *iot_lib = NULL;

				if ( source->lib_handle )
					iot_lib = source->lib_handle->iot;
				if ( source != metric->parent )
				{
					/* add to list */
					if ( !source->metric_first )
						source->metric_first = metric;
					if( source->metric_last )
					{
						source->metric_last->metric_next = metric;
						metric->metric_prev = source->metric_last;
					}
					source->metric_last = metric;
					metric->parent = source;
				}

				if ( iot_lib )
				{
					char name[ IOT_NAME_MAX_LEN + 1u ];
					size_t name_len = 0u;
					enum iot_type type = IOT_TYPE_NULL;
					if ( *source->source_name != '\0' )
					{
						os_strncpy( name,
							source->source_name,
							IOT_NAME_MAX_LEN );
						name_len = os_strlen( name );
						if ( *source->source_version != '\0' )
						{
							os_snprintf(
								&name[name_len],
								IOT_NAME_MAX_LEN - name_len,
								"%s%s",
								IOT_SPLIT_VERSION,
								source->source_version );
							name_len +=
								os_strlen( &name[name_len] );
						}
						os_strncpy( &name[name_len],
							IOT_SPLIT_PARENT,
							IOT_NAME_MAX_LEN - name_len );
						name_len +=
							os_strlen( &name[name_len] );
					}
					os_strncpy( &name[name_len],
						metric->metric_name,
						IOT_NAME_MAX_LEN - name_len );
					name_len +=
						os_strlen( metric->metric_name )
						+ 1u;
					if ( *metric->metric_version != '\0' )
					{
						os_snprintf(
							&name[name_len],
							IOT_NAME_MAX_LEN - name_len,
							"%s%s",
							IOT_SPLIT_VERSION,
							metric->metric_version );
					}
					switch ( metric->metric_type )
					{
					case WRA_TYPE_NULL:
						type = IOT_TYPE_NULL;
						break;
					case WRA_TYPE_BOOLEAN:
						type = IOT_TYPE_BOOL;
						break;
					case WRA_TYPE_FLOAT:
						type = IOT_TYPE_FLOAT64;
						break;
					case WRA_TYPE_INTEGER:
						type = IOT_TYPE_INT32;
						break;
					case WRA_TYPE_RAW:
						type = IOT_TYPE_RAW;
						break;
					case WRA_TYPE_STRING:
						type = IOT_TYPE_STRING;
					}
					metric->telemetry = iot_telemetry_allocate(
						iot_lib, name, type );
					result = WRA_STATUS_FAILURE;
					if ( metric->telemetry )
					{
						/* set other attributes for backwards compatibility */
						if ( *metric->metric_units != '\0' )
							iot_telemetry_attribute_set(
								metric->telemetry,
								IOT_FLAG_UDMP_SAMPLES_UNITS,
								IOT_TYPE_STRING,
								metric->metric_units );
						if ( metric->samples_max > 0u )
							iot_telemetry_attribute_set(
								metric->telemetry,
								IOT_FLAG_UDMP_SAMPLES_MAX,
								IOT_TYPE_UINT8,
								metric->samples_max );
						if ( metric->samples_min > 0u )
							iot_telemetry_attribute_set(
								metric->telemetry,
								IOT_FLAG_UDMP_SAMPLES_MIN,
								IOT_TYPE_UINT8,
								metric->samples_min );

						/* register telemetry object */
						result = (wra_status_t)iot_telemetry_register(
							metric->telemetry, NULL, 0u );
					}
				}
			}
		}
	}
	return result;
}

wra_status_t wra_metric_units_set( wra_metric_t* metric, const char* units )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( metric )
	{
		os_memzero( metric->metric_units, IOT_NAME_MAX_LEN );
		if ( units )
			os_strncpy( metric->metric_units, units,
				IOT_NAME_MAX_LEN - 1u );
		result = WRA_STATUS_SUCCESS;
	}
	return result;
}

iot_timestamp_t wra_timestamp_to_iot( const wra_timestamp_t *time_stamp )
{
	iot_timestamp_t result = 0u;
	if ( time_stamp )
		result = ( (iot_timestamp_t)time_stamp->tv_sec * IOT_MILLISECONDS_IN_SECOND ) +
			 ( (iot_timestamp_t)time_stamp->tv_nsec / IOT_NANOSECONDS_IN_MILLISECOND );
	return result;
}

