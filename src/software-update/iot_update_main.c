/**
 * @file
 * @brief Source file for software updates specific functions
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */
#include "iot_update_main.h"
#include "../api/shared/iot_defs.h"
#include "iot_json.h"
#include "os.h"
#include "utilities/app_arg.h"
#include "iot_build.h"
#include <errno.h>
#include <stdarg.h>                    /* for va_list, va_start, va_end */
#include <stdio.h>   /* for printf */
#include <string.h>  /* for strncpy */
#include <sys/wait.h>   /* for waitpid */


/**
 * @brief Maximum length for ota script/command output
 */
#define IOT_UPDATE_COMMAND_OUTPUT_MAX_LEN 1024u
/**
 * @brief Json file for ota
 */
#define IOT_UPDATE_JSON "update.json"
/**
 * @brief Maximum length for iot each message log
 */
#define IOT_UPDATE_LOG_MAX_LEN 128u
/**
 * @brief Log file for ota
 */
#define IOT_UPDATE_LOGFILE "iot_install_updates.log"
/**
 * @brief Maximum length for iot update timestamp
 */
#define IOT_UPDATE_TIMESTAMP_LENGTH 16u

#if defined( __unix__ ) && !defined( __ANDROID__ )
#	define COMMAND_PREFIX      "sudo "
#else
#	define COMMAND_PREFIX      ""
#endif

/** @brief iot update log output */
enum iot_update_log_output
{
	/** @brief log to send to cloud */
	IOT_UPDATE_LOG_CLOUD_ONLY = 0,
	/** @brief log to file */
	IOT_UPDATE_LOG_FILE_ONLY,
	/** @brief log to file and cloud */
	IOT_UPDATE_LOG_FILE_CLOUD
};

/** @brief IDP MEC availability */
enum iot_update_mec_available
{
	/** @brief IDP MEC is unavailable */
	IOT_UPDATE_MEC_UNAVAILABLE = 0,
	/** @brief IDP MECis available */
	IOT_UPDATE_MEC_AVAILABLE
};

/** @brief Enable/disable IDP MEC */
enum iot_update_mec_enable
{
	/** @brief Disable IDP MEC */
	IOT_UPDATE_MEC_DISABLE = 0,
	/** @brief Enable IDP MEC */
	IOT_UPDATE_MEC_ENABLE
};

/** @brief IDP MEC security status */
enum iot_update_mec_status
{
	/** @brief MEC security enabled */
	IOT_UPDATE_MEC_STATUS_ENABLED = 0,
	/** @brief MEC security not enabled or in update status*/
	IOT_UPDATE_MEC_STATUS_NOT_ENABLED,
	/** @brief MEC security unknown status */
	IOT_UPDATE_MEC_STATUS_UNKNOWN
};

/** @brief iot update install phase */
enum iot_update_install_phase
{
	/** @brief pre-install phase */
	OTA_PHASE_PRE_INSTALL = 0,
	/** @brief install phase */
	OTA_PHASE_INSTALL,
	/** @brief post-install phase */
	OTA_PHASE_POST_INSTALL,
	/** @brief an occured error during installation */
	OTA_PHASE_ERROR
};

/**
 * @brief Structure containing information for iot update script
 */
struct iot_update_install_script {
	/** @brief install phase */
	const char *name;
	/** @brief install script/command */
	char *script;
};

/**
 * @brief Contains main code for iot update install
 *
 * @param[in]      sw_update_path           update package directory
 *
 * @retval OS_STATUS_BAD_PARAMETER    on failure
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_SUCCESS          on success
 */
static int iot_update( const char * sw_update_path);

/**
 * @brief Enable/disable IDP mec security
 *
 * @param[in]      enable       action to enable/disable MEC
 *
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_SUCCESS          on success
 */
static os_status_t iot_update_mec_enable(enum iot_update_mec_enable enable );
/**
 * @brief Check if it is IDP security system
 *
 * @retval a mec availability status            MEC available/unavailable
 */
