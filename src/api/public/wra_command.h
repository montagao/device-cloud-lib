/**
 * @file
 * @brief Header file using the Wind River Agent library commands
 *
 * @copyright Copyright (C) 2015-2016 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software may
 * be licensed only pursuant to the terms of an applicable Wind River license
 * agreement.  No license to Wind River intellectual property rights is granted
 * herein.  All rights not licensed by Wind River are reserved by Wind River.
 */
#ifndef WRA_COMMAND_H
#define WRA_COMMAND_H

#include "wra_types.h"
#include <stdint.h> /* for int64_t */
#include <time.h> /* for struct timespec */

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/* functions */
/**
 * @brief Allocates memory for a new command which will NOT send back a response when completed
 *
 * @param[in]      name                          name of the command
 *
 * @return a reference to the new command
 *
 * @see wra_command_free
 */
WRA_API wra_command_t* wra_command_allocate( const char* name );

/**
 * @brief Deregisters a command from the cloud
 *
 * @param[in]      command                       command to unregister
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for command deregistration
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for command deregistration
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_NO_MEMORY                  not enough system memory
 * @retval WRA_STATUS_NOT_INITIALIZED            library not initialized with the cloud
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for deregistration
 *
 * @see wra_command_register
 */
WRA_API wra_status_t wra_command_deregister( wra_command_t* command, wra_millisecond_t max_time_out );

/**
 * @brief Destroys a previously allocated command
 *
 * @param[in]      command                       command to destroy
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for command deallocation
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for command deallocation
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for command deallocation
 *
 * @see wra_command_allocate
 */
WRA_API wra_status_t wra_command_free( wra_command_t* command, wra_millisecond_t max_time_out );

/**
 * @brief Adds a parameter to the command
 *
 * @param[in]      command                       command to add the parameter to
 * @param[in]      id                            name of the parameter
 * @param[in]      type                          data type of the parameter
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for parameter addition
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for parameter addition
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                type isn't supported (i.e. WRA_DATA_TYPE_NULL)
 * @retval WRA_STATUS_FULL                       maximum number of parameters reached
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for command deallocation
 */
WRA_API wra_status_t wra_command_parameter_add( wra_command_t* command, const char* id, wra_type_t type, wra_millisecond_t max_time_out );

/**
 * @brief Returns the boolean value of a parameter
 *
 * @param[in]      request                       command request containing parameter
 * @param[in]      id                            parameter name
 * @param[out]     value                         value that was obtained
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                type doesn't match parameter
 * @retval WRA_STATUS_NOT_FOUND                  parameter id not found
 * @retval WRA_STATUS_NOT_INITIALIZED            parameter has not been set a value exists
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_command_parameter_boolean_set
 */
WRA_API wra_status_t wra_command_parameter_boolean_get( const wra_command_request_t* request, const char* id, wra_bool_t* value );

/**
 * @brief Sets the boolean value of a parameter
 *
 * @param[in]      request                       command request containing parameter
 * @param[in]      id                            parameter name
 * @param[out]     value                         value to set
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                type doesn't match parameter
 * @retval WRA_STATUS_NOT_FOUND                  parameter id not found
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_command_parameter_boolean_get
 */
WRA_API wra_status_t wra_command_parameter_boolean_set( wra_command_request_t* request, const char* id, wra_bool_t value );

/**
 * @brief Returns the value of a floating-point parameter
 *
 * @param[in]      request                       command request containing parameter
 * @param[in]      id                            parameter name
 * @param[out]     value                         value that was obtained
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                type doesn't match parameter
 * @retval WRA_STATUS_NOT_FOUND                  parameter id not found
 * @retval WRA_STATUS_NOT_INITIALIZED            parameter has not been set a value exists
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_command_parameter_float_set
 */
WRA_API wra_status_t wra_command_parameter_float_get( const wra_command_request_t* request, const char* id, double* value );

/**
 * @brief Sets the value of a floating-point parameter
 *
 * @param[in]      request                       command request containing parameter
 * @param[in]      id                            parameter name
 * @param[out]     value                         value to set
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                type doesn't match parameter
 * @retval WRA_STATUS_NOT_FOUND                  parameter id not found
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_command_parameter_float_get
 */
