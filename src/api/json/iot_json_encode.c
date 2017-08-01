/**
 * @file
 * @brief source file for IoT library json decoding functionality
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "../public/iot_json.h"

#include "iot_json_base.h"

#include "os.h"

#ifdef IOT_JSON_JANSSON
/**
 * @brief Maximum supportable json depth
 */
#define JSON_MAX_DEPTH                10u

/**
 * @brief object encapsulating the encode and decode functions
 */
struct iot_json_encoder
{
	json_error_t error;              /**< @brief holds error information */
	unsigned int depth;              /**< @brief current depth */
	unsigned int flags;              /**< @brief output flags */
	json_t **j_cur;                  /**< @brief current object for each level */
	json_t *j_top;                   /**< @brief top-level jansson object */
	char *output;                    /**< @brief saved dumped string */
};

/**
 * @brief helper function for encoding primative types with jansson
 *
 * @param[in]      json                object being encodeded
 * @param[in]      key                 key for the new object
 * @param[in]      obj                 new object (steals the reference)
 */
static iot_status_t iot_json_encode_key(
	iot_json_encoder_t *json,
	const char *key,
	json_t *obj );

#else /* ifdef IOT_JSON_JANSSON */
/** @brief Maximum output depth */
typedef unsigned long iot_json_encode_struct_t;

/**
 * @brief An error occurred during adding a new item, so don't encode
 */
#define JSON_STRUCT_BITS                 3       /* # of bits to define struct */

/**
 * @brief Maximum supportable json depth
 */
#define JSON_MAX_DEPTH                 ((sizeof(iot_json_encode_struct_t)* 8)/JSON_STRUCT_BITS)

/**
 * @brief internal structure for composing json messages (16 bytes)
 */
struct iot_json_encoder
{
	char *buf;                       /**< @brief start of write buffer */
	char *cur;                       /**< @brief position in write buffer */
	unsigned int flags;              /**< @brief output flags */
	size_t len;                      /**< @brief size of json buffer */
	iot_json_encode_struct_t structs;    /**< @brief array of structures */
};

/** @brief JSON tokens for the start of objects & arrays */
static const char JSON_CHARS_START[] = { '[', '{', '{' };
/** @brief JSON tokens for the end of objects & arrays */
static const char JSON_CHARS_END[] = { ']', '}', '}' };

/**
 * @brief determines the current depth of structures at the current position
 *
 * @param[in]      json                object being encoded
 *
 * @return the depth at the current position
 */
static IOT_SECTION unsigned int iot_json_encode_depth(
	const iot_json_encoder_t *json );

/**
 * @brief calculated the number number of printable characters in an integer
 *
 * @param[in]      i                   integer to calculate number of characters
 * @param[in]      neg                 add characters for negative number
 *
 * @return number of printable characters in the number
 */
static IOT_SECTION size_t iot_json_encode_intlen(
	iot_uint64_t i,
	iot_bool_t neg );

/**
 * @brief helper function to start a new object
 *
 * @param[in,out]  json                destination json object
 * @param[in]      key                 (optional) key for the new item
 * @param[in]      value_len           length of the value for the new object
 * @param[out]     added_parent        (optional) whether this call required
 *                                     adding a new json object to ensure valid
 *                                     json remained
 */
static IOT_SECTION iot_status_t iot_json_encode_key(
	iot_json_encoder_t *json,
	const char *key,
	size_t value_len,
	iot_bool_t *added_parent );

/**
 * @brief helper function to calculate the length of a string for encoding
 *        in JSON
 *
 * @note This function handles adding counts for extra characters required to
 *       black-slash characters in json string
 *
 * @param[in]      str                 sring to get length of
 *
 * @return the length of the string in characters (adding for escape characters)
 */
static IOT_SECTION size_t iot_json_encode_strlen(
	const char* str );

/**
 * @brief copies the string in src to the destination buffer in JSON format
 *
 * @note This function handles adding escape characters into the destination
 *       buffer when copying the string
 *
 * @param[in,out]  dest                destination buffer
 * @param[in]      src                 source buffer
 * @param[in]      num                 size of the destination buffer
 *
 * @return a pointer to the destination buffer
 */
