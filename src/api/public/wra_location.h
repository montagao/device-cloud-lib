/**
 * @file
 * @brief Header file for location sources and samples using the Wind River Agent library
 *
 * @copyright Copyright (C) 2015-2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software may
 * be licensed only pursuant to the terms of an applicable Wind River license
 * agreement.  No license to Wind River intellectual property rights is granted
 * herein.  All rights not licensed by Wind River are reserved by Wind River.
 */
#ifndef WRA_LOCATION_H
#define WRA_LOCATION_H

/* includes */
#include "wra_types.h"
#include <stdint.h> /* for uint8_t */
#include <stdlib.h> /* for size_t */

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/* functions */
/**
 * @brief Sets the accuracy for the location sample
 *
 * @param[in,out]  sample                        object to modify
 * @param[in]      accuracy                      accuracy for the location sample (in metres)
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_location_allocate
 */
WRA_API wra_status_t wra_location_accuracy_set( wra_location_t* sample, double accuracy );

/**
 * @brief Allocates memory for a new location sample
 *
 * @param[in]      latitude                      latitude for the new sample
 * @param[in]      longitude                     longitude for the new sample
 *
 * @return a reference to the new location source
 *
 * @see wra_location_free
 */
WRA_API wra_location_t* wra_location_allocate( double latitude, double longitude );

/**
 * @brief Sets the accuracy of the altitude for the location sample
 *
 * @param[in,out]  sample                        object to modify
 * @param[in]      accuracy                      altitude accuracy for the location sample (in metres)
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_location_allocate
 * @see wra_location_altitude_set
 */
WRA_API wra_status_t wra_location_altitude_accuracy_set( wra_location_t* sample, double accuracy );

/**
 * @brief Sets the altitude for the location sample
 *
 * @param[in,out]  sample                        object to modify
 * @param[in]      altitude                      altitude for the location sample (in metres)
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_location_allocate
 * @see wra_location_altitude_accuracy_set
 */
WRA_API wra_status_t wra_location_altitude_set( wra_location_t* sample, double altitude );

/**
 * @brief Free memory associated with a location sample
 *
 * @param[in,out]  sample                        sample to free
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_location_allocate
 */
WRA_API wra_status_t wra_location_free( wra_location_t* sample );

/**
 * @brief Sets the heading for the location sample
 *
 * @param[in,out]  sample                        object to modify
 * @param[in]      heading                       heading for the location sample (in degrees)
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_location_allocate
 */
WRA_API wra_status_t wra_location_heading_set( wra_location_t* sample, double heading );

/**
 * @brief Publishes a location sample to the cloud
 *
 * @param[in,out]  lib_handle                    handle to the agent
 * @param[in]      sample                        sample to publish
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for location to publish
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for the location to publish
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_NO_MEMORY                  out of memory to store sample
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out before being able to send
 *
 * @see wra_location_allocate
 */
WRA_API wra_status_t wra_location_publish( wra_t* lib_handle, wra_location_t* sample,
	wra_millisecond_t max_time_out );

/**
 * @brief Sets the minimum & maximum number of samples to publish per message to the cloud
 *
 * @param[in,out]  lib_handle                    handle to the agent
 * @param[in]      min                           new minimum value to set
 * @param[in]      max                           new maximum value to set
 * @param[in]      max_time_out                  maximum time to wait in milliseconds to update sample minimum and maximum
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for the number of samples to publish per message to be updated
 * @note Setting either the @p min or @p max parameter to a value of @p 0, will set the minimum or
 *       maximum to the default value
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out before being able to send
 */
WRA_API wra_status_t wra_location_samples_set( wra_t* lib_handle, uint8_t min, uint8_t max,
	wra_millisecond_t max_time_out );

/**
 * @brief Sets the source for the location sample
 *
 * @param[in,out]  sample                        object to modify
 * @param[in]      source                        source generating the location
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_location_allocate
 */
WRA_API wra_status_t wra_location_source_set( wra_location_t* sample, wra_location_source_t source );

/**
 * @brief Sets the speed for the location sample
 *
 * @param[in,out]  sample                        object to modify
 * @param[in]      speed                         speed for the location sample in (m/s)
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_location_allocate
 */
WRA_API wra_status_t wra_location_speed_set( wra_location_t* sample, double speed );

/**
 * @brief Sets the tag for the location sample
 *
 * @param[in,out]  sample                        object to modify
 * @param[in]      tag                           tag for the location sample
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_location_allocate
 */
WRA_API wra_status_t wra_location_tag_set( wra_location_t* sample, const char* tag );

/**
 * @brief Sets the timestamp for the location sample
 *
 * @param[in,out]  sample                        object to modify
 * @param[in]      time_stamp                    timestamp for the location sample
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_location_allocate
 */
WRA_API wra_status_t wra_location_timestamp_set( wra_location_t* sample, const wra_timestamp_t* time_stamp );

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef WRA_LOCATION_H */

