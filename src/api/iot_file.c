/**
 * @file
 * @brief source file containing file transfer implementation
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "public/iot.h"

#include "iot_common.h"            /* for iot_common_arg_set */
#include "iot_plugin.h"            /* for plug-in support */

#include "shared/iot_types.h"      /* for struct iot */
#include "os.h"                    /* operating system abstraction */

#include <archive.h>               /* for archiving functions */
#include <archive_entry.h>         /* for adding files to an archive */

#define IOT_DEFAULT_RUNTIME_DIR  "/var/lib/iot"
#define IOT_DEFAULT_DOWNLOAD_DIR IOT_DEFAULT_RUNTIME_DIR "/download"
#define IOT_DEFAULT_UPLOAD_DIR   IOT_DEFAULT_RUNTIME_DIR "/upload"

/**
 * @brief Transfer a file/directory to/from cloud
 *
 * @param[in]      handle              library handle
 * @param[out]     txn                 transaction status (optional)
 * @param[in]      max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely)
 * @param[in]      use_global_store    Use global file store
 * @param[in]      file_name           cloud's file name (optional)
 *                                     if file name is not given, local file
 *                                     name will be used, if it is a directory,
 *                                     the full path will be used where all
 *                                     the separators are replaced with dashes
 * @param[in]      file_path           location of the local file.
 *                                     if path is not absolute, it will
 *                                     be relative to default directory.
 *                                     if it is a directory instead of a file,
 *                                     all files within that directory will be
 *                                     bundled together.
 * @param[in]      func                callback function to give
 *                                     progress update (optional)
 *                                     if none is given, progress will
 *                                     be printed in the logs at
 *                                     a regular interval
 * @param[in]      user_data           user's specific data for progress
 *                                     callback (optional)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          operation successful
 */
static iot_status_t iot_file_transfer(
	iot_t *handle,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	iot_bool_t use_global_store,
	iot_operation_t op,
	const char * const file_name,
	const char * const file_path,
	iot_file_progress_callback_t *func,
	void *user_data );

/**
 * @brief Transfer a file/directory to/from cloud
 *
 * @param[in,out]  path                path to the directory to archive
 *                                     and path to the archived file
 * @param[out]     len                 length of the buffer
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          operation successful
 */
static iot_status_t iot_file_archive_directory(
	char *path, size_t len );

iot_status_t iot_file_send(
	iot_t *handle,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	iot_file_store_t storage,
	const char *file_name,
	const char *file_path,
	iot_file_progress_callback_t *func,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;

	/* Need a flag to indicate to the subsystem to use the global
	 * file store or private one in the thing scope */
	iot_bool_t use_global_store = IOT_FALSE;

	if ( storage & IOT_FILE_FLAG_GLOBAL )
		use_global_store = IOT_TRUE;

	if ( handle && file_path )
		result = iot_file_transfer(
			handle, txn, max_time_out,
			use_global_store, IOT_OPERATION_FILE_PUT,
			file_name, file_path,
			func, user_data );
	return result;
}

iot_status_t iot_file_receive(
	iot_t *handle,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	iot_file_store_t storage,
	const char *file_name,
	const char *file_path,
	iot_file_progress_callback_t *func,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;

	/* Need a flag to indicate to the subsystem to use the global
	 * file store or private one in the thing scope */
	iot_bool_t use_global_store = IOT_FALSE;

	if ( storage & IOT_FILE_FLAG_GLOBAL )
		use_global_store = IOT_TRUE;

	if ( handle && file_path )
		result = iot_file_transfer(
			handle, txn, max_time_out,
			use_global_store, IOT_OPERATION_FILE_GET,
			file_name, file_path,
			func, user_data );
	return result;
}

