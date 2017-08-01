/**
 * @file
 * @brief Source file for location sources and samples within the Wind River
 * Internet of Things (IoT) library
 *
 * @copyright Copyright (C) 2015-2016 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software may
 * be licensed only pursuant to the terms of an applicable Wind River license
 * agreement.  No license to Wind River intellectual properly rights is granted
 * herein.  All rights not licensed by Wind River are reserved by Wind River.
 */

#include "public/iot.h"
#include "public/wra_location.h"

#include "shared/iot_types.h"
#include "shared/wra_internal.h"       /* for location flags */

/**< @brief Pointer to location_telemetry object */
static struct iot_telemetry *location_telemetry = NULL;

/**
 * @brief internal implementation of deregistration
 *
 * @param[in,out]  lib_handle          library handle
 * @param[in]      location            object to modify
 * @param[in]      abs_time_out        maximum time to wait
 *
 * @retval WRA_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS          on success
 */
static wra_status_t wra_location_deregister_implementation( wra_t* lib_handle,
	wra_location_t* location, const wra_timestamp_t* abs_time_out );

/**
 * @brief internal implementation of registration & deregistration
 *
 * @param[in,out]  lib_handle          library handle
 * @param[in]      location            object to modify
 * @param[in]      location_register   whether to register or deregister action
 * @param[in]      max_time_out        maximum time to wait
 *
 * @retval WRA_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS          on success
 */
static wra_status_t wra_location_register( wra_t* lib_handle,
	wra_location_t* location,
	wra_bool_t location_register,
	wra_millisecond_t max_time_out );

/**
 * @brief Internal implementation of registration
 *
 * @param[in,out]  lib_handle          library handle
 * @param[in]      location            object to modify
 * @param[in]      abs_time_out        maximum time to wait
 *
 * @retval WRA_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS          on success
 */
static wra_status_t wra_location_register_implementation( wra_t* lib_handle,
	struct wra_location* location, const wra_timestamp_t* abs_time_out );

#define LOCATION_NAME  "location"

wra_status_t wra_location_accuracy_set( wra_location_t* sample, double accuracy )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( sample && sample->location )
	{
		result = (wra_status_t)iot_location_accuracy_set(
			sample->location,
			(iot_float64_t)accuracy );
	}
	return result;
}

wra_location_t* wra_location_allocate( double latitude, double longitude )
{
	struct iot_location* location_sample = NULL;
	struct wra_location* sample = NULL;

	location_sample = iot_location_allocate(
			(iot_float64_t)latitude, (iot_float64_t)longitude );
	if ( location_sample )
	{
		sample = (struct wra_location*)os_malloc(
			sizeof( struct wra_location ) );
		if ( sample )
		{
			os_memzero( sample, sizeof( struct wra_location ) );
			sample->location = location_sample;
		}
	}
	return sample;
}

wra_status_t wra_location_altitude_accuracy_set( wra_location_t* sample,
	double accuracy )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( sample && sample->location )
	{
		result = (wra_status_t)iot_location_altitude_accuracy_set(
			sample->location,
			(iot_float64_t)accuracy );
	}
	return result;
}

wra_status_t wra_location_altitude_set( wra_location_t* sample,
	double altitude )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( sample && sample->location )
	{
		result = (wra_status_t)iot_location_altitude_set(
			sample->location,
			(iot_float64_t)altitude );
	}
	return result;
}

wra_status_t wra_location_free( wra_location_t* sample )
{

	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( sample )
	{
		if ( sample->location )
			iot_location_free( sample->location );
		/* clear memory to help in cases of use after free */
		os_memzero( sample, sizeof( struct wra_location ) );
		os_free_null( (void **)&sample );
		result = WRA_STATUS_SUCCESS;
	}
	return result;
}

wra_status_t wra_location_register( wra_t* lib_handle, wra_location_t* location,
	wra_bool_t location_register, wra_millisecond_t max_time_out )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if (  location )
	{
		wra_timestamp_t  end_time;
		wra_timestamp_t* end_time_ptr;

		/* determine the end time */
		end_time_ptr = wra_common_time_relative_to_absolute( &end_time,
			max_time_out );
		if ( location_register != WRA_FALSE ) /* register */
		{
			result = wra_location_register_implementation(
				lib_handle,location, end_time_ptr );
		}
		else /* deregister */
		{
			result = wra_location_deregister_implementation(
				lib_handle,location, end_time_ptr );
		}
	}
	return result;
}

wra_status_t wra_location_deregister_implementation( wra_t* lib_handle,
	wra_location_t* location, const wra_timestamp_t *UNUSED(abs_time_out) )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( location && lib_handle && location_telemetry )
		result = (wra_status_t)
			iot_telemetry_deregister(
			location_telemetry, NULL, 0u );
	return result;
}