static IOT_SECTION char *iot_json_encode_strncpy(
	char *dest,
	const char *src,
	size_t num );

/**
 * @brief helper function for starting a new object or array json structure
 *
 * @param[in]      json                object being encoded
 * @param[in]      s                   type of object to start
 */
static IOT_SECTION iot_status_t iot_json_encode_struct_end(
	iot_json_encoder_t *json,
	iot_json_type_t s );

/**
 * @brief helper function for ending a new object or array json structure
 *
 * @param[in]      json                object being encoded
 * @param[in]      s                   type of object to start
 */
static iot_status_t iot_json_encode_struct_start(
	iot_json_encoder_t *json,
	const char *key,
	iot_json_type_t s );
#endif /* else IOT_JSON_JANSSON */

#ifdef IOT_JSON_JANSSON
iot_status_t iot_json_encode_key(
	iot_json_encoder_t *json,
	const char *key,
	json_t *obj )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( json && json->depth > 0u )
	{
		result = IOT_STATUS_FULL;
		if ( json->depth < JSON_MAX_DEPTH )
		{
			json_t *cur_obj = json->j_cur[json->depth - 1u];
			if ( key )
			{
				json_object_set_new( cur_obj, key, obj );
				result = IOT_STATUS_SUCCESS;
			}
			else
			{
				json_t *arr_obj = NULL;
				if ( !json_is_array( cur_obj ) )
				{
					arr_obj = json_array();
					json_object_set( cur_obj, NULL, arr_obj );
					cur_obj = arr_obj;
				}
				json_array_append_new( cur_obj, obj );
				result = IOT_STATUS_SUCCESS;
			}
		}
	}

	if ( result != IOT_STATUS_SUCCESS )
		json_decref( obj );
	return result;
}
#endif /* ifdef IOT_JSON_JANSSON */

iot_status_t iot_json_encode_array_end(
	iot_json_encoder_t *json )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
#ifdef IOT_JSON_JANSSON
	if ( json && json->depth > 0u )
	{
		json_t *const j_obj = json->j_cur[json->depth - 1u];
		json_decref( j_obj );
		--json->depth;
		result = IOT_STATUS_SUCCESS;
	}
#else /* ifdef IOT_JSON_JANSSON */
	result = iot_json_encode_struct_end( json, IOT_JSON_TYPE_ARRAY );
#endif /* else IOT_JSON_JANSSON */
	return result;
}

iot_status_t iot_json_encode_array_start(
	iot_json_encoder_t *json,
	const char *key )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
#ifdef IOT_JSON_JANSSON
	if ( json )
	{
		result = IOT_STATUS_FULL;
		if ( json->depth > 0u && json->depth < JSON_MAX_DEPTH )
		{
			json_t *new_obj = json_array();
			result = iot_json_encode_key( json, key, new_obj );
			json->j_cur[json->depth] = new_obj;
			++json->depth;
		}
	}
#else /* ifdef IOT_JSON_JANSSON */
	result = iot_json_encode_struct_start( json, key, IOT_JSON_TYPE_ARRAY );
#endif /* else IOT_JSON_JANSSON */
	return result;
}

iot_status_t iot_json_encode_bool(
	iot_json_encoder_t *json,
	const char *key,
	iot_bool_t value )
{
	iot_status_t result;
#ifdef IOT_JSON_JANSSON
	result = iot_json_encode_key( json, key, json_boolean( value ) );
#else /* ifdef IOT_JSON_JANSSON */
	iot_bool_t added_parent = IOT_FALSE;
	const size_t value_len = 5u + ( value ? -1 : 0 );
	result = iot_json_encode_key( json, key, value_len + 2u, &added_parent );
	if ( result == IOT_STATUS_SUCCESS )
	{
		if ( value )
			os_strncpy( json->cur, "true", value_len );
		else
			os_strncpy( json->cur, "false", value_len );
		json->cur += value_len;

		if ( added_parent )
			result = iot_json_encode_object_end( json );
	}
#endif /* else IOT_JSON_JANSSON */
	return result;
}