static enum iot_update_mec_available iot_update_mec_is_available( void );
/**
 * @brief Check IDP mec security status
 *
 * @retval a mec status          MEC status
 */
static enum iot_update_mec_status iot_update_mec_status( void );
 /**
 * @brief Function to log iot update information
 *
 * @param[in, out] lib    	pointer to iot lib
 * @param[in, out] log_file     log file to write to
 * @param[in]      output       log to file/cloud
 * @param[in]      format       string format
 */
static void iot_update_log( iot_t *lib,os_file_t log_file,
	enum iot_update_log_output output, const char *fmt,... )
	__attribute__((format(printf,4,5)));
/**
 * @brief Function to parse iot update json file
 *
 * @param[in]      json_file             json file to be parsed
 * @param[in, out] iot_update_install    install script
 *
 * @retval OS_STATUS_FAILURE          on failure
 * @retval OS_STATUS_SUCCESS          on success
 */
static os_status_t iot_update_parse_json(
	const char* json_file, struct iot_update_install_script * iot_update_install);

/** @brief it should be implemented in osal. will be removed from here */
#define LOOP_WAIT_TIME 100u
static os_status_t iot_os_system_run(
	const char *command,
	int *exit_status,
	char *out_buf[2u],
	size_t out_len[2u],
	iot_millisecond_t max_time_out );

os_status_t iot_os_system_run(
	const char *command,
	int *exit_status,
	char *out_buf[2u],
	size_t out_len[2u],
	iot_millisecond_t max_time_out )
{
	int command_output_fd[2u][2u] =
		{ { -1, -1 }, { -1, -1 } };
	size_t i;
	const int output_fd[2u] = { STDOUT_FILENO, STDERR_FILENO };
	os_status_t result = OS_STATUS_SUCCESS;
	iot_timestamp_t start_time;
	int system_result = -1;
	/*iot_millisecond_t time_elapsed;*/
	const iot_bool_t wait_for_return =
		( out_buf[0] != NULL && out_len[0] > 0 ) ||
		( out_buf[1] != NULL && out_len[1] > 0 );

	os_time( &start_time, NULL );

	/* set a default exit status */
	if ( exit_status )
		*exit_status = -1;

	/* capture the stdout & stderr of the command and send it back
	 * as the response */
	if ( wait_for_return != IOT_FALSE )
		for ( i = 0u; i < 2u && result == OS_STATUS_SUCCESS; ++i )
			if ( pipe( command_output_fd[i] ) != 0 )
				result = OS_STATUS_IO_ERROR;

	if ( result == OS_STATUS_SUCCESS )
	{
		if (  wait_for_return != IOT_FALSE )
		{
			const pid_t pid = fork();
			result = OS_STATUS_NOT_EXECUTABLE;
			if ( pid != -1 )
			{
				if ( pid == 0 )
				{
					/* Create a new session for the child process.
					 */
					pid_t sid = setsid();
					if ( sid < 0 )
						exit( errno );
					/* redirect child stdout/stderr to the pipe */
					for ( i = 0u; i < 2u; ++i )
					{
						dup2( command_output_fd[i][1], output_fd[i] );
						close( command_output_fd[i][0] );
					}
#ifdef __ANDROID__
					execl( "/system/bin/sh", "sh", "-c", command, (char *)NULL );
#else
					execl( "/bin/sh", "sh", "-c", command, (char *)NULL );
#endif
					/* Process failed to be replaced, return failure */
					exit( errno );
				}

				for ( i = 0u; i < 2u; ++i )
					close( command_output_fd[i][1] );

				result = OS_STATUS_INVOKED;
				errno = 0;
				do {
					waitpid( pid, &system_result, WNOHANG );
					/*iot_os_time_elapsed( &start_time, &time_elapsed );*/
					os_time_sleep( LOOP_WAIT_TIME, IOT_FALSE );
				} while ( ( errno != ECHILD ) &&
					( !WIFEXITED( system_result ) ) &&
					( !WIFSIGNALED( system_result ) ) &&
					( max_time_out == 0u /*|| time_elapsed < max_time_out */) );

				if ( ( errno != ECHILD ) &&
					!WIFEXITED( system_result ) &&
					!WIFSIGNALED( system_result ) )
				{
					kill( pid, SIGTERM );
					waitpid( pid, &system_result, WNOHANG );
					result = OS_STATUS_TIMED_OUT;
				}
				else
					result = OS_STATUS_SUCCESS;

				fflush( stdout );
				fflush( stderr );

				for ( i = 0u; i < 2u; ++i )
				{
					if ( out_buf[i] && out_len[i] > 0u )
					{
						out_buf[i][0] = '\0';
						/* if we are able to read from pipe */
						if ( command_output_fd[i][0] != -1 )
						{
							const ssize_t output_size =
								read( command_output_fd[i][0],
								out_buf[i], out_len[i] - 1u );
							if ( output_size >= 0 )
								out_buf[i][ output_size ] = '\0';
						}
					}
				}

				if ( WIFEXITED( system_result ) )
					system_result = WEXITSTATUS( system_result );
				else if ( WIFSIGNALED( system_result ) )
					system_result = WTERMSIG( system_result );
				else
					system_result = WIFEXITED( system_result );
				if ( exit_status )
					*exit_status = system_result;
			}
		}
		else
		{
			system_result = system( command );
			if ( exit_status &&
			     system_result != -1 &&
			     system_result != 127 )
				*exit_status = 0;
		}
	}
	return result;
}

