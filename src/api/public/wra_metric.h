/**
 * @file
 * @brief Header file for metric data using the Wind River Agent library
 *
 * @copyright Copyright (C) 2015-2016 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software may
 * be licensed only pursuant to the terms of an applicable Wind River license
 * agreement.  No license to Wind River intellectual property rights is granted
 * herein.  All rights not licensed by Wind River are reserved by Wind River.
 */
#ifndef WRA_METRIC_H
#define WRA_METRIC_H

/* includes */
#include "wra_types.h"
#include <stdint.h> /* for uint8_t */
#include <stdlib.h> /* for size_t */

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/* functions */
/**
 * @brief Allocates memory for a new metric
 *
 * @param[in]      name                          name of the metric
 * @param[in]      version                       version string of the metric
 *
 * @return a reference to the new metric
 *
 * @see wra_metric_free
 */
WRA_API wra_metric_t* wra_metric_allocate( const char* name, const char* version );

/**
 * @brief Deregisters a metric from the cloud
 *
 * @param[in]      metric                        metric to deregister
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for metric deregistration
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for metric deregistration
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_NO_MEMORY                  not enough system memory
 * @retval WRA_STATUS_NOT_INITIALIZED            library not initialized with the cloud
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for deregistration
 *
 * @see wra_metric_register
 */
WRA_API wra_status_t wra_metric_deregister( wra_metric_t* metric, wra_millisecond_t max_time_out );

/**
 * @brief Destroys a previously allocated metric
 *
 * @param[in]      metric                        metric to destroy
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for metric deallocation
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for metric deallocation
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for deallocation
 *
 * @see wra_metric_allocate
 */
WRA_API wra_status_t wra_metric_free( wra_metric_t* metric, wra_millisecond_t max_time_out );

/**
 * @brief Publishes a boolean value sample to a metric
 *
 * @param[in,out]  metric                        metric to publish to
 * @param[in]      value                         value to set
 * @param[in]      time_stamp                    time to set as published time (optional)
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for metric sample publication
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for metric sample publication
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                type of the metric doesn't match what is being set
 * @retval WRA_STATUS_NO_MEMORY                  out of memory to store sample
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for publish
 *
 * @see wra_metric_type_set
 */
WRA_API wra_status_t wra_metric_publish_boolean( wra_metric_t* metric, wra_bool_t value,
	const wra_timestamp_t* time_stamp, wra_millisecond_t max_time_out );

/**
 * @brief Publishes a float value sample to a metric
 *
 * @param[in,out]  metric                        metric to publish to
 * @param[in]      value                         value to set
 * @param[in]      time_stamp                    time to set as published time (optional)
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for metric sample publication
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for metric sample publication
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                type of the metric doesn't match what is being set
 * @retval WRA_STATUS_NO_MEMORY                  out of memory to store sample
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for publish
 *
 * @see wra_metric_type_set
 */
WRA_API wra_status_t wra_metric_publish_float( wra_metric_t* metric, double value,
	const wra_timestamp_t* time_stamp, wra_millisecond_t max_time_out );

/**
 * @brief Publishes a integer value sample to a metric
 *
 * @param[in,out]  metric                        metric to publish to
 * @param[in]      value                         value to set
 * @param[in]      time_stamp                    time to set as published time (optional)
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for metric sample publication
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for metric sample publication
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                type of the metric doesn't match what is being set
 * @retval WRA_STATUS_NO_MEMORY                  out of memory to store sample
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for publish
 *
 * @see wra_metric_type_set
 */
WRA_API wra_status_t wra_metric_publish_integer( wra_metric_t* metric, uint32_t value,
	const wra_timestamp_t* time_stamp, wra_millisecond_t max_time_out );

/**
 * @brief Publishes a string value sample to a metric
 *
 * @param[in,out]  metric                        metric to publish to
 * @param[in]      value                         value to set
 * @param[in]      time_stamp                    time to set as published time (optional)
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for metric sample publication
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for metric sample publication
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                type of the metric doesn't match what is being set
 * @retval WRA_STATUS_NO_MEMORY                  out of memory to store sample
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for publish
 *
 * @see wra_metric_type_set
 */
WRA_API wra_status_t wra_metric_publish_string( wra_metric_t* metric, const char* value,
	const wra_timestamp_t* time_stamp, wra_millisecond_t max_time_out );

/**
 * @brief Publishes a raw value sample to a metric
 *
 * @param[in,out]  metric                        metric to publish to
 * @param[in]      value                         value to set
 * @param[in]      len                           length of the data
 * @param[in]      time_stamp                    time to set as published time (optional)
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for metric sample publication
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for metric sample publication
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                type of the metric doesn't match what is being set
 * @retval WRA_STATUS_NO_MEMORY                  out of memory to store sample
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for publish
 *
 * @see wra_metric_type_set
 */
WRA_API wra_status_t wra_metric_publish_raw( wra_metric_t* metric, const void* value, size_t len,
	const wra_timestamp_t* time_stamp, wra_millisecond_t max_time_out );

/**
 * @brief Registers a metric with a source
 *
 * @param[in,out]  source                        source to register with
 * @param[in,out]  metric                        metric to register
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for metric registration
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for metric registration
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_NO_MEMORY                  not enough system memory
 * @retval WRA_STATUS_NOT_INITIALIZED            library not initialized with the cloud
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for registration
 *
 * @see wra_command_deregister
 */
WRA_API wra_status_t wra_metric_register( wra_source_t* source, wra_metric_t* metric,
	wra_millisecond_t max_time_out );

/**
 * @brief Sets the maximum amount of samples to transmit
 * @note The minimum will be adjusted if larger than the new maximum
 *
 * @param[in,out]  metric                        metric to modify
 * @param[in]      max                           maximum to send (0 to use default)
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_metric_sample_min_set
 */
WRA_API wra_status_t wra_metric_sample_max_set( wra_metric_t* metric, uint8_t max );

/**
 * @brief Sets the minimum amount of samples to transmit
 * @note The maximum will be adjusted if smaller than the new minimum
 *
 * @param[in,out]  metric                        metric to modify
 * @param[in]  min                               minimum to send (0 to use default)
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_metric_sample_max_set
 */
WRA_API wra_status_t wra_metric_sample_min_set( wra_metric_t* metric, uint8_t min );

/**
 * @brief Sets the data type for the metric
 *
 * @param[in,out]  metric                        metric to modify
 * @param[in]      type                          data type to set to
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_metric_publish_boolean
 * @see wra_metric_publish_integer
 * @see wra_metric_publish_float
 * @see wra_metric_publish_string
 * @see wra_metric_publish_raw
 */
WRA_API wra_status_t wra_metric_type_set( wra_metric_t* metric, wra_type_t type );

/**
 * @brief Sets the unit for the metric
 *
 * @param[in,out]  metric                        metric to modify
 * @param[in]      units                         string used to specify the units
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 */
WRA_API wra_status_t wra_metric_units_set( wra_metric_t* metric, const char* units );

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef WRA_METRIC_H */

