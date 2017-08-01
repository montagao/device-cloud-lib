/**
 * @file
 * @brief source file containing telemetry implementation
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

#include "iot_common.h"           /* for iot_common_arg_set */
#include "iot_plugin.h"           /* for plug-in support */

#include "shared/iot_types.h"     /* for struct iot */
#include "os.h"                   /* operating system abstraction */

/**
 * @brief Sets the value of a piece of telemetry attribute data
 *
 * @param[in,out]  telemetry           telemetry object to set
 * @param[in]      name                attribute name
 * @param[in]      data                attribute value to set
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STAUTS_FULL             maximum number of attributes reached
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t iot_telemetry_attribute_set_data(
	iot_telemetry_t *telemetry,
	const char *name,
	const struct iot_data *data );

/**
 * @brief Internal function to publish a telemetry sample
 *
 * @param[in,out]  telemetry           telemetry object sample is for
 * @param[out]     txn                 transaction status (optional)
 * @param[in]      max_time_out        maximum time to wait
 *                                     (0 = wait indefinitely)
 * @param[in]      data                sample data to publish
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      type does not match registered type
 * @retval IOT_STAUTS_NOT_INITIALIZED  telemetry object is not initialized
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t iot_telemetry_publish_data(
	iot_telemetry_t *telemetry,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	const struct iot_data *data );


iot_telemetry_t *iot_telemetry_allocate(
	iot_t *lib,
	const char *name,
	iot_type_t type )
{
	struct iot_telemetry *result = NULL;
	if ( lib && name && *name != '\0' )
	{
#ifndef IOT_NO_THREAD_SUPPORT
		os_thread_mutex_lock( &lib->telemetry_mutex );
#endif
		if ( lib->telemetry_count < IOT_TELEMETRY_MAX )
		{
			const unsigned int count = lib->telemetry_count;
			iot_bool_t is_in_heap = IOT_FALSE;

			/* look for free telemetry in stack */
			result = lib->telemetry_ptr[count];

			/* allocate telemetry in heap if none is available in stack */
			if ( !result )
			{
				result = (struct iot_telemetry *)os_malloc(
					sizeof( struct iot_telemetry ) );
				is_in_heap = IOT_TRUE;
			}

			if ( result )
			{
				unsigned int cur_idx = 0u;
				unsigned int min_idx = 0u;
				unsigned int max_idx = count;
				size_t name_len;
				os_memzero( result,
					sizeof( struct iot_telemetry ) );

				name_len = os_strlen( name );
				if ( name_len > IOT_NAME_MAX_LEN )
					name_len = IOT_NAME_MAX_LEN;
#ifdef IOT_STACK_ONLY
				result->attribute = result->_attribute;
				result->name = result->_name;
#else /* ifdef IOT_STACK_ONLY */
				result->name = os_malloc( name_len + 1u );
#endif /* else IOT_STACK_ONLY */
				if ( result->name )
				{
					os_strncpy( result->name, name, name_len );
					result->name[name_len] = '\0';
					result->lib = lib;
					result->type = type;
#ifndef IOT_STACK_ONLY
					result->is_in_heap = is_in_heap;
#endif /* ifndef IOT_STACK_ONLY */

					/* place in alphabetical order */
					while ( max_idx - min_idx > 0u )
					{
						int cmp_result;
						cur_idx = (max_idx - min_idx) / 2u + min_idx;
						cmp_result = os_strncmp( name,
							lib->telemetry_ptr[cur_idx]->name,
							IOT_NAME_MAX_LEN );
						if ( cmp_result > 0 )
						{
							++cur_idx;
							min_idx = cur_idx;
						}
						else
							max_idx = cur_idx;
					}

					/* insert into proper spot in list */
					os_memmove( &lib->telemetry_ptr[cur_idx + 1u],
						&lib->telemetry_ptr[cur_idx],
						sizeof(struct iot_telemetry *) * (count - cur_idx) );
					lib->telemetry_ptr[cur_idx] = result;
					++lib->telemetry_count;
				}
#ifndef IOT_STACK_ONLY
				else if ( is_in_heap )
					os_free_null( (void**)&result );
#endif /* ifndef IOT_STACK_ONLY */
			}
			else
				IOT_LOG( lib, IOT_LOG_ERROR,
					"failed to allocate memory for telemetry: %s",
					name );
		}
		else
			IOT_LOG( lib, IOT_LOG_ERROR,
				"no remaining space (max: %u) for telemetry: %s",
				IOT_TELEMETRY_MAX, name );
#ifndef IOT_NO_THREAD_SUPPORT
		os_thread_mutex_unlock( &lib->telemetry_mutex );
#endif
	}
	return result;
}

