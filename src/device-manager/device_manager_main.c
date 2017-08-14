/**
 * @brief Source file for the device-manager app.
 *
 * @copyright Copyright (C) 2016-2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "device_manager_main.h"

#ifndef _WRS_KERNEL
#	include "device_manager_file.h"
#endif /* ifndef _WRS_KERNEL */

#if !defined (_WIN32 ) && !defined ( _WRS_KERNEL )
/*#	include "device_manager_scripts.h"*/
#endif /*#if !define ( _WIN32 ) && !define ( _WRS_KERNEL )*/

#include "os.h"           /* for os specific functions */
#include "api/shared/iot_types.h"      /* for IOT_ACTION_NO_RETURN, IOT_RUNTIME_DIR */
#include "utilities/app_arg.h"        /* for struct app_arg & functions */
#include "utilities/app_config.h"     /* for reading config file */
#include "utilities/app_log.h"        /* for app_log function */
#include "utilities/app_path.h"       /* for app_path_which function */

#include "iot_build.h"

#include <stdlib.h>                    /* for EXIT_SUCCESS, EXIT_FAILURE */

/* Note: for backwards compatibility, register under two services instead of one
 */
/** @brief Name of the Restore Factory Images action */
#define DEVICE_MANAGER_RESTORE_FACTORY_IMAGES  SOFTWARE_MANAGER_SERVICE_AND_VERSION "Restore Factory Images"
/** @brief Name of the Dump Log Files action*/
#define DEVICE_MANAGER_DUMP_LOG_FILES          SOFTWARE_MANAGER_SERVICE_AND_VERSION "Dump Log Files"

/** @brief Name of the decommission device action */
#define DEVICE_MANAGER_DECOMMISSION_DEVICE     DEVICE_MANAGER_SERVICE_AND_VERSION "decommission_device"
/** @brief Name of the device shutdown action*/
#define DEVICE_MANAGER_DEVICE_SHUTDOWN         DEVICE_MANAGER_SERVICE_AND_VERSION "device_shutdown"
/** @brief Name of the device reboot action*/
#define DEVICE_MANAGER_DEVICE_REBOOT           DEVICE_MANAGER_SERVICE_AND_VERSION "device_reboot"
/** @brief Name of the agent reset action*/
#define DEVICE_MANAGER_AGENT_RESET             DEVICE_MANAGER_SERVICE_AND_VERSION "agent_reset"
/** @brief Name of the remote login action */
#define DEVICE_MANAGER_REMOTE_LOGIN            DEVICE_MANAGER_SERVICE_AND_VERSION "Remote Login"
/** @brief Name of the remote login protocol action */
#define DEVICE_MANAGER_REMOTE_LOGIN_PROTOCOL   DEVICE_MANAGER_SERVICE_AND_VERSION "Remote Login Protocols"
/** @brief Name of "host" parameter for remote login action */
#define REMOTE_LOGIN_PARAM_HOST                "host"
/** @brief Name of "protocol" parameter for remote login action */
#define REMOTE_LOGIN_PARAM_PROTOCOL            "protocol"
/** @brief Name of "url" parameter for remote login action */
#define REMOTE_LOGIN_PARAM_URL                 "url"

#if defined( __unix__ ) && !defined( __ANDROID__ )
#	define COMMAND_PREFIX                      "sudo "
#else
#	define COMMAND_PREFIX                      ""
#endif

#if defined( __ANDROID__ )
/** @brief Command and parameters to enable adb over network on the device */
/** @brief Command and parameters to enable telnetd on the device */
#define ENABLE_TELNETD_LOCALHOST \
	"if [ 0 -eq $( netstat | grep 23 | grep -c LISTEN ) ]; then busybox telnetd -l /system/bin/sh -b 127.0.0.1:23; fi"
#endif /* __ANDROID__ */

/** @brief IoT device manager service ID */
#define IOT_DEVICE_MANAGER_ID         IOT_DEVICE_MANAGER_TARGET
#ifdef _WIN32
	/** @brief Remote Desktop service ID on Windows */
#	define IOT_REMOTE_DESKTOP_ID  "TermService"
#endif

/**
 * @brief Structure containing application specific data
 */
#ifndef _WRS_KERNEL
extern struct device_manager_info APP_DATA;
struct device_manager_info APP_DATA;
#else
/** @todo fix after enabling read cofigure file in vxWorks*/
static struct device_manager_info APP_DATA;
#endif
/**
 * @brief Structure defining information about remote login protocols
 */
struct remote_login_protocol
{
	const char *name;           /**< @brief protocol name */
	iot_uint16_t port;          /**< @brief protocol port */
};

/**
 * @brief Enables or disables certain actions
 *
 * @param[in,out]  device_manager_info           application specific data
 * @param[in]      flag                          flag for the action to set
 * @param[in]      value                         value to set
 *
 * @retval IOT_STATUS_SUCCESS                    on success
 * @retval IOT_STATUS_BAD_PARAMETER              bad parameter
 */
iot_status_t device_manager_action_enable(
	struct device_manager_info *device_manager_info,
	iot_uint16_t flag, iot_bool_t value );
/**
 * @brief Initializes the application
 *
 * @param[in]      app_path                      path to the application
 * @param[in,out]  device_manager                application specific data
 *
 * @retval IOT_STATUS_SUCCESS                    on success
 * @retval IOT_STATUS_*                          on failure
 */
static iot_status_t device_manager_initialize( const char *app_path,
	struct device_manager_info *device_manager );

/**
 * @brief Correctly formats the command to call iot-control with the full path
 *        and some options.
 *
 * @param[out]     full_path                     buffer to write command to
 * @param[in]      max_len                       size of destination buffer
 * @param[in]      device_manager                application specific data
 * @param[in]      options                       string with option flags for
 *                                               iot-control
 *
 * @retval IOT_STATUS_BAD_PARAMETER              Invalid parameter
 * @retval IOT_STATUS_FULL                       Buffer is not large enough
 * @retval IOT_STATUS_SUCCESS                    Successfully created command
 */
static iot_status_t device_manager_make_control_command( char *full_path,
	size_t max_len, struct device_manager_info *device_manager,
	const char *options );

/**
 * @brief Cleans up the application before exitting
 *
 * @param[in]  device_manager                    application specific data
 *
 * @retval IOT_STATUS_SUCCESS                    on success
 * @retval IOT_STATUS_*                          on failure
 */
static iot_status_t device_manager_terminate(
	struct device_manager_info *device_manager );

/**
 * @brief Handles terminatation signal and tears down gracefully
 *
 * @param[in]      signum                        signal number that was caught
 */
static void device_manager_sig_handler( int signum );

/**
 * @brief Registers device-manager related actions
 *
 * @param[in]  device_manager                    application specific data
 *
 * @retval IOT_STATUS_SUCCESS                    on success
 * @retval IOT_STATUS_*                          on failure
 */
static iot_status_t device_manager_actions_register(
	struct device_manager_info *device_manager );

/**
 * @brief Deregisters device-manager related actions
 *
 * @param[in]  device_manager                    application specific data
 *
 * @retval IOT_STATUS_SUCCESS                    on success
 * @retval IOT_STATUS_*                          on failure
 */
iot_status_t device_manager_actions_deregister(
	struct device_manager_info *device_manager );

/**
 * @brief Reads the agent configuration file
 *
 * @param[in,out]  device_manager_info           pointer to the agent data structure
 * @param[in]      app_path                      path to the executable calling the
 *                                               function
 * @param[in]      config_file                   command line provided configuation file
 *                                               path (if specified)
 *
 * @retval IOT_STATUS_BAD_PARAMETER              invalid parameter passed to the function
 * @retval IOT_STATUS_NOT_FOUND                  could not find the agent configuration
 *                                               file
 * @retval IOT_STATUS_*                          on failure
 */
