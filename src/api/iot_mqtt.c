/**
 * @file
 * @brief source file defining common mqtt function implementations
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "public/iot_mqtt.h"

#include "shared/iot_defs.h"
#include "shared/iot_types.h"

#ifdef IOT_MQTT_MOSQUITTO
#include <mosquitto.h>
#else
#include <MQTTClient.h>
#endif

/** @brief Defualt MQTT port for non-SSL connections */
#define IOT_MQTT_PORT                  1883
/** @brief Default MQTT port for SSL connections */
#define IOT_MQTT_PORT_SSL              8883

/** @brief number of time MQTT initalize has been called */
static unsigned int MQTT_INIT_COUNT = 0u;

#ifdef IOT_MQTT_MOSQUITTO
/**
 * @brief callback called when a connection is terminated
 *
 * @param[in]      mosq                mosquitto instance calling the callback
 * @param[in]      user_data           user data provided in <mosquitto_new>
 * @param[in]      rc                  the reason for the disconnection
 */
static IOT_SECTION void iot_mqtt_on_disconnect(
	struct mosquitto *mosq,
	void *user_data,
	int rc );
/**
 * @brief callback called when a message is delivered
 *
 * @param[in]      mosq                mosquitto instance calling the callback
 * @param[in]      user_data           user data provided in <mosquitto_new>
 * @param[in]      msg_id              id of the message delivered
 */
static IOT_SECTION void iot_mqtt_on_delivery(
	struct mosquitto *mosq,
	void *user_data,
	int msg_id );
/**
 * @brief callback called when a message is received
 *
 * @param[in]      mosq                mosquitto instance calling the callback
 * @param[in]      user_data           user data provided in <mosquitto_new>
 * @param[in]      message             message being received
 */
static IOT_SECTION void iot_mqtt_on_message(
	struct mosquitto *mosq,
	void *user_data,
	const struct mosquitto_message *message );
/**
 * @brief callback called on a subscription
 *
 * @param[in]      mosq                mosquitto instance calling the callback
 * @param[in]      user_data           user data provided in <mosquitto_new>
 * @param[in]      msg_id              id of the message delivered
 * @param[in]      qos_count           the number of granted subscriptions
 *                                     (size of granted_qos).
 * @param[in]      granted_qos         an array of integers indicating the
 *                                     granted QoS for each of the subscriptions.
 */
static IOT_SECTION void iot_mqtt_on_subscribe(
	struct mosquitto *mosq,
	void *obj,
	int msg_id,
	int qos_count,
	const int *granted_qos );
/**
 * @brief callback called on a subscription
 *
 * @param[in]      mosq                mosquitto instance calling the callback
 * @param[in]      user_data           user data provided in <mosquitto_new>
 * @param[in]      level               the log message level from the values:
 *                                     MOSQ_LOG_INFO, MOSQ_LOG_NOTICE,
 *                                     MOSQ_LOG_WARNING, MOSQ_LOG_ERR,
 *                                     MOSQ_LOG_DEBUG
 * @param[in]      str                 the message string
 */
void iot_mqtt_on_log(
	struct mosquitto *mosq,
	void *obj,
	int level,
	const char *str );
#else
/**
 * @brief callback called when a connection is terminated
 *
 * @param[in]      user_data           context user data
 * @param[in]      cause               string indicating the reason
 */
static IOT_SECTION void iot_mqtt_on_disconnect(
	void *user_data,
	char *cause );
/**
 * @brief callback called when a message is delivered
 *
 * @param[in]      user_data           context user data
 * @param[in]      dt                  delievery token for the message
 */
static IOT_SECTION void iot_mqtt_on_delivery(
	void *user_data,
	MQTTClient_deliveryToken dt );
/**
 * @brief callback called when a message is received
 *
 * @param[in]      user_data           context user data
 * @param[in]      topic               topic message received on
 * @param[in]      topic_len           length of the topic string
 * @param[in]      message             message being received
 */
static IOT_SECTION int iot_mqtt_on_message(
	void *user_data,
	char *topic,
	int topic_len,
	MQTTClient_message *message );
#endif

/** @brief number of seconds before sending a keep alive message */
#define IOT_MQTT_KEEP_ALIVE            240
/** @brief maximum length for an mqtt connection url */
#define IOT_MQTT_URL_MAX               64u