#ifndef IOT_JSON_JANSSON
unsigned int iot_json_encode_depth( const iot_json_encoder_t *json )
{
	unsigned int i = 0u;
	iot_json_encode_struct_t j = json->structs;
	while ( j )
	{
		if ( j & 0x1 ) ++i;
		j >>= 1;
	}
	return i;
}
#endif /* ifndef IOT_JSON_JANSSON */

const char *iot_json_encode_dump(
	iot_json_encoder_t *json )
{
	const char *result = NULL;
#ifdef IOT_JSON_JANSSON
	/* setup jansson flags */
	size_t flags = JSON_PRESERVE_ORDER;
	if ( json && !( json->flags & IOT_JSON_FLAG_EXPAND ) )
		flags |= JSON_COMPACT;
	if ( json && json->flags >> IOT_JSON_INDENT_OFFSET )
		flags |= JSON_INDENT( json->flags >> IOT_JSON_INDENT_OFFSET );

	/* if previous dump free memory */
	if ( json && json->output )
	{
#ifdef IOT_STACK_ONLY
		json_free_t free_fn = os_free;
#if JANSSON_VERSION_HEX >= 0x020800
		json_get_alloc_funcs( NULL, &free_fn );
#endif /* JANSSON_VERSION_HEX >= 0x020800 */
		if ( free_fn )
			free_fn( json->output );
#else /* ifdef IOT_STACK_ONLY */
		iot_json_free( json->output );
#endif /* else IOT_STACK_ONLY */
	}

	if ( json )
		result = json->output = json_dumps( json->j_cur[0u], flags );
#else /* ifdef IOT_JSON_JANSSON */
	/* 3 least-significant bits are for errors */
	if ( json && !( json->flags & ((1 << 3)-1) ) )
	{
		if ( iot_json_encode_object_end( json ) == IOT_STATUS_SUCCESS )
		{
			*json->cur = '\0';
			result = json->buf;
		}
	}
#endif /* else IOT_JSON_JANSSON */
	return result;
}

iot_json_encoder_t *iot_json_encode_initialize(
	char *buf,
	size_t len,
	unsigned int flags )
{
	struct iot_json_encoder *json = NULL;

#ifdef IOT_JSON_JANSSON
	size_t extra_space = (sizeof( json_t *) * JSON_MAX_DEPTH);
#else /* ifdef IOT_JSON_JANSSON */
	size_t extra_space = 3u; /* for '{', '}' and '\0' */
#endif /* else IOT_JSON_JANSSON */

	if (
#ifndef IOT_STACK_ONLY
	     ( flags & IOT_JSON_FLAG_DYNAMIC ) ||
#endif /* ifndef IOT_STACK_ONLY */
	     ( buf && len >= sizeof(struct iot_json_encoder) + extra_space ) )
	{
#ifndef IOT_STACK_ONLY
		if ( flags & IOT_JSON_FLAG_DYNAMIC )
		{
			json = (struct iot_json_encoder *)iot_json_realloc(
				NULL, sizeof(struct iot_json_encoder) + extra_space );
			if ( json )
				os_memzero( json, sizeof( struct iot_json_encoder ) );
		}
		else
		{
#endif /* ifndef IOT_STACK_ONLY */
			json = (struct iot_json_encoder*)(buf);
			os_memzero( json, sizeof( struct iot_json_encoder ) );
#ifndef IOT_JSON_JANSSON
			json->buf = buf + sizeof(struct iot_json_encoder);
			json->cur = json->buf;
			json->len = len - sizeof(struct iot_json_encoder);
#endif /* ifndef IOT_JSON_JANSSON */
#ifndef IOT_STACK_ONLY
		}

		if ( json )
		{
#endif /* ifndef IOT_STACK_ONLY */
#ifdef IOT_JSON_JANSSON
			json->j_cur = (json_t**)(buf + sizeof(struct iot_json_encoder));
#else /* ifdef IOT_JSON_JANSSON */
			json->structs = 0u;
#endif /* else IOT_JSON_JANSSON */
			json->flags = flags;
			/* go into first object */
			iot_json_encode_object_start( json, NULL );
#ifndef IOT_STACK_ONLY
		}
#endif /* ifndef IOT_STACK_ONLY */
	}
	return json;
}

