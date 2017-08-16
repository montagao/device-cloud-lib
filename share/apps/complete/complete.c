/**
 * @brief Source file for the complete example app.
 *
 * @copyright Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include <iot.h>

#include <ctype.h>   /* for alnum */
#include <stdlib.h>  /* for EXIT_SUCCESS & EXIT_FAILURE */
#include <string.h>  /* for strncpy */
#include <stdio.h>   /* for printf */
#include <signal.h>  /* for SIGINT */
#include <time.h>    /* for time, srand, rand */
#ifdef _WIN32
#	include <Windows.h>  /* for Sleep */
#else
#	include <unistd.h>   /* for sleep */
#	include <errno.h>    /* for errno */
#endif

/** @brief maximum value of json integer in range [-(2**53)+1, (2**53)-1] */
#define MAX_JSON_INT 9007199254740991u
/** @brief Default maximum number of iterations before stopping telemetry */
#define MAX_LOOP_ITERATIONS 360
/** @brief Number of milliseconds in a second */
#define MILLISECONDS_IN_SECOND     1000u
/** @brief minimum value of json integer in range [-(2**53)+1, (2**53)-1] */
#define MIN_JSON_INT -9007199254740991
/** @brief Default wait time between sending samples */
#define POLL_INTERVAL_SEC 10u
/** @brief max path length */
#define SCRIPT_PATH_MAX 512u
/** @brief Tag max length */
#define TAG_MAX_LEN 128u

#ifdef _WIN32
	/** @brief directory separator */
	#define DIR_SEP '\\'
	/** @brief test script's name */
	#define TEST_SCRIPT "test_script.cmd"
#else
	/** @brief directory separator */
	#define DIR_SEP '/'
	/** @brief test script's name */
	#define TEST_SCRIPT "test_script.sh"
#endif

/* Set up named parameters for a sample action to validate actions with
 * parameters */
#define PARAM_NAME_STR                 "param_str"
#define PARAM_NAME_INT                 "param_int"
#define PARAM_NAME_FLOAT               "param_float"
#define PARAM_NAME_BOOL                "param_bool"
#define PARAM_OUT_NAME_BOOL            "param_out"

#ifdef _WRS_KERNEL
	/**
	 * @brief Redefine main function to prevent name collision in global
	 * scope
	 */
#	define main      app_complete_main
#endif

static iot_bool_t running = IOT_TRUE;
static iot_telemetry_t *telemetry_temp = NULL;
static iot_telemetry_t *telemetry_light = NULL;

/* add one of each telemetry type to test */
static iot_telemetry_t *telemetry_boolean = NULL;
static iot_telemetry_t *telemetry_location = NULL;
static iot_telemetry_t *telemetry_string = NULL;
static iot_telemetry_t *telemetry_int8 = NULL;
static iot_telemetry_t *telemetry_int16 = NULL;
static iot_telemetry_t *telemetry_int32 = NULL;
static iot_telemetry_t *telemetry_int64 = NULL;
static iot_telemetry_t *telemetry_uint8 = NULL;
static iot_telemetry_t *telemetry_uint16 = NULL;
static iot_telemetry_t *telemetry_uint32 = NULL;
static iot_telemetry_t *telemetry_uint64 = NULL;
static iot_telemetry_t *telemetry_raw = NULL;

static iot_alarm_t *alarm_test = NULL;

static iot_action_t *enable_location_cmd = NULL;
static iot_action_t *enable_telemetry_cmd = NULL;
static iot_action_t *script_cmd = NULL;
static iot_action_t *test_params_cmd = NULL;
static iot_bool_t send_location = IOT_FALSE;
static iot_bool_t send_telemetry = IOT_FALSE;

/**
 * @brief Sleep handler
 *
 * @param[in]      delay              Length of time in seconds to delay.
 */
static void do_sleep( unsigned int delay );

/**
 * @brief Random string generation handler.
 *
 * @param[out]     dest                Buffer to store random string.
 * @param[in]      length              Size of the random string to generate.
 */
static void generate_random_string( char *dest, size_t length );

/**
 * @brief Send telemetry data to the agent
 *
 * @return a handle to the iot library
 */
static iot_t *initialize( void );