#ifndef _WRS_KERNEL
static iot_status_t device_manager_config_read(
	struct device_manager_info *device_manager_info,
	const char *app_path, const char *config_file );
#endif
#if defined( __ANDROID__ )
/**
 * @brief Run OS command
 *
 * @param[in]      command                       command
 * @param[in]      blocking_action               blocking_action
 *                                               IOT_TRUE: blocking
 *                                               IOT_FALSE: non_blocking
 *
 * @retval IOT_STATUS_SUCCESS                    on success
 * @retval IOT_STATUS_FAILURE                    on failure
 * @retval IOT_STATUS_BAD_PARAMETER              bad parameter
 */
static iot_status_t device_manager_run_os_command( const char *cmd,
	iot_bool_t blocking_action );
/**
 * @brief Callback function to return the agent decommission
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t on_action_agent_decommission(
	iot_action_request_t* request,
	void *user_data );
/**
 * @brief Callback function to return the agent reboot
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t on_action_agent_reboot(
	iot_action_request_t* request,
	void *user_data );
/**
 * @brief Callback function to return the agent reset
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t on_action_agent_reset(
	iot_action_request_t* request,
	void *user_data );
/**
 * @brief Callback function to return the agent shutdown
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t on_action_agent_shutdown(
	iot_action_request_t* request,
	void *user_data );
#endif /* __ANDROID__ */
/**
 * @brief Callback function to return the remote login
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_SUCCESS          on success
 */
#ifndef _WRS_KERNEL
/*FIXME*/
/*static iot_status_t on_action_remote_login(*/
/*iot_action_request_t* request,*/
/*void *user_data );*/
/**
 * @brief Callback function to return the remote login protocol
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_SUCCESS          on success
 */
/*FIXME*/
/*static iot_status_t on_action_remote_login_protocols(*/
/*iot_action_request_t* request,*/
/*void *user_data );*/

#if defined( __ANDROID__ )
iot_status_t on_action_agent_decommission(
	iot_action_request_t* request,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	struct device_manager_info * const device_manager =
		(struct device_manager_info *) user_data;
	iot_t *const iot_lib = device_manager->iot_lib;

	if ( device_manager && request )
	{
		char cmd_decommission[] = "iot-control --decommission";
		char cmd_reboot[] = "iot-control --reboot --delay 5000 &";
		result = device_manager_run_os_command(
			cmd_decommission, IOT_TRUE );
		if ( result == IOT_STATUS_SUCCESS )
			result = device_manager_run_os_command(
				cmd_reboot, IOT_FALSE );
	}
	return result;
}
iot_status_t on_action_agent_reboot(
	iot_action_request_t* request,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	struct device_manager_info * const device_manager =
		(struct device_manager_info *) user_data;
	iot_t *const iot_lib = device_manager->iot_lib;

	if ( device_manager && request )
	{
		char cmd[] = "iot-control --reboot --delay 5000 &";
		result = device_manager_run_os_command( cmd, IOT_FALSE );
	}
	return result;
}
iot_status_t on_action_agent_reset(
	iot_action_request_t* request,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	struct device_manager_info * const device_manager =
		(struct device_manager_info *) user_data;
	iot_t *const iot_lib = device_manager->iot_lib;

	if ( device_manager && request )
	{
		char cmd[] = "iot-control --restart --delay 5000 &";
		result = device_manager_run_os_command( cmd, IOT_FALSE );
	}
	return result;
}
iot_status_t on_action_agent_shutdown(
	iot_action_request_t* request,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	struct device_manager_info * const device_manager =
		(struct device_manager_info *) user_data;
	iot_t *const iot_lib = device_manager->iot_lib;

	if ( device_manager && request )
	{
		char cmd[] = "iot-control --shutdown --delay 5000 &";
		result = device_manager_run_os_command( cmd, IOT_FALSE );
	}
	return result;
}
#endif /* __ANDROID__ */

#if 0
iot_status_t on_action_remote_login( iot_action_request_t* request,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	struct device_manager_info * const device_manager =
		(struct device_manager_info *) user_data;
	iot_t *const iot_lib = device_manager->iot_lib;

	if ( device_manager && request )
	{
		static struct remote_login_protocol protocols[] = {
			{ "ssh", 22u },
			{ "rdp", 3389u },
			{ "vnc", 5900u },
			{ "telnet", 23u },
			{ NULL, 0u }
		};
		struct remote_login_protocol *protocol;
		char relay_cmd[ PATH_MAX + 1u ];
		size_t relay_cmd_len;
		const char *host_in = NULL;
		const char *url_in = NULL;
		const char *protocol_in = NULL;
		iot_uint16_t port = 0u;
		char *out_buf[2u] = { NULL, NULL };
		size_t out_len[2u] = { 0u, 0u };
		char log_path[ PATH_MAX + 1u ];

		/* read parameters */
		iot_action_parameter_get( request,
			REMOTE_LOGIN_PARAM_HOST, IOT_TRUE, IOT_TYPE_STRING,
			&host_in );
		iot_action_parameter_get( request,
			REMOTE_LOGIN_PARAM_PROTOCOL, IOT_TRUE, IOT_TYPE_STRING,
			&protocol_in );
		result = iot_action_parameter_get( request,
			REMOTE_LOGIN_PARAM_URL, IOT_TRUE, IOT_TYPE_STRING,
			&url_in );

		IOT_LOG( iot_lib, IOT_LOG_TRACE, "Remote login params host=%s, protocol=%s, url=%s\n",
			host_in, protocol_in, url_in );

		/* convert port to port number */
		port = (iot_uint16_t)os_atoi( protocol_in );
		protocol = &protocols[0u];
		while ( protocol_in && port == 0u && protocol->name != NULL )
		{
			if ( os_strncmp( protocol_in, protocol->name,
				os_strlen( protocol->name ) ) == 0 )
				port = protocol->port;
			++protocol;
		}

		/* build full relay command */
		relay_cmd[ PATH_MAX ] = '\0';
		os_strncpy( relay_cmd, IOT_RELAY_TARGET, PATH_MAX );
		relay_cmd_len = os_strlen( relay_cmd );

		if ( host_in && *host_in != '\0' )
		{
			os_snprintf( &relay_cmd[ relay_cmd_len ],
				PATH_MAX - relay_cmd_len,
				" --host \"%s\"", host_in );
			relay_cmd_len = os_strlen( relay_cmd );
		}

		if ( port > 0u && url_in && *url_in != '\0' )
		{
			char relay_status_file[ PATH_MAX + 1u ];
			char relay_status_pass[ PATH_MAX + 1u ];
			char relay_status_fail[ PATH_MAX + 1u ];

			int max_wait_ms = 60 * IOT_MILLISECONDS_IN_SECOND;
			int ms_counter = 0;
			int loop_wait_ms = 100u;
			char runtime_path[ PATH_MAX +1u ];
			iot_status_t res;
			os_dir_t dptr;
			char file_path[ PATH_MAX +1u ];
#			define RELAY_FILE_PREFIX "relay-"
#			define SUCCESS "SUCCESS"
#			define FAILURE "FAILURE"
			app_path_runtime_directory_get( runtime_path, PATH_MAX );

			/* Reap any existing relay status files.
			 * On windows, this will clean up every time the device
			 * manager starts */
			res = os_directory_open( runtime_path, &dptr );
			if ( res == IOT_STATUS_SUCCESS )
			{
				while ( ( os_directory_next( &dptr,
					IOT_TRUE, file_path, PATH_MAX )
						== OS_STATUS_SUCCESS ) )
				{
					/* look for relay temp files and delete them */
					char * str = NULL;
					IOT_LOG( iot_lib, IOT_LOG_TRACE, "test clean up rule for %s\n", file_path );
					str = os_strstr( file_path, RELAY_FILE_PREFIX );
					if ( str != NULL )
						if ( OS_STATUS_SUCCESS != os_file_delete( file_path ) )
							IOT_LOG( iot_lib, IOT_LOG_ERROR,
								"test clean up rule for %s failed\n",
								file_path );
				}
				os_directory_close( &dptr );
			}

			/* create a status file so that the relay
			 * app can communicate back to the cloud any error during
			 * initialization. Pass this file name to the
			 * relay app. */
			os_snprintf( relay_status_file, PATH_MAX,
				"%s%c%sXXXXXX", runtime_path, OS_DIR_SEP,
				RELAY_FILE_PREFIX );

			os_file_temp( relay_status_file, 0 );

			IOT_LOG( iot_lib, IOT_LOG_TRACE,
				"Remote login connection status file=%s\n",
				relay_status_file );

			/* Now create the pass fail flag file names
			 * based on relay_status_file.  Websocket
			 * app will append -SUCCESS.txt
			 * or -FAILURE.txt to it. Later poll for one of
			 * those files  */
			os_snprintf( relay_status_pass, PATH_MAX,
				"%s-%s.txt", relay_status_file, SUCCESS );
			os_snprintf( relay_status_fail, PATH_MAX,
				"%s-%s.txt", relay_status_file, FAILURE);

			os_make_path( log_path, PATH_MAX, IOT_RUNTIME_DIR,
				"log", "iot-relay.log", NULL );

			os_snprintf( &relay_cmd[ relay_cmd_len ],
				PATH_MAX - relay_cmd_len,
				" --port %u  --verbose --notification \"%s\" --log-file \"%s\" \"%s\"",
				(unsigned int)port, relay_status_file,
				log_path, url_in);

#if !defined( _WIN32 ) && !defined( _WRS_KERNEL )
			relay_cmd_len = os_strlen( relay_cmd );
			/* start the cmd detached so that it won't be defunct
			after disconnects */
			os_snprintf( &relay_cmd[ relay_cmd_len ],
				PATH_MAX - relay_cmd_len, " &" );
#endif /* if !defined( _WIN32 ) && !defined( _WRS_KERNEL ) */

			IOT_LOG( iot_lib, IOT_LOG_TRACE, "Remote login cmd:\n%s\n",
				relay_cmd );

			/* run relay command, as a background
			 * process.  Websocket app will write its
			 * status to a file and poll here until the
			 * status file shows up. */
			result =  os_system_run( relay_cmd, NULL,
					out_buf, out_len, 0u );
			if ( result == IOT_STATUS_INVOKED )
			{
				int poll_done = -1;
				IOT_LOG( iot_lib, IOT_LOG_TRACE, "Waiting for notification file:%s\n",
					relay_status_pass );
				do {
					if (os_file_exists(
					relay_status_pass ) == IOT_TRUE )
					{

						IOT_LOG( iot_lib, IOT_LOG_DEBUG, "Found success flag %s\n",
							relay_status_pass );
						result = IOT_STATUS_SUCCESS;
						poll_done = 0;
					}
					else if (os_file_exists(
					relay_status_fail ) == IOT_TRUE )
					{

						IOT_LOG( iot_lib, IOT_LOG_DEBUG, "Found failure flag %s\n",
							relay_status_fail );
						result = IOT_STATUS_FAILURE;
						poll_done = 0;
					}

					os_time_sleep( loop_wait_ms, IOT_TRUE );
					ms_counter += loop_wait_ms;
				}
				while ( poll_done != 0 && ms_counter < max_wait_ms );
			}

			if ( result != IOT_STATUS_SUCCESS )
				result = IOT_STATUS_FAILURE;
		}
	}
	return result;
}
#endif