iot_status_t iot_json_encode_integer(
	iot_json_encoder_t *json,
	const char *key,
	iot_int64_t value )
{
	iot_status_t result;
#ifdef IOT_JSON_JANSSON
	result = iot_json_encode_key( json, key, json_integer( value ) );
#else /* ifdef IOT_JSON_JANSSON */
	const size_t value_len = iot_json_encode_intlen( value, value < 0 );
	iot_bool_t added_parent = 0;
	result = iot_json_encode_key( json, key, value_len,
		&added_parent );
	if ( result == IOT_STATUS_SUCCESS )
	{
		char *dest = &json->cur[value_len - 1u];
		if ( value <= 0 )
		{
			if ( value < 0 )
			{
				*json->cur = '-';
				value *= -1;
			}
			else
				*json->cur = '0';
			++json->cur;
		}
		while ( value > 0 )
		{
			*dest = (value % 10 + '0');
			value /= 10;
			--dest;
			++json->cur;
		}

		if ( added_parent )
			result = iot_json_encode_object_end( json );
	}
#endif /* else IOT_JSON_JANSSON */
	return result;
}

#ifndef IOT_JSON_JANSSON
size_t iot_json_encode_intlen( iot_uint64_t i, iot_bool_t neg )
{
	size_t len = 0u;
	if ( neg )
		++len;
	while ( i > 0 )
	{
		i /= 10;
		++len;
	}
	return len;
}

iot_status_t iot_json_encode_key(
	iot_json_encoder_t *json,
	const char *key,
	size_t value_len,
	iot_bool_t *added_parent )
{
	iot_status_t result = IOT_STATUS_SUCCESS;
	size_t key_len = 0u;
	size_t extra_space = 0u;
	if ( json && key )
	{
		if ( !( json->structs & IOT_JSON_TYPE_OBJECT ) )
		{
			result = iot_json_encode_struct_start( json, NULL,
				IOT_JSON_TYPE_OBJECT << 1 );
			*added_parent = IOT_TRUE;
		}
		key_len = iot_json_encode_strlen( key );
		extra_space += 3u; /* for '"' around key, ':' */
	}

	if ( json && result == IOT_STATUS_SUCCESS )
	{
		size_t space = json->len - (json->cur - json->buf);
		iot_bool_t add_comma = 0;
		unsigned int indent = (json->flags >> IOT_JSON_INDENT_OFFSET);
		const unsigned int depth = iot_json_encode_depth( json );

		/* space required for closing current level */
		if ( indent )
		{
			unsigned int i;
			for ( i = 0u; i < depth; ++i )
				extra_space += indent * i + 1u;
		}

		/* space required for initial ',' + optional space */
		if ( json->cur > json->buf &&
			*(json->cur - 1) != JSON_CHARS_START[0] &&
			*(json->cur - 1) != JSON_CHARS_START[1] &&
			*(json->cur - 1) != JSON_CHARS_START[2] )
		{
			add_comma = 1;
			++extra_space;
			if ( !indent && ( json->flags & IOT_JSON_FLAG_EXPAND ) )
				++extra_space;
		}

		/* space required to add space around `:` */
		if ( json->flags & IOT_JSON_FLAG_EXPAND )
			++extra_space;

		/* how much space is required to indent/close the object */
		if ( indent )
			extra_space += ( indent * 2u * depth ) + 1u; /* +1 for '\n' */

#ifndef IOT_STACK_ONLY
		if ( json->flags & IOT_JSON_FLAG_DYNAMIC )
		{
			const size_t new_space =
				json->len + key_len + value_len + extra_space;
			void *const new_buf = iot_json_realloc( json->buf, new_space );
			if ( new_buf )
			{
				json->cur = (char*)new_buf + (json->cur - json->buf);
				json->buf = new_buf;
				json->len = new_space;
				space = new_space;
			}
		}
#endif /* ifndef IOT_STACK_ONLY */

		if ( key_len + value_len + extra_space <= space )
		{
			if ( add_comma )
			{
				*json->cur = ',';
				++json->cur;
				if ( !indent && ( json->flags & IOT_JSON_FLAG_EXPAND ) )
				{
					*json->cur = ' ';
					++json->cur;
				}
			}
			if ( indent )
			{
				if ( depth > 0u )
				{
					*json->cur = '\n';
					++json->cur;
				}
				indent *= depth;
				while ( indent )
				{
					*json->cur = ' ';
					++json->cur;
					--indent;
				}
			}
			if ( key )
			{
				*json->cur = '"';
				++json->cur;
				iot_json_encode_strncpy(
					json->cur, key, key_len );
				json->cur += key_len;
				os_strncpy( json->cur, "\":", 2u );
				json->cur += 2u;
				if ( json->flags & IOT_JSON_FLAG_EXPAND )
				{
					*json->cur = ' ';
					++json->cur;
				}
			}
			result = IOT_STATUS_SUCCESS;
		}
		else
			result = IOT_STATUS_NO_MEMORY;
	}
	return result;
}
#endif /* ifndef IOT_JSON_JANSSON */