int iot_update( const char * sw_update_path)
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( sw_update_path /*&& iot_os_directory_exists( sw_update_path )*/ )
	{
		char cwd[1024u];
		os_file_t sw_update_logfile = NULL;
		iot_t * iot_lib;
		char device_uuid[IOT_ID_MAX_LEN] = "";
		char thing_key[IOT_ID_MAX_LEN] = "";
		size_t len = 0;

		if ( app_get_device_uuid( "/etc/iot/device_id", device_uuid,
				IOT_ID_MAX_LEN ) != OS_STATUS_SUCCESS )
		{
			os_fprintf( OS_STDERR, "Failed to get device uuid. Exit!\n");
			os_exit( 1 );
		}

		len = os_strlen(IOT_UPDATE_TARGET) + os_strlen(device_uuid) + 1u;
		if ( len < IOT_ID_MAX_LEN )
			len = IOT_ID_MAX_LEN;

		os_snprintf( thing_key, len + 1, "%s-%s",
				device_uuid, IOT_UPDATE_TARGET );
		os_printf( "thing_key: %s\n", thing_key );

		iot_lib = iot_initialize( thing_key, NULL, 0 );
		result = iot_connect( iot_lib, 0u );

		if ( result == OS_STATUS_SUCCESS )
		{
			iot_update_log (iot_lib, NULL,
				IOT_UPDATE_LOG_CLOUD_ONLY,
				"iot_update: Start!");

			os_directory_current( cwd, PATH_MAX );
			os_directory_change(sw_update_path);

			sw_update_logfile = os_file_open(
				IOT_UPDATE_LOGFILE, OS_WRITE|OS_APPEND );
			if ( sw_update_logfile == NULL)
				result = OS_STATUS_FAILURE;
		}

		if ( result == OS_STATUS_SUCCESS )
		{
			char pre_install_script[ PATH_MAX + 1u ];
			char install_script[ PATH_MAX + 1u ];
			char post_install_script[ PATH_MAX + 1u ];
			char err_install_script[ PATH_MAX + 1u ];
			char need_reboot[ 128u ];
			enum iot_update_mec_status mec_original_status =
				IOT_UPDATE_MEC_STATUS_UNKNOWN;
			enum iot_update_mec_available  mec_available =
				IOT_UPDATE_MEC_UNAVAILABLE;
			char *out_buf[2u];
			size_t out_len[2u];
			char buf_stderr[ IOT_UPDATE_COMMAND_OUTPUT_MAX_LEN + 1u ];
			char buf_stdout[ IOT_UPDATE_COMMAND_OUTPUT_MAX_LEN + 1u ];

			struct iot_update_install_script iot_update_install[] = {
				{ "pre_install" ,  pre_install_script },
				{ "install"     ,  install_script },
				{ "post_install",  post_install_script },
				{ "error_action" , err_install_script },
				{ "reboot",        need_reboot},
				{NULL, NULL}
			};

			out_buf[0] = buf_stdout;
			out_len[0] = IOT_UPDATE_COMMAND_OUTPUT_MAX_LEN;
			out_buf[1] = buf_stderr;
			out_len[1] = IOT_UPDATE_COMMAND_OUTPUT_MAX_LEN;

			iot_update_log( iot_lib, sw_update_logfile,
				IOT_UPDATE_LOG_FILE_ONLY,
				"Downloading and extracting the Update Package ... Successful! " );

			result = OS_STATUS_FAILURE;
			mec_available = iot_update_mec_is_available();
			if ( mec_available == IOT_UPDATE_MEC_AVAILABLE )
			{
				mec_original_status = iot_update_mec_status();

				if ( mec_original_status == IOT_UPDATE_MEC_STATUS_ENABLED)
				{
					result = iot_update_mec_enable( IOT_UPDATE_MEC_DISABLE );
					if ( result == OS_STATUS_SUCCESS )
						iot_update_log( iot_lib, sw_update_logfile,
							IOT_UPDATE_LOG_FILE_ONLY,
							"MEC security is disabled" );
					else
						iot_update_log( iot_lib, sw_update_logfile,
							IOT_UPDATE_LOG_FILE_ONLY,
							"Disable MEC failed" );				}
				else
					iot_update_log( iot_lib, sw_update_logfile,
						IOT_UPDATE_LOG_FILE_ONLY,
						"MEC original status is not enabled,"
						"don't need to be disabled" );
			}

			/*
			 * parse update.json file
			 */
			/*if ( os_file_exists ( IOT_UPDATE_JSON ) )*/
			{
				result = iot_update_parse_json(
					IOT_UPDATE_JSON,
					iot_update_install);
			}
			if ( result == OS_STATUS_SUCCESS )
			{
				size_t i;
				size_t j;
				int exit_status;
				/*
				 * execute install scripts/commands
				 */
				for ( i = OTA_PHASE_PRE_INSTALL; 
					i < OTA_PHASE_ERROR && 
					result == OS_STATUS_SUCCESS; 
					i++ )
				{
					if ( iot_update_install[i].script[0] != '\0' &&
						os_strcmp( iot_update_install[i].script, " " ) != 0)
					{
						char *param = iot_update_install[i].script ;
						size_t  param_length =
							os_strlen( iot_update_install[i].script );

						/* remove escape character backslash '\' added in Json file*/
						while ( ( param = os_strstr( param, "\\") ) != NULL )
						{
							char *dst = param;
							char *src = param + 1;
							os_memmove(dst, src, param_length + 1 );
							param_length -= 1;
							/* if the next character is still '\'
							 * it is character which is escaped
							 * don't delete it
							 */
							if(param && param[0] == '\\')
							{
								param ++;
								param_length -= 1;
							}
						}

						for ( j = 0u; j < 2u; ++j )
						{
							if ( out_buf[j] )
							{
								os_memset(
								out_buf[j], 0,out_len[j] );
							}
						}
						exit_status = -1;
						iot_update_log( iot_lib, sw_update_logfile,
							IOT_UPDATE_LOG_FILE_CLOUD,
							"Executing %s (%s) ... Start!",
							iot_update_install[i].name,
							iot_update_install[i].script );

						result = iot_os_system_run(
							iot_update_install[i].script, 
							&exit_status,
							out_buf, out_len, 0 );

						for ( j = 0u; j < 2u; ++j )
						{
							if ( out_buf[j] && out_buf[j][0] != '\0' )
							{
								os_fprintf(
								sw_update_logfile,
								"%s log: \n%s \n",
								iot_update_install[i].name,
								out_buf[j] );
							}
						}

						if ( result != OS_STATUS_SUCCESS || exit_status != 0 )
						{
							iot_update_log( iot_lib, sw_update_logfile,
								IOT_UPDATE_LOG_FILE_CLOUD,
								"Error: Executing %s (%s) ... Failed! ",
								iot_update_install[i].name, iot_update_install[i].script );

							if ( iot_update_install[OTA_PHASE_ERROR].script[0] != '\0' &&
								os_strcmp( iot_update_install[OTA_PHASE_ERROR].script, " " ) != 0)
							{
								iot_update_log( iot_lib, sw_update_logfile,
									IOT_UPDATE_LOG_FILE_CLOUD,
									"Executing %s (%s) ... Start!",
									iot_update_install[OTA_PHASE_ERROR].name,
									iot_update_install[OTA_PHASE_ERROR].script);

								result = iot_os_system_run(
									iot_update_install[OTA_PHASE_ERROR].script,
									&exit_status,
									out_buf, out_len, 0 );

								if ( result == OS_STATUS_SUCCESS )
								{
									iot_update_log( iot_lib, sw_update_logfile,
										IOT_UPDATE_LOG_FILE_CLOUD,
										"Executing %s (%s) ... "
										"Successful!",
										iot_update_install[OTA_PHASE_ERROR].name,
										iot_update_install[OTA_PHASE_ERROR].script);
								}
								else
								{
									iot_update_log( iot_lib, sw_update_logfile,
										IOT_UPDATE_LOG_FILE_CLOUD,
										"Executing %s (%s) ... "
										"Failed!",
										iot_update_install[OTA_PHASE_ERROR].name,
										iot_update_install[OTA_PHASE_ERROR].script);
								}
							}
							else
								iot_update_log( iot_lib, sw_update_logfile,
									IOT_UPDATE_LOG_FILE_ONLY,
									"Info: No %s script provided ",
									iot_update_install[OTA_PHASE_ERROR].name
									);
							result = OS_STATUS_FAILURE;
						}
						else
						{
							iot_update_log( iot_lib, sw_update_logfile,
								IOT_UPDATE_LOG_FILE_CLOUD,
								"Executing %s (%s) ... Successful!",
								iot_update_install[i].name,
								iot_update_install[i].script );
						}
					}
					else
						iot_update_log( iot_lib, sw_update_logfile,
							IOT_UPDATE_LOG_FILE_ONLY,
							"Info: No %s script provided ",
							iot_update_install[i].name
							);
				}
				if ( mec_available == IOT_UPDATE_MEC_AVAILABLE )
				{
					if ( mec_original_status == IOT_UPDATE_MEC_STATUS_ENABLED )
					{
						if ( iot_update_mec_status() ==
							IOT_UPDATE_MEC_STATUS_NOT_ENABLED )
						{
							if ( ( result = iot_update_mec_enable(IOT_UPDATE_MEC_ENABLE)) !=
								OS_STATUS_SUCCESS )
								iot_update_log( iot_lib, sw_update_logfile,
									IOT_UPDATE_LOG_FILE_ONLY,
									"Enable MEC failed" );
							else
								iot_update_log( iot_lib, sw_update_logfile,
									IOT_UPDATE_LOG_FILE_ONLY,
									"MEC is enabled" );
						}
					}
					else
						iot_update_log( iot_lib, sw_update_logfile,
							IOT_UPDATE_LOG_FILE_ONLY,
							"MEC original status is not enabled,"
							"don't need to be enabled" );
				}
			}

			/*
			 * check if it need reboot
			 */
			if ( (result == OS_STATUS_SUCCESS) 
				&& os_strncmp( need_reboot,
				"yes", 3u ) == 0)
			{
				/*reboot = 1;*/
				iot_update_log( iot_lib, sw_update_logfile,
					IOT_UPDATE_LOG_FILE_CLOUD,
					"Device will reboot after 1 minute... "
					);
			}
			/*
			 * ota install result
			 */
			if ( result == OS_STATUS_SUCCESS )
			{
				iot_update_log( iot_lib, sw_update_logfile,
					IOT_UPDATE_LOG_FILE_CLOUD,
					"Software Update Finish... Successful! "
					);
			}
			else
			{
				iot_update_log( iot_lib, sw_update_logfile,
					IOT_UPDATE_LOG_FILE_CLOUD,
					"Software Update Finish... Failed! " );
			}

			if ( sw_update_logfile )
			{
				os_file_close( sw_update_logfile );
				sw_update_logfile = NULL;
			}
#if 0
			/*
			 * copy ota log file to device upload directory
			 */
			if (IOT_UPDATE_LOGFILE[0] != '\0' /*&&
				iot_os_file_exists( IOT_UPDATE_LOGFILE )*/ )
			{
				char device_upload_dir[ PATH_MAX + 1u ];

				os_fsync ( IOT_UPDATE_LOGFILE );
				if ( OS_STATUS_SUCCESS == iot_os_make_path( device_upload_dir,
					PATH_MAX, IOT_RUNTIME_DIR, "upload", NULL ) )
				{
					if ( iot_os_directory_exists( device_upload_dir ) )
					{
						char device_upload_ota_logfile[ PATH_MAX + 1u ];
						char device_upload_ota_logfile_backup[ PATH_MAX + 1u ];
						char timestamp[IOT_UPDATE_TIMESTAMP_LENGTH  + 1u];
						unsigned int i = 0;

						iot_os_time_format( timestamp, IOT_UPDATE_TIMESTAMP_LENGTH );
						while ( timestamp[i] != '\0' && i <= IOT_UPDATE_TIMESTAMP_LENGTH )
						{
							if (timestamp[i] == ' ' || timestamp[i] == ':')
								timestamp[i] = '_';
							i ++;
						}
						iot_os_snprintf( device_upload_ota_logfile_backup,
							PATH_MAX,
							"%s.%s",
							IOT_UPDATE_LOGFILE,
							timestamp);
						if ( OS_STATUS_SUCCESS ==
							iot_os_make_path(
								device_upload_ota_logfile,
								PATH_MAX,
								device_upload_dir,
								device_upload_ota_logfile_backup,
								NULL )
							)
						{
							iot_os_file_copy(
								IOT_UPDATE_LOGFILE,
								device_upload_ota_logfile );
						}
					}
				}
			}
#endif
		}
		else
			iot_update_log( iot_lib, sw_update_logfile,
				IOT_UPDATE_LOG_FILE_CLOUD,
				"Error: Open iot update json file ... Failed!\n " );
		if ( cwd[0] != '\0' )
			os_directory_change(cwd);
#if 0
		/*
		 * reboot if required
		 */
		if ( reboot == 1)
		{
#ifdef _WIN32
			iot_os_system_shutdown( IOT_TRUE, 1 );
#else
			char *buf[2] = { NULL, NULL };
			size_t buf_len[2] = { 0u, 0u };
			char command_with_params[ 32u ];
			iot_os_strncpy( command_with_params,
				COMMAND_PREFIX"iot-control --reboot 1",
				31u );
			iot_os_system_run( command_with_params,
				NULL, buf, buf_len, 0u );
#endif
		}
#endif
	}
	return result;
}

