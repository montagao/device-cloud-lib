/**
 * @file
 * @brief Wind River IoT configuration file reader
 *
 * @copyright Copyright (C) 2018 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "app_config.h"

#include <os.h>				/* for os_* functions */
#include "app_path.h"		/* for app_path_* functions */
#include "app_json.h"		/* for app_json_* functions */
#include "../public/iot_json.h"		/* for iot_json_* types */
#include "../api/shared/iot_types.h"	/* for os_* functions */

/** @brief Structure containing configuration information */
struct app_config
{
	app_json_decoder_t *json;          /** @brief json interface object for parsing */
	const app_json_item_t *json_root;
};

/**
 * @brief Helper function to get the path to proxy configuration file
 *
 * @param[out]     file_path           path to connection file
 * @param[in]      len                 length of buffer
 *
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameters
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t app_config_get_proxy_file_path(
char *file_path, size_t len );

iot_status_t app_config_close( struct app_config *config )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( config )
	{
		app_json_decode_terminate( config->json );
		os_free( (void *)config );
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

struct app_config *app_config_open(/* iot_t *iot_lib,*/ const char *file_path )
{
	// TODO: We only use iot_lib for logging purposes. 
	// should we remove it to reduce dependencies?
	char config_file[ PATH_MAX + 1u ];
	iot_bool_t file_found = IOT_FALSE;
	unsigned int i;
	const char *paths[] = { file_path, NULL, NULL, NULL };
	struct app_config *result = NULL;
	char config_dir[ PATH_MAX + 1u ];
	/* char runtime_dir[ PATH_MAX + 1u ]; */
	char current_dir[ PATH_MAX + 1u ];
	char exe_dir[ PATH_MAX + 1u ];

	/* required json init variables */
	app_json_decoder_t *json = NULL; 
	char *json_string = NULL;
	size_t json_size = 0;
	const app_json_item_t *json_root = NULL; /* root json object */
	char err_msg[1024u];
	size_t err_len = 1024u ;
	os_file_t fd;

	/* locate the config file */
	config_file[0] = '\0';
	if ( !file_path || *file_path == '\0' )
	{
		/* if user doesn't specify file_path, check in
		 * config_dir, runtime_dir, current_dir and exe_dir
		 * for default configuration file */
		if ( app_path_config_directory_get( config_dir, PATH_MAX )
			== IOT_STATUS_SUCCESS && config_dir[0] != '\0' )
			paths[0u] = config_dir;
		/* if ( app_path_runtime_directory_get( runtime_dir, PATH_MAX ) */ /*FIXME*/
		/* 	== IOT_STATUS_SUCCESS && runtime_dir[0] != '\0' ) */
			/* paths[1u] = runtime_dir; */
		if ( os_directory_current( current_dir, PATH_MAX )
			== OS_STATUS_SUCCESS && current_dir[0] != '\0' )
			paths[2u] = current_dir;
		if ( app_path_executable_directory_get( exe_dir, PATH_MAX )
			== IOT_STATUS_SUCCESS && exe_dir[0] != '\0' )
			paths[3u] = exe_dir;
	}

	for ( i = 0u; i < sizeof( paths ) / sizeof( const char * ) &&
		paths[i] && file_found == IOT_FALSE; ++i )
	{
		const char *const path = paths[i];
		config_file[0] = '\0';
		if ( path )
		{
			if ( !file_path || *file_path == '\0' )
			{
				os_make_path( config_file, PATH_MAX,
					path, IOT_DEFAULT_FILE_DEVICE_MANAGER, NULL );
			}
			else
				os_strncpy( config_file, path,
					PATH_MAX + 1u );
			os_env_expand( config_file, PATH_MAX, PATH_MAX ); /* TODO : Verify correctness */
		}

		if ( *config_file != '\0' )
		{
			/* IOT_LOG( iot_lib, IOT_LOG_INFO, */
			/* 	"Looking for configuration file: %s", config_file ); */

			if ( os_file_exists( config_file ) )
			{
				fd = os_file_open( config_file, OS_READ );
				if ( fd == NULL )
					file_found = IOT_FALSE; /* for compiler warnings */
					/* IOT_LOG( iot_lib, IOT_LOG_ERROR, */
					/* 	"Cannot read configuration: %s", */
						/* config_file ); */
				else
					file_found = IOT_TRUE;
				os_file_close ( fd );
			}
		}
	}

