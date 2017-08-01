/**
 * @file
 * @brief header file declaring common mqtt function implementations
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */
#ifndef IOT_MQTT_H
#define IOT_MQTT_H

#include <iot.h>

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/**
 * @brief Structure for passing in mqtt SSL configuration information
 */
typedef struct iot_mqtt_ssl
{
	/** @brief client certificate file path */
	const char *cert_file;
	/** @breif client key file path */
	const char *key_file;
	/** @brief client certificate authority file or directory path */
	const char *ca_path;
	/** @brief if true, allow connections to privately signed certificates */
	iot_bool_t insecure;
} iot_mqtt_ssl_t;

/**
 * @brief internal MQTT structure
 */
typedef struct iot_mqtt iot_mqtt_t;

/** @brief signature of function to be called when a disconnection is detected */
typedef void (*iot_mqtt_disconnect_callback_t)(
	void *user_data,
	iot_bool_t unexpected );

/** @brief signature of function to be called when a message is delivered */
typedef void (*iot_mqtt_delivery_callback_t)(
	void *user_data,
	int msg_id );

/**
 * @brief signature of function to be called when a message is received
 */
typedef void (*iot_mqtt_message_callback_t)(
	void *user_data,
	const char *topic,
	void *payload,
	size_t payload_len,
	int qos,
	iot_bool_t retain );

/**
 * @brief connects to an MQTT server
 *
 * @param[in]      client_id           id of the client
 * @param[in]      host                host server to connect to
 * @param[in]      port                (optional) port to connect on
 * @param[in]      ssl_conf            (optional) secure connection information
 * @param[in]      username            user name to connect with
 * @param[in]      password            password to connect with
 * @param[in]      max_time_out        maximum time to wait
 *                                     (0 = wait indefinitely)
 *
 * @retval         NULL                failed to connect
 * @retval         !NULL               successful connection established
 */
IOT_API IOT_SECTION iot_mqtt_t* iot_mqtt_connect(
	const char *client_id,
	const char *host,
	iot_uint16_t port,
	iot_mqtt_ssl_t *ssl_conf,
	const char *username,
	const char *password,
	iot_millisecond_t max_time_out );

/**
 * @brief disconnects from an MQTT server
 *
 * @param[in]      mqtt                MQTT object to disconnect
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          operation failed
 * @retval IOT_STATUS_SUCCESS          operation successful
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_disconnect(
	iot_mqtt_t* mqtt
	);

/**
 * @brief initializes MQTT functionality
 *
 * @retval IOT_STATUS_SUCCESS                    always
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_initialize( void );

/**
 * @brief publishes a message to the mqtt connection to a specified topic
 *
 * @param[in]      mqtt                MQTT object to transmit to
 * @param[in]      topic               topic to transmit on
 * @param[in]      payload             message to transmit
 * @param[in]      payload_len         size of message
 * @param[in]      qos                 MQTT QOS level to use (0, 1 or 3)
 * @param[in]      retain              retain the message
 * @param[out]     msg_id              message id assigned to the message
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          operation failed
 * @retval IOT_STATUS_SUCCESS          operation successful
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_publish(
	iot_mqtt_t *mqtt,
	const char *topic,
	const void *payload,
	size_t payload_len,
	int qos,
	iot_bool_t retain,
	int *msg_id );

/**
 * @brief sets the callback for notification of a disconnection
 *
 * @param[in]      mqtt                MQTT object to set callback on
 * @param[in]      cb                  call back to be set
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_SUCCESS          operation successful
 *
 * @see iot_mqtt_set_delivery_callback
 * @see iot_mqtt_set_message_callback
 * @see iot_mqtt_set_user_data
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_set_disconnect_callback(
	iot_mqtt_t *mqtt,
	iot_mqtt_disconnect_callback_t cb );

/**
 * @brief sets the callback for notification of a message delivery
 *
 * @param[in]      mqtt                MQTT object to set callback on
 * @param[in]      cb                  call back to be set
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_SUCCESS          operation successful
 *
 * @see iot_mqtt_set_disconnect_callback
 * @see iot_mqtt_set_message_callback
 * @see iot_mqtt_set_user_data
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_set_delivery_callback(
	iot_mqtt_t *mqtt,
	iot_mqtt_delivery_callback_t cb );

/**
 * @brief sets the callback for receiving incoming messages
 *
 * @param[in]      mqtt                MQTT object to set callback on
 * @param[in]      cb                  call back to be set
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_SUCCESS          operation successful
 *
 * @see iot_mqtt_set_disconnect_callback
 * @see iot_mqtt_set_delivery_callback
 * @see iot_mqtt_set_user_data
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_set_message_callback(
	iot_mqtt_t *mqtt,
	iot_mqtt_message_callback_t cb );

/**
 * @brief sets the user data to pass when a callback is called
 *
 * @param[in]      mqtt                MQTT object to set callback on
 * @param[in]      user_data           user data to pass to callbacks
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_SUCCESS          operation successful
 *
 * @see iot_mqtt_set_disconnect_callback
 * @see iot_mqtt_set_delivery_callback
 * @see iot_mqtt_set_message_callback
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_set_user_data(
	iot_mqtt_t *mqtt,
	void *user_data );

/**
 * @brief subscribes for messages on an MQTT topic
 *
 * @param[in]      mqtt                MQTT object to subscribe to
 * @param[in]      topic               topic to subscribe for (may include wildcards)
 * @param[in]      qos                 MQTT QOS level to use (0, 1 or 3)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          operation failed
 * @retval IOT_STATUS_SUCCESS          operation successful
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_subscribe(
	iot_mqtt_t *mqtt,
	const char *topic,
	int qos );

/**
 * @brief terminates MQTT functionality
 *
 * @retval IOT_STATUS_SUCCESS                    always
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_terminate( void );

/**
 * @brief unsubscribes from an MQTT topic
 *
 * @param[in]      mqtt                MQTT object to unsubscribe from
 * @param[in]      topic               topic to unsubscribe for (may include wildcards)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          operation failed
 * @retval IOT_STATUS_SUCCESS          operation successful
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_unsubscribe(
	iot_mqtt_t *mqtt,
	const char *topic );

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */
#endif /* ifndef IOT_MQTT_H */