iot_status_t iot_telemetry_attribute_get(
	const iot_telemetry_t *telemetry,
	const char *name,
	iot_bool_t convert,
	iot_type_t type, ... )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( telemetry && name )
	{
		va_list args;
		size_t i;
		result = IOT_STATUS_NOT_FOUND;
		va_start( args, type );
		for( i = 0u; i < telemetry->attribute_count &&
			result == IOT_STATUS_NOT_FOUND; ++i )
		{
			if ( os_strncmp( telemetry->attribute[i].name,
				name, IOT_NAME_MAX_LEN ) == 0 )
				result = iot_common_arg_get(
					&telemetry->attribute[i].data,
					convert, type, args );
		}
		va_end( args );
	}
	return result;
}

iot_status_t iot_telemetry_attribute_set(
	iot_telemetry_t *telemetry,
	const char* name,
	iot_type_t type, ... )
{
	va_list args;
	struct iot_data data;
	os_memzero( &data, sizeof( struct iot_data ) );
	va_start( args, type );
	iot_common_arg_set( &data, IOT_TRUE, type, args );
	va_end( args );
	return iot_telemetry_attribute_set_data( telemetry, name, &data );
}

iot_status_t iot_telemetry_attribute_set_data(
	iot_telemetry_t *telemetry,
	const char *name,
	const struct iot_data *data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( telemetry && data )
	{
		unsigned int i;
		struct iot_attribute *attr = NULL;

		/* see if this is an attribute update */
		for ( i = 0u;
			attr == NULL && i < telemetry->attribute_count; ++i )
		{
			if ( os_strcmp( telemetry->attribute[i].name, name ) == 0 )
				attr = &telemetry->attribute[i];
		}

		/* adding a new attribute */
		result = IOT_STATUS_FULL;
		if ( !attr && telemetry->attribute_count < IOT_ATTRIBUTE_MAX )
		{
#ifndef IOT_STACK_ONLY
			if ( !telemetry->attribute )
			{
				void *ptr = os_realloc( telemetry->attribute,
					sizeof( struct iot_attribute ) *
						( telemetry->attribute_count + 1u ) );
				if ( ptr )
					telemetry->attribute = ptr;
			}
			if ( telemetry->attribute )
#endif /* ifndef IOT_STACK_ONLY */
			{
				attr = &telemetry->attribute[telemetry->attribute_count];
				os_memzero( attr, sizeof( struct iot_attribute ) );
				++telemetry->attribute_count;
			}
		}

		/* add or update the attribute */
		if ( attr )
		{
			iot_bool_t update = IOT_TRUE;
			/* add name if not already given */
			if ( !attr->name || *attr->name == '\0' )
			{
				size_t name_len = os_strlen( name );
				if ( name_len > IOT_NAME_MAX_LEN )
					name_len = IOT_NAME_MAX_LEN;
#ifndef IOT_STACK_ONLY
				attr->name = os_malloc( sizeof( char ) * (name_len + 1u) );
#endif /* ifndef IOT_STACK_ONLY */
				if ( !attr->name )
					update = IOT_FALSE;
				else
				{
					os_strncpy( attr->name, name, name_len );
					attr->name[name_len] = '\0';
				}
			}

			if ( update )
			{
				/** @todo fix this to take ownership */
				os_memcpy( &attr->data, data,
					sizeof( struct iot_data ) );
				result = IOT_STATUS_SUCCESS;
			}
		}
	}
	return result;
}

iot_status_t iot_telemetry_attribute_set_raw(
	iot_telemetry_t *telemetry,
	const char* name,
	size_t length,
	const void *ptr )
{
	struct iot_data data;
	os_memzero( &data, sizeof( struct iot_data ) );
	data.type = IOT_TYPE_RAW;
	data.value.raw.ptr = ptr;
	data.value.raw.length = length;
	data.has_value = IOT_TRUE;
	return iot_telemetry_attribute_set_data( telemetry, name, &data );
}

iot_status_t iot_telemetry_deregister(
	iot_telemetry_t *telemetry,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( telemetry )
	{
		result = IOT_STATUS_NOT_INITIALIZED;
		if ( telemetry->lib )
		{
			if ( telemetry->state != IOT_ITEM_DEREGISTERED )
			{
				result = iot_plugin_perform( telemetry->lib,
					txn, &max_time_out,
					IOT_OPERATION_TELEMETRY_DEREGISTER,
					NULL, NULL );
				if ( result == IOT_STATUS_SUCCESS )
					telemetry->state = IOT_ITEM_DEREGISTERED;
				else
					telemetry->state = IOT_ITEM_DEREGISTER_PENDING;
			}
		}
	}
	return result;
}

