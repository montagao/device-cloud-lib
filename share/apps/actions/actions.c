/**
 * @brief Source file for the actions app
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

#include <signal.h>  /* for SIGINT */
#include <stdlib.h>  /* for malloc, free, EXIT_SUCCESS & EXIT_FAILURE */
#include <string.h>  /* for strrchr, strlen */
#include <stdio.h>   /* for printf */
#include <time.h>    /* for nanosleep */
#ifdef _WIN32
#	include <Windows.h>  /* for SleepEx */
#else
#	include <unistd.h>   /* for readlink */
#	include <errno.h>    /* for errno */
#endif

/** @brief time in milliseconds to wait in main loop */
#define POLL_INTERVAL_MSEC 2000u

/** @brief name of string parameter */
#define PARAM_NAME_STR                 "param_str"
/** @brief name of integer parameter */
#define PARAM_NAME_INT                 "param_int"
/** @brief name of float parameter */
#define PARAM_NAME_FLOAT               "param_float"
/** @brief name of boolean parameter */
#define PARAM_NAME_BOOL                "param_bool"
/** @brief name of outbound parameter */
#define PARAM_OUT_NAME_BOOL            "param_out"
#ifndef PATH_MAX
/** @brief max path length */
#define PATH_MAX 512u
#endif

#ifdef _WIN32
	/** @brief directory separator */
	#define DIR_SEP '\\'
	/** @brief test script's name */
	#define TEST_SCRIPT "actions_script.cmd"
#else
	/** @brief directory separator */
	#define DIR_SEP '/'
#	ifdef  __ANDROID__
	/** @brief test script's name */
	#define TEST_SCRIPT "actions_script_android.sh"
#	else
	/** @brief test script's name */
	#define TEST_SCRIPT "actions_script.sh"
#	endif
#endif

#ifdef _WRS_KERNEL
	/**
	 * @brief Redefine main function to prevent name collision in global
	 * scope
	 */
#	define main      app_action_main
#endif

/** @brief Whether the application is still running */
static iot_bool_t running = IOT_TRUE;

/** @brief Pointer to the quit action */
static iot_action_t *quit_action = NULL;
/** @brief Pointer to the script action */
static iot_action_t *script_action = NULL;
/** @brief Pointer to the test parameters action */
static iot_action_t *test_params_action = NULL;

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
 * @brief Function called to quit the application
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_SUCCESS          always
 */