/**
 * @brief Function called to handle log messages generated from the iot library
 *
 * @param[in]      log_level           message level
 * @param[in]      log_source          source that generated the message
 * @param[in]      message             message that was generated
 * @param[in]      user_data           user specific data
 */
static void log_handler( iot_log_level_t log_level, iot_log_source_t *log_source,
	const char *message, void *user_data );

/**
 * @brief Callback function for enabling & disabling sending location
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_SUCCESS          always
 */
static iot_status_t on_enable_disable_location(
	iot_action_request_t* request, void* user_data );

/**
 * @brief Callback function for enabling & disabling sending telemetery
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_SUCCESS          always
 */
static iot_status_t on_enable_disable_telemetry(
	iot_action_request_t* request, void* user_data );

/**
 * @brief Callback function for testing parameters
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t on_action_test_parameters(
	iot_action_request_t* request,
	void* user_data );

/**
 * @brief Random number generator
 *
 * @param[in]      min                 minimum number to generate
 * @param[in]      max                 maximum number to generate
 *
 * @return a randomly generated number
 */
static double random_num( double min, double max );

/**
 * @brief Send location data to the agent.
 *
 * @param[in]      iot_lib             iot library handle
 */
static void send_location_sample( iot_t *iot_lib );

/**
 * @brief Send telemetry data to the agent.
 *
 * @param[in]      iot_lib             iot library handle
 */
static void send_telemetry_sample( iot_t *iot_lib );

/**
 * @brief Handles terminatation signal and tears down gracefully
 *
 * @param[in]      signo                         Signal to trap.
 */
static void sig_handler( int signo );

/**
 * @brief Text sample used for random string generation.
 */
static const char text_buffer[] =
	"Arma virumque cano, Troiae qui primus ab oris "
	"Italiam, fato profugus, Laviniaque venit "
	"litora, multum ille et terris iactatus et alto "
	"vi superum saevae memorem Iunonis ob iram;"
	"multa quoque et bello passus, dum conderet urbem,"
	"inferretque deos Latio, genus unde Latinum,"
	"Albanique patres, atque altae moenia Romae.";

/**
 * @brief Text sample length.
 */
#define MAX_TEXT_SIZE 64

void generate_random_string( char *dest, size_t length )
{
#define MAX_WORDS 64
	const char *str;
	unsigned int word_count = 0u;
	const char *words[MAX_WORDS];

	/* setup array containing pointer to words */
	str = text_buffer;
	while ( *str != '\0' && word_count < MAX_WORDS )
	{
		/* skip all no word characters */
		while ( !isalnum( *str ) ) ++str;
		if ( *str != '\0' )
		{
			words[word_count] = str;
			++word_count;
			while ( isalnum( *str ) ) ++str;
		}
	}

	/* copy words into buffer */
	while( length > 1u )
	{
		double index;
		const char *word;

		index = (unsigned int)random_num( 0, word_count - 1u );
		word = words[(unsigned int)index];
		while( isalnum( *word ) && ( length-- > 1u ) )
			*dest++ = *word++;

		if ( length > 1u )
		{
			*dest++ = ' ';
			length--;
		}
	}
	*dest = '\0';
}

