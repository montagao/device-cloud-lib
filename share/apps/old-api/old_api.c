/**
 * @brief Source file for the pseudo-telemetry app.
 *
 * @copyright Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include <wra_command.h>
#include <wra_common.h>
#include <wra_location.h>
#include <wra_metric.h>
#include <wra_service.h>
#include <wra_source.h>
#include <wra_types.h>

#include <ctype.h>   /* for alnum */
#include <signal.h>  /* for SIGINT, signal */
#include <stdlib.h>  /* for EXIT_SUCCESS & EXIT_FAILURE */
#include <stdio.h>   /* for printf */
#include <string.h>  /* for strncpy */
#include <time.h>    /* for time, srand, rand */
#ifdef _WIN32
#	include <Windows.h>  /* for SleepEx */
#else
#	include <unistd.h>   /* for sleep */
#	include <errno.h>    /* for errno */
#endif

/** @brief Number of milliseconds in a second */
#define MILLISECONDS_IN_SECOND     1000u
/** @brief Number of nanoseconds in a millisecond */
#define NANOSECONDS_IN_MILLISECOND 1000000u
/** @brief Interval between samples */
#define POLL_INTERVAL_MSEC 2000u
/** @brief Service name */
#define SERVICE_NAME "Pseudo"
/** @brief Service version */
#define SERVICE_VERSION "1.0"
/** @brief Data source name */
#define SOURCE_NAME    "Pseudo"
/** @brief Data source version */
#define SOURCE_VERSION "1.0"
/** @brief Tag max length */
#define TAG_MAX_LEN 128u
/** @brief Text sample length */
#define MAX_LOG_SIZE 256u
/** @brief max path length */
#define PATH_MAX 512u

#ifdef _WIN32
	/** @brief directory separator */
	#define DIR_SEP '\\'
	/** @brief test script's name */
	#define TEST_SCRIPT "old_api_script.cmd"
#else
	/** @brief directory separator */
	#define DIR_SEP '/'
	/** @brief test script's name */
	#define TEST_SCRIPT "old_api_script.sh"
#endif

#ifdef _WRS_KERNEL
	/**
	 * @brief Redefine main function to prevent name collision in global
	 * scope
	 */
#	define main      app_old_api_main
#endif

static iot_bool_t running = IOT_TRUE;
static wra_t* wra_lib;
static wra_service_t* wra_serv;
static wra_source_t* datasource_temperature;
static wra_metric_t* metric_temp;
static wra_metric_t* metric_light;

/* add one of each metric type to test */
static wra_metric_t* metric_boolean;
static wra_metric_t* metric_string;
static wra_metric_t* metric_integer;
static wra_metric_t* metric_raw;

static wra_command_t* enable_cmd;
static wra_command_t* enable_cmd_location;
static wra_command_t* script_cmd;
static wra_bool_t send_location = IOT_FALSE;
static wra_bool_t send_telemetry = IOT_FALSE;

/**
 * @brief Debug log wrapper for printf, used for callbacks.
 *
 * @param[in]      log_level                     Log level to print out.
 * @param[in]      msg                           Message string to print out.
 * @param[in]      user_data                     Unused.
 */
static void debug_log( wra_log_level_t log_level, const char* msg, void* user_data );

/**
 * @brief Sleep handler
 *
 * @param[in]      delay              Length of time in seconds to delay.
 */
static void do_sleep( unsigned int delay );

/**
 * @brief Send telemetry data to the agent
 *
 * @retval IOT_TRUE                              success
 * @retval IOT_FALSE                             failure
 */
static iot_bool_t initialize( void );

/**
 * @brief Callback function for enabling & disabling sending telemetery
 *
 * @param[in,out]  request                       request invoked by the cloud
 * @param[in]      user_data                     not used
 *
 * @retval WRA_COMMAND_SUCCESS                   always
 */
static wra_command_status_t on_enable_disable_telemetry(
	wra_command_request_t* request, void* user_data );

/**
 * @brief Callback function for enabling & disabling sending location
 *
 * @param[in,out]  request                       request invoked by the cloud
 * @param[in]      user_data                     not used
 *
 * @retval WRA_COMMAND_SUCCESS                   always
 */
static wra_command_status_t on_enable_disable_location(
	wra_command_request_t* request, void* user_data );

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
 * @brief Random string generation handler.
 *
 * @param[out]     dest                Buffer to store random string.
 * @param[in]      length              Size of the random string to generate.
 */
static void random_string( char *dest, size_t length );

/**
 * @brief Send location data to the agent.
 */