static iot_status_t on_action_quit(
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
 * @brief Handles terminatation signal and tears down gracefully
 *
 * @param[in]      signo               signal to trap
 */
static void sig_handler( int signo );

static iot_t* initialize( void )
{
	iot_t* iot_lib;
	iot_status_t status = IOT_STATUS_FAILURE;

	iot_lib = iot_initialize( "action-app", NULL, 0 );
	iot_log_callback_set( iot_lib, &log_handler, NULL );
	status = iot_connect( iot_lib, 0u );
	if ( status == IOT_STATUS_SUCCESS )
	{
		char *script_name_location = NULL;
		char script_path[PATH_MAX];
		memset( script_path, 0, PATH_MAX );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "%s", "Connected" );

		/* Find script path from the full path of the binary.
		 * Script will be found in the same directory as the binary */
#if defined( _WIN32 )
		GetModuleFileNameA( NULL, script_path, PATH_MAX );
#else
		if ( readlink("/proc/self/exe", script_path, PATH_MAX) < IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib, IOT_LOG_ERROR, "Failed to readlink. Reason: %s", strerror(errno) );
#endif
		script_path[ PATH_MAX - 1u ] = '\0';
		if ( (script_name_location = strrchr( script_path, DIR_SEP )) != NULL )
		{
			script_name_location++;
			strncpy( script_name_location, TEST_SCRIPT, PATH_MAX - ( script_name_location - script_path ) - 1u );
		}
		script_path[PATH_MAX - 1u] = '\0';

		/* Allocate actions */
		IOT_LOG( iot_lib, IOT_LOG_INFO, "Registering action: %s",
			"quit_app" );
		quit_action = iot_action_allocate( iot_lib, "quit_app" );

		status = iot_action_register_callback( quit_action,
			&on_action_quit, NULL, NULL, 0u );
		if ( status != IOT_STATUS_SUCCESS )
		{
			IOT_LOG( iot_lib, IOT_LOG_ERROR,
				"Failed to register action. Reason: %s",
				iot_error( status ) );
		}
		IOT_LOG( iot_lib, IOT_LOG_INFO, "Registering action: %s",
			"test_script" );
		script_action = iot_action_allocate( iot_lib,
			"test_script" );
		iot_action_parameter_add( script_action,
			"param1", IOT_PARAMETER_IN, IOT_TYPE_STRING, 0u );
		iot_action_parameter_add( script_action,
			"param2", IOT_PARAMETER_IN, IOT_TYPE_STRING, 0u );

		if ( script_path[0] )
		{
			status = iot_action_register_command( script_action,
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
		IOT_LOG( iot_lib, IOT_LOG_INFO, "%s",
			"Registering action test_parameters" );
		test_params_action = iot_action_allocate( iot_lib,
			"test_parameters" );

		/* 1. param str */
		iot_action_parameter_add( test_params_action,
			PARAM_NAME_STR, IOT_PARAMETER_IN, IOT_TYPE_STRING, 0u );

		/*2. param_int */
		iot_action_parameter_add( test_params_action,
			PARAM_NAME_INT, IOT_PARAMETER_IN, IOT_TYPE_INT32, 0u );

		/* 3. param_float */
		iot_action_parameter_add( test_params_action,
			PARAM_NAME_FLOAT, IOT_PARAMETER_IN, IOT_TYPE_FLOAT64, 0u );

		/* 4. param_bool */
		iot_action_parameter_add( test_params_action,
			PARAM_NAME_BOOL, IOT_PARAMETER_IN, IOT_TYPE_BOOL, 0u );

		/*5. param to call set on  */
		iot_action_parameter_add( test_params_action,
			PARAM_OUT_NAME_BOOL, IOT_PARAMETER_OUT,
			IOT_TYPE_BOOL, 0u );

		status = iot_action_register_callback(test_params_action,
			&on_action_test_parameters, NULL, NULL, 0u );
		if ( status != IOT_STATUS_SUCCESS )
		{
			IOT_LOG( iot_lib, IOT_LOG_ERROR,
				"Failed to register action. Reason: %s",
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
	(void)(log_level);
	(void)(log_source);
	(void)(user_data);
	printf( "%s\n", message );
}

iot_status_t on_action_quit(
	iot_action_request_t* request,
	void* user_data )
{
	(void)(request);
	(void)(user_data);
	printf( "Quit action application...\n" );
	running = IOT_FALSE;
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

void sig_handler( int signo )
{
	if ( signo == SIGINT )
        {
		printf( "Received termination signal...\n" );
		running = IOT_FALSE;
	}
}

int main( void )
{
	iot_t *iot_lib = initialize();
	if ( iot_lib )
	{
#ifndef _WIN32
		struct timespec t;
#endif
		signal( SIGINT, sig_handler );

		while ( running != IOT_FALSE )
		{
#ifdef _WIN32
			SleepEx( POLL_INTERVAL_MSEC, TRUE );
#else
			t.tv_sec = 0;
			t.tv_nsec = POLL_INTERVAL_MSEC * 1000;
			nanosleep( &t, NULL );
#endif
		}
	}

	/* Test Free API (calls deregister) */
	iot_action_free( script_action, 0u );

	/* Test deregister API */
	iot_action_deregister( test_params_action, NULL, 0u );

	/* Test not deregistering, terminate should do it */
	/*iot_action_deregister( quit_action, NULL, 0u );*/

	/* Terminate (calls deregister/free for any remaining actions) */
	iot_terminate( iot_lib, 0 );
	IOT_LOG( iot_lib, IOT_LOG_INFO, "%s", "Exiting..." );
	return EXIT_SUCCESS;
}