static iot_t* initialize( void )
{
	iot_t* iot_lib;
	iot_status_t status = IOT_STATUS_FAILURE;

	iot_lib = iot_initialize( "complete-app", NULL, 0 );
	iot_log_callback_set( iot_lib, &log_handler, NULL );
	status = iot_connect( iot_lib, 0u );
	if ( status == IOT_STATUS_SUCCESS )
	{
		char *script_name_location = NULL;
		char script_path[SCRIPT_PATH_MAX];
		memset( script_path, 0, SCRIPT_PATH_MAX );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "%s", "Connected" );

		/* Find script path from the full path of the binary.
		 * Script will be found in the same directory as the binary */
#if defined( _WIN32 )
		GetModuleFileNameA( NULL, script_path, SCRIPT_PATH_MAX );
#else
		if ( readlink("/proc/self/exe", script_path, SCRIPT_PATH_MAX) < IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib, IOT_LOG_ERROR, "Failed to readlink. Reason: %s", strerror(errno) );
#endif
		script_path[ SCRIPT_PATH_MAX - 1u ] = '\0';
		if ( (script_name_location = strrchr( script_path, DIR_SEP )) != NULL )
		{
			script_name_location++;
			strncpy( script_name_location, TEST_SCRIPT, SCRIPT_PATH_MAX - ( script_name_location - script_path ) - 1u );
		}
		script_path[SCRIPT_PATH_MAX - 1u] = '\0';

		/* Allocate telemetry items */
		telemetry_light = iot_telemetry_allocate( iot_lib,
			"light", IOT_TYPE_FLOAT64 );
		telemetry_temp = iot_telemetry_allocate( iot_lib,
			"temp", IOT_TYPE_FLOAT32 );
		iot_telemetry_option_set( telemetry_temp,
			"udmp:units", IOT_TYPE_STRING, "Celsius" );
		telemetry_boolean = iot_telemetry_allocate( iot_lib,
			"bool", IOT_TYPE_BOOL );
		telemetry_string = iot_telemetry_allocate( iot_lib,
			"string", IOT_TYPE_STRING );
		telemetry_int8 = iot_telemetry_allocate( iot_lib,
			"int8", IOT_TYPE_INT8 );
		telemetry_int16 = iot_telemetry_allocate( iot_lib,
			"int16", IOT_TYPE_INT16 );
		telemetry_int32 = iot_telemetry_allocate( iot_lib,
			"int32", IOT_TYPE_INT32 );
		telemetry_int64 = iot_telemetry_allocate( iot_lib,
			"int64", IOT_TYPE_INT64 );
		telemetry_uint8 = iot_telemetry_allocate( iot_lib,
			"uint8", IOT_TYPE_UINT8 );
		telemetry_uint16 = iot_telemetry_allocate( iot_lib,
			"uint16", IOT_TYPE_UINT16 );
		telemetry_uint32 = iot_telemetry_allocate( iot_lib,
			"uint32", IOT_TYPE_UINT32 );
		telemetry_uint64 = iot_telemetry_allocate( iot_lib,
			"uint64", IOT_TYPE_UINT64 );
		telemetry_raw = iot_telemetry_allocate( iot_lib,
			"raw", IOT_TYPE_RAW );
		telemetry_location = iot_telemetry_allocate( iot_lib,
			"location", IOT_TYPE_LOCATION );

		alarm_test = iot_alarm_register( iot_lib, "alarm_test" );

		/* Register telemetry items */
		IOT_LOG( iot_lib, IOT_LOG_INFO, "Registering telemetry: %s",
			"light" );
		iot_telemetry_register( telemetry_light, NULL, 0u );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "Registering telemetry: %s",
			"temp" );
		iot_telemetry_register( telemetry_temp, NULL, 0u );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "Registering telemetry: %s",
			"bool" );
		iot_telemetry_register( telemetry_boolean, NULL, 0u );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "Registering telemetry: %s",
			"string" );
		iot_telemetry_register( telemetry_string, NULL, 0u );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "registering telemetry: %s",
			"int8" );
		iot_telemetry_register( telemetry_int8, NULL, 0u );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "registering telemetry: %s",
			"int16" );
		iot_telemetry_register( telemetry_int16, NULL, 0u );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "registering telemetry: %s",
			"int32" );
		iot_telemetry_register( telemetry_int32, NULL, 0u );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "registering telemetry: %s",
			"int64" );
		iot_telemetry_register( telemetry_int64, NULL, 0u );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "registering telemetry: %s",
			"uint8" );
		iot_telemetry_register( telemetry_uint8, NULL, 0u );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "registering telemetry: %s",
			"uint16" );
		iot_telemetry_register( telemetry_uint16, NULL, 0u );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "registering telemetry: %s",
			"uint32" );
		iot_telemetry_register( telemetry_uint32, NULL, 0u );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "registering telemetry: %s",
			"uint64" );
		iot_telemetry_register( telemetry_uint64, NULL, 0u );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "Registering telemetry: %s",
			"raw" );
		iot_telemetry_register( telemetry_raw, NULL, 0u );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "Registering location : %s",
			"location" );
		iot_telemetry_register( telemetry_location, NULL, 0u );

		/* Allocate actions */
		IOT_LOG( iot_lib, IOT_LOG_INFO, "Registering action: %s",
			"telemetry_ON_OFF" );
		enable_telemetry_cmd = iot_action_allocate( iot_lib,
			"telemetry_ON_OFF" );
		status = iot_action_register_callback( enable_telemetry_cmd,
			&on_enable_disable_telemetry, NULL, NULL, 0u );
		if ( status != IOT_STATUS_SUCCESS )
		{
			IOT_LOG( iot_lib, IOT_LOG_ERROR,
				"Failed to register telemetry_ON_OFF command. "
				"Reason: %s",
				iot_error( status ) );
		}
		IOT_LOG( iot_lib, IOT_LOG_INFO, "Registering action: %s",
			"location_ON_OFF" );
		enable_location_cmd = iot_action_allocate( iot_lib,
			"location_ON_OFF" );
		status = iot_action_register_callback( enable_location_cmd,
			&on_enable_disable_location, NULL, NULL, 0u );
		if ( status != IOT_STATUS_SUCCESS )
		{
			IOT_LOG( iot_lib, IOT_LOG_ERROR,
				"Failed to register location_ON_OFF command. "
				"Reason: %s",
				iot_error( status ) );
		}
		IOT_LOG( iot_lib, IOT_LOG_INFO, "Registering action: %s",
			"test_script" );
		script_cmd = iot_action_allocate( iot_lib,
			"test_script" );
		iot_action_parameter_add( script_cmd,
			"param1", IOT_PARAMETER_IN, IOT_TYPE_STRING, 0u );
		iot_action_parameter_add( script_cmd,
			"param2", IOT_PARAMETER_IN, IOT_TYPE_STRING, 0u );

		if ( script_path[0] )
		{
			status = iot_action_register_command( script_cmd,
				script_path, NULL, 0u );
			if ( status != IOT_STATUS_SUCCESS )
			{
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register script. Reason: %s",
					iot_error( status ) );
			}
		}
		else
			IOT_LOG( iot_lib, IOT_LOG_ERROR, "Cannot find %s for test_script action", TEST_SCRIPT );

		/* create the test params action */
		IOT_LOG( iot_lib, IOT_LOG_INFO, "%s","Registering action test_parameters" );
		test_params_cmd = iot_action_allocate( iot_lib, "test_parameters" );

		/* 1. param str */
		iot_action_parameter_add( test_params_cmd,
			PARAM_NAME_STR, IOT_PARAMETER_IN, IOT_TYPE_STRING, 0u );

		/*2. param_int */
		iot_action_parameter_add( test_params_cmd,
			PARAM_NAME_INT, IOT_PARAMETER_IN, IOT_TYPE_INT32, 0u );

		/* 3. param_float */
		iot_action_parameter_add( test_params_cmd,
			PARAM_NAME_FLOAT, IOT_PARAMETER_IN, IOT_TYPE_FLOAT64, 0u );

		/* 4. param_bool */
		iot_action_parameter_add( test_params_cmd,
			PARAM_NAME_BOOL, IOT_PARAMETER_IN, IOT_TYPE_BOOL, 0u );

		/*5. param to call set on  */
		iot_action_parameter_add( test_params_cmd,
				PARAM_OUT_NAME_BOOL, IOT_PARAMETER_OUT, IOT_TYPE_BOOL, 0u );

		status = iot_action_register_callback(test_params_cmd,
			&on_action_test_parameters, NULL, NULL, 0u );
		if ( status != IOT_STATUS_SUCCESS )
		{
			IOT_LOG( iot_lib, IOT_LOG_ERROR,
				"Failed to register command. Reason: %s",
				iot_error( status ) );
		}
	}
	else
		IOT_LOG( iot_lib, IOT_LOG_ERROR, "%s", "Failed to connect" );
	if ( status != IOT_STATUS_SUCCESS )
	{
		iot_terminate( iot_lib, 0u );
		iot_lib = NULL;
	}
	return iot_lib;
}