static void send_location_sample( void );

/**
 * @brief Send telemetry data to the agent.
 */
static void send_telemetry_sample( void );

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

void debug_log( wra_log_level_t log_level, const char* msg,
	void* user_data )
{
	unsigned int i = 0u;
	const char *const prefix[] =
		{ "Fatal", "Alert", "Critical", "Error", "Warning", "Notice",
		  "Info", "Debug", "Trace" };
	(void)(user_data);

	/* ensure log level is a valid enumeration value */
	if ( (unsigned int)log_level <= WRA_LOG_TRACE )
		i = (unsigned int)log_level;
	printf( "%s: %s\n", prefix[i], msg );
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

static iot_bool_t initialize( void )
{
	iot_bool_t result = IOT_FALSE;
	wra_status_t status = WRA_STATUS_FAILURE;
	wra_lib = wra_initialize( NULL );
	wra_log_callback( wra_lib, &debug_log, NULL );
	status = wra_connect( wra_lib, 0u );
	if ( status == WRA_STATUS_SUCCESS )
	{
		char *script_name_location = NULL;
		char script_path[PATH_MAX];
		memset( script_path, 0, PATH_MAX );

		/* Find script path from the full path of the binary.
		 * Script will be found in the same directory as the binary */
#if defined( _WIN32 )
		GetModuleFileNameA( NULL, script_path, PATH_MAX );
#else
		if ( readlink( "/proc/self/exe", script_path, PATH_MAX ) < IOT_STATUS_SUCCESS )
			printf( "Failed to readlink. Reason: %s", strerror(errno) );
#endif
		script_path[ PATH_MAX - 1u ] = '\0';
		if ( (script_name_location = strrchr( script_path, DIR_SEP )) != NULL )
		{
			script_name_location++;
			strncpy( script_name_location, TEST_SCRIPT, PATH_MAX - ( script_name_location - script_path ) - 1u );
		}
		script_path[PATH_MAX - 1u] = '\0';

		debug_log( WRA_LOG_INFO, "Connected", NULL );
		datasource_temperature = wra_source_allocate( SOURCE_NAME, SOURCE_VERSION );

		metric_light = wra_metric_allocate( "light", SOURCE_VERSION );
		metric_temp = wra_metric_allocate( "temp", SOURCE_VERSION );
		metric_boolean = wra_metric_allocate( "bool-test", SOURCE_VERSION );
		metric_string = wra_metric_allocate( "string-test", SOURCE_VERSION );
		metric_integer = wra_metric_allocate( "int-test", SOURCE_VERSION );
		metric_raw = wra_metric_allocate( "raw-test", SOURCE_VERSION );

		wra_metric_type_set( metric_light, WRA_TYPE_FLOAT );
		wra_metric_type_set( metric_temp, WRA_TYPE_FLOAT );
		wra_metric_type_set( metric_boolean, WRA_TYPE_BOOLEAN );
		wra_metric_type_set( metric_string, WRA_TYPE_STRING );
		wra_metric_type_set( metric_integer, WRA_TYPE_INTEGER );
		wra_metric_type_set( metric_raw, WRA_TYPE_RAW );

		wra_metric_register( datasource_temperature, metric_light, 0 );
		wra_metric_register( datasource_temperature, metric_temp, 0 );
		wra_metric_register( datasource_temperature, metric_boolean, 0 );
		wra_metric_register( datasource_temperature, metric_string, 0 );
		wra_metric_register( datasource_temperature, metric_integer, 0 );
		wra_metric_register( datasource_temperature, metric_raw, 0 );

		status = wra_source_register( wra_lib, datasource_temperature, 0 );
		if ( status != WRA_STATUS_SUCCESS )
			printf( "Failed to register source. Reason: %s",
				wra_error( status ) );

		wra_serv = wra_service_allocate( SERVICE_NAME, SERVICE_VERSION );
		enable_cmd = wra_command_allocate( "enable_disable_telemetry" );
		status = wra_service_register( wra_lib, wra_serv, 0 );
		if ( status != WRA_STATUS_SUCCESS )
			printf( "Failed to register service. Reason: %s",
				wra_error( status ) );
		status = wra_command_register_callback( enable_cmd,
			&on_enable_disable_telemetry, NULL );
		if ( status == WRA_STATUS_SUCCESS )
			status = wra_command_register( wra_serv, enable_cmd, 0 );
		if ( status != WRA_STATUS_SUCCESS )
			printf( "Failed to register enable command. Reason: %s",
				wra_error( status ) );
		enable_cmd_location = wra_command_allocate( "enable_disable_location" );
		status = wra_command_register_callback( enable_cmd_location,
			&on_enable_disable_location, NULL );
		if ( status == WRA_STATUS_SUCCESS )
			status = wra_command_register( wra_serv, enable_cmd_location, 0 );
		if ( status != WRA_STATUS_SUCCESS )
			printf( "Failed to register enable location command. Reason: %s",
				wra_error( status ) );
		script_cmd = wra_command_allocate( "test_script" );
		wra_command_parameter_add( script_cmd,
			"param1", WRA_TYPE_STRING, 0u );
		wra_command_parameter_add( script_cmd,
			"param2", WRA_TYPE_STRING, 0u );

		if ( script_path[0] )
		{
			wra_command_register_script( script_cmd, script_path );
			status = wra_command_register( wra_serv, script_cmd, 0 );
			if ( status != WRA_STATUS_SUCCESS )
				printf( "Failed to register script. Reason: %s",
					wra_error( status ) );
		}
		else
			printf( "Cannot find %s for test_script action", TEST_SCRIPT );
	}
	else
		debug_log( WRA_LOG_ERROR, "Failed to connect", NULL );
	if ( status == WRA_STATUS_SUCCESS )
		result = IOT_TRUE;
	return result;
}

wra_command_status_t on_enable_disable_location(
	wra_command_request_t* request, void* user_data )
{
	(void)(request);
	(void)(user_data);
	if ( send_location == WRA_FALSE )
	{
		debug_log( WRA_LOG_INFO, "Sending location...", NULL );
		send_location = WRA_TRUE;
	}
	else
	{
		debug_log( WRA_LOG_INFO, "Disabling location...", NULL );
		send_location = WRA_FALSE;
	}
	return WRA_COMMAND_COMPLETE;
}

wra_command_status_t on_enable_disable_telemetry(
	wra_command_request_t* request, void* user_data )
{
	(void)(request);
	(void)(user_data);
	if ( send_telemetry == WRA_FALSE )
	{
		debug_log( WRA_LOG_INFO, "Sending telemetry...", NULL );
		send_telemetry = WRA_TRUE;
	}
	else
	{
		debug_log( WRA_LOG_INFO, "Disabling telemetry...", NULL );
		send_telemetry = WRA_FALSE;
	}
	return WRA_COMMAND_COMPLETE;
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

void random_string( char *dest, size_t length )
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

void send_location_sample( void )
{
	wra_status_t status = WRA_STATUS_FAILURE;
	/* A location sample */
	wra_location_t *sample;

	/* Accuracy of latitude & longitude in metres */
	double accuracy = 0.0;
	/* Altitude in metres */
	double altitude = 0.0;
	/* Range of the altitude in metres */
	double altitude_accuracy = 0.0;
	/* Direction heading */
	double heading = 0.0;
	/* Latitude in degrees */
	double latitude = 0.0;
	/* Longitude in degrees */
	double longitude = 0.0;
	/* Location source type */
	wra_location_source_t source = (wra_location_source_t)
		WRA_LOCATION_SOURCE_UNKNOWN ;
	/* Speed being currently travelled in metres/second */
	double speed = 0.0;
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
	source = (wra_location_source_t)random_num(
		IOT_LOCATION_SOURCE_FIXED, IOT_LOCATION_SOURCE_WIFI );

	tag_size = random_num( 0, TAG_MAX_LEN );
	random_string( tag, tag_size );

	/* create a sample with random values */
	sample = wra_location_allocate( latitude, longitude );
	if ( sample )
	{
		/* set values for the sample */
		status = wra_location_accuracy_set( sample, accuracy );
		if ( status != WRA_STATUS_SUCCESS)
			printf( "Setting coordinate accuracy failed: %s",
				wra_error( status ) );
		status = wra_location_altitude_set( sample, altitude );
		if ( status != WRA_STATUS_SUCCESS)
			printf( "Setting altitude failed: %s",
				wra_error( status ) );
		status = wra_location_altitude_accuracy_set( sample,
				altitude_accuracy );
		if ( status != WRA_STATUS_SUCCESS)
			printf( "Setting altitude accuracy failed: %s",
				wra_error( status ) );
		status = wra_location_heading_set( sample, heading );
		if ( status != WRA_STATUS_SUCCESS)
			printf( "Setting heading failed: %s",
				wra_error( status ) );
		status = wra_location_speed_set( sample, speed );
		if ( status != WRA_STATUS_SUCCESS)
			printf( "Setting speed failed: %s",
				wra_error( status ) );
		status = wra_location_source_set( sample, source );
		if ( status != WRA_STATUS_SUCCESS)
			printf( "Setting source failed: %s",
				wra_error( status ) );
		status = wra_location_tag_set( sample, tag );
		if ( status != WRA_STATUS_SUCCESS)
			printf( "Setting tag failed: %s",
				wra_error( status ) );
	
		/* print out the location values */
		printf( LOG_FORMAT,
			latitude, longitude, accuracy, altitude,
			altitude_accuracy, heading, speed, source, tag );

		/* publish location sample */
		status = wra_location_publish( wra_lib, sample, 0 );
		if ( status != WRA_STATUS_SUCCESS)
			printf( "Sample publish failed: %s",
				wra_error( status ) );
	}
	else
		printf( "Failed to allocate a location sample ( %f %f )",
			(double)latitude, (double)longitude );

	/* free the sample */
	if ( sample )
		wra_location_free( sample );
}

void send_telemetry_sample( void )
{
	double light = 0.0;
	double temperature = 0.0;
	wra_bool_t bool_test = 1;
	const char *string_test = "This is a string test";
	const char *raw_test = "This is a raw test";
	uint32_t int_test = 666u;

#define MSG_TMPL "Metric Samples:\n\ttemp  :%f\n\tlight :%f\n\tbool  :%d\n\tstring: %s\n\tint   :%u\n\traw:   %s"

	light = random_num( 100.0, 1000.0 );
	temperature = random_num( 1.0, 45.0 );

	printf( MSG_TMPL,
		temperature, light, bool_test, string_test, int_test, raw_test);

	if ( wra_metric_publish_float( metric_light, light, NULL, 0 ) != WRA_STATUS_SUCCESS )
		printf("Failed to publish metric_light :%f\n", light );
	if ( wra_metric_publish_float( metric_temp, temperature, NULL, 0 ) != WRA_STATUS_SUCCESS )
		printf("Failed to publish metric_temp :%f\n", temperature );
	if ( wra_metric_publish_boolean( metric_boolean, bool_test, NULL, 0 ) != WRA_STATUS_SUCCESS )
		printf("Failed to publish metric_boolean :%d\n", bool_test );
	if ( wra_metric_publish_string( metric_string, string_test, NULL, 0 ) != WRA_STATUS_SUCCESS )
		printf("Failed to publish metric_string :%s\n", string_test );
	if ( wra_metric_publish_integer( metric_integer, int_test, NULL, 0 ) != WRA_STATUS_SUCCESS )
		printf("Failed to publish metric_integer :%u\n", int_test );
	if ( wra_metric_publish_raw( metric_raw, raw_test, strlen( raw_test )+1, NULL, 0 ) != WRA_STATUS_SUCCESS )
		printf("Failed to publish metric_raw :%s\n", raw_test );
}

void sig_handler( int signo )
{
	(void)(signo);
	debug_log( WRA_LOG_INFO, "Received termination signal...\n", NULL );
	running = IOT_FALSE;
}

int main( void )
{
	if ( initialize() == IOT_TRUE )
	{
		signal( SIGINT, sig_handler );
		if ( send_telemetry != IOT_FALSE )
			debug_log( WRA_LOG_INFO, "Sending telemetry...", NULL );
		else
			debug_log( WRA_LOG_INFO, "Disabling telemetry...", NULL );
		if ( send_location != IOT_FALSE )
			debug_log( WRA_LOG_INFO, "Sending location...", NULL );
		else
			debug_log( WRA_LOG_INFO, "Disabling location...", NULL );
		while ( running != IOT_FALSE )
		{
			if ( send_telemetry != IOT_FALSE )
				send_telemetry_sample();
			if ( send_location != IOT_FALSE )
				send_location_sample();

			do_sleep( POLL_INTERVAL_MSEC / MILLISECONDS_IN_SECOND );
		}
		debug_log( WRA_LOG_INFO, "Exiting...", NULL );
	}

	/* Free up resources allocated */
	wra_command_free( enable_cmd, 0 );
	wra_command_free( enable_cmd_location, 0 );
	wra_metric_free( metric_light, 0 );
	wra_metric_free( metric_temp, 0 );
	wra_metric_free( metric_boolean, 0 );
	wra_metric_free( metric_string, 0 );
	wra_metric_free( metric_integer, 0 );
	wra_metric_free( metric_raw, 0 );
	wra_source_free( datasource_temperature, 0 );
	wra_service_free( wra_serv, 0 );
	wra_terminate( wra_lib, 0 );
	return EXIT_SUCCESS;
}