/*FIXME*/
/*iot_status_t on_action_remote_login_protocols( iot_action_request_t* request,*/
/*void *user_data )*/
/*{*/
/*iot_status_t result = IOT_STATUS_FAILURE;*/
/*struct device_manager_info * const device_manager =*/
/*(struct device_manager_info *) user_data;*/
/*if ( device_manager && *device_manager->remote_login_protocols != '\0' )*/
/*{*/
/*result = iot_action_parameter_set( request,*/
/*REMOTE_LOGIN_PARAM_PROTOCOL, IOT_TYPE_STRING,*/
/*device_manager->remote_login_protocols );*/
/*}*/
/*return result;*/
/*}*/
#endif

iot_status_t device_manager_actions_deregister(
	struct device_manager_info *device_manager )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( device_manager )
	{

#ifndef _WRS_KERNEL
		iot_action_t *const dump_log_files = device_manager->dump_log_files;
		iot_action_t *const agent_reset = device_manager->agent_reset;
		iot_action_t *const decommission_device = device_manager->decommission_device;
		iot_action_t *const device_shutdown = device_manager->device_shutdown;
		iot_action_t *const remote_login = device_manager->remote_login;
		iot_action_t *const remote_login_protocols = device_manager->remote_login_protocol;
#endif /* ifndef _WRS_KERNEL */
		iot_action_t *const device_reboot = device_manager->device_reboot;

#if !defined( WIN32 ) && !defined( _WRS_KERNEL )
		iot_action_t *const restore_factory_images = device_manager->restore_factory_images;

		/* restore_factory_images */
		if ( restore_factory_images )
		{
			iot_action_deregister( restore_factory_images, NULL, 0u );
			iot_action_free( restore_factory_images, 0u );
			device_manager->restore_factory_images = NULL;
		}
#endif /* if !defined( WIN32 ) && !defined( _WRS_KERNEL ) */

#ifndef _WRS_KERNEL
		/* device_shutdown */
		if ( device_shutdown )
		{
			iot_action_deregister( device_shutdown, NULL, 0u );
			iot_action_free( device_shutdown, 0u );
			device_manager->device_shutdown = NULL;
		}
#endif /* ifndef _WRS_KERNEL */

		/* device_reboot */
		if ( device_reboot )
		{
			iot_action_deregister( device_reboot, NULL, 0u );
			iot_action_free( device_reboot, 0u );
			device_manager->device_reboot = NULL;
		}

#ifndef _WRS_KERNEL
		/* decommission_device */
		if ( decommission_device )
		{
			iot_action_deregister( decommission_device, NULL, 0u );
			iot_action_free( decommission_device, 0u );
			device_manager->decommission_device = NULL;
		}

		/* agent_reset */
		if ( agent_reset )
		{
			iot_action_deregister( agent_reset, NULL, 0u );
			iot_action_free( agent_reset, 0u );
			device_manager->agent_reset = NULL;
		}

		/* dump_log_files */
		if ( dump_log_files )
		{
			iot_action_deregister( dump_log_files, NULL, 0u );
			iot_action_free( dump_log_files, 0u );
			device_manager->dump_log_files = NULL;
		}

		/* remote_login */
		if ( remote_login )
		{
			iot_action_deregister( remote_login, NULL, 0u );
			iot_action_free( remote_login, 0u );
			device_manager->remote_login = NULL;
		}
		/* remote_login_protocol */
		if ( remote_login_protocols )
		{
			iot_action_deregister( remote_login_protocols, NULL, 0u );
			iot_action_free( remote_login_protocols, 0u );
			device_manager->remote_login_protocol = NULL;
		}

		/* manifest(ota) */
		device_manager_ota_deregister( device_manager );

#ifndef NO_FILEIO_SUPPORT
		/* file-io */
		/*FIXEM*/
		/*device_manager_file_deregister( device_manager );*/
#endif /* ifndef NO_FILEIO_SUPPORT */
#endif /* ifndef _WRS_KERNEL */
		result = IOT_STATUS_SUCCESS;
	}

	return result;
}

