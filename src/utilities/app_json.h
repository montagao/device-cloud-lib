
#ifndef APP_JSON_H
#define APP_JSON_H

#include "iot.h"
#include "jsmn.h"


/** @brief Possible types of json object */
typedef enum {
	APP_JSON_UNDEFINED = 0,
	APP_JSON_OBJECT = 1,
	APP_JSON_ARRAY = 2,
	APP_JSON_STRING = 3,
	APP_JSON_BOOL = 4,
	APP_JSON_INTEGER = 5,
	APP_JSON_FLOAT = 6,
	APP_JSON_NULL = 7,
} app_json_type_t;


/** @brief Container struct for all jsmn objects */
typedef struct app_json
{
	char *json_string;
	int string_len;
	jsmntok_t *token;
	int num_tokens;
	iot_bool_t is_root;
	iot_bool_t from_file;
} app_json_t;



/**
 * @brief Get a jsmn object from a jsmn array by index
 *
 * @note jarr must be of type APP_JSON_ARRAY
 *
 * @param[out]     child               Pointer to empty app_json_t to populate
 * @param[in]      parent              jsmn array to retrieve from
 * @param[in]      index               Index of object to retrieve
 *
 * @retval IOT_STATUS_SUCCESS          Successfully retrieved jsmn object
 * @retval IOT_STATUS_BAD_PARAMETER    Invalid/missing parameter
 * @retval IOT_STATUS_BAD_REQUEST      jobj not of type APP_JSON_ARRAY
 * @retval IOT_STATUS_OUT_OF_RANGE     Index not within array bounds
 *
 * @see app_json_get_type
 */
iot_status_t app_json_array_get(
	app_json_t *child,
	const app_json_t *parent,
	int index
);

/**
 * @brief Free array of tokens allocated by root jsmn object
 *
 * @note jobj must be the root jsmn object
 * @note Warning: will break pointers on all other related app_json_t structs
 * @note json_string referenced by jobj will only be freed if jobj was obtained
 *       with app_json_parse_file
 *
 * @param[in]      jobj                jsmn root object to free
 */
void app_json_free(
	app_json_t *jobj
);

/**
 * @brief Get bool value from a jsmn bool object
 *
 * @note jobj must be of type APP_JSON_BOOL
 *
 * @param[out]     value               Pointer to iot_bool_t
 * @param[in]      jobj                jsmn bool
 *
 * @retval IOT_STATUS_SUCCESS          Successfully retrieved bool
 * @retval IOT_STATUS_BAD_PARAMETER    Invalid/missing parameter
 * @retval IOT_STATUS_BAD_REQUEST      jobj not of type APP_JSON_BOOL
 *
 * @see app_json_get_type
 */
iot_status_t app_json_get_bool_value(
	iot_bool_t *value,
	const app_json_t *jobj
);

/**
 * @brief Get double value from a jsmn float object
 *
 * @note jobj must be of type APP_JSON_FLOAT
 *
 * @param[out]     value               Pointer to double
 * @param[in]      jobj                jsmn float
 *
 * @retval IOT_STATUS_SUCCESS          Successfully retrieved double
 * @retval IOT_STATUS_BAD_PARAMETER    Invalid/missing parameter
 * @retval IOT_STATUS_BAD_REQUEST      jobj not of type APP_JSON_FLOAT
 *
 * @see app_json_get_type
 */
iot_status_t app_json_get_double_value(
	double *value,
	const app_json_t *jobj
);

/**
 * @brief Get long value from a jsmn integer object
 *
 * @note jobj must be of type APP_JSON_INTEGER
 *
 * @param[out]     value               Pointer to long
 * @param[in]      jobj                jsmn bool
 *
 * @retval IOT_STATUS_SUCCESS          Successfully retrieved long
 * @retval IOT_STATUS_BAD_PARAMETER    Invalid/missing parameter
 * @retval IOT_STATUS_BAD_REQUEST      jobj not of type APP_JSON_INTEGER
 *
 * @see app_json_get_type
 */
iot_status_t app_json_get_long_value(
	long *value,
	const app_json_t *jobj
);

/**
 * @brief Get string value from a jsmn string object
 *
 * @note jobj must be of type APP_JSON_STRING
 *
 * @param[out]     buff                Pointer to empty string to copy to
 * @param[in]      maxlen              Length of string buffer
 * @param[in]      jobj                jsmn bool
 *
 * @retval IOT_STATUS_SUCCESS          Successfully retrieved string
 * @retval IOT_STATUS_BAD_PARAMETER    Invalid/missing parameter
 * @retval IOT_STATUS_BAD_REQUEST      jobj not of type APP_JSON_STRING
 * @retval IOT_STATUS_NO_MEMORY        char buffer is not large enough
 *
 * @see app_json_get_type
 */
