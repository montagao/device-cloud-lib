/**
 * @file
 * @brief Header file using the Wind River Agent library services
 *
 * @copyright Copyright (C) 2015-2016 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software may
 * be licensed only pursuant to the terms of an applicable Wind River license
 * agreement.  No license to Wind River intellectual property rights is granted
 * herein.  All rights not licensed by Wind River are reserved by Wind River.
 */
#ifndef WRA_SERVICE_H
#define WRA_SERVICE_H

#include "wra_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/* functions */
/**
 * @brief Allocates memory for a new service
 *
 * @param[in]      name                          name of the service
 * @param[in]      version                       version of the service (optional)
 *
 * @return a reference to the new service
 *
 * @see wra_service_free
 */
WRA_API wra_service_t* wra_service_allocate( const char* name, const char* version );

/**
 * @brief Deregisters a service from the cloud
 *
 * @param[in,out]  service                       service to unregister
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for service deregistration
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for service deregistration
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_NO_MEMORY                  not enough system memory
 * @retval WRA_STATUS_NOT_INITIALIZED            library not initialized with the cloud
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  time out value reached
 *
 * @see wra_service_register
 */
WRA_API wra_status_t wra_service_deregister( wra_service_t* service, wra_millisecond_t max_time_out );

/**
 * @brief Returns the reference to a previously registered service
 *
 * @param[in]      lib_handle                    handle to the agent
 * @param[in]      name                          name of the service
 * @param[in]      version                       version of the service (optional)
 * @param[out]     match                         matching service that is found, NULL if not found
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_NOT_FOUND                  no match found
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_service_register
 * @see wra_service_deregister
 */
WRA_API wra_status_t wra_service_find( const wra_t* lib_handle, const char* name, const char* version, wra_service_t** match );

/**
 * @brief Destroys a previously allocated service
 *
 * @param[in,out]  service                       service to destroy
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for service deallocation
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for service deallocation
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  time out value reached
 *
 * @see wra_service_allocate
 */
WRA_API wra_status_t wra_service_free( wra_service_t* service, wra_millisecond_t max_time_out );

/**
 * @brief Registers a service with the agent
 *
 * @param[in,out]  lib_handle                    handle to the agent
 * @param[in,out]  service                       service to register
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for service registration
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for service registration
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_NO_MEMORY                  not enough system memory
 * @retval WRA_STATUS_NOT_INITIALIZED            library not initialized with the cloud
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  time out value reached
 *
 * @see wra_action_deregister
 */
WRA_API wra_status_t wra_service_register( wra_t* lib_handle, wra_service_t* service,
	const wra_millisecond_t max_time_out );

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef WRA_SERVICE_H */