iot_status_t device_manager_actions_register(
	struct device_manager_info *device_manager )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( device_manager )
	{
		iot_t *const iot_lib = device_manager->iot_lib;
		iot_action_t *device_reboot = NULL;
		char command_path[ PATH_MAX + 1u ];
#ifndef _WRS_KERNEL
		iot_action_t *dump_log_files = NULL;
		iot_action_t *agent_reset = NULL;
		/*iot_action_t *decommission_device = NULL;*/
		iot_action_t *device_shutdown = NULL;
		/*FIXME*/
		/*iot_action_t *remote_login = NULL;*/
		/*iot_action_t *remote_login_protocols = NULL;*/
#ifndef _WIN32
		/*iot_action_t *restore_factory_images = NULL;*/
		/*FIXME*/
		/* restore factory image */
		/*if ( device_manager->enabled_actions &*/
		/*DEVICE_MANAGER_ENABLE_RESTORE_FACTORY_IMAGES )*/
		/*{*/
		/*restore_factory_images = iot_action_allocate( iot_lib,*/
		/*DEVICE_MANAGER_RESTORE_FACTORY_IMAGES );*/
		/*iot_action_flags_set( restore_factory_images,*/
		/*IOT_ACTION_NO_RETURN | IOT_ACTION_EXCLUSIVE_DEVICE );*/
		/*os_make_path( command_path, PATH_MAX,*/
		/*device_manager->app_path,*/
		/*DEVICE_MANAGER_RESTORE_FACTORY_IMAGES_SCRIPT, NULL );*/
		/*result = iot_action_register_command( restore_factory_images,*/
		/*command_path, NULL, 0u );*/
		/*if ( result == IOT_STATUS_SUCCESS )*/
		/*device_manager->restore_factory_images =*/
		/*restore_factory_images;*/
		/*else*/
		/*IOT_LOG( iot_lib, IOT_LOG_ERROR,*/
		/*"Failed to register %s action. Reason: %s",*/
		/*DEVICE_MANAGER_RESTORE_FACTORY_IMAGES,*/
		/*iot_error(result) );*/
		/*}*/
#endif /* ifndef _WIN32 */
		/* device shutdown */
		if ( device_manager->enabled_actions &
			DEVICE_MANAGER_ENABLE_DEVICE_SHUTDOWN )
		{
			device_shutdown = iot_action_allocate( iot_lib,
				DEVICE_MANAGER_DEVICE_SHUTDOWN );
			iot_action_flags_set( device_shutdown,
				IOT_ACTION_NO_RETURN | IOT_ACTION_EXCLUSIVE_DEVICE );
#if defined( __ANDROID__ )
			result = iot_action_register_callback(
				device_shutdown, &on_action_agent_shutdown,
				(void*)device_manager, NULL, 0u );
#else
			result = device_manager_make_control_command( command_path,
				PATH_MAX, device_manager, " --shutdown" );
			if ( result == IOT_STATUS_SUCCESS )
				result = iot_action_register_command( device_shutdown,
					command_path, NULL, 0u );
#endif
			if ( result == IOT_STATUS_SUCCESS )
				device_manager->device_shutdown = device_shutdown;
			else
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action. Reason: %s",
					DEVICE_MANAGER_DEVICE_SHUTDOWN,
					iot_error(result) );
		}

		/* decommission device */
		/*FIXME*/
		/*if ( device_manager->enabled_actions &*/
		/*DEVICE_MANAGER_ENABLE_DECOMMISSION_DEVICE )*/
		/*{*/
		/*decommission_device = iot_action_allocate( iot_lib,*/
		/*DEVICE_MANAGER_DECOMMISSION_DEVICE );*/
		/*iot_action_flags_set( decommission_device,*/
		/*IOT_ACTION_NO_RETURN | IOT_ACTION_EXCLUSIVE_DEVICE );*/
		/*#if defined( __ANDROID__ )*/
		/*result = iot_action_register_callback(*/
		/*decommission_device, &on_action_agent_decommission,*/
		/*(void*)device_manager, NULL, 0u );*/
		/*#else*/
		/*#	if defined( _WIN32 )*/
		/*result = device_manager_make_control_command( command_path,*/
		/*PATH_MAX, device_manager, " --decommission --reboot" );*/
		/*#	else *//* defined( _WIN32 ) */
		/*result = os_make_path( command_path, PATH_MAX,*/
		/*device_manager->app_path,*/
		/*DEVICE_MANAGER_DECOMMISSION_SCRIPT, NULL );*/
		/*#	endif *//* defined( _WIN32 ) */
		/*if ( result == IOT_STATUS_SUCCESS )*/
		/*result = iot_action_register_command( decommission_device,*/
		/*command_path, NULL, 0u );*/
		/*#endif*/
		/*if ( result == IOT_STATUS_SUCCESS )*/
		/*device_manager->decommission_device =*/
		/*decommission_device;*/
		/*else*/
		/*IOT_LOG( iot_lib, IOT_LOG_ERROR,*/
		/*"Failed to register %s action. Reason: %s",*/
		/*DEVICE_MANAGER_DECOMMISSION_DEVICE,*/
		/*iot_error(result) );*/
		/*}*/

		/* agent reset */
		if ( device_manager->enabled_actions &
			DEVICE_MANAGER_ENABLE_AGENT_RESET )
		{
			agent_reset = iot_action_allocate( iot_lib,
				DEVICE_MANAGER_AGENT_RESET );
			iot_action_flags_set( agent_reset,
				IOT_ACTION_NO_RETURN | IOT_ACTION_EXCLUSIVE_DEVICE );
#if defined( __ANDROID__ )
			result = iot_action_register_callback(
				agent_reset, &on_action_agent_reset,
				(void*)device_manager, NULL, 0u );
#else
			result = device_manager_make_control_command( command_path,
				PATH_MAX, device_manager, " --restart" );
			if ( result == IOT_STATUS_SUCCESS )
				result = iot_action_register_command( agent_reset,
					command_path, NULL, 0u );
#endif
			if ( result == IOT_STATUS_SUCCESS )
				device_manager->agent_reset = agent_reset;
			else
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action. Reason: %s",
					DEVICE_MANAGER_AGENT_RESET,
					iot_error(result) );
		}

		/* dump log files */
		if ( device_manager->enabled_actions &
			DEVICE_MANAGER_ENABLE_DUMP_LOG_FILES )
		{
			dump_log_files = iot_action_allocate( iot_lib,
				DEVICE_MANAGER_DUMP_LOG_FILES );
			iot_action_flags_set( dump_log_files,
				IOT_ACTION_EXCLUSIVE_APP );
			result = device_manager_make_control_command( command_path,
				PATH_MAX, device_manager, " --dump" );
			if ( result == IOT_STATUS_SUCCESS )
				result = iot_action_register_command( dump_log_files,
					command_path, NULL, 0u );
			if ( result == IOT_STATUS_SUCCESS )
				device_manager->dump_log_files = dump_log_files;
			else
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action. Reason: %s",
					DEVICE_MANAGER_DUMP_LOG_FILES,
					iot_error(result) );
		}

#ifndef NO_FILEIO_SUPPORT
		/* file-io */
		/*FIXME*/
		/*if ( device_manager->enabled_actions &*/
		/*DEVICE_MANAGER_ENABLE_FILE_TRANSFERS )*/
		/*if ( device_manager_file_register( device_manager ) != IOT_STATUS_SUCCESS )*/
		/*IOT_LOG( iot_lib, IOT_LOG_ERROR, "%s",*/
		/*"Failed to register file-io actions" );*/
#endif /* ifndef NO_FILEIO_SUPPORT */

		/* manifest (ota) */
		if ( device_manager->enabled_actions &
			DEVICE_MANAGER_ENABLE_SOFTWARE_UPDATE )
			if ( device_manager_ota_register( device_manager ) != IOT_STATUS_SUCCESS )
				IOT_LOG( iot_lib, IOT_LOG_ERROR, "%s",
					"Failed to register manifest(ota) actions" );

		/* remote_login */
		/*FIXME*/