iot_status_t iot_file_transfer(
	iot_t *handle,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	iot_bool_t use_global_store,
	iot_operation_t op,
	const char * const file_name,
	const char * const file_path,
	iot_file_progress_callback_t *func,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( handle && file_path )
	{
		iot_file_transfer_t transfer;

		result = IOT_STATUS_FAILURE;
		os_memzero( &transfer, sizeof( iot_file_transfer_t ) );
		transfer.op = op;
		transfer.callback = func;
		transfer.user_data = user_data;
		transfer.use_global_store = use_global_store;

		/* Use default directories if the
		 * path provided is not absolute */
		if ( file_path && file_path[0] == OS_DIR_SEP )
			os_strncpy( transfer.path, file_path, PATH_MAX );
		else
			os_snprintf( transfer.path, PATH_MAX, "%s%c%s",
				(op == IOT_OPERATION_FILE_PUT)?
					IOT_DEFAULT_UPLOAD_DIR :
					IOT_DEFAULT_DOWNLOAD_DIR,
				OS_DIR_SEP,
				file_path );

		/* Use the file_name to rename it on the cloud */
		if ( file_name && file_name[0] != '\0' )
			os_strncpy( transfer.name, file_name, PATH_MAX );
		else
		{
			/* if it's a directory, use the path's name
			 * with dashes and tar extension */
			if ( os_directory_exists( transfer.path ) )
			{
				size_t i = 0u;
				os_snprintf( transfer.name, PATH_MAX,
					"%s.tar", transfer.path + 1u );

				while( transfer.name[i] != '\0' )
				{
					if ( transfer.name[i] == OS_DIR_SEP )
						transfer.name[i] = '-';
					++i;
				}
				if ( i > 0 && transfer.name[i - 1] == OS_DIR_SEP )
					transfer.name[i - 1] = '\0';
			}
			else
				/* If name was not defined, take it
				 * from the basename(path)*/
				os_strncpy( transfer.name,
						os_strrchr( transfer.path, OS_DIR_SEP ) + 1u,
						PATH_MAX );
		}

		if ( op == IOT_OPERATION_FILE_PUT )
		{
			if ( os_directory_exists( transfer.path ) )
				result = iot_file_archive_directory( transfer.path, PATH_MAX );
			else if ( os_file_exists( transfer.path) )
				result = IOT_STATUS_SUCCESS;
			else
				os_printf( "Error: %s does not exist\n",
					transfer.path );
		}
		else
		{
			/* Check if the directory exists and writable */
			char dir[ PATH_MAX ];
			os_strncpy( dir, transfer.path, PATH_MAX );
			*os_strrchr( dir, OS_DIR_SEP ) = '\0';

			if ( os_file_exists( dir ) )
				result = IOT_STATUS_SUCCESS;
			else
			{
				/* Create it if it doesn't exists */
				os_printf( "Info: Creating directory %s\n", dir );
				result = os_directory_create_nowait( dir );
				if ( result != IOT_STATUS_SUCCESS )
					os_printf( "Error: Failed to create dir %s\n", dir );
			}
		}

		/* send the request */
		if ( result == IOT_STATUS_SUCCESS )
			result = iot_plugin_perform( handle,
				txn, &max_time_out,
				IOT_OPERATION_FILE_PUT, &transfer, NULL );
	}
	return result;
}

iot_status_t iot_file_archive_directory( char *path, size_t len )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( path && len )
	{
		os_dir_t dir;
		struct archive *archive;
		char archive_path[ PATH_MAX + 1u ];

		/* get a temporary file name */
		os_snprintf( archive_path, PATH_MAX, "%s%cfileXXXXXX.tar",
			IOT_DEFAULT_RUNTIME_DIR, OS_DIR_SEP );
		os_file_temp( archive_path, 4 );

		/* compress all files in upload directories */
		archive = archive_write_new();
		archive_write_add_filter_none( archive );
		archive_write_set_format_pax_restricted( archive );
		archive_write_open_filename( archive, archive_path );

		if ( os_directory_open( path, &dir ) == OS_STATUS_SUCCESS)
		{
			char file_name[ PATH_MAX + 1u ];
			iot_bool_t walk_directory = IOT_TRUE;

			while ( walk_directory )
			{
				char file_path[ PATH_MAX + 1u ];
				os_file_t input_file;
				struct stat file_stat;

				os_directory_next( &dir, IOT_TRUE, file_name, PATH_MAX );

				os_snprintf( file_path, PATH_MAX, "%s%c%s", path, OS_DIR_SEP, file_name );

				stat( file_name, &file_stat );
				input_file = os_file_open( file_name, OS_READ );
				if ( input_file )
				{
					/* create an archive of all files */
					char buff[1024];
					size_t buff_len;
					struct archive_entry *entry;

					entry = archive_entry_new();

					/* Note: set the file details individually.  Calling archive_entry_copy_stat(
					 * entry, &file_stat ) is easier but it corrupts archives on 32b architectures,
					 * e.g. quark. */
					archive_entry_set_size(entry, file_stat.st_size );
					archive_entry_set_filetype(entry, AE_IFREG );
					archive_entry_set_perm(entry, 0644 );
					archive_entry_set_pathname( entry, file_name );

					/* stat struct does not store
					 * the birthtime.  That is part of the file system */
					archive_entry_set_atime( entry, file_stat.st_atime, 0 );
					archive_entry_set_birthtime( entry, file_stat.st_ctime, 0 );
					archive_entry_set_ctime( entry, file_stat.st_ctime, 0 );
					archive_entry_set_mtime( entry, file_stat.st_mtime, 0 );

					archive_write_header( archive, entry );
					while ( ( buff_len = os_file_read( buff, 1, sizeof( buff ), input_file ) ) )
						archive_write_data( archive, buff, buff_len );

					archive_entry_free( entry );
					os_file_close( input_file );
					result = IOT_STATUS_SUCCESS;
				}
				else
					walk_directory = IOT_FALSE;
			}
			os_directory_close( &dir );
		}

		archive_write_close( archive );
		archive_write_free( archive );

		if ( result == IOT_STATUS_SUCCESS )
			os_strncpy( path, archive_path, len );
	}
	return result;
}

iot_status_t iot_file_progress_percentage_get(
	iot_file_progress_t *progress,
	iot_float32_t *percentage )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( progress && percentage )
	{
		*percentage = progress->percentage;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_file_progress_status_get(
	iot_file_progress_t *progress,
	iot_status_t *status )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( progress && status )
	{
		*status = progress->status;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_file_progress_is_completed(
	iot_file_progress_t *progress,
	iot_bool_t *is_completed )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( progress && is_completed )
	{
		*is_completed = progress->completed;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}