/** @brief internal object containing information for managing the connection */
struct iot_mqtt
{
#ifdef IOT_MQTT_MOSQUITTO
	/** @brief pointer to the mosquitto client instance */
	struct mosquitto *mosq;
#else
	/** @brief paho client instance */
	MQTTClient client;
	/** @brief whether the client is expected to be connected */
	iot_bool_t connected;
#endif
	/** @brief callback to call when a disconnection is detected */
	iot_mqtt_disconnect_callback_t on_disconnect;
	/** @brief callback to call when a message is delivered */
	iot_mqtt_delivery_callback_t   on_delivery;
	/** @brief callback to call when a message is received */
	iot_mqtt_message_callback_t on_message;
	/** @brief user specified data to pass to callbacks */
	void * user_data;
};

iot_mqtt_t* iot_mqtt_connect(
	const char *client_id,
	const char *host,
	iot_uint16_t port,
	iot_mqtt_ssl_t *ssl_conf,
	const char *username,
	const char *password,
	iot_millisecond_t max_time_out )
{
	iot_mqtt_t *result = NULL;
	if ( host && client_id )
	{
		if ( port == 0u )
		{
			if ( ssl_conf )
				port = IOT_MQTT_PORT_SSL;
			else
				port = IOT_MQTT_PORT;
		}

		result = (iot_mqtt_t*)os_malloc( sizeof( struct iot_mqtt ) );
		if ( result )
		{
#ifdef IOT_MQTT_MOSQUITTO
			(void)max_time_out;
			os_memzero( result, sizeof( struct iot_mqtt ) );
			result->mosq = mosquitto_new( client_id, true, result );
			if ( result->mosq )
			{
				mosquitto_disconnect_callback_set( result->mosq,
					iot_mqtt_on_disconnect );
				mosquitto_message_callback_set( result->mosq,
					iot_mqtt_on_message );
				mosquitto_publish_callback_set( result->mosq,
					iot_mqtt_on_delivery );
				mosquitto_subscribe_callback_set( result->mosq,
					iot_mqtt_on_subscribe );
				mosquitto_log_callback_set( result->mosq,
					iot_mqtt_on_log );
				if ( username && password )
					mosquitto_username_pw_set( result->mosq,
						username, password );
				if ( ssl_conf && port != 1883u )
				{
					mosquitto_tls_set( result->mosq,
						ssl_conf->ca_path, NULL,
						ssl_conf->cert_file,
						ssl_conf->key_file, NULL );
					mosquitto_tls_insecure_set(
						result->mosq,
						ssl_conf->insecure );
				}

				if ( mosquitto_connect( result->mosq,
					host, port, IOT_MQTT_KEEP_ALIVE ) == MOSQ_ERR_SUCCESS )
					mosquitto_loop_start( result->mosq );
				else
					os_free_null( (void**)&result );
			}
			else
				os_free_null( (void**)&result );
#else /* ifdef IOT_MQTT_MOSQUITTO */
			char url[IOT_MQTT_URL_MAX + 1u];
			if ( ssl_conf && port != 1883u )
				os_snprintf( url, IOT_MQTT_URL_MAX,
					"ssl://%s:%d", host, port );
			else
				os_snprintf( url, IOT_MQTT_URL_MAX,
					"tcp://%s:%d", host, port );
			url[ IOT_MQTT_URL_MAX ] = '\0';
			os_memzero( result, sizeof( struct iot_mqtt ) );

			if ( MQTTClient_create( &result->client, url,
				client_id, MQTTCLIENT_PERSISTENCE_NONE,
				NULL ) == MQTTCLIENT_SUCCESS )
			{
				MQTTClient_connectOptions conn_opts =
					MQTTClient_connectOptions_initializer;

				/* ssl */
				MQTTClient_SSLOptions ssl_opts =
					MQTTClient_SSLOptions_initializer;

				MQTTClient_setCallbacks( result->client, result,
					iot_mqtt_on_disconnect, iot_mqtt_on_message,
					iot_mqtt_on_delivery );
				conn_opts.keepAliveInterval = IOT_MQTT_KEEP_ALIVE;
				conn_opts.cleansession = 1;
				conn_opts.username = username;
				conn_opts.password = password;

				if ( ssl_conf && port != 1883u )
				{ /* ssl */
					/* ssl args */
					ssl_opts.trustStore = ssl_conf->ca_path;
					ssl_opts.enableServerCertAuth =
						!ssl_conf->insecure;

					/* client cert and key */
					ssl_opts.keyStore = ssl_conf->cert_file;
					ssl_opts.privateKey = ssl_conf->key_file;

					conn_opts.ssl = &ssl_opts;
				}
				if ( max_time_out > 0u )
					conn_opts.connectTimeout =
						(max_time_out /
						IOT_MILLISECONDS_IN_SECOND) + 1u;
				if ( MQTTClient_connect( result->client,
						&conn_opts ) == MQTTCLIENT_SUCCESS )
					result->connected = IOT_TRUE;
				else
				{
					MQTTClient_destroy( &result->client );
					os_free_null( (void**)&result );
				}
			}
			else
				os_free_null( (void**)&result );
#endif /* else IOT_MQTT_MOSQUITTO */
		}
	}
	return result;
}

