
#include "app_json.h"

#include "os.h"


iot_status_t app_json_array_get( app_json_t *child, const app_json_t *parent,
	int index )
{
	iot_status_t status = IOT_STATUS_BAD_PARAMETER;
	if ( parent && child )
	{
		status = IOT_STATUS_BAD_REQUEST;
		if ( parent->token->type == JSMN_ARRAY )
		{
			status = IOT_STATUS_OUT_OF_RANGE;
			if ( index < parent->token->size && index >= 0 )
			{
				int child_count = parent->token->size;
				int token_count = parent->token->size;
				int i;
				for ( i = 1; i <= parent->num_tokens && child_count > 0; ++i )
				{
					if ( token_count == child_count )
					{
						if ( parent->token->size - token_count == index )
						{
							child->json_string = parent->json_string;
							child->string_len = parent->string_len;
							child->token = parent->token+i;
							child->num_tokens = 0;
							child->is_root = IOT_FALSE;

							token_count = parent->token[i].size;
							while ( token_count > 0 )
							{
								++i;
								token_count += parent->token[i].size - 1;
								++child->num_tokens;
							}
							status = IOT_STATUS_SUCCESS;
							break;
						}
						--child_count;
					}
					token_count += parent->token[i].size;
					--token_count;
				}
			}
		}
	}
	return status;
}

void app_json_free( app_json_t *jobj )
{
	if ( jobj->is_root == IOT_TRUE )
	{
		os_free( (void **)&jobj->token );
		if ( jobj->from_file == IOT_TRUE )
			os_free( (void **)&jobj->json_string );
	}
}

iot_status_t app_json_get_bool_value( iot_bool_t *value,
	const app_json_t *jobj )
{
	iot_status_t status = IOT_STATUS_BAD_PARAMETER;
	if ( jobj && value )
	{
		status = IOT_STATUS_BAD_REQUEST;
		if ( jobj->token->type == JSMN_PRIMITIVE )
		{
			char primchar = jobj->json_string[jobj->token->start];
			switch ( primchar )
			{
				case 't':
				case 'T':
					*value = IOT_TRUE;
					status = IOT_STATUS_SUCCESS;
					break;
				case 'f':
				case 'F':
					*value = IOT_FALSE;
					status = IOT_STATUS_SUCCESS;
					break;
			}
		}
	}
	return status;
}

iot_status_t app_json_get_double_value( double *value, const app_json_t *jobj )
{
	iot_status_t status = IOT_STATUS_BAD_PARAMETER;
	if ( jobj && value )
	{
		status = IOT_STATUS_BAD_REQUEST;
		if ( app_json_get_type( jobj ) == APP_JSON_FLOAT )
		{
			*value = os_strtod( jobj->json_string + jobj->token->start,
				NULL );
			status = IOT_STATUS_SUCCESS;
		}
	}
	return status;
}

iot_status_t app_json_get_long_value( long *value, const app_json_t *jobj )
{
	iot_status_t status = IOT_STATUS_BAD_PARAMETER;
	if ( jobj && value )
	{
		status = IOT_STATUS_BAD_REQUEST;
		if ( app_json_get_type( jobj ) == APP_JSON_INTEGER )
		{
			*value = os_strtol( jobj->json_string + jobj->token->start,
				NULL );
			status = IOT_STATUS_SUCCESS;
		}
	}
	return status;
}

iot_status_t app_json_get_string_value( char *buff, int maxlen,
	const app_json_t *jobj )
{
	iot_status_t status = IOT_STATUS_BAD_PARAMETER;
	if ( jobj && buff )
	{
		status = IOT_STATUS_BAD_REQUEST;
		if ( jobj->token->type == JSMN_STRING )
		{
			status = IOT_STATUS_NO_MEMORY;
			if ( maxlen > jobj->token->end - jobj->token->start )
			{
				os_strncpy( buff, jobj->json_string + jobj->token->start,
					jobj->token->end - jobj->token->start );
				buff[jobj->token->end - jobj->token->start] = '\0';

				status = IOT_STATUS_SUCCESS;
			}
		}
	}
	return status;
}

app_json_type_t app_json_get_type( const app_json_t *jobj )
{
	app_json_type_t type = APP_JSON_UNDEFINED;
	if ( jobj )
	{
		switch ( jobj->token->type )
		{
			case JSMN_OBJECT:
				type = APP_JSON_OBJECT;
				break;
			case JSMN_ARRAY:
				type = APP_JSON_ARRAY;
				break;
			case JSMN_STRING:
				type = APP_JSON_STRING;
				break;
			case JSMN_PRIMITIVE:
				switch ( jobj->json_string[jobj->token->start] )
				{
					int i;
					case 't': case 'T': case 'f': case 'F':
						type = APP_JSON_BOOL;
						break;
					case '-': case '0': case '1': case '2': case '3': case '4':
					case '5': case '6': case '7': case '8': case '9':
						i = jobj->token->start;
						type = APP_JSON_INTEGER;
						while ( i < jobj->token->end )
						{
							if ( jobj->json_string[i] == '.' )
							{
								type = APP_JSON_FLOAT;
								break;
							}
							++i;
						}
						break;
					case 'n': case 'N':
						type = APP_JSON_NULL;
						break;
					default:
						type = APP_JSON_UNDEFINED;
				}
				break;
			default:
				type = APP_JSON_UNDEFINED;
		}
	}
	return type;
}

