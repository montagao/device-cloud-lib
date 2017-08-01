/**
 * @file
 * @brief Header file for common functionality the Wind River Internet of Things (IoT) library
 *
 * @copyright Copyright (C) 2015-2016 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software may
 * be licensed only pursuant to the terms of an applicable Wind River license
 * agreement.  No license to Wind River intellectual property rights is granted
 * herein.  All rights not licensed by Wind River are reserved by Wind River.
 */
#ifndef WRA_COMMON_H
#define WRA_COMMON_H

/* includes */
#include "wra_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/**
 * @brief Connects the client to the agent
 *
 * @param[in]      lib_handle                    library handle
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for connection
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for connection
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_FAILURE                    on failure
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for state retreival
 */
WRA_API wra_status_t wra_connect( wra_t* lib_handle, wra_millisecond_t max_time_out );

/**
 * @brief Disconnects the client from the agent
 *
 * @param[in]      lib_handle                    library handle
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for disconnection
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for disconnection
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_FAILURE                    on failure
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for state retreival
 */
WRA_API wra_status_t wra_disconnect( wra_t* lib_handle, wra_millisecond_t max_time_out );

/**
 * @brief Translates a library error number into an error message
 *
 * @param[in]      code                          id of the error to return the message for
 *
 * @return the string corresponding to the error identifier specified
 */
WRA_API const char* wra_error( wra_status_t code );

/**
 * @brief Initialize the library
 *
 * @param[in]      token                         unique token identifying device (optional)
 *
 * @return A reference handle to the library
 *
 * @see wra_terminate
 */
WRA_API wra_t* wra_initialize( const char* token );

/**
 * @brief Sets a callback to call when the library has a log message
 *
 * @param[in,out]  lib_handle                    library handle
 * @param[in]      log_callback                  callback to be called on a log message
 * @param[in]      user_data                     user specific data to pass to the callback
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 */
WRA_API wra_status_t wra_log_callback( wra_t* lib_handle, wra_log_callback_t* log_callback, void* user_data );

/**
 * @brief Registers a call back that is called whenever the state changes by the agent
 *
 * @param[in]      lib_handle                    library handle
 * @param[in]      state_callback                callback to be called when state changes
 * @param[in]      user_data                     user specific data to pass to the callback
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 */
WRA_API wra_status_t wra_state_callback( wra_t* lib_handle, wra_state_callback_t* state_callback, void* user_data );

/**
 * @brief Allows the client to query the current state from the agent
 *
 * @param[in]      lib_handle                    library handle
 * @param[out]     state                         current state of state
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for state retrieval
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for a state retrieval.
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for state retreival
 */
WRA_API wra_status_t wra_state_get( wra_t* lib_handle, wra_state_t* state, wra_millisecond_t max_time_out );

/**
 * @brief Destroys memory associated with the library
 *
 * @param[in]      lib_handle                    library handle
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for termination
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for a termination signal.
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_FAILURE                    on failure
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for termination
 *
 * @see wra_initialize
 */
WRA_API wra_status_t wra_terminate( wra_t* lib_handle, wra_millisecond_t max_time_out );

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef WRA_COMMON_H */