void log_handler( iot_log_level_t log_level, iot_log_source_t *log_source,
	const char *message, void *user_data )
{
	(void)log_level;
	(void)log_source;
	(void)user_data;
	printf( "%s\n", message );
}

iot_status_t on_enable_disable_location(
	iot_action_request_t* request,
	void* user_data )
{
	(void)(request);
	(void)(user_data);
	if ( send_location == IOT_FALSE )
	{
		printf( "Sending location...\n" );
		send_location = IOT_TRUE;
	}
	else
	{
		printf( "Disabling location...\n" );
		send_location = IOT_FALSE;
	}
	return IOT_STATUS_SUCCESS;
}

iot_status_t on_enable_disable_telemetry(
	iot_action_request_t* request,
	void* user_data )
{
	(void)(request);
	(void)(user_data);
	if ( send_telemetry == IOT_FALSE )
	{
		printf( "Sending telemetry...\n" );
		send_telemetry = IOT_TRUE;
	}
	else
	{
		printf( "Disabling telemetry...\n" );
		send_telemetry = IOT_FALSE;
	}
	return IOT_STATUS_SUCCESS;
}

iot_status_t on_action_test_parameters(
	iot_action_request_t* request,
	void* user_data )
{
	iot_status_t result = IOT_STATUS_SUCCESS;
	iot_status_t status = IOT_STATUS_FAILURE;
	const char *param_str = NULL;
	int param_int = 0;
	double param_float = 0.0;
	iot_bool_t param_bool = IOT_FALSE;
	(void)(user_data);

	/*success path test each parameter.*/
	/*1. string */
	status = iot_action_parameter_get( request, PARAM_NAME_STR,
		IOT_FALSE, IOT_TYPE_STRING, &param_str );
	if ( status != IOT_STATUS_SUCCESS )
	{
		result = IOT_STATUS_BAD_PARAMETER;
		printf( "Failed to get value for parameter: %s\n",
			PARAM_NAME_STR );
	}
	else
		printf( "Value for parameter: %s = %s\n", PARAM_NAME_STR,
			param_str );

	/*int */
	if ( result == IOT_STATUS_SUCCESS )
	{
		status = iot_action_parameter_get( request, PARAM_NAME_INT,
				IOT_FALSE, IOT_TYPE_INT32, &param_int );
		if ( status != IOT_STATUS_SUCCESS )
		{
			result = IOT_STATUS_BAD_PARAMETER;
			printf( "Failed to get value for parameter: %s\n",
				PARAM_NAME_INT );
		}
		else
			printf( "Value for parameter: %s = %d\n",
				PARAM_NAME_INT, param_int );
	}

	/*float */
	if ( result == IOT_STATUS_SUCCESS )
	{
		status = iot_action_parameter_get( request, PARAM_NAME_FLOAT,
				IOT_FALSE, IOT_TYPE_FLOAT64, &param_float );
		if ( status != IOT_STATUS_SUCCESS )
		{
			result = IOT_STATUS_BAD_PARAMETER;
			printf( "Failed to get value for parameter: %s\n",
				PARAM_NAME_FLOAT );
		}
		else
			printf( "Value for parameter: %s = %f\n",
				PARAM_NAME_FLOAT, param_float );
	}

	/*bool */
	if ( result == IOT_STATUS_SUCCESS )
	{
		status = iot_action_parameter_get( request, PARAM_NAME_BOOL,
				IOT_FALSE, IOT_TYPE_BOOL, &param_bool );
		if ( status != IOT_STATUS_SUCCESS )
		{
			result = IOT_STATUS_BAD_PARAMETER;
			printf( "Failed to get value for parameter: %s\n",
				PARAM_NAME_BOOL );
		}
		else
			printf( "Value for parameter: %s = %d\n",
				PARAM_NAME_BOOL, param_bool );
	}

	/* test setting a parameter */
	iot_action_parameter_set( request, PARAM_OUT_NAME_BOOL, IOT_TYPE_BOOL,
		IOT_FALSE );
	return result;
}