os_status_t iot_update_mec_enable( enum iot_update_mec_enable enable )
{
	char *buf[2] = { NULL, NULL };
	size_t buf_len[2] = { 0u, 0u };
	char command [32u];
	int exit_status = 0;
	os_status_t result = OS_STATUS_FAILURE;

	if ( enable == IOT_UPDATE_MEC_ENABLE )
		os_snprintf( command,
			31u,
			COMMAND_PREFIX" sadmin eu"
			);
	else
		os_snprintf( command,
			31u,
			COMMAND_PREFIX" sadmin bu"
			);

	result = iot_os_system_run(
		command,
		&exit_status,
		buf, buf_len, 0 );

	if ( result == OS_STATUS_SUCCESS && exit_status == 0 )
	{
		enum iot_update_mec_status mec_status;
		result = OS_STATUS_FAILURE;
		mec_status = iot_update_mec_status();
		if( ( enable == IOT_UPDATE_MEC_ENABLE &&
			mec_status == IOT_UPDATE_MEC_STATUS_ENABLED ) ||
			( enable == IOT_UPDATE_MEC_DISABLE &&
			mec_status == IOT_UPDATE_MEC_STATUS_NOT_ENABLED )
			)
			result = OS_STATUS_SUCCESS;
	}
	else
		result = OS_STATUS_FAILURE;
	return result;
}