iot_status_t app_json_get_ulong_value( unsigned long *value,
	const app_json_t *jobj )
{
	iot_status_t status = IOT_STATUS_BAD_PARAMETER;
	if ( jobj && value )
	{
		status = IOT_STATUS_BAD_REQUEST;
		if ( app_json_get_type( jobj ) == APP_JSON_INTEGER &&
			jobj->json_string[jobj->token->start] != '-' )
		{
			*value = os_strtoul( jobj->json_string + jobj->token->start,
				NULL );
			status = IOT_STATUS_SUCCESS;
		}
	}
	return status;
}

iot_status_t app_json_object_get( app_json_t *child, const app_json_t *parent,
	const char *key )
{
	iot_status_t status = IOT_STATUS_BAD_PARAMETER;
	if ( parent && key && child )
	{
		status = IOT_STATUS_BAD_REQUEST;
		if ( parent->token->type == JSMN_OBJECT )
		{
			int i;
			int result;
			int key_len = os_strlen( key );
			int child_count = parent->token->size;
			int token_count = parent->token->size;

			status = IOT_STATUS_NOT_FOUND;
			for ( i = 1; i < parent->num_tokens && child_count > 0; ++i )
			{
				if ( token_count == child_count )
				{
					if ( parent->token[i].type == JSMN_STRING &&
						parent->token[i].end - parent->token[i].start == key_len )
					{
						result = os_strncmp( parent->json_string +
							parent->token[i].start, key, key_len );
						if ( result == 0 )
						{
							child->json_string = parent->json_string;
							child->string_len = parent->string_len;
							child->token = parent->token+i+1;
							child->num_tokens = 0;
							child->is_root = IOT_FALSE;

							++i;
							token_count = parent->token[i].size;
							while ( token_count > 0 )
							{
								++i;
								token_count += parent->token[i].size - 1;
								++child->num_tokens;
							}
							status = IOT_STATUS_SUCCESS;
							break;
						}
					}
					--child_count;
				}
				token_count += parent->token[i].size;
				--token_count;
			}
		}
	}
	return status;
}

iot_status_t app_json_parse_file( app_json_t *jobj, const char *file_path,
	size_t max_size )
{
	iot_status_t status = IOT_STATUS_BAD_PARAMETER;
	if ( file_path && jobj )
	{
		os_file_t fd;
		status = IOT_STATUS_FILE_OPEN_FAILED;
		fd = os_file_open( file_path, OS_READ );
		if ( fd )
		{
			size_t size = (size_t)os_file_get_size_handle( fd );
			status = IOT_STATUS_NO_MEMORY;
			if ( max_size > size || max_size == 0 )
			{
				char *json_string = (char *)os_malloc( size + 1 );
				if ( json_string )
				{
					size = os_file_read( json_string, 1, size, fd );
					json_string[size] = '\0';
					status = IOT_STATUS_IO_ERROR;
					if ( size != 0 )
					{
						status = app_json_parse_string( jobj,
							json_string, size );
						jobj->from_file = IOT_TRUE;
						if ( status != IOT_STATUS_SUCCESS )
							os_free( (void *)json_string );
					}
				}
			}
			os_file_close( fd );
		}
	}
	return status;
}

iot_status_t app_json_parse_string( app_json_t *jobj, char *json_string,
	int string_len )
{
	iot_status_t status = IOT_STATUS_BAD_PARAMETER;
	int result;
	jsmn_parser parser;

	if ( jobj && json_string )
	{
		status = IOT_STATUS_PARSE_ERROR;
		jsmn_init( &parser );
		result = jsmn_parse( &parser, json_string, string_len, NULL, 0 );
		if ( result > 0 )
		{
			status = IOT_STATUS_NO_MEMORY;
			jobj->token = os_malloc( sizeof( jsmntok_t ) * result );
			if ( jobj->token )
			{
				status = IOT_STATUS_PARSE_ERROR;
				os_memzero( jobj->token, sizeof( jsmntok_t ) * result );
				jsmn_init( &parser );
				result = jsmn_parse( &parser, json_string, string_len,
					jobj->token, result );
				if ( result > 0 )
				{
					jobj->json_string = json_string;
					jobj->string_len = string_len;
					jobj->num_tokens = result;
					jobj->is_root = IOT_TRUE;
					jobj->from_file = IOT_FALSE;
					status = IOT_STATUS_SUCCESS;
				}
				else
					os_free( (void **)&jobj->token );
			}
		}
	}
	return status;
}

int app_json_size( const app_json_t *jobj )
{
	int size = 0;
	if ( jobj && jobj->token )
		size = jobj->token->size;
	return size;
}