double random_num( double min, double max )
{
	static int rand_init = 0;

	/* take a time seed to get better randomness */
	if (!rand_init )
	{
		srand( (unsigned int)time( NULL ) );
		rand_init = 1;
	}
	return min + (rand() / (double)RAND_MAX) * ( max - min );
}

void send_location_sample( iot_t *iot_lib_hdl )
{
	iot_status_t status = IOT_STATUS_FAILURE;

	/* A location sample */
	iot_location_t *sample;

	/* Accuracy of latitude & longitude in metres */
	iot_float64_t accuracy = 0.0;
	/* Altitude in metres */
	iot_float64_t altitude = 0.0;
	/* Range of the altitude in metres */
	iot_float64_t altitude_accuracy = 0.0;
	/* Direction heading */
	iot_float64_t heading = 0.0;
	/* Latitude in degrees */
	iot_float64_t latitude = 0.0;
	/* Longitude in degrees */
	iot_float64_t longitude = 0.0;
	/* Location source type */
	iot_location_source_t source = IOT_LOCATION_SOURCE_UNKNOWN;
	/* Speed being currently travelled in metres/second */
	iot_float64_t speed = 0.0;
	/* Location tag */
	char tag[TAG_MAX_LEN];

	iot_uint32_t tag_size;
#define LOG_FORMAT "Location:\n" \
	"\tlatitude         :%f\n" \
	"\tlongitude        :%f\n" \
	"\taccuracy         :%f\n" \
	"\taltitude         :%f\n" \
	"\taltitude_accuracy:%f\n" \
	"\theading          :%f\n" \
	"\tspeed            :%f\n" \
	"\tsource           :%u\n" \
	"\ttag              :%s\n"

	/* Random location values */
	latitude = random_num( -90.0, 90.0 );
	longitude = random_num( -180.0, 180.0 );
	accuracy = random_num( 0.0, 1000.0 );
	altitude = random_num( -15.0, 10000.0 );
	altitude_accuracy = random_num( 0.0, 1000.0 );
	heading = random_num( 0.0, 360.0 );
	speed = random_num( 0.0, 10000.0 );
	source = (iot_location_source_t)random_num(
		IOT_LOCATION_SOURCE_FIXED, IOT_LOCATION_SOURCE_WIFI );

	tag_size = (iot_uint32_t)random_num( 0, TAG_MAX_LEN );
	generate_random_string( tag, tag_size );

	/* create a sample with random values */
	sample = iot_location_allocate( 0.0, 0.0 );
	if ( sample )
	{
		/* set values for the sample */
		if ( ( status = iot_location_set( sample, latitude,
			longitude ) ) != IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
				"Setting latitude and longitude failed: %s",
				iot_error( status ) );
		if ( ( status = iot_location_accuracy_set( sample, accuracy ) )
			!= IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
				"Setting coordinate accuracy failed: %s",
				iot_error( status ) );
		if ( ( status = iot_location_altitude_set( sample, altitude ) )
			!= IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
				"Setting altitude failed: %s",
				iot_error( status ) );
		if ( ( status = iot_location_altitude_accuracy_set( sample,
			altitude_accuracy ) ) != IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
				"Setting altitude accuracy failed: %s",
				iot_error( status ) );
		if ( ( status = iot_location_heading_set( sample, heading ) )
			!= IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
				"Setting heading failed: %s",
				iot_error( status ) );
		if ( ( status = iot_location_speed_set( sample, speed ) )
			!= IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
				"Setting speed failed: %s",
				iot_error( status ) );
		if ( ( status = iot_location_source_set( sample, source ) )
			!= IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
				"Setting source failed: %s",
				iot_error( status ) );
		if ( ( status = iot_location_tag_set( sample, tag ) )
			!= IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
				"Setting tag failed: %s",
				iot_error( status ) );

		/* print out the location values */
		IOT_LOG( iot_lib_hdl, IOT_LOG_INFO,
			LOG_FORMAT,
			latitude, longitude, accuracy, altitude,
			altitude_accuracy, heading, speed,
			(iot_uint32_t)source, tag );

		/* publish location telemetry */
		if ( ( status = iot_telemetry_publish( telemetry_location,
			NULL, 0, IOT_TYPE_LOCATION, sample ) )
			!= IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
				"Sample publish failed: %s",
				iot_error( status ) );
	}
	else
		IOT_LOG( iot_lib_hdl, IOT_LOG_ERROR, "%s",
			"Faile to allocate a location sample.\n" );

	/* free the sample */
	if ( sample )
		iot_location_free( sample );
}