#if 0
		if ( ( device_manager->enabled_actions &
			DEVICE_MANAGER_ENABLE_REMOTE_LOGIN )
			&& *device_manager->remote_login_protocols != '\0' )
		{
			remote_login_protocols = iot_action_allocate( iot_lib,
				DEVICE_MANAGER_REMOTE_LOGIN_PROTOCOL );

			/* param to call set on  */
			iot_action_parameter_add( remote_login_protocols,
				REMOTE_LOGIN_PARAM_PROTOCOL,
				IOT_PARAMETER_OUT_REQUIRED,
				IOT_TYPE_STRING, 0u );

			result = iot_action_register_callback(
				remote_login_protocols,
				&on_action_remote_login_protocols,
					(void*)device_manager, NULL, 0u );
			if ( result != IOT_STATUS_SUCCESS )
			{
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action. Reason: %s",
					DEVICE_MANAGER_REMOTE_LOGIN_PROTOCOL,
					iot_error(result) );
			}

			remote_login = iot_action_allocate( iot_lib,
				DEVICE_MANAGER_REMOTE_LOGIN );

			/* param to call set on  */
			iot_action_parameter_add( remote_login,
				REMOTE_LOGIN_PARAM_HOST,
				IOT_PARAMETER_IN,
				IOT_TYPE_STRING, 0u );
			iot_action_parameter_add( remote_login,
				REMOTE_LOGIN_PARAM_PROTOCOL,
				IOT_PARAMETER_IN_REQUIRED,
				IOT_TYPE_STRING, 0u );
			iot_action_parameter_add( remote_login,
				REMOTE_LOGIN_PARAM_URL,
				IOT_PARAMETER_IN_REQUIRED,
				IOT_TYPE_STRING, 0u );

			result = iot_action_register_callback(
				remote_login,
				&on_action_remote_login,
					(void*)device_manager, NULL, 0u );
			if ( result != IOT_STATUS_SUCCESS )
			{
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action. Reason: %s",
					DEVICE_MANAGER_REMOTE_LOGIN,
					iot_error(result) );
			}
		}
#endif

		/* device reboot */
		if ( device_manager->enabled_actions &
			DEVICE_MANAGER_ENABLE_DEVICE_REBOOT )
#endif
		{
			device_reboot = iot_action_allocate( iot_lib,
				DEVICE_MANAGER_DEVICE_REBOOT );
			iot_action_flags_set( device_reboot,
				IOT_ACTION_NO_RETURN | IOT_ACTION_EXCLUSIVE_DEVICE );
#if defined( __ANDROID__ )
			result = iot_action_register_callback(
				device_reboot, &on_action_agent_reboot,
				(void*)device_manager, NULL, 0u );
#else
#	ifndef _WRS_KERNEL
			result = device_manager_make_control_command( command_path,
				PATH_MAX, device_manager, " --reboot" );
			if ( result == IOT_STATUS_SUCCESS )
				result = iot_action_register_command( device_reboot,
					command_path, NULL, 0u );
#	else
			/**
			 * @todo This part should be rewritten in the future when iot-control becomes
			 * available on VxWorks.  Currently, we just call a "reboot" command to
			 * stimulate a reboot
			 */
			result = iot_action_register_command( device_reboot,
				"reboot", NULL, 0u );
#	endif
#endif
			if ( result == IOT_STATUS_SUCCESS )
				device_manager->device_reboot = device_reboot;
			else
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action",
					DEVICE_MANAGER_DEVICE_REBOOT );
		}
	}
	return result;
}

static void device_manager_sig_handler( int signum )
{
	if ( signum == SIGTERM || signum == SIGINT )
	{
		IOT_LOG( NULL, IOT_LOG_INFO, "%s",
			"Received signal, Quitting..." );
		if ( APP_DATA.iot_lib )
			APP_DATA.iot_lib->to_quit = IOT_TRUE;
	}
	if ( signum == SIGCHLD )
		os_process_cleanup( );
}

iot_status_t device_manager_initialize( const char *app_path,
	struct device_manager_info *device_manager )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( device_manager )
	{
		iot_t *iot_lib = NULL;
		char *p_path = NULL;
#ifndef NO_FILEIO_SUPPORT
		struct device_manager_file_io_info *file_io = &device_manager->file_io_info;
		os_thread_mutex_t *file_transfer_lock = &file_io->file_transfer_mutex;

		/*FIXME*/
		/* file-io init is not thread safe, has to be called first */
		/*if ( device_manager_file_initialize( device_manager, IOT_TRUE )*/
		/*!= IOT_STATUS_SUCCESS )*/
		/*{*/
		/*os_fprintf( OS_STDERR,*/
		/*"Error: %s", "Failed to initialize curl library for file-io" );*/
		/*return IOT_STATUS_FAILURE;*/
		/*}*/
#endif /* ifndef NO_FILEIO_SUPPORT */

		iot_lib = iot_initialize( "device-manager", app_path, 0u );
		if ( iot_lib == NULL )
		{
			os_fprintf( OS_STDERR,
				"Error: %s", "Failed to initialize IOT library" );
			return IOT_STATUS_FAILURE;
		}

#ifdef _WRS_KERNEL
		/* Set user specified default log level */
		iot_log_level_set_string( iot_lib, IOT_LOG_LEVEL );
#endif /* ifdef _WRS_KERNEL */
		iot_log_callback_set( iot_lib, &app_log, NULL );

		/* Find the absolute path to where the application resides */
		os_strncpy( device_manager->app_path, app_path, PATH_MAX );
		p_path = os_strrchr( device_manager->app_path,
			OS_DIR_SEP );
		if ( p_path )
			*p_path = '\0';
		else
		{
#ifndef __ANDROID__
			os_strncpy( device_manager->app_path, ".", PATH_MAX );
#else
			os_strncpy( device_manager->app_path, "/system/bin", PATH_MAX );
#endif /* __ANDROID__ */
		}

		result = iot_connect( iot_lib, 0u );
		if ( result == IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib, IOT_LOG_INFO, "%s", "Connected" );
		else
			IOT_LOG( iot_lib, IOT_LOG_INFO, "%s", "Failed to connect\n" );

		if ( result == IOT_STATUS_SUCCESS )
		{
			device_manager->iot_lib = iot_lib;

#if !defined( NO_THREAD_SUPPORT ) && !defined( NO_FILEIO_SUPPORT )
			if ( os_thread_mutex_create( file_transfer_lock ) != OS_STATUS_SUCCESS )
				IOT_LOG( iot_lib, IOT_LOG_ERROR, "%s", "Failed to create file_transfer_mutex" );
#endif /* if !defined( NO_THREAD_SUPPORT ) && !defined( NO_FILEIO_SUPPORT ) */
			if ( device_manager_actions_register( device_manager ) != IOT_STATUS_SUCCESS )
				IOT_LOG( iot_lib, IOT_LOG_ERROR, "%s",	"Failed to register device-manager actions" );
#ifndef NO_FILEIO_SUPPORT
#	ifdef	__ANDROID__
			/* check pending file transfer status
			 * after run time dir is available */
			device_manager_file_initialize(
				device_manager, IOT_FALSE );
#	endif	/* __ANDROID__ */
#endif /* ifndef NO_FILEIO_SUPPORT */
		}
		else
		{
			iot_terminate( iot_lib, 0u );
		}
	}
	return result;
}