WRA_API wra_status_t wra_command_parameter_float_set( wra_command_request_t* request, const char* id, double value );

/**
 * @brief Returns the value of an integer parameter
 *
 * @param[in]      request                       command request containing parameter
 * @param[in]      id                            parameter name
 * @param[out]     value                         value that was obtained
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                type doesn't match parameter
 * @retval WRA_STATUS_NOT_FOUND                  parameter id not found
 * @retval WRA_STATUS_NOT_INITIALIZED            parameter has not been set a value exists
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_command_parameter_integer_set
 */
WRA_API wra_status_t wra_command_parameter_integer_get( const wra_command_request_t* request, const char* id, int32_t* value );

/**
 * @brief Sets the value of an integer parameter
 *
 * @param[in]       request                      command request containing parameter
 * @param[in]       id                           parameter name
 * @param[out]      value                        value to set
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                type doesn't match parameter
 * @retval WRA_STATUS_NOT_FOUND                  parameter id not found
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_command_parameter_integer_get
 */
WRA_API wra_status_t wra_command_parameter_integer_set( wra_command_request_t* request, const char* id, int32_t value );

/**
 * @brief Returns the value of a raw memory parameter
 *
 * @param[in]      request                       command request containing parameter
 * @param[in]      id                            parameter name
 * @param[out]     dest                          location to store result
 * @param[in]      len                           length of buffer
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                type doesn't match parameter or buffer too small
 * @retval WRA_STATUS_NOT_FOUND                  parameter id not found
 * @retval WRA_STATUS_NOT_INITIALIZED            parameter has not been set a value exists
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_command_parameter_raw_set
 */
WRA_API wra_status_t wra_command_parameter_raw_get( const wra_command_request_t* request, const char* id, void* dest, size_t len );

/**
 * @brief Sets the value of a raw memory parameter
 *
 * @param[in]      request                       command request containing parameter
 * @param[in]      id                            parameter name
 * @param[out]     value                         value to set
 * @param[in]      len                           length of data
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                type doesn't match parameter
 * @retval WRA_STATUS_NO_MEMORY                  unable to allocate memory for storage
 * @retval WRA_STATUS_NOT_FOUND                  parameter id not found
 * @retval WRA_STATUS_SUCCESS                    success
 *
 * @see wra_command_parameter_raw_get
 */
WRA_API wra_status_t wra_command_parameter_raw_set( wra_command_request_t* request, const char* id, const void* value, size_t len );

/**
 * @brief Returns the value of a string parameter
 *
 * @param[in]      request                       command request containing parameter
 * @param[in]      id                            parameter name
 * @param[out]     dest                          location to store result
 * @param[in]      len                           length of buffer
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                type doesn't match parameter or buffer too small
 * @retval WRA_STATUS_NOT_FOUND                  parameter id not found
 * @retval WRA_STATUS_NOT_INITIALIZED            parameter has not been set a value exists
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_command_parameter_string_set
 */
WRA_API wra_status_t wra_command_parameter_string_get( const wra_command_request_t* request, const char* id, char* dest, size_t len );

/**
 * @brief Sets the string value of a parameter
 *
 * @param[in]      request                       command request containing parameter
 * @param[in]      id                            parameter name
 * @param[out]     value                         value to set
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                type doesn't match parameter
 * @retval WRA_STATUS_NO_MEMORY                  unable to allocate memory for storage
 * @retval WRA_STATUS_NOT_FOUND                  parameter id not found
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_command_parameter_string_get
 */
WRA_API wra_status_t wra_command_parameter_string_set( wra_command_request_t* request, const char* id, const char* value );

/**
 * @brief Registers a command with a service
 *
 * @param[in]      service                       service to register the command with
 * @param[in]      command                       command to register
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for command registration
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for command registration
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_NO_MEMORY                  not enough system memory
 * @retval WRA_STATUS_NOT_INITIALIZED            library not initialized with the cloud
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for next request
 *
 * @see wra_command_deregister
 */