void send_telemetry_sample( iot_t *iot_lib )
{
	iot_status_t result;
	iot_float64_t light;
	iot_float32_t temperature;
	static iot_bool_t bool_test = IOT_FALSE;
	iot_int64_t int_test;
	iot_uint64_t uint_test;
	char string_test[MAX_TEXT_SIZE];
	size_t sample_size;
	double random_value;
	iot_severity_t alarm_severity;
	(void)iot_lib;

	random_value = random_num( (double)MIN_JSON_INT, (double)MAX_JSON_INT );
	int_test = (iot_int64_t)random_value;
	random_value = random_num( 0.0, (double)MAX_JSON_INT );
	uint_test = (iot_uint64_t)random_value;
	light = random_num( 100.0, 1000.0 );
	temperature = (iot_float32_t)random_num( 1.0, 45.0 );
	random_value = random_num( 0, MAX_TEXT_SIZE - 1);
	sample_size = (size_t)random_value;
	generate_random_string( string_test, sample_size );
	alarm_severity = (iot_int8_t)random_num(0, 6);

	IOT_LOG( iot_lib, IOT_LOG_INFO,"%s",
		"+--------------------------------------------------------+");
	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending light : %f", (double)((iot_float64_t)light) );
	result = iot_telemetry_publish( telemetry_light, NULL, 0, IOT_TYPE_FLOAT64, (iot_float64_t)light );
	IOT_LOG( iot_lib, IOT_LOG_INFO, " ...... Result: %s", iot_error( result ) );

	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending temp  : %f", (double)((iot_float32_t)temperature) );
	result = iot_telemetry_publish( telemetry_temp, NULL, 0, IOT_TYPE_FLOAT32, (iot_float32_t)temperature );
	IOT_LOG( iot_lib, IOT_LOG_INFO, " ...... Result: %s", iot_error( result ) );

	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending bool  : %s", (bool_test == IOT_FALSE ? "false" : "true") );
	result = iot_telemetry_publish( telemetry_boolean, NULL, 0, IOT_TYPE_BOOL, bool_test );
	IOT_LOG( iot_lib, IOT_LOG_INFO, " ...... Result: %s", iot_error( result ) );

	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending string: %s", string_test );
	result = iot_telemetry_publish( telemetry_string, NULL, 0, IOT_TYPE_STRING, string_test );
	IOT_LOG( iot_lib, IOT_LOG_INFO, " ...... Result: %s", iot_error( result ) );

	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending int8  : %hhd", (signed char)((iot_int8_t)int_test) );
	result = iot_telemetry_publish( telemetry_int8, NULL, 0, IOT_TYPE_INT8, (iot_int8_t)int_test );
	IOT_LOG( iot_lib, IOT_LOG_INFO, " ...... Result: %s", iot_error( result ) );

	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending int16 : %hd", (short)((iot_int16_t)int_test) );
	result = iot_telemetry_publish( telemetry_int16, NULL, 0, IOT_TYPE_INT16, (iot_int16_t)int_test );
	IOT_LOG( iot_lib, IOT_LOG_INFO, " ...... Result: %s", iot_error( result ) );

	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending int32 : %ld", (long int)((iot_int32_t)int_test) );
	result = iot_telemetry_publish( telemetry_int32, NULL, 0, IOT_TYPE_INT32, (iot_int32_t)int_test );
	IOT_LOG( iot_lib, IOT_LOG_INFO, " ...... Result: %s", iot_error( result ) );

	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending int64 : %lld", (long long int)((iot_int64_t)int_test) );
	result = iot_telemetry_publish( telemetry_int64, NULL, 0, IOT_TYPE_INT64, (iot_int64_t)int_test );
	IOT_LOG( iot_lib, IOT_LOG_INFO, " ...... Result: %s", iot_error( result ) );

	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending uint8 : %hhu", (unsigned char)((iot_uint8_t)uint_test) );
	result = iot_telemetry_publish( telemetry_uint8, NULL, 0, IOT_TYPE_UINT8, (iot_uint8_t)uint_test );
	IOT_LOG( iot_lib, IOT_LOG_INFO, " ...... Result: %s", iot_error( result ) );

	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending uint16: %hu", (unsigned short)((iot_uint16_t)uint_test) );
	result = iot_telemetry_publish( telemetry_uint16, NULL, 0, IOT_TYPE_UINT16, (iot_uint16_t)uint_test );
	IOT_LOG( iot_lib, IOT_LOG_INFO, " ...... Result: %s", iot_error( result ) );

	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending uint32: %lu", (unsigned long int)((iot_uint32_t)uint_test) );
	result = iot_telemetry_publish( telemetry_uint32, NULL, 0, IOT_TYPE_UINT32, (iot_uint32_t)uint_test );
	IOT_LOG( iot_lib, IOT_LOG_INFO, " ...... Result: %s", iot_error( result ) );

	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending uint64: %llu",
		(unsigned long long int)((iot_uint64_t)uint_test) );
	result = iot_telemetry_publish( telemetry_uint64, NULL, 0, IOT_TYPE_UINT64, (iot_uint64_t)uint_test );
	IOT_LOG( iot_lib, IOT_LOG_INFO, " ...... Result: %s", iot_error( result ) );

	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending raw   : %s", string_test );
	result = iot_telemetry_publish_raw( telemetry_raw, NULL, 0, sample_size, string_test );
	IOT_LOG( iot_lib, IOT_LOG_INFO, " ...... Result: %s", iot_error( result ) );

	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending alarm   : %d", alarm_severity );
	result = iot_alarm_publish( alarm_test, alarm_severity, string_test );
	IOT_LOG( iot_lib, IOT_LOG_INFO, " ...... Result: %s", iot_error( result ) );

	/* toggle the boolean value for next sample */
	if ( bool_test == IOT_FALSE )
		bool_test = IOT_TRUE;
	else if ( bool_test == IOT_TRUE )
		bool_test = IOT_FALSE;
}