iot_status_t iot_json_encode_object_end(
	iot_json_encoder_t *json )
{
	iot_status_t result = IOT_STATUS_IO_ERROR;
#ifdef IOT_JSON_JANSSON
	if ( json && json->depth > 0u )
	{
		/*
		json_t *const j_obj = json->j_cur[json->depth - 1u];
		json_decref( j_obj );
		*/
		--json->depth;
	}
#else /* ifdef IOT_JSON_JANSSON */
	result = iot_json_encode_struct_end( json, IOT_JSON_TYPE_OBJECT );
#endif /* else IOT_JSON_JANSSON */
	return result;
}

iot_status_t iot_json_encode_object_start(
	iot_json_encoder_t *json,
	const char *key )
{
	iot_status_t result = IOT_STATUS_FULL;
#ifdef IOT_JSON_JANSSON
	if ( json && json->depth < JSON_MAX_DEPTH )
	{
		json_t *const new_obj = json_object();
		result = IOT_STATUS_NO_MEMORY;
		if ( new_obj )
		{
			result = IOT_STATUS_SUCCESS;
			/* not top object, add reference to parent */
			if ( json->depth > 0u )
			{
				json_t *const cur_obj =
					json->j_cur[json->depth - 1u];
				result = IOT_STATUS_NO_MEMORY;
				if ( json_object_set( cur_obj, key, new_obj ) == 0 )
					result = IOT_STATUS_SUCCESS;
			}

			if ( result == IOT_STATUS_SUCCESS )
			{
				json->j_cur[json->depth] = new_obj;
				++json->depth;
			}
		}
	}
#else /* ifdef IOT_JSON_JANSSON */
	result = iot_json_encode_struct_start( json, key, IOT_JSON_TYPE_OBJECT );
#endif /* else IOT_JSON_JANSSON */
	return result;
}