enum iot_update_mec_available iot_update_mec_is_available( void )
{
#if 0
	char iot_update_path[PATH_MAX + 1u];
	enum iot_update_mec_available  mec_available = IOT_UPDATE_MEC_UNAVAILABLE;

	if ( app_path_which( iot_update_path, PATH_MAX, NULL, "sadmin" ))
		mec_available = IOT_UPDATE_MEC_AVAILABLE;
	return mec_available;
#else
	return IOT_UPDATE_MEC_UNAVAILABLE;
#endif
}
enum iot_update_mec_status iot_update_mec_status( void )
{
	char *out_buf[2u];
	size_t out_len[2u];
	char buf_stderr[ IOT_UPDATE_COMMAND_OUTPUT_MAX_LEN + 1u ];
	char buf_stdout[ IOT_UPDATE_COMMAND_OUTPUT_MAX_LEN + 1u ];
	int exit_status = 0;
	os_status_t result = OS_STATUS_FAILURE;
	enum iot_update_mec_status mec_status = IOT_UPDATE_MEC_STATUS_UNKNOWN;

	out_buf[0] = buf_stdout;
	out_len[0] = IOT_UPDATE_COMMAND_OUTPUT_MAX_LEN;
	out_buf[1] = buf_stderr;
	out_len[1] = IOT_UPDATE_COMMAND_OUTPUT_MAX_LEN;