iot_status_t iot_mqtt_disconnect(
	iot_mqtt_t* mqtt )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( mqtt )
	{
		result = IOT_STATUS_FAILURE;
#ifdef IOT_MQTT_MOSQUITTO
		if ( mosquitto_disconnect( mqtt->mosq ) == MOSQ_ERR_SUCCESS )
			result = IOT_STATUS_SUCCESS;
		mosquitto_loop_stop( mqtt->mosq, true );
		mosquitto_destroy( mqtt->mosq );
		mqtt->mosq = NULL;
#else /* ifdef IOT_MQTT_MOSQUITTO */
		mqtt->connected = IOT_FALSE;
		if ( MQTTClient_disconnect( mqtt->client, IOT_MQTT_KEEP_ALIVE )  == MQTTCLIENT_SUCCESS )
			result = IOT_STATUS_SUCCESS;

		MQTTClient_destroy( &mqtt->client );
#endif /* else IOT_MQTT_MOSQUITTO */
		os_free( mqtt );
	}
	return result;
}

iot_status_t iot_mqtt_initialize( void )
{
	if ( MQTT_INIT_COUNT == 0u )
	{
#ifdef IOT_MQTT_MOSQUITTO
		mosquitto_lib_init();
#endif /* ifdef IOT_MQTT_MOSQUITTO */
	}
	++MQTT_INIT_COUNT;
	return IOT_STATUS_SUCCESS;
}

#ifdef IOT_MQTT_MOSQUITTO
void iot_mqtt_on_disconnect(
	struct mosquitto *UNUSED(mosq),
	void *user_data,
	int rc )
{
	iot_mqtt_t *const mqtt = (iot_mqtt_t *)user_data;
	if ( mqtt && mqtt->on_disconnect )
	{
		const iot_bool_t unexpected = rc ? IOT_TRUE : IOT_FALSE;
		mqtt->on_disconnect( mqtt->user_data, unexpected );
	}
}

void iot_mqtt_on_delivery(
	struct mosquitto *UNUSED(mosq),
	void *user_data,
	int msg_id )
{
	iot_mqtt_t *const mqtt = (iot_mqtt_t *)user_data;
	if ( mqtt && mqtt->on_disconnect )
		mqtt->on_delivery( mqtt->user_data, msg_id );
}

void iot_mqtt_on_message(
	struct mosquitto *UNUSED(mosq),
	void *user_data,
	const struct mosquitto_message *message )
{
	iot_mqtt_t *const mqtt = (iot_mqtt_t *)user_data;
	if ( mqtt && mqtt->on_message )
		mqtt->on_message( mqtt->user_data, message->topic,
		message->payload, (size_t)message->payloadlen, message->qos,
		message->retain );
}

void iot_mqtt_on_subscribe(
	struct mosquitto *UNUSED(mosq),
	void *UNUSED(obj),
	int UNUSED(msg_id),
	int UNUSED(qos_count),
	const int *UNUSED(granted_qos) )
{
}

void iot_mqtt_on_log(
	struct mosquitto *UNUSED(mosq),
	void *UNUSED(obj),
	int UNUSED(level),
	const char *UNUSED(str) )
{
}


#else /* ifdef IOT_MQTT_MOSQUITTO */
void iot_mqtt_on_disconnect(
	void *user_data,
	char *UNUSED(cause) )
{
	iot_mqtt_t *const mqtt = (iot_mqtt_t *)user_data;
	if ( mqtt && mqtt->on_disconnect )
	{
		const iot_bool_t unexpected = mqtt->connected;
		mqtt->connected = IOT_FALSE;
		mqtt->on_disconnect( mqtt->user_data, unexpected );
	}
}

void iot_mqtt_on_delivery(
	void *user_data,
	MQTTClient_deliveryToken dt )
{
	iot_mqtt_t *const mqtt = (iot_mqtt_t *)user_data;
	if ( mqtt && mqtt->on_delivery )
		mqtt->on_delivery( mqtt->user_data, (int)dt );
}