iot_status_t app_json_get_string_value(
	char *buff,
	int maxlen,
	const app_json_t *jobj
);

/**
 * @brief Get type from a jsmn object
 *
 * @param[in]      jobj                jsmn bool
 * @param[out]     dest                Pointer to empty string to copy to
 * @param[in]      maxlen              Length of string buffer
 *
 * @retval APP_JSON_UNDEFINED
 * @retval APP_JSON_OBJECT
 * @retval APP_JSON_ARRAY
 * @retval APP_JSON_STRING
 * @retval APP_JSON_BOOL
 * @retval APP_JSON_INTEGER
 * @retval APP_JSON_FLOAT
 * @retval APP_JSON_NULL
 */
app_json_type_t app_json_get_type(
	const app_json_t *jobj
);

/**
 * @brief Get unsigned long value from a jsmn integer object
 *
 * @note jobj must be of type APP_JSON_INTEGER
 *
 * @param[out]     value               Pointer to unsigned long
 * @param[in]      jobj                jsmn integer
 *
 * @retval IOT_STATUS_SUCCESS          Successfully retrieved unsigned long
 * @retval IOT_STATUS_BAD_PARAMETER    Invalid/missing parameter
 * @retval IOT_STATUS_BAD_REQUEST      jobj not of type APP_JSON_INTEGER or
 *                                     not unsigned
 *
 * @see app_json_get_type
 */
iot_status_t app_json_get_ulong_value(
	unsigned long *value,
	const app_json_t *jobj
);

/**
 * @brief Get a jsmn object from a jsmn object by key
 *
 * @note jobj must be of type APP_JSON_OBJECT
 *
 * @param[out]     child               Pointer to empty app_json_t to populate
 * @param[in]      parent              jsmn object
 * @param[in]      key                 Key string of object to retrieve
 *
 * @retval IOT_STATUS_SUCCESS          Successfully retrieved jsmn object
 * @retval IOT_STATUS_BAD_PARAMETER    Invalid/missing parameter
 * @retval IOT_STATUS_BAD_REQUEST      jobj not of type APP_JSON_OBJECT
 * @retval IOT_STATUS_NOT_FOUND        Could not find token with value of key
 *
 * @see app_json_get_type
 */
iot_status_t app_json_object_get(
	app_json_t *child,
	const app_json_t *parent,
	const char *key
);

/**
 * @brief Get root jsmn object by directly parsing a file
 *
 * @note json_string will be freed by app_json_free ONLY when the app_json_t
 *       struct is obtained with this function.
 *
 * @param[out]     jobj                Root jsmn object
 * @param[in]      file_path           Path to the file to read from
 * @param[in]      max_size            Maximum size of string to allocate,
 *                                     0 denotes no limit
 *
 * @retval IOT_STATUS_SUCCESS          Successfully parsed file
 * @retval IOT_STATUS_BAD_PARAMETER    Invalid/missing parameter
 * @retval IOT_STATUS_PARSE_ERROR      Error parsing contents of file
 * @retval IOT_STATUS_NO_MEMORY        Could not allocate JSON string or
 *                                     token array. File size may exceed
 *                                     maximum specified size.
 * @retval IOT_STATUS_FILE_OPEN_FAILED Failed to open file at file_path
 * @retval IOT_STATUS_IO_ERROR         Failed to read file
 *
 * @see app_json_parse_string
 * @see app_json_free
 */
iot_status_t app_json_parse_file(
	app_json_t *jobj,
	const char *file_path,
	size_t max_size
);

/**
 * @brief Get root jsmn object by parsing a JSON string
 *
 * @note json_string will not be freed by app_json_free when the app_json_t
 *       struct is obtained with this function.
 *
 * @param[out]     jobj                Root jsmn object
 * @param[in]      json_string         JSON to be parsed as a string
 * @param[in]      string_len          Length of JSON string to be parsed
 *
 * @retval IOT_STATUS_SUCCESS          Successfully parsed JSON string
 * @retval IOT_STATUS_BAD_PARAMETER    Invalid/missing parameter
 * @retval IOT_STATUS_PARSE_ERROR      Error parsing string
 * @retval IOT_STATUS_NO_MEMORY        Could not allocate token array
 *
 * @see app_json_parse_file
 * @see app_json_free
 */
iot_status_t app_json_parse_string(
	app_json_t *jobj,
	char *json_string,
	int string_len
);

/**
 * @brief Get the number of children of a jsmn object
 *
 * @param[in]      jobj                jsmn object to count children
 *
 * @return number of children of jobj
 */
int app_json_size(
	const app_json_t *jobj
);

#endif /* ifndef APP_JSON_H */