iot_status_t device_manager_terminate(
	struct device_manager_info *device_manager )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( device_manager )
	{
		iot_t *iot_lib = device_manager->iot_lib;
#if !defined( NO_THREAD_SUPPORT ) && !defined( NO_FILEIO_SUPPORT )
		os_thread_mutex_t *file_transfer_lock = &device_manager->file_io_info.file_transfer_mutex;
#endif /* if !defined( NO_THREAD_SUPPORT ) && !defined( NO_FILEIO_SUPPORT ) */

#if ( IOT_DEFAULT_ENABLE_PERSISTENT_ACTIONS == 0 )
		device_manager_actions_deregister( device_manager );
#endif

#if !defined( NO_THREAD_SUPPORT ) && !defined( NO_FILEIO_SUPPORT )
		os_thread_mutex_destroy( file_transfer_lock );
#endif /* if !defined( NO_THREAD_SUPPORT ) && !defined( NO_FILEIO_SUPPORT ) */

		iot_disconnect( iot_lib, 0u );
		iot_terminate( iot_lib, 0u );

#ifndef NO_FILEIO_SUPPORT
		/* must be done last */
		/*FIXME*/
		/*device_manager_file_terminate( device_manager );*/
#endif /* ifndef NO_FILEIO_SUPPORT */
	}
	return result;
}

iot_status_t device_manager_action_enable(
	struct device_manager_info *device_manager_info,
	iot_uint16_t flag, iot_bool_t value )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( device_manager_info )
	{
		if ( value )
			device_manager_info->enabled_actions |= flag;
		else
			device_manager_info->enabled_actions &= ~flag;
	}
	return result;
}