	result = iot_os_system_run(
		COMMAND_PREFIX"sadmin status",
		&exit_status,
		out_buf, out_len, 0 );

	if ( result == OS_STATUS_SUCCESS && exit_status == 0 )
	{
		if ( os_strstr( out_buf[0], "enable" ) != NULL )
			mec_status = IOT_UPDATE_MEC_STATUS_ENABLED;
		else
			mec_status = IOT_UPDATE_MEC_STATUS_NOT_ENABLED;
	}

	return mec_status;
}

os_status_t iot_update_parse_json(
	const char* json_file, struct iot_update_install_script * iot_update_install)
{
	os_status_t status = OS_STATUS_BAD_PARAMETER;
	if (json_file && iot_update_install )
	{
		os_file_t fd;
		char *json_string;
		size_t size = 0;

		status = OS_STATUS_FILE_OPEN_FAILED;
		fd = os_file_open( json_file, OS_READ );
		if( fd )
		{
			long cur_pos = os_file_tell( fd );
			if ( os_file_seek( fd, 0, SEEK_END ) == 0 )
			{
				size = os_file_tell( fd );
				if ( cur_pos != (long)size )
					os_file_seek( fd, cur_pos, SEEK_SET );
			}
		}
		else
			os_fprintf(OS_STDERR,
				" failed to open json file!\n");

		if ( size != 0 )
		{
			json_string = (char *)os_malloc( size + 1 );
			if ( json_string )
			{
				char buf[1024u];
				iot_json_decoder_t *json;
				iot_json_item_t *root;

				size = os_file_read( json_string, 1, size, fd );
				json_string[size] = '\0';
				json = iot_json_decode_initialize( buf, 1024u, 0u );
				if ( json &&
					iot_json_decode_parse( json, json_string, size,
						&root, NULL, 0u) == IOT_STATUS_SUCCESS )
				{
					if ( 1 )
					{
						const char *bad_field = NULL;
						size_t i = 0;
						iot_json_item_t *j_obj;
						/*iot_json_decode_array_start( json );*/
						while (iot_update_install[i].name != NULL
							&& iot_update_install[i].script
							&& bad_field == NULL)
						{
							j_obj = iot_json_decode_object_find(
								json, root, iot_update_install[i].name );

							if ( j_obj &&
								iot_json_decode_type( json, j_obj )
								== IOT_JSON_TYPE_STRING )
							{
								const char *v = NULL;
								size_t v_len = 0u;

								iot_json_decode_string( json,
									j_obj,
									&v,
									&v_len );
								os_snprintf( iot_update_install[i].script,
									PATH_MAX,
									"%.*s",
									(int)v_len,
									v );
								iot_update_install[i].script[ PATH_MAX ] = '\0';
								os_printf("%s: %s\n",
									iot_update_install[i].name,
									iot_update_install[i].script);
								i ++;
							}
							else
								bad_field = iot_update_install[i].name;
						}

						if (bad_field == NULL && i != 0 )
							status = OS_STATUS_SUCCESS;
						else
							os_fprintf( OS_STDERR,
								"Error: invalid field: %s",
								bad_field);
					}
				}
				iot_json_decode_terminate( json );
				os_free( json_string );
			}
		}
		else
			os_fprintf(OS_STDERR,
				" the size of json file is 0!\n");
	}

	return status;
}