	if ( file_found != IOT_FALSE )
	{
		/* IOT_LOG( iot_lib, IOT_LOG_INFO, */
		/* 	"Configuration file found: %s", config_file ); */
	
#ifdef IOT_STACK_ONLY
		/* initialize json decoder */
		char buffer[1024u];
		json = app_json_decode_initialize( 
			buffer, 1024u, 0u );
#else
		json = app_json_decode_initialize( NULL, 0u,
			IOT_JSON_FLAG_DYNAMIC );
#endif
		result = os_malloc(
				sizeof( struct app_config ) );
		json_size = (size_t)os_file_size( config_file );
		if ( result && json && json_size )
		{
			iot_status_t status = IOT_STATUS_FAILURE;
			os_memzero( result,
				sizeof( struct app_config ) );
			status = app_json_decode_parse( json,
				json_string,
				json_size, &json_root, err_msg, err_len  );

			if ( status != IOT_STATUS_SUCCESS )
			{
				/* IOT_LOG( iot_lib, IOT_LOG_ERROR, */
				/* 	"Error loading configuration file %s; " */
					/* "app_json_decode_parse failed : %d", */
					/* config_file, status ); */
				os_free( (void *)result );
				result = NULL;
			}
			else 
			{
				result->json = json;
				result->json_root = json_root;
			}
		}
		/* else */
			/* IOT_LOG( iot_lib, IOT_LOG_ERROR, */
			/* 	"Error loading configuration file %s; " */
			/* 	"Unable to allocate sufficient memory", */
				/* config_file ); */
	}
	return result;
}

iot_status_t app_config_read_boolean( const struct app_config *config,
	const char *group, const char *field, iot_bool_t *value )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( config && config->json && config->json_root && field && value )
	{
		app_json_decoder_t * json = config->json;
		const app_json_item_t *json_root = config->json_root;
		const app_json_item_t *json_bool = NULL;
		if ( group ) 
		{
			const app_json_item_t *json_group = NULL;
			json_group = app_json_decode_object_find( json,
				json_root, group );
			json_bool = app_json_decode_object_find( json,
				json_group, field );
		}
		else
			json_bool = app_json_decode_object_find( json, json_root, field );

		if ( json_bool )
			result = app_json_decode_bool( json, json_bool, value );
	}
	return result;
}

/* str_len is out */
iot_status_t app_config_read_string( const struct app_config *config,
	const char *group, const char *field, const char **value, size_t* str_len )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( config && config->json && config->json_root && field && value )
	{
		app_json_decoder_t * json = config->json;
		const app_json_item_t *json_root = config->json_root;
		const app_json_item_t *json_str = NULL;
		if ( group ) 
		{
			const app_json_item_t *json_group = NULL;
			json_group = app_json_decode_object_find( json,
				json_root, group );
			json_str = app_json_decode_object_find( json,
				json_group, field );
		}
		else
			json_str = app_json_decode_object_find( json, json_root, field );

		if (json_str )
			result = app_json_decode_string( json, json_str, value, str_len );
	}
	return result;
}

// ignore for now
/* iot_status_t app_config_read_string_array( const struct app_config *config, */
/* 	const char *group, const char *field, char token, char *value, */
/* 	size_t max ) */
/* { */
/* 	iot_status_t result = IOT_STATUS_BAD_PARAMETER; */
/* 	if ( config && field && value ) */
/* 	{ */
/* 		app_json_t json_array; */
/* 		app_json_t json_value; */
/* 		int size; */
/* 		int i; */
/* 		char *ptr; */

/* 		if ( group ) */
/* 		{ */
/* 			app_json_t json_group; */
/* 			result = app_json_object_get( &json_group, &config->json, group ); */
/* 			if ( result == IOT_STATUS_SUCCESS ) */
/* 				result = app_json_object_get( &json_array, &json_group, field ); */
/* 		} */
/* 		else */
/* 			result = app_json_object_get( &json_array, &config->json, field ); */

/* 		if ( result == IOT_STATUS_SUCCESS ) */
/* 		{ */
/* 			*value = '\0'; */
/* 			ptr = value; */
/* 			size = app_json_size( &json_array ); */
/* 			for ( i = 0; i < size && result == IOT_STATUS_SUCCESS; ++i ) */
/* 			{ */
/* 				result = app_json_array_get( &json_value, &json_array, i ); */
/* 				if ( result == IOT_STATUS_SUCCESS ) */
/* 				{ */
/* 					result = app_json_get_string_value( ptr, */
/* 						max - (ptr-value), &json_value ); */
/* 					if ( result == IOT_STATUS_SUCCESS && i < size-1 ) */
/* 					{ */
/* 						ptr = os_strchr( ptr, '\0' ); */
/* 						ptr[0] = token; */
/* 						++ptr; */
/* 					} */
/* 				} */
/* 			} */
/* 			if ( result != IOT_STATUS_SUCCESS ) */
/* 				*value = '\0'; */
/* 		} */
/* 	} */
/* 	return result; */
/* } */

iot_status_t app_config_read_integer( const struct app_config *config,
	const char *group, const char *field, iot_int64_t *value )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( config && config->json && config->json_root && field && value )
	{
		app_json_decoder_t * json = config->json;
		const app_json_item_t *json_root = config->json_root;
		const app_json_item_t *json_int = NULL;
		if ( group ) 
		{
			const app_json_item_t *json_group = NULL;
			json_group = app_json_decode_object_find( json,
				json_root, group );
			json_int = app_json_decode_object_find( json,
				json_group, field );
		}
		else
			json_int = app_json_decode_object_find( json, json_root, field );

		if (json_int )
			result = app_json_decode_integer( json, json_int, value );
	}
	return result;
}

