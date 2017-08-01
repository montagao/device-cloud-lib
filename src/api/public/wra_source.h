/**
 * @file
 * @brief Header file using the Wind River Agent library sources
 *
 * @copyright Copyright (C) 2015-2016 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software may
 * be licensed only pursuant to the terms of an applicable Wind River license
 * agreement.  No license to Wind River intellectual property rights is granted
 * herein.  All rights not licensed by Wind River are reserved by Wind River.
 */
#ifndef WRA_SOURCE_H
#define WRA_SOURCE_H

#include "wra_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/* functions */
/**
 * @brief Allocates memory for a new source
 *
 * @param[in]      name                          name of the source
 * @param[in]      version                       version of the source (optional)
 *
 * @return a reference to the new source
 *
 * @see wra_source_free
 */
WRA_API wra_source_t* wra_source_allocate( const char* name, const char* version );

/**
 * @brief Deregisters a source from the cloud
 *
 * @param[in,out]  source                        source to unregister
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for source deregistration
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for source deregistration
 *
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_NO_MEMORY                  not enough system memory
 * @retval WRA_STATUS_NOT_INITIALIZED            library not initialized with the cloud
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  time out value reached
 *
 * @see wra_source_register
 */
WRA_API wra_status_t wra_source_deregister( wra_source_t* source, wra_millisecond_t max_time_out );

/**
 * @brief Returns the reference to a previously registered source
 *
 * @param[in]      lib_handle                    handle to the agent
 * @param[in]      name                          name of the source
 * @param[in]      version                       version of the source (optional)
 * @param[out]     match                         matching source that is found, NULL if not found
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_NOT_FOUND                  no match found
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_source_register
 * @see wra_source_deregister
 */
WRA_API wra_status_t wra_source_find( const wra_t* lib_handle, const char* name, const char* version, wra_source_t** match );

/**
 * @brief Destroys a previously allocated source
 *
 * @param[in]      source                        source to destroy
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for source deallocation
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for source deallocation
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  time out value reached
 *
 * @see wra_source_allocate
 */
WRA_API wra_status_t wra_source_free( wra_source_t* source, wra_millisecond_t max_time_out );

/**
 * @brief Registers a source with the agent
 *
 * @param[in]      lib_handle                    handle to the agent
 * @param[in]      source                        source to register
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for source registration
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for source registration
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_NO_MEMORY                  not enough system memory
 * @retval WRA_STATUS_NOT_INITIALIZED            library not initialized with the cloud
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  time out value reached
 *
 * @see wra_action_deregister
 */
WRA_API wra_status_t wra_source_register( wra_t* lib_handle, wra_source_t* source,
	wra_millisecond_t max_time_out );

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef WRA_SOURCE_H */