iot_status_t iot_json_encode_real(
	iot_json_encoder_t *json,
	const char *key,
	iot_float64_t value )
{
	iot_status_t result;
#ifdef IOT_JSON_JANSSON
	result = iot_json_encode_key( json, key, json_real( (double)value ) );
#else /* ifdef IOT_JSON_JANSSON */
/** @brief number of decimals to display for real data */
#define JSON_ENCODE_MAX_DECIMALS 6
	iot_bool_t added_parent = 0;

	iot_uint64_t i = (iot_uint64_t)value;
	double frac = value - i;
	size_t int_len;
	size_t value_len;

	/* handle negative numbers */
	if ( value < 0.0 )
	{
		i = (iot_uint64_t)(value * -1.0);
		frac = (value * -1.0) - i;
	}

	int_len = iot_json_encode_intlen( i, value < 0.0 );
	value_len = int_len + JSON_ENCODE_MAX_DECIMALS + 1u;
	result = iot_json_encode_key( json, key, value_len, &added_parent );
	if ( result == IOT_STATUS_SUCCESS )
	{
		char *dest = &json->cur[int_len - 1u];
		if ( value < 0.0 || i == 0 )
		{
			if ( value < 0.0 )
				*json->cur = '-';
			else
				*json->cur = '0';
			++json->cur;
		}
		while ( i > 0u )
		{
			*dest = (i % 10 + '0');
			i /= 10;
			--dest;
			++json->cur;
		}

		*json->cur++ = '.';
		i = 0u;
		do {
			int j;
			frac *= 10.0;
			j = (int)(frac);
			*json->cur++ = '0' + j;
			frac -= j;
			++i;
		} while ( /*frac > 0.0 && */ i < JSON_ENCODE_MAX_DECIMALS );
		if ( added_parent )
			result = iot_json_encode_object_end( json );
	}
#endif /* else IOT_JSON_JANSSON */
	return result;
}

iot_status_t iot_json_encode_string(
	iot_json_encoder_t *json,
	const char *key,
	const char *value )
{
	iot_status_t result;
#ifdef IOT_JSON_JANSSON
	result = iot_json_encode_key( json, key, json_string( value ) );
#else /* ifdef IOT_JSON_JANSSON */
	iot_bool_t added_parent = 0;
	size_t value_len;

	if ( !value )
		value = "";
	value_len = iot_json_encode_strlen( value );
	result = iot_json_encode_key( json, key, value_len + 2u, &added_parent );
	if ( json && result == IOT_STATUS_SUCCESS )
	{
		*json->cur++ = '"';
		iot_json_encode_strncpy( json->cur, value, value_len );
		json->cur += value_len;
		*json->cur++ = '"';
		if ( added_parent )
			result = iot_json_encode_object_end( json );
	}
#endif /* else IOT_JSON_JANSSON */
	return result;
}

#ifndef IOT_JSON_JANSSON
size_t iot_json_encode_strlen(
	const char* str )
{
	size_t result = 0u;
	while ( *str != '\0' )
	{
		/* add additional character for null-terminator */
		if ( *str == '\"' || *str == '\\' || *str == '\b' ||
		     *str == '\f' || *str == '\n' || *str == '\r' ||
		     *str == '\t' )
			++result;
		++result;
		++str;
	}
	return result;
}

char *iot_json_encode_strncpy(
	char *dest,
	const char *src,
	size_t num )
{
	iot_bool_t skip_char = IOT_FALSE;
	char *out = dest;
	size_t i = 0u;
	while ( i < num && *src != '\0' )
	{
		if ( *src == '\"' || *src == '\\' || *src == '\b' ||
		     *src == '\f' || *src == '\n' || *src == '\r' ||
		     *src == '\t' )
		{
			if ( i + 1u < num )
			{
				*out = '\\';
				++out;
				++i;
			}
			else
				skip_char = IOT_TRUE;
		}

		if ( skip_char == IOT_FALSE )
		{
			switch ( *src )
			{
			case '\b':
				*out = 'b';
				break;
			case '\f':
				*out = 'f';
				break;
			case '\n':
				*out = 'n';
				break;
			case '\r':
				*out = 'r';
				break;
			case '\t':
				*out = 't';
				break;
			default:
				*out = *src;
			}
			++src;
			++out;
			++i;
		}
	}

	/* pad remaining with zero's */
	for ( ; i < num; ++i )
		*out = '\0';
	return dest;
}
#endif /* ifndef IOT_JSON_JANSSON */