iot_status_t app_config_get_proxy_file_path(
	char *path, size_t size )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( path )
	{
		char config_dir[ PATH_MAX + 1u ];
		os_memzero( path, size );
		result = app_path_config_directory_get( config_dir, PATH_MAX );
		if ( result == IOT_STATUS_SUCCESS )
			os_make_path( path, size, config_dir, IOT_DEFAULT_FILE_PROXY, NULL );
	}
	return result;
}

iot_status_t app_config_read_proxy_file(
	struct iot_proxy *proxy_info )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( proxy_info )
	{
		char file_path[ PATH_MAX + 1u ];

		os_memzero( proxy_info, sizeof( struct iot_proxy ) );
		result = app_config_get_proxy_file_path(
			file_path, sizeof( file_path ) );
		if ( result == IOT_STATUS_SUCCESS &&
			file_path[0] != '\0' && os_file_exists( file_path ) )
		{
			const char *const proxy_group = "proxy";
			struct app_config *const config =
				app_config_open( file_path );
			result = IOT_STATUS_NOT_FOUND;
			if ( config )
			{
				char *host= NULL;
				char *username = NULL ;
				char *password = NULL;

				const char *temp_string = NULL;
				size_t temp_string_len;
				iot_int64_t temp_value = 0u;

				result = app_config_read_string( config,
					proxy_group, "host", &temp_string, &temp_string_len );


				if ( result == IOT_STATUS_SUCCESS &&
					temp_string[0] != '\0' )
				{
					host = os_malloc( temp_string_len + 1 );
					os_strncpy( host, temp_string, temp_string_len );
					host[temp_string_len] = '\0';
					proxy_info->host = host;
					if ( result == IOT_STATUS_SUCCESS )
					{
						result = app_config_read_integer(
							config, proxy_group,
							"port", &temp_value );
						if ( result == IOT_STATUS_SUCCESS )
							proxy_info->port = temp_value;
					}
				}

				if ( result == IOT_STATUS_SUCCESS )
				{
					result = app_config_read_string(
						config, proxy_group, "type",
						&temp_string, &temp_string_len );

					if ( os_strcmp( temp_string, "HTTP" ) == 0 )
						proxy_info->type = IOT_PROXY_HTTP;
					else if ( os_strcmp( temp_string, "SOCKS5" ) )
						proxy_info->type = IOT_PROXY_SOCKS5;
					else
						proxy_info->type = IOT_PROXY_UNKNOWN;
				}

				if ( result == IOT_STATUS_SUCCESS )
				{
					iot_status_t result2;
					result2 = app_config_read_string(
						config, proxy_group,
						"username", &temp_string, &temp_string_len );
					if ( result2 == IOT_STATUS_SUCCESS &&
						temp_string[0] != '\0' )
					{
						username = os_malloc( temp_string_len + 1 );
						os_strncpy( username, temp_string, temp_string_len );
						username[temp_string_len] = '\0';
						proxy_info->username = username;

						result2 = app_config_read_string(
							config, proxy_group,
							"password", &temp_string, &temp_string_len );
						if ( result2 == IOT_STATUS_SUCCESS
							&& temp_string[0] != '\0' )
						{
							password = os_malloc( temp_string_len + 1 );
							os_strncpy( password, temp_string, temp_string_len );
							password[temp_string_len] = '\0';
							proxy_info->password = password;
						}
					}
				}
				app_config_close( config );
			}
		}
	}
	return result;
}

/* TODO: Use json encoder instead of hardcoded proxy string */
/*
iot_status_t app_config_write_proxy_file(
	const struct iot_proxy *proxy_info )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( proxy_info )
	{
		char file_path[ PATH_MAX + 1u ];
		char buffer[ 1024u ];
		app_json_encoder_t *json_enc;
#ifdef IOT_STACK_ONLY
		json_enc = app_json_encode_initialize(
			buffer, 1024u, 0u );
#else
		json_enc = app_json_encode_initialize(
			NULL, 0u, IOT_JSON_FLAG_DYNAMIC );
#endif
		result = app_config_get_proxy_file_path(
			file_path, sizeof( file_path ) );
		if ( result == IOT_STATUS_SUCCESS )
		{
			os_file_t proxy_file;
			proxy_file = os_file_open( file_path,
				OS_WRITE|OS_CREATE );

	

			if ( proxy_file )
			{
				os_fprintf( proxy_file, PROXY_JSON,
						proxy_info->host, proxy_info->port,
						proxy_info->type, proxy_info->username,
						proxy_info->password );
				os_file_close( proxy_file );
				os_file_chown( file_path, IOT_USER );
			}
		}
	}
	return result;
}
*/