#ifndef _WRS_KERNEL
iot_status_t device_manager_config_read(
	struct device_manager_info *device_manager_info,
	const char *app_path, const char *config_file )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;

	if ( device_manager_info && app_path )
	{
		struct device_manager_file_io_info *const file_io =
			&device_manager_info->file_io_info;
		/*struct iot_proxy *const proxy = &file_io->proxy_info;*/
		struct app_config *config = NULL;
		char install_dir[PATH_MAX + 1u];
		char temp_string[PATH_MAX + 1u];
		char *p_path;
		iot_bool_t has_rdp = IOT_FALSE;

		/* Find install dir to be used later */
		os_strncpy( install_dir, app_path, PATH_MAX );
		p_path = os_strstr( install_dir, IOT_BIN_DIR );
		if ( p_path )
			*p_path = '\0';
		else
			os_strncpy( install_dir, ".", PATH_MAX );

		/* Default values */
		os_strncpy( device_manager_info->runtime_dir,
			IOT_RUNTIME_DIR_DEFAULT, PATH_MAX );
		os_env_expand( device_manager_info->runtime_dir, PATH_MAX );
		device_manager_info->runtime_dir[PATH_MAX] = '\0';

		file_io->ssl_validate = SSL_DEFAULT_VALIDATE;

		file_io->cert_path[0] = '\0';
		if ( IOT_DEFAULT_CERT_PATH[0] != '\0' )
		{
			os_strncpy( temp_string, IOT_DEFAULT_CERT_PATH,
				PATH_MAX );
			os_env_expand( temp_string, PATH_MAX );
			if ( os_path_is_absolute( temp_string ) == IOT_TRUE )
				os_strncpy( file_io->cert_path, temp_string, PATH_MAX );
			else
				os_make_path( file_io->cert_path, PATH_MAX,
					install_dir, temp_string, NULL );
			file_io->cert_path[PATH_MAX] = '\0';
		}

		/* set the default value for remote login.  Can be
		 * overriden in the iot.cfg file. */
#ifdef _WIN32
		iot_status_t service_status = IOT_STATUS_BAD_PARAMETER;
		iot_bool_t self_started = IOT_FALSE;
		has_rdp = IOT_TRUE;

		/* query query_count times with a sleep interval_ms in between */
		iot_millisecond_t interval_ms = 1000;
		unsigned int query_count = 10u;
		unsigned int i;
		service_status = os_service_query ( IOT_REMOTE_DESKTOP_ID, 0u );
		if ( service_status != IOT_STATUS_SUCCESS )
		{
			for ( i = 0; i < query_count;  i++ )
			{
				service_status = os_service_query (
					IOT_DEVICE_MANAGER_ID, 0u );

				if ( service_status == IOT_STATUS_SUCCESS )
				{
					self_started = IOT_TRUE;
					break;
				}
				os_time_sleep( interval_ms, IOT_FALSE );
			}
			if ( self_started == IOT_TRUE )
			{
				service_status = os_service_start (
					IOT_REMOTE_DESKTOP_ID, 0u );
				if ( service_status != IOT_STATUS_SUCCESS )
					has_rdp = IOT_FALSE;
			}
			else
				has_rdp = IOT_FALSE;
		}
#else
		if ( app_path_which( NULL, 0u, NULL, "xrdp" ) > 0u )
			has_rdp = IOT_TRUE;
#endif
		p_path = device_manager_info->remote_login_protocols;
		if ( has_rdp != IOT_FALSE )
		{
			os_strncpy( p_path, "rdp,",
				REMOTE_LOGIN_PROTOCOL_MAX );
			p_path += os_strlen( "rdp," );
		}

		/* check if vnc is available */
		if ( app_path_which( NULL, 0u, NULL, "vncserver" ) > 0u )
		{
			os_strncpy( p_path, "vnc,",
				REMOTE_LOGIN_PROTOCOL_MAX );
			p_path += os_strlen( "vnc," );
		}

#if !defined( __ANDROID__ )
		/* check if sshd is available */
		if ( app_path_which( NULL, 0u, NULL, "sshd" ) > 0u )
		{
			os_strncpy( p_path, "ssh,",
				REMOTE_LOGIN_PROTOCOL_MAX );
			p_path += os_strlen( "ssh," );
		}

#else
		/* check if telnet is available */
		if ( app_path_which( NULL, 0u, NULL, "busybox" ) > 0u )
		{
			const char cmd[] = "netstat | grep 23 | grep -c LISTEN";
#define buf_sz 32u
			char buf_std[buf_sz] = "\0";
			char buf_err[buf_sz] = "\0";
			char *out_buf[2u] = { buf_std, buf_err };
			size_t out_len[2u] = { buf_sz, buf_sz };
			int retval = -1;
			if ( os_system_run( cmd, &retval, out_buf,
				out_len, 0u ) == IOT_STATUS_SUCCESS &&
					retval >= 0 )
			{
				if ( '1' == buf_std[0] )
				{
					os_strncpy( p_path, "telnet,",
						REMOTE_LOGIN_PROTOCOL_MAX );
					p_path += os_strlen( "telnet," );
				}
				else
					IOT_LOG( NULL, IOT_LOG_ERROR,
						"No service for port 23. "
						"netstat: stdout %s "
						"stderr %s retVal %d\n",
						buf_std, buf_err, retval );
			}
		}
#endif /* defined( __ANDROID__ ) */

		/* remote trailing comma */
		if ( p_path != device_manager_info->remote_login_protocols )
		{
			--p_path;
			*p_path = '\0';
		}

#if defined( __ANDROID__ )
		if ( *device_manager_info->remote_login_protocols != '\0' )
			IOT_LOG( NULL, IOT_LOG_INFO,
				"rlogin-protocol %s \n",
				device_manager_info->remote_login_protocols );
		else
			IOT_LOG( NULL, IOT_LOG_INFO, "%s",
				"rlogin-protocol is empty \n" );
#endif /* defined( __ANDROID__ ) */

		/* standard actions */
		device_manager_info->enabled_actions = 0u;
#ifndef _WRS_KERNEL
		if ( IOT_DEFAULT_ENABLE_SOFTWARE_UPDATE )
			device_manager_action_enable( device_manager_info,
				DEVICE_MANAGER_ENABLE_SOFTWARE_UPDATE, IOT_TRUE );
		if ( IOT_DEFAULT_ENABLE_FILE_TRANSFERS )
			device_manager_action_enable( device_manager_info,
				DEVICE_MANAGER_ENABLE_FILE_TRANSFERS, IOT_TRUE );
		if ( IOT_DEFAULT_ENABLE_DECOMMISSION_DEVICE )
			device_manager_action_enable( device_manager_info,
				DEVICE_MANAGER_ENABLE_DECOMMISSION_DEVICE, IOT_TRUE );
		if ( IOT_DEFAULT_ENABLE_DUMP_LOG_FILES )
			device_manager_action_enable( device_manager_info,
				DEVICE_MANAGER_ENABLE_DUMP_LOG_FILES, IOT_TRUE );
		if ( IOT_DEFAULT_ENABLE_DEVICE_SHUTDOWN )
			device_manager_action_enable( device_manager_info,
				DEVICE_MANAGER_ENABLE_DEVICE_SHUTDOWN, IOT_TRUE );
		if ( IOT_DEFAULT_ENABLE_AGENT_RESET )
			device_manager_action_enable( device_manager_info,
				DEVICE_MANAGER_ENABLE_AGENT_RESET, IOT_TRUE );
#ifndef _WIN32
		if ( IOT_DEFAULT_ENABLE_RESTORE_FACTORY_IMAGES )
			device_manager_action_enable( device_manager_info,
				DEVICE_MANAGER_ENABLE_RESTORE_FACTORY_IMAGES, IOT_TRUE );
#endif /* ifndef _WIN32 */
#endif /* ifndef _WRS_KERNEL */
		if ( IOT_DEFAULT_ENABLE_DEVICE_REBOOT )
			device_manager_action_enable( device_manager_info,
				DEVICE_MANAGER_ENABLE_DEVICE_REBOOT, IOT_TRUE );
		if ( IOT_DEFAULT_ENABLE_REMOTE_LOGIN )
			device_manager_action_enable( device_manager_info,
				DEVICE_MANAGER_ENABLE_REMOTE_LOGIN, IOT_TRUE );

		/* set default directories for file transfer */
		/*FIXME*/
		/*device_manager_file_set_default_directories( device_manager_info );*/

		/* set default of uploaded file removal */
		file_io->upload_file_remove = IOT_DEFAULT_UPLOAD_REMOVE_ON_SUCCESS;

		/* Read config file */
		result = IOT_STATUS_NOT_FOUND;
		if ( ( config = app_config_open( NULL, config_file ) ) )
		{
			iot_bool_t temp_bool = IOT_FALSE;
			result = app_config_read_string( config, NULL, "runtime_dir", temp_string, PATH_MAX );
			if ( result == IOT_STATUS_SUCCESS && temp_string[0] != '\0' )
			{
				os_strncpy(
					device_manager_info->runtime_dir,
					temp_string, PATH_MAX );
				os_env_expand(
					device_manager_info->runtime_dir, PATH_MAX );
				device_manager_info->runtime_dir[PATH_MAX] = '\0';
				if ( os_directory_create(
						device_manager_info->runtime_dir,
						DIRECTORY_CREATE_MAX_TIMEOUT )
					!= OS_STATUS_SUCCESS )
					IOT_LOG( NULL, IOT_LOG_ERROR, "Failed to create %s ", device_manager_info->runtime_dir );
			}

			result = app_config_read_boolean( config, NULL, "ssl_validation", &temp_bool );
			if ( result == IOT_STATUS_SUCCESS )
				file_io->ssl_validate = temp_bool;

			result = app_config_read_string( config, NULL, "cert_path", temp_string, PATH_MAX );
			if ( result == IOT_STATUS_SUCCESS && temp_string[0] != '\0' )
			{
				char cert_path[PATH_MAX];
				os_strncpy( cert_path, temp_string,
					PATH_MAX );
				os_env_expand( cert_path, PATH_MAX );
				if ( os_path_is_absolute( cert_path ) != IOT_FALSE )
					os_strncpy( file_io->cert_path,
						cert_path, PATH_MAX );
				else
					os_make_path( file_io->cert_path,
						PATH_MAX, install_dir, cert_path, NULL );
				file_io->cert_path[PATH_MAX] = '\0';
			}

			/* Read the remote login protocol from the
			 * iot.cfg.  Null is a valid value to disable it.*/
			result = app_config_read_string_array( config, NULL,
				"remote_login_protocols", ',', temp_string,
				REMOTE_LOGIN_PROTOCOL_MAX );
			if ( result == IOT_STATUS_SUCCESS )
			{
				os_strncpy(
					device_manager_info->remote_login_protocols,
					temp_string, REMOTE_LOGIN_PROTOCOL_MAX );
			}
			/*FIXME*/
			/*result = app_config_read_string( config, "proxy", "username", temp_string, PATH_MAX );*/
			/*if ( result == IOT_STATUS_SUCCESS && temp_string[0] != '\0' )*/
			/*os_strncpy( proxy->username, temp_string, PATH_MAX );*/

			/*result = app_config_read_string( config, "proxy", "password", temp_string, PATH_MAX );*/
			/*if ( result == IOT_STATUS_SUCCESS && temp_string[0] != '\0' )*/
			/*os_strncpy( proxy->password, temp_string, PATH_MAX );*/

			/* Standard actions */
			result = app_config_read_boolean( config, "actions_enabled",
				"shutdown_device", &temp_bool );
			if ( result == IOT_STATUS_SUCCESS )
				device_manager_action_enable( device_manager_info,
					DEVICE_MANAGER_ENABLE_DEVICE_SHUTDOWN, temp_bool );

			result = app_config_read_boolean( config, "actions_enabled",
				"reboot_device", &temp_bool );
			if ( result == IOT_STATUS_SUCCESS )
				device_manager_action_enable( device_manager_info,
					DEVICE_MANAGER_ENABLE_DEVICE_REBOOT, temp_bool );

			result = app_config_read_boolean( config, "actions_enabled",
				"reset_agent", &temp_bool );
			if ( result == IOT_STATUS_SUCCESS )
				device_manager_action_enable( device_manager_info,
					DEVICE_MANAGER_ENABLE_AGENT_RESET, temp_bool );

			result = app_config_read_boolean( config, "actions_enabled",
				"file_transfers", &temp_bool );
			if ( result == IOT_STATUS_SUCCESS )
				device_manager_action_enable( device_manager_info,
					DEVICE_MANAGER_ENABLE_FILE_TRANSFERS, temp_bool );

			result = app_config_read_boolean( config, "actions_enabled",
				"software_update", &temp_bool );
			if ( result == IOT_STATUS_SUCCESS )
				device_manager_action_enable( device_manager_info,
					DEVICE_MANAGER_ENABLE_SOFTWARE_UPDATE, temp_bool );

			result = app_config_read_boolean( config, "actions_enabled",
				"restore_factory_images", &temp_bool );
			if ( result == IOT_STATUS_SUCCESS )
				device_manager_action_enable( device_manager_info,
					DEVICE_MANAGER_ENABLE_RESTORE_FACTORY_IMAGES, temp_bool );

			result = app_config_read_boolean( config, "actions_enabled",
				"decommission_device", &temp_bool );
			if ( result == IOT_STATUS_SUCCESS )
				device_manager_action_enable( device_manager_info,
					DEVICE_MANAGER_ENABLE_DECOMMISSION_DEVICE, temp_bool );

			result = app_config_read_boolean( config, "actions_enabled",
				"dump_log_files", &temp_bool );
			if ( result == IOT_STATUS_SUCCESS )
				device_manager_action_enable( device_manager_info,
					DEVICE_MANAGER_ENABLE_DUMP_LOG_FILES, temp_bool );

			result = app_config_read_boolean( config, "actions_enabled",
				"remote_login", &temp_bool );
			if ( result == IOT_STATUS_SUCCESS )
				device_manager_action_enable( device_manager_info,
					DEVICE_MANAGER_ENABLE_REMOTE_LOGIN, temp_bool );

			result = app_config_read_string_array( config, NULL,
				"upload_additional_dirs", OS_ENV_SPLIT, temp_string, PATH_MAX );
			/*FIXME*/
			/*if ( result == IOT_STATUS_SUCCESS )*/
			/*device_manager_file_add_upload_directories(*/
			/*device_manager_info, temp_string, OS_ENV_SPLIT );*/

			result = app_config_read_boolean( config, NULL,
				"upload_remove_on_success", &temp_bool );
			if ( result == IOT_STATUS_SUCCESS )
				file_io->upload_file_remove = temp_bool;

			app_config_close( config );
		}

		/* set verification values based on the state of ssl validation */
		/* These 2 fields are used to enable/disable ssl verification.
		 * They will be used for CURLOPT_SSL_VERIFYHOST and CURLOPT_SSL_VERIFYPEER
		 * options. For CURLOPT_SSL_VERIFYPEER, a value of 1L means peer's
		 * certificate is verified, and 0L means it is not. For CURLOPT_SSL_VERIFYHOST,
		 * with value of 1L, curl_easy_setopt() will return an error and the
		 * option value will not be changed; it was previously (libcurl 7.28.0 and
		 * earlier) a debug option of some sorts, but it is no longer supported due to
		 * frequently leading to programmer mistakes. Value of 2L means the server's
		 * certificate is validated and 0L means it is not */
		if ( file_io->ssl_validate != IOT_FALSE )
		{
			file_io->ssl_host_verification = 2L;
			file_io->ssl_peer_verification = 1L;
		}
		else
		{
			file_io->ssl_host_verification = 0L;
			file_io->ssl_peer_verification = 0L;
		}

		/* Network related settings are located in a separate config file */
		/*app_config_read_proxy_file( proxy );*/
	}
	return result;
}
#endif