iot_status_t iot_telemetry_free(
	iot_telemetry_t *telemetry,
	iot_millisecond_t max_time_out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( telemetry )
	{
		struct iot *const lib = telemetry->lib;
		result = IOT_STATUS_NOT_INITIALIZED;
		if ( lib )
		{
			unsigned int i, max;
			result = iot_telemetry_deregister( telemetry, NULL,
				max_time_out );
#ifndef IOT_NO_THREAD_SUPPORT
			os_thread_mutex_lock( &lib->telemetry_mutex );
#endif
			/* find telemetry within the library */
			max = lib->telemetry_count;
			for ( i = 0u; ( i < max ) &&
				( lib->telemetry_ptr[ i ] != telemetry );
				++i );

			if ( i < max )
			{
#ifndef IOT_STACK_ONLY
				const iot_bool_t is_in_heap =
					telemetry->is_in_heap;
#endif /* ifndef IOT_STACK_ONLY */
				/* free any heap allocated storage */
				size_t j;
				for ( j = 0u; j < telemetry->attribute_count; ++j )
				{
					os_free_null(
						(void **)&telemetry->attribute[j].data.heap_storage );
				}

				/* remove from library */
				os_memmove(
					&lib->telemetry_ptr[ i ],
					&lib->telemetry_ptr[ i + 1u ],
					sizeof( struct iot_telemetry * ) * ( max - i - 1u ) );

				/* set lib to NULL */
				telemetry->lib = NULL;

				/* clear/free the telemetry */
				--lib->telemetry_count;
#ifndef IOT_STACK_ONLY
				for ( j = 0u; j < telemetry->attribute_count; ++j )
				{
					os_free_null( (void**)&telemetry->attribute[j].name );
					os_free_null( (void**)&telemetry->attribute[j].data.heap_storage );
				}
				os_free_null( (void**)&telemetry->attribute );
				os_free_null( (void**)&telemetry->name );
				if ( is_in_heap == IOT_FALSE )
				{
					lib->telemetry_ptr[
						lib->telemetry_count] = telemetry;
				}
				else
				{
					lib->telemetry_ptr[
						lib->telemetry_count] = NULL;
					os_free_null( (void**)&telemetry );
				}
#else
				lib->telemetry_ptr[
					lib->telemetry_count] = telemetry;
#endif /* ifndef IOT_STACK_ONLY */
				result = IOT_STATUS_SUCCESS;
			}

#ifndef IOT_NO_THREAD_SUPPORT
			os_thread_mutex_unlock( &lib->telemetry_mutex );
#endif
		}
	}
	return result;
}

const char *iot_telemetry_name_get(
	const iot_telemetry_t *t )
{
	const char *result = NULL;
	if ( t )
		result = t->name;
	return result;
}

iot_status_t iot_telemetry_publish(
	iot_telemetry_t *telemetry,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	iot_type_t type, ... )
{
	va_list args;
	struct iot_data data;
	va_start( args, type );
	iot_common_arg_set( &data, IOT_FALSE, type, args );
	va_end( args );
	return iot_telemetry_publish_data( telemetry, txn, max_time_out, &data );
}

iot_status_t iot_telemetry_publish_data( iot_telemetry_t *telemetry,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	const struct iot_data *data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( telemetry && data )
	{
		result = IOT_STATUS_NOT_INITIALIZED;
		if ( telemetry->lib )
		{
			result = IOT_STATUS_BAD_REQUEST;
			if( telemetry->type == IOT_TYPE_NULL ||
				telemetry->type == data->type )
			{
#ifndef IOT_NO_THREAD_SUPPORT
				os_thread_mutex_lock(
					&telemetry->lib->telemetry_mutex );
#endif
				result = iot_plugin_perform(
					telemetry->lib, txn, &max_time_out,
						IOT_OPERATION_TELEMETRY_PUBLISH,
						telemetry, data );
				if ( result == IOT_STATUS_SUCCESS )
					telemetry->time_stamp = 0u;
#ifndef IOT_NO_THREAD_SUPPORT
				os_thread_mutex_unlock(
					&telemetry->lib->telemetry_mutex );
#endif
			}
		}
	}
	return result;
}

iot_status_t iot_telemetry_publish_raw( iot_telemetry_t *telemetry,
	iot_transaction_t *txn, iot_millisecond_t max_time_out, size_t length,
	const void *ptr )
{
	struct iot_data data;
	os_memzero( &data, sizeof( struct iot_data ) );
	data.type = IOT_TYPE_RAW;
	data.value.raw.ptr = ptr;
	data.value.raw.length = length;
	data.has_value = IOT_TRUE;
	return iot_telemetry_publish_data( telemetry, txn, max_time_out, &data );
}

iot_status_t iot_telemetry_register(
	iot_telemetry_t *telemetry,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( telemetry )
	{
		result = IOT_STATUS_NOT_INITIALIZED;
		if ( telemetry->lib )
		{
			result = iot_plugin_perform( telemetry->lib,
				txn, &max_time_out, IOT_OPERATION_TELEMETRY_REGISTER,
				telemetry, NULL );
			if ( result == IOT_STATUS_SUCCESS )
				telemetry->state = IOT_ITEM_REGISTERED;
			else
				telemetry->state = IOT_ITEM_REGISTER_PENDING;
		}
	}
	return result;
}

iot_status_t iot_telemetry_timestamp_set(
	iot_telemetry_t *telemetry,
	iot_timestamp_t time_stamp )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( telemetry )
	{
		telemetry->time_stamp = time_stamp;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

