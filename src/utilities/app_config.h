/**
 * @file
 * @brief Wind River IoT configuration file reader header file
 *
 * @copyright Copyright (C) 2016-2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "../api/public/iot.h"            /* for iot_status_t */
#include "../api/public/iot_mqtt.h"      /* for iot_status_t */
#include "../api/shared/iot_types.h"      /* for proxy structures */

/** @brief proxy config path */


/** @brief structure containing configuration file information */
struct app_config;

/**
 * @brief closes a configuration file
 *
 * @param[in]      config              configuration file object to close
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_SUCCESS          on success
 */

IOT_SECTION iot_status_t app_config_close( struct app_config *config );

/**
 * @brief opens a configuration file for reading
 *
 * @param[in]      iot_lib             library handle (for logging)
 * @param[in]      file_path           file path to open
 *
 * @return a configuration file object
 */
IOT_SECTION struct app_config *app_config_open(
	/* iot_t *iot_lib, */ const char *file_path );

/**
 * @brief reads a boolean from a configuration file
 *
 * @param[in]      config              configuration file object
 * @param[in]      group               group containing field (optional)
 * @param[in]      field               field to search for
 * @param[out]     value               output destination
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_NOT_FOUND        field not found
 * @retval IOT_STATUS_SUCCESS          on success
 */
IOT_SECTION iot_status_t app_config_read_boolean(
	struct app_config *config, const char *group,
	const char *field, iot_bool_t *value );

/**
 * @brief reads a string from a configuration file
 *
 * @param[in]      config              configuration file object
 * @param[in]      group               group containing field (optional)
 * @param[in]      field               field to search for
 * @param[out]     value               output destination
 * @param[out]     str_len             length of read string
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_NOT_FOUND        field not found
 * @retval IOT_STATUS_SUCCESS          on success
 */
IOT_SECTION iot_status_t app_config_read_string(
	struct app_config *config, const char *group, const char *field,
	const char **value, size_t* str_len );

/**
 * @brief reads an array of strings from a configuration file
 *
 * @param[in]      config              configuration file object
 * @param[in]      group               group containing field (optional)
 * @param[in]      field               field to search for
 * @param[in]      token               token used to split items in result
 * @param[out]     value               output destination
 * @param[in]      max                 output destination maximum size
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_NOT_FOUND        field not found
 * @retval IOT_STATUS_SUCCESS          on success
 */
IOT_SECTION iot_status_t app_config_read_string_array( struct app_config *config,
	const char *group, const char *field, char token, char *value,
	size_t max );

/**
 * @brief reads an integer from a configuration file
 *
 * @param[in]      config              configuration file object
 * @param[in]      group               group containing field (optional)
 * @param[in]      field               field to search for
 * @param[out]     value               output destination
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_NOT_FOUND        field not found
 * @retval IOT_STATUS_SUCCESS          on success
 */
IOT_SECTION iot_status_t app_config_read_integer( struct app_config *config,
	const char *group, const char *field, iot_int64_t *value );

/**
 * @brief reads proxy configuration file
 *
 * @param[out]     proxy_info          pointer to proxy info struct to read
 *                                     from a file
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
IOT_SECTION iot_status_t app_config_read_proxy_file(
	struct iot_proxy *proxy_info );

/**
 * @brief reads a json array from the top level of a json file, changing the context
 * of all other operations with relation to the array - does not support recursive 
 * operations i.e. cannot read a json array of json objects within a json array
 *
 * @param[in]     config         	   configuration file object
 * @param[in]     field                field to search for
 *
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
IOT_SECTION iot_status_t app_config_read_json_array_start( struct app_config *config,
	const char *field );


/**
 * @brief changes read json context back to the root json object and ends an array operation
 *
 * @param[in]       config			   configuration file object
 *
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 */
IOT_SECTION iot_status_t app_config_read_json_array_end( struct app_config *config );

/**
 * @brief iterates and changes json context to the next json object in the array
 *
 * @param[in]       config			   configuration file object
 *
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 */
IOT_SECTION iot_status_t app_config_read_json_array_next( struct app_config *config );

/**
 * @brief writes network settings to connection configuration file
 *
 * @param[in]      proxy_info          pointer to proxy info struct to write
 *                                     to a file
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
/*FIXME*/
/*iot_status_t app_config_write_proxy_file(*/
/*const struct iot_proxy *proxy_info );*/

#endif /* ifndef APP_CONFIG_H */