WRA_API wra_status_t wra_command_register( wra_service_t* service, wra_command_t* command, wra_millisecond_t max_time_out );

/**
 * @brief Adds the ability to call a function on command invocation
 *
 * @param[in]      command                       command to associate with
 * @param[in]      func                          function to call
 * @param[in]      user_data                     user data to pass to the function when called
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_command_register_script
 */
WRA_API wra_status_t wra_command_register_callback( wra_command_t* command, wra_command_callback_t* func, void* user_data );

/**
 * @brief Adds the ability to execute an external process on an command invocation
 *
 * @param[in]      command                       command to associate with
 * @param[in]      script                        script to execute whe command is triggered
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 *
 * @see wra_command_register_callback
 */
WRA_API wra_status_t wra_command_register_script( wra_command_t* command, const char* script );

/**
 * @brief Waits up to the specified time out for an incoming command to occur
 *
 * @param[in]      lib_handle                    handle to the cloud
 * @param[in]      max_time_out                  maximum time to wait in milliseconds for a command to arrive
 *
 * @note Setting the parameter @p max_time_out to a value of @p 0, will cause the function to wait
 *       forever for a command to arrive
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_BAD_REQUEST                received an invalid request from agent
 * @retval WRA_STATUS_NOT_EXECUTABLE             specified script not executable
 * @retval WRA_STATUS_NOT_FOUND                  command request received for command that isn't registered
 * @retval WRA_STATUS_NOT_INITIALIZED            command was not initialized with the library
 * @retval WRA_STATUS_SUCCESS                    on success
 * @retval WRA_STATUS_TIMED_OUT                  timed out while waiting for next request
 * @retval WRA_STATUS_FAILURE                    communication with agent failed
 *
 * @see wra_command_execute
 * @see wra_command_deregister
 */
WRA_API wra_status_t wra_command_wait( wra_t* lib_handle, wra_millisecond_t max_time_out );

/**
 * @brief sets the response field of the given request to a be of type boolean with the value specified in data
 *
 * @param[in,out]  request                       the request to add the response value to
 * @param[in]      id                            response field id
 * @param[in]      data                          the value of the response
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 */
WRA_API wra_status_t wra_command_response_boolean_set( wra_command_request_t* request, const char* id, wra_bool_t data );

/**
 * @brief sets the response field of the given request to a be of type integer with the value specified in data
 *
 * @param[in,out]  request                       the request to add the response value to
 * @param[in]      id                            response field id
 * @param[in]      data                          the value of the response
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 */
WRA_API wra_status_t wra_command_response_integer_set( wra_command_request_t* request, const char* id, int32_t data );

/**
 * @brief sets the response field of the given request to a be of type float (double) with the value specified in data
 *
 * @param[in,out]  request                       the request to add the response value to
 * @param[in]      id                            response field id
 * @param[in]      data                          the value of the response
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_SUCCESS                    on success
 */
WRA_API wra_status_t wra_command_response_float_set( wra_command_request_t* request, const char* id, double data );

/**
 * @brief sets the response field of the given request to a be of type string with the value specified in data
 *
 * @param[in,out]  request                       the request to add the response value to
 * @param[in]      id                            response field id
 * @param[in]      data                          the value of the response
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_NO_MEMORY                  not enough system memory
 * @retval WRA_STATUS_SUCCESS                    on success
 */
WRA_API wra_status_t wra_command_response_string_set( wra_command_request_t* request, const char* id, const char* data );

/**
 * @brief sets the response field of the given request to a be of type raw with the value specified in data
 *
 * @param[in,out]  request                       the request to add the response value to
 * @param[in]      id                            response field id
 * @param[in]      data                          the value of the response
 * @param[in]      length                        the length of the raw response data
 *
 * @retval WRA_STATUS_BAD_PARAMETER              invalid parameter passed to function
 * @retval WRA_STATUS_NO_MEMORY                  not enough system memory
 * @retval WRA_STATUS_SUCCESS                    on success
 */
WRA_API wra_status_t wra_command_response_raw_set( wra_command_request_t* request, const char* id, const void* data, size_t length );

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef WRA_COMMAND_H */