wra_status_t wra_location_register_implementation( wra_t* lib_handle,
	struct wra_location* location,
	const wra_timestamp_t* UNUSED(abs_time_out) )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( location && lib_handle)
	{
		size_t i;
		result = WRA_STATUS_SUCCESS;

		/* loop twice: once to deregister, once to register */
		for ( i = 0u; i < 2u && result == WRA_STATUS_SUCCESS; ++i )
		{
			/* deregister */
			if ( i == 0u && location )
			{
				if ( location_telemetry )
					result = (wra_status_t)
						iot_telemetry_deregister(
						location_telemetry, NULL, 0u );
			}
			else if ( i == 1u &&location ) /* register */
			{
				result = WRA_STATUS_FAILURE;

				if ( location_telemetry == NULL )
					location_telemetry = iot_telemetry_allocate(
						lib_handle->iot, LOCATION_NAME,
						IOT_TYPE_LOCATION );
		
				if ( location_telemetry )
				{
					/* register telemetry object */
					result = (wra_status_t)
						iot_telemetry_register(
						location_telemetry, NULL, 0u );
				}
			}
		}
	}
	return result;
}

wra_status_t wra_location_heading_set( wra_location_t* sample, double heading )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( sample && sample->location )
	{
		result = (wra_status_t)iot_location_heading_set(
			sample->location, (iot_float64_t)heading );
	}
	return result;
}

wra_status_t wra_location_publish( wra_t* lib_handle, wra_location_t* sample,
	wra_millisecond_t UNUSED(max_time_out) )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;

	if ( lib_handle && sample )
	{
		if ( location_telemetry == NULL )
		{
			if ( wra_location_register(
				lib_handle, sample, WRA_TRUE, 0u )
					== WRA_STATUS_SUCCESS )
				result = WRA_STATUS_SUCCESS;
		}
		else
			result = WRA_STATUS_SUCCESS;

		if ( result == WRA_STATUS_SUCCESS )
		{
			result = (wra_status_t)iot_telemetry_publish(
				location_telemetry, NULL, 0,
				IOT_TYPE_LOCATION, sample->location );
		}
	}
	return result;
}

wra_status_t wra_location_samples_set( wra_t* lib_handle,
	uint8_t min, uint8_t max, wra_millisecond_t UNUSED(max_time_out) )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( lib_handle )
	{
		result = WRA_STATUS_BAD_REQUEST;
		if ( min > 0u && max > 0u && max >= min )
		{
			/** @todo Add mutliple samples support */
			result = WRA_STATUS_FAILURE;
		}
	}
	return result;
}

wra_status_t wra_location_source_set( wra_location_t* sample, wra_location_source_t source )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( sample && sample->location )
	{
		switch( source )
		{
			case WRA_LOCATION_SOURCE_FIXED:
				result = (wra_status_t) iot_location_source_set(
					sample->location,
					IOT_LOCATION_SOURCE_FIXED );
				break;
			case WRA_LOCATION_SOURCE_GPS:
				result = (wra_status_t) iot_location_source_set(
					sample->location,
					IOT_LOCATION_SOURCE_GPS );
				break;
			case WRA_LOCATION_SOURCE_WIFI:
				result = (wra_status_t) iot_location_source_set(
					sample->location,
					IOT_LOCATION_SOURCE_WIFI );
				break;
			case WRA_LOCATION_SOURCE_UNKNOWN:
				result = (wra_status_t) iot_location_source_set(
					sample->location,
					IOT_LOCATION_SOURCE_UNKNOWN );
				break;
		}
	}
	return result;
}

wra_status_t wra_location_speed_set( wra_location_t* sample, double speed )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( sample && sample->location )
	{
		result = (wra_status_t)iot_location_speed_set(
			sample->location,
			(iot_float64_t)speed );
	}
	return result;
}

wra_status_t wra_location_tag_set( wra_location_t* sample, const char* tag )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( sample && sample->location )
	{
		result = (wra_status_t)iot_location_tag_set(
			sample->location, tag );
	}
	return result;
}

wra_status_t wra_location_timestamp_set( wra_location_t* sample, const wra_timestamp_t* time_stamp )
{
	wra_status_t result = WRA_STATUS_BAD_PARAMETER;
	if ( sample )
	{
		if ( time_stamp )
			os_memcpy( &sample->time_stamp, time_stamp,
				sizeof( wra_timestamp_t ) );
		else
			os_memzero( &sample->time_stamp,
				sizeof( wra_timestamp_t ) );
		result = WRA_STATUS_SUCCESS;
	}
	return result;
}