void sig_handler( int signo )
{
	if ( signo == SIGINT )
	{
		printf( "Received termination signal...\n" );
		running = IOT_FALSE;
	}
}

void do_sleep( unsigned int delay )
{

	/* hide the ifdefs in a function for readability */
#ifdef _WIN32
	/* windows sleep is milliseconds */
	Sleep( delay * MILLISECONDS_IN_SECOND );
#else
	sleep( delay );
#endif
}

int main( int argc, char *argv[] )
{
	iot_t *iot_lib = initialize();
	(void)(argv);
	if ( iot_lib )
	{
		int count = 0;

		signal( SIGINT, sig_handler );

		/* If any arg is passed in, then start telemetry */
		if ( argc > 1 )
			send_telemetry = IOT_TRUE;

		IOT_LOG( iot_lib, IOT_LOG_INFO,
			"Telemetry interval: %d seconds", POLL_INTERVAL_SEC );

		while ( running != IOT_FALSE )
		{
			if ( send_telemetry != IOT_FALSE ||
				send_location != IOT_FALSE )
			{
				if ( send_location != IOT_FALSE )
					send_location_sample( iot_lib );
				if ( send_telemetry != IOT_FALSE )
					send_telemetry_sample( iot_lib );
				count++;

				/* Stop sending samples after a max iteration */
				if ( count > MAX_LOOP_ITERATIONS )
				{
					/* empty place holders to call
					 * on_enable_disable_telemetry */
					iot_action_request_t* request = NULL;
					void* user_data = NULL;
					IOT_LOG( iot_lib, IOT_LOG_INFO,
						"Max poll interval reached %d. "
						"Stopping telemetry and/or location",
						POLL_INTERVAL_SEC );
					if ( send_telemetry == IOT_TRUE )
						on_enable_disable_telemetry(
							request,
							user_data);
					if ( send_location == IOT_TRUE )
						on_enable_disable_location(
							request,
							user_data);
					count = 0;
				}
				do_sleep( POLL_INTERVAL_SEC );
			}
			else
				/* sleep 1 sec while waiting for the action to  start */
				do_sleep( 1 );
		}
	}

	/* Test Free API */
	iot_action_free( script_cmd, 0u );

	/* Test deregister API */
	iot_action_deregister( test_params_cmd, NULL, 0u );

	/* Test not deregistering, terminate should do it */
	/*iot_action_deregister( enable_telemetry_cmd, NULL, 0u );*/

	/* Test Free API for telemetry resources allocated */
	iot_telemetry_free( telemetry_light, 0 );

	/* Test Free API for alarm resources */
	iot_alarm_deregister( alarm_test );

	/* Terminate */
	IOT_LOG( iot_lib, IOT_LOG_INFO, "%s", "Exiting..." );
	iot_terminate( iot_lib, 0 );
	return EXIT_SUCCESS;
}