/* main entry point function */
int iot_update_main( int argc, char *argv[] )
{
	int result = OS_STATUS_BAD_PARAMETER;
	if ( argc == 2 && (os_strcmp( argv[1], "-h") == 0
		|| os_strcmp( argv[1], "--help")) )
		os_printf("Command format:\n"
			"iot-update --path [software udpate package path]");
	else if (argc == 3 )
	{
		if ( *argv[2] != '\0' &&
			( os_strcmp(argv[1], "-p") == 0 || os_strcmp(argv[1], "--path") == 0 ) )
		{
			result = iot_update( argv[2] );

		}
		else
			os_printf("Wrong command parameter! It should be:\n"
				"iot-update --path [software udpate package path]\n");
	}
	else
		os_printf("Wrong command!\n"
			"Please type iot-update --help for help\n");

	return result;
}

void iot_update_log( iot_t *UNUSED(lib),os_file_t log_file,
	enum iot_update_log_output output, const char *fmt,... )
{
	char sw_update_log[ IOT_UPDATE_LOG_MAX_LEN + 1u];
	char timestamp[IOT_UPDATE_TIMESTAMP_LENGTH  + 1u];
	os_timestamp_t t;
	va_list args;

	/* Print time stamp
	 * Add it later
	 */

	os_time(&t, NULL );
	os_time_format( timestamp, IOT_UPDATE_TIMESTAMP_LENGTH,
		"%Y-%m-%dT%H:%M:%S", t, IOT_FALSE );

	va_start( args, fmt );
	os_vsnprintf( sw_update_log, IOT_UPDATE_LOG_MAX_LEN, fmt, args );
	sw_update_log[ IOT_UPDATE_LOG_MAX_LEN ] = '\0';
	va_end( args );

	if ( log_file && ( output == IOT_UPDATE_LOG_CLOUD_ONLY ||
			output == IOT_UPDATE_LOG_FILE_CLOUD ) )
		os_fprintf(log_file,
			"------------------------------------------------------\n"
			"- %s\n"
			"- %s\n"
			"------------------------------------------------------\n",
			timestamp,
			sw_update_log );

	/*if (lib && ( output == IOT_UPDATE_LOG_CLOUD_ONLY ||*/
	/*output == IOT_UPDATE_LOG_FILE_CLOUD ) )*/

		/*iot_plugin_perform(*/
		/*lib, NULL, 0,*/
		/*IOT_OPERATION_EVENT_LOG_PUBLISH,*/
		/*NULL, sw_update_log );*/
}