#ifndef IOT_JSON_JANSSON
iot_status_t iot_json_encode_struct_end(
	iot_json_encoder_t *json,
	iot_json_type_t s )
{
	iot_status_t result = IOT_STATUS_SUCCESS;
	if ( json && json->structs & s )
	{
		unsigned int depth = iot_json_encode_depth( json ) - 1u;
		const unsigned int indent = (json->flags >> IOT_JSON_INDENT_OFFSET);
		size_t space = json->len - (json->cur - json->buf);
		iot_json_encode_struct_t i;

		/* ] } */
		for ( i = 0u; i < JSON_STRUCT_BITS; ++i )
		{
			if ( ( (iot_json_encode_struct_t)(1u) << (i) ) &
				json->structs )
			{
				/* +2 for '\n' + ']' or '}' character */
				if ( (indent * depth) + 2u > space )
				{
#ifndef IOT_STACK_ONLY
					if ( json->flags & IOT_JSON_FLAG_DYNAMIC )
					{
						const size_t new_space =
							(depth * indent) + 2u;
						void *const new_buf = iot_json_realloc(
							json->buf,
							json->len + new_space );
						if ( new_buf )
						{
							json->cur = (char*)new_buf +
								(json->cur - json->buf);
							json->buf = new_buf;
							json->len += new_space;
							space += new_space;
						}
						else
							result = IOT_STATUS_NO_MEMORY;
					}
					else
#endif /* ifndef IOT_STACK_ONLY */
						result = IOT_STATUS_NO_MEMORY;
				}

				if ( result == IOT_STATUS_SUCCESS )
				{
					const char end_ch = JSON_CHARS_END[i];
					if ( indent )
					{
						unsigned int j = indent * depth;
						*json->cur++ = '\n';

						while ( j )
						{
							*json->cur++ = ' ';
							--j;
						}
						space -= (indent + 1u);
						--depth;
					}
					*json->cur++ = end_ch;
					--space;
				}
			}
		}
		json->structs >>= JSON_STRUCT_BITS;
	}
	else
		result = IOT_STATUS_IO_ERROR;

	return result;
}

iot_status_t iot_json_encode_struct_start(
	iot_json_encoder_t *json,
	const char *key,
	iot_json_type_t s )
{
	iot_status_t result = IOT_STATUS_FULL;
	if ( json && iot_json_encode_depth( json ) < JSON_MAX_DEPTH )
	{
		iot_bool_t added_parent = IOT_FALSE;
		/* for '[' & ']' */
		unsigned int indent = (json->flags >> IOT_JSON_INDENT_OFFSET);
		result = iot_json_encode_key( json, key, 2u + indent,
			&added_parent );
		if ( result == IOT_STATUS_SUCCESS )
		{
			unsigned int i;
			for ( i = JSON_STRUCT_BITS; i > 0u; --i )
			{
				/* {, [ */
				unsigned int i_1 = i - 1u;
				if ( s & ( 1 << ( i_1 ) ) )
				{
					indent = (json->flags >> IOT_JSON_INDENT_OFFSET);
					*json->cur++ = JSON_CHARS_START[i_1];
				}
			}
			if ( !added_parent )
				json->structs <<= JSON_STRUCT_BITS;
			json->structs |= (iot_json_encode_struct_t)s;
		}
	}
	return result;
}
#endif /* ifndef IOT_JSON_JANSSON */

void iot_json_encode_terminate(
	iot_json_encoder_t *json )
{
#ifdef IOT_JSON_JANSSON
	if ( json )
	{
		if ( json->output )
		{
			json_free_t free_fn = os_free;
#if JANSSON_VERSION_HEX >= 0x020800
			json_get_alloc_funcs( NULL, &free_fn );
#endif /* if JANSSON_VERSION_HEX >= 0x020800 */
			if ( free_fn )
				free_fn( json->output );
		}
		if ( json->j_cur[0] )
			json_decref( json->j_cur[0] );
	}
#endif /* ifdef IOT_JSON_JANSSON */

#ifdef IOT_STACK_ONLY
	(void)json;
#else /* ifdef IOT_STACK_ONLY */
	if ( json && ( json->flags & IOT_JSON_FLAG_DYNAMIC ) )
	{
#ifndef IOT_JSON_JANSSON
		if ( json->buf )
			iot_json_free( json->buf );
#endif /* ifndef IOT_JSON_JANSSON */
		iot_json_free( json );
	}
#endif /* else IOT_STACK_ONLY */
}