static iot_status_t device_manager_make_control_command( char *full_path,
	size_t max_len, struct device_manager_info *device_manager,
	const char *options )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( full_path && device_manager && options )
	{
		char *ptr = NULL;
		size_t offset = 0u;

		result = IOT_STATUS_SUCCESS;
		os_strncpy( full_path, COMMAND_PREFIX, max_len );
		full_path[ max_len - 1u ] = '\0';
		offset = os_strlen( full_path );
		ptr = full_path + offset;
#if defined( _WIN32 )
		if ( offset < max_len - 1u )
		{
			*ptr = '"';
			++ptr;
			*ptr = '\0';
			++offset;
		}
		else
			result = IOT_STATUS_FULL;
#endif /* defined( _WIN32 ) */
		if ( result == IOT_STATUS_SUCCESS )
			result = os_make_path( ptr, max_len - offset,
				device_manager->app_path,
				IOT_CONTROL_TARGET, NULL );
		if ( result == IOT_STATUS_SUCCESS )
		{
			full_path[ max_len - 1u ] = '\0';
			offset = os_strlen( full_path );
			ptr = full_path + offset;
#if defined( _WIN32 )
			if ( offset < max_len - 1u )
			{
				*ptr = '"';
				++ptr;
				*ptr = '\0';
				++offset;
			}
			else
				result = IOT_STATUS_FULL;
#endif /* defined( _WIN32 ) */
			if ( *options != ' ' && offset < max_len - 1u )
			{
				*ptr = ' ';
				++ptr;
				*ptr = '\0';
				++offset;
			}
			if ( os_strlen( options ) < max_len - offset )
			{
				os_strncpy( ptr, options, max_len - offset );
				full_path[ max_len - 1u ] = '\0';
			}
			else
				result = IOT_STATUS_FULL;
		}
	}
	return result;
}

#if defined( __ANDROID__ )
iot_status_t device_manager_run_os_command( const char *cmd,
	iot_bool_t blocking_action )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( cmd )
	{
		char buf[1] = "\0";
		char *out_buf[2u] = { buf, buf };
		size_t out_len[2u] = { 1u, 1u };
		int retval = -1;

		if ( blocking_action == IOT_FALSE )
		{
			size_t i;
			for ( i = 0u; i < 2u; ++i )
			{
				out_buf[i] = NULL;
				out_len[i] = 0u;
			}
		}
		if ( os_system_run( cmd, &retval, out_buf,
			out_len, 0u ) == IOT_STATUS_SUCCESS &&
			     retval >= 0 )
			result = IOT_STATUS_SUCCESS;
		else
		{
			result = IOT_STATUS_FAILURE;
			IOT_LOG( APP_DATA.iot_lib, IOT_LOG_TRACE,
				"OS cmd (%s): return value %d",
				cmd, retval );
		}
	}
	return result;
}
#endif /* __ANDROID__ */

int device_manager_main( int argc, char *argv[] )
{
	int result = EXIT_FAILURE;
	const char *config_file = NULL;
	struct app_arg args[] = {
		{ 'c', "configure", 0, "file", &config_file,
			"configuration file", 0u },
		{ 'h', "help", 0, NULL, NULL, "display help menu", 0u },
		{ 's', "service", 0, NULL, NULL, "run as a service", 0u },
		{ 0, NULL, 0, NULL, NULL, NULL, 0u }
	};

	result = app_arg_parse(args, argc, argv, NULL);
	if (result == EXIT_FAILURE || app_arg_count(args, 'h', NULL))
		app_arg_usage(args, 36u, argv[0],
		IOT_DEVICE_MANAGER_TARGET, NULL, NULL);
	else if (result == EXIT_SUCCESS)
	{
#if defined( __ANDROID__ )
		const char *os_config_command[] = {
			ENABLE_TELNETD_LOCALHOST,
			NULL
		};
		const char **os_command = os_config_command;
		while ( os_command && *os_command )
		{
			device_manager_run_os_command(
				*os_command, IOT_TRUE );
			++os_command;
		}
#endif /* __ANDROID__ */

		os_memzero( &APP_DATA, sizeof(struct device_manager_info) );

/** @todo vxWorks checking iot.cfg will be implemented later */
#ifndef _WRS_KERNEL
		device_manager_config_read( &APP_DATA, argv[0], config_file );
#endif
		if ( app_arg_count( args, 's', "service" ) > 0u )
		{
			const char *remove_args[] = { "-s", "--service" };
			result = os_service_run(
				IOT_DEVICE_MANAGER_TARGET, device_manager_main,
				argc, argv,
				sizeof( remove_args ) / sizeof( const char* ),
				remove_args, &device_manager_sig_handler,
				APP_DATA.runtime_dir );
		}
		else
		{

			if ( device_manager_initialize(argv[0], &APP_DATA )
				== IOT_STATUS_SUCCESS )
			{
				os_terminate_handler(device_manager_sig_handler);
				IOT_LOG( APP_DATA.iot_lib, IOT_LOG_INFO, "%s",
					"Ready for some actions..." );

				while ( APP_DATA.iot_lib &&
					APP_DATA.iot_lib->to_quit == IOT_FALSE )
				{
#ifdef	__ANDROID__
					device_manager_file_create_default_directories( &APP_DATA, 1u );
#endif
					os_time_sleep( POLL_INTERVAL_MSEC,
						IOT_FALSE );
#ifndef NO_FILEIO_SUPPORT
					/*FIXME*/
					/*device_manager_file_check_pending_transfers(*/
					/*&APP_DATA );*/
#endif
				}
#ifndef NO_FILEIO_SUPPORT
				/*FIXME*/
				/*device_manager_file_cancel_all( &APP_DATA );*/
#endif
				IOT_LOG(APP_DATA.iot_lib, IOT_LOG_INFO, "%s",
					"Exiting..." );
				result = EXIT_SUCCESS;
			}
			else
				IOT_LOG( NULL, IOT_LOG_INFO, "%s",
					"Failed to initialize device-manager" );

			device_manager_terminate(&APP_DATA);
		}
	}
	return result;
}