int iot_mqtt_on_message(
	void *user_data,
	char *topic,
	int UNUSED(topic_len),
	MQTTClient_message *message )
{
	iot_mqtt_t *const mqtt = (iot_mqtt_t *)user_data;
	if ( mqtt && mqtt->on_message )
		mqtt->on_message( mqtt->user_data, topic, message->payload,
		(size_t)message->payloadlen, message->qos, message->retained );

	/* message succesfully handled */
	return (int)IOT_TRUE;
}
#endif /* else IOT_MQTT_MOSQUITTO */

iot_status_t iot_mqtt_publish(
	iot_mqtt_t *mqtt,
	const char *topic,
	const void *payload,
	size_t payload_len,
	int qos,
	iot_bool_t retain,
	int *msg_id )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	int mid = 0;
	qos = 1;
	if ( mqtt )
	{
#ifdef IOT_MQTT_MOSQUITTO
		result = IOT_STATUS_FAILURE;
		if ( mosquitto_publish( mqtt->mosq, &mid, topic, payload_len,
			payload, qos, retain ) == MOSQ_ERR_SUCCESS )
			result = IOT_STATUS_SUCCESS;
	
#else /* ifdef IOT_MQTT_MOSQUITTO */
		MQTTClient_deliveryToken dt;
		void *pl = os_malloc( payload_len );
		result = IOT_STATUS_NO_MEMORY;
		if ( pl )
		{
			os_memcpy( pl, payload, payload_len );
			result = IOT_STATUS_FAILURE;
			if ( MQTTClient_publish( mqtt->client, topic, payload_len,
				pl, qos, retain, &dt ) == MQTTCLIENT_SUCCESS )
			{
				mid = (int)dt;
				result = IOT_STATUS_SUCCESS;
			}
			os_free( pl );
		}
#endif /* else IOT_MQTT_MOSQUITTO */
	}
	if ( msg_id ) *msg_id = mid;
	return result;
}

iot_status_t iot_mqtt_set_disconnect_callback(
	iot_mqtt_t *mqtt,
	iot_mqtt_disconnect_callback_t cb )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( mqtt )
	{
		mqtt->on_disconnect = cb;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_mqtt_set_delivery_callback(
	iot_mqtt_t *mqtt,
	iot_mqtt_delivery_callback_t cb )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( mqtt )
	{
		mqtt->on_delivery = cb;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_mqtt_set_message_callback(
	iot_mqtt_t *mqtt,
	iot_mqtt_message_callback_t cb )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( mqtt )
	{
		mqtt->on_message = cb;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_mqtt_set_user_data(
	iot_mqtt_t *mqtt,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( mqtt )
	{
		mqtt->user_data = user_data;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_mqtt_subscribe( iot_mqtt_t *mqtt, const char *topic, int qos )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( mqtt )
	{
		result = IOT_STATUS_FAILURE;
#ifdef IOT_MQTT_MOSQUITTO
		if ( mosquitto_subscribe( mqtt->mosq, NULL, topic, qos ) == MOSQ_ERR_SUCCESS )
			result = IOT_STATUS_SUCCESS;
#else /* ifdef IOT_MQTT_MOSQUITTO */
		if ( MQTTClient_subscribe( mqtt->client, topic, qos ) == MQTTCLIENT_SUCCESS )
			result = IOT_STATUS_SUCCESS;
#endif /* else IOT_MQTT_MOSQUITTO */
	}
	return result;
}

iot_status_t iot_mqtt_terminate( void )
{
	--MQTT_INIT_COUNT;
	if ( MQTT_INIT_COUNT == 0u )
	{
#ifdef IOT_MQTT_MOSQUITTO
		mosquitto_lib_cleanup();
#endif /* ifdef IOT_MQTT_MOSQUITTO */
	}
	return IOT_STATUS_SUCCESS;
}

iot_status_t iot_mqtt_unsubscribe( iot_mqtt_t *mqtt, const char *topic )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( mqtt )
	{
		result = IOT_STATUS_FAILURE;
#ifdef IOT_MQTT_MOSQUITTO
		if ( mosquitto_unsubscribe( mqtt->mosq, NULL, topic ) == MOSQ_ERR_SUCCESS )
			result = IOT_STATUS_SUCCESS;
#else /* ifdef IOT_MQTT_MOSQUITTO */
		if ( MQTTClient_unsubscribe( mqtt->client, topic ) == MQTTCLIENT_SUCCESS )
			result = IOT_STATUS_SUCCESS;
#endif /* else IOT_MQTT_MOSQUITTO */
	}
	return result;
}
