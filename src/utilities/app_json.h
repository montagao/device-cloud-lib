/**
 * @file
 * @brief Header file for json operations within the IoT library
 *
 * @copyright Copyright (C) 2017-2018 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied."
 */
#ifndef APP_JSON_H
#define APP_JSON_H

#include "api/public/iot.h"

#ifdef __cplusplus
extern C {
#endif

/** @brief Signature for the function to call to reallocate dynamic memory */
typedef void *(*app_json_realloc_t)(void *, size_t);
/** @brief Signature for the function to call to free dynamic memory */
typedef void (*app_json_free_t)(void *);

/** @brief type of json structure */
typedef enum app_json_type
{
	APP_JSON_TYPE_NULL     = 0x0,   /* 0000 0000 (0) */
	APP_JSON_TYPE_ARRAY    = 0x1,   /* 0000 0001 (1) */
	APP_JSON_TYPE_OBJECT   = 0x2,   /* 0000 0010 (2) */
	APP_JSON_TYPE_BOOL     = 0x4,   /* 0000 0100 (4) */
	APP_JSON_TYPE_INTEGER  = 0x8,   /* 0000 1000 (8) */
	APP_JSON_TYPE_REAL     = 0x10,  /* 0001 0000 (16) */
	APP_JSON_TYPE_STRING   = 0x20   /* 0010 0000 (32) */
} app_json_type_t;

/**
 * @brief Wheter to add a space after characters
 *
 * When this flag is set the output is: ": " & ", " vs. ":" & ","
 */
#define APP_JSON_FLAG_EXPAND           (1)
#ifndef IOT_STACK_ONLY
/**
 * @brief Use dynamic memory allocation for internal objects
 */
#define APP_JSON_FLAG_DYNAMIC          (APP_JSON_FLAG_EXPAND << 1)
#endif /* ifndef IOT_STACK_ONLY */
/**
 * @brief Internal macro used for bit-shifting, number of bits to shift by
 */
#define APP_JSON_INDENT_OFFSET         2
/**
 * @brief If @p x is >0 add a new-line and the number of spaces indicated for
 *        each item
 *
 * @note must be less than or equal to:
 *       sizeof( unsigned int ) * 8 - JSON_INDENT_OFFSET
 */
#define APP_JSON_FLAG_INDENT(x)        ((x) << APP_JSON_INDENT_OFFSET)

/* BASE SUPPORT */
#ifndef IOT_STACK_ONLY
/**
 * @brief retrieves the function for dynamic memory allocation
 *
 * @param[out]     mptr                pointer to the realloction function
 * @param[out]     fptr                pointer to the free function
 */
IOT_API IOT_SECTION void app_json_allocation_get(
	app_json_realloc_t **mptr,
	app_json_free_t **fptr );

/**
 * @brief sets the function for dynamic memory allocation
 *
 * @param[in]      mptr                pointer to the realloction function
 * @param[in]      fptr                pointer to the free function
 */
IOT_API IOT_SECTION void app_json_allocation_set(
	app_json_realloc_t *mptr,
	app_json_free_t *fptr );
#endif /* ifndef IOT_STACK_ONLY */

/* DECODE SUPPORT */
/******************/

/**
 * @brief helper functions for determining JSON type
 * @{
 */
#define app_json_decode_is_array( json, item ) ( app_json_decode_type( (json), (item) ) == JSON_TYPE_ARRAY )
#define app_json_decode_is_boolean( json, item ) ( app_json_decode_type( (json), (item) ) == JSON_TYPE_BOOLEAN )
#define app_json_decode_is_integer( json, item ) ( app_json_decode_type( (json), (item) ) == JSON_TYPE_INTEGER )
#define app_json_decode_is_object( json, item ) ( app_json_decode_type( (json), (item) ) == JSON_TYPE_OBJECT )
#define app_json_decode_is_real( json, item ) ( app_json_decode_type( (json), (item) ) == JSON_TYPE_REAL )
#define app_json_decode_is_string( json, item ) ( app_json_decode_type( (json), (item) ) == JSON_TYPE_STRING )
/** @} */

/** @brief Represents a JSON decoder object */
typedef struct app_json_decoder app_json_decoder_t;
/** @brief Represents a JSON item (object, array, string, real, etc.) */
typedef void app_json_item_t;
/** @brief Represents an object for iterating through items in a JSON array */
typedef void app_json_array_iterator_t;
/** @brief Represents an object for iterating through items in a JSON object */
typedef void app_json_object_iterator_t;

/**
 * @brief Returns the element in array at position index.
 *
 * The valid range for index is from 0 to the return value of
 * json_array_size() minus 1.
 *
 * @param[in]      decoder             JSON decoder object
 * @param[in]      item                JSON array
 * @param[in]      index               index of item to retrieve
 * @param[in,out]  out                 returned item at the specified index
 *
 * @returns if @c item is not a JSON array (or NULL) or the index is out of
 *          range then root will be NULL
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      @c item does not point to a JSON array
 * @retval IOT_STATUS_NOT_FOUND        @c index is out of bounds
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see app_json_decode_array_size
 */
IOT_API IOT_SECTION iot_status_t app_json_decode_array_at(
	const app_json_decoder_t *decoder,
	const app_json_item_t *item,
	size_t index,
	const app_json_item_t **out );

/**
 * @brief Returns an iterator for iterating through a JSON array
 *
 * @param[in]      decoder             JSON decoder object
 * @param[in]      item                JSON array to create iterator for
 *
 * @retval NULL    item is not a JSON array or @c item or @c decoder is NULL or
 *                 JSON object contains no elements
 * @retval !NULL   pointer to an iterator to iterate through the given array
 *
 * @see app_json_decode_array_iterator_next
 * @see app_json_decode_array_size
 */
IOT_API IOT_SECTION const app_json_array_iterator_t *app_json_decode_array_iterator(
	const app_json_decoder_t *decoder,
	const app_json_item_t *item );

/**
 * @brief Returns an iterator pointing to the next item in a JSON array
 *
 * @param[in]      decoder             JSON decoder object
 * @param[in]      item                JSON array to currently being iterated
 * @param[in,out]  iter                current JSON iterator
 *
 * @note the object pointed to by @c iter is no longer valid after this call
 *
 * @retval NULL    an invalid parameter passed to the function or at the end of
 *                 the JSON object
 * @retval !NULL   pointer the next location in the given object
 *
 * @see app_json_decode_array_iterator
 * @see app_json_decode_array_iterator_value
 * @see app_json_decode_array_size
 */
IOT_API IOT_SECTION const app_json_array_iterator_t *app_json_decode_array_iterator_next(
	const app_json_decoder_t *decoder,
	const app_json_item_t *item,
	const app_json_array_iterator_t *iter );

/**
 * @brief Returns value for the object that an iterator currently points to
 *
 * @param[in]      decoder             JSON decoder object
 * @param[in]      item                JSON array to currently being iterated
 * @param[in]      iter                iterator to retieve key for
 * @param[in,out]  out                 JSON object that is at the iterator
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see app_json_decode_array_iterator
 * @see app_json_decode_array_iterator_next
 */
IOT_API IOT_SECTION iot_status_t app_json_decode_array_iterator_value(
	const app_json_decoder_t *decoder,
	const app_json_item_t *item,
	const app_json_array_iterator_t *iter,
	const app_json_item_t **out );

/**
 * @brief Returns the number of elements in an array
 *
 * @param[in]      decoder             JSON decoder object
 * @param[in]      item                JSON array
 *
 * @return Returns the number of elements in an array, or 0 if array is NULL
 * or not a JSON array.
 *
 * @see app_json_decode_array_at
 */
IOT_API IOT_SECTION size_t app_json_decode_array_size(
	const app_json_decoder_t *decoder,
	const app_json_item_t *item );

/**
 * @brief Returns the associated boolean value
 *
 * @param[in]      decoder             JSON decoder object
 * @param[in]      item                JSON boolean
 * @param[in,out]  value               returned boolean value
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      @c item does not point to a JSON boolean
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see app_json_decode_type
 * @see app_json_encode_bool
 */
IOT_API IOT_SECTION iot_status_t app_json_decode_bool(
	const app_json_decoder_t *decoder,
	const app_json_item_t *item,
	iot_bool_t *value );

/**
 * @brief Initializes the JSON decoding system
 *
 * @note specifying the flag APP_JSON_FLAG_DYNAMIC indicates to use dynamic
 * memory on the heap for allocating the JSON decoder object and JSON tokens.
 * In this case, the parameters @c buf and @c len are ignored.
 *
 * @param[in,out]  buf                 memory to use for the base parser
 * @param[in]      len                 amount of memory in the buf parameter
 * @param[in]      flags               flags for indicating parsing support
 *
 * @return a valid JSON decoder object
 *
 * @see app_json_decode_parse
 * @see app_json_decode_terminate
 */
IOT_API IOT_SECTION app_json_decoder_t *app_json_decode_initialize(
	void *buf,
	size_t len,
	unsigned int flags );

/**
 * @brief Returns the associated integer value
 *
 * @param[in]      decoder             JSON decoder object
 * @param[in]      item                JSON integer
 * @param[in,out]  value               returned integer value
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      @c item does not point to a JSON integer
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see app_json_decode_number
 * @see app_json_decode_real
 * @see app_json_decode_type
 * @see app_json_encode_integer
 */
IOT_API IOT_SECTION iot_status_t app_json_decode_integer(
	const app_json_decoder_t *decoder,
	const app_json_item_t *item,
	iot_int64_t *value );

/**
 * @brief Returns the associated integer or real number value
 *
 * @param[in]      decoder             JSON decoder object
 * @param[in]      item                JSON integer or JSON real number
 * @param[in,out]  value               returned number value
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      @c item does not point to a JSON integer
 *                                     or real number
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see app_json_decode_integer
 * @see app_json_decode_real
 * @see app_json_decode_type
 */
IOT_API IOT_SECTION iot_status_t app_json_decode_number(
	const app_json_decoder_t *decoder,
	const app_json_item_t *item,
	iot_float64_t *value );

/**
 * @brief Returns the item mathcing the given key in an object
 *
 * @param[in]      decoder             JSON decoder object
 * @param[in]      object              JSON object to search through
 * @param[in]      key                 item key to find
 *
 * @return pointer to the JSON item matching the key given or NULL if not found
 *
 * @see app_json_decode_object_find_len
 * @see app_json_decode_object_iterator
 * @see app_json_decode_object_iterator_key
 */
IOT_API IOT_SECTION const app_json_item_t *app_json_decode_object_find(
	const app_json_decoder_t *decoder,
	const app_json_item_t *object,
	const char *key );

/**
 * @brief Returns the item mathcing the given key in an object
 *
 * @param[in]      decoder             JSON decoder object
 * @param[in]      object              JSON object to search through
 * @param[in]      key                 item key to find
 * @param[in]      key_len             length of key (set to 0 for NULL-terminated)
 *
 * @return pointer to the JSON item matching the key given or NULL if not found
 *
 * @see app_json_decode_object_find
 * @see app_json_decode_object_iterator
 * @see app_json_decode_object_iterator_key
 */
IOT_API IOT_SECTION const app_json_item_t *app_json_decode_object_find_len(
	const app_json_decoder_t *decoder,
	const app_json_item_t *object,
	const char *key,
	size_t key_len );

/**
 * @brief Returns an iterator for iterating through a JSON object
 *
 * @param[in]      decoder             JSON decoder object
 * @param[in]      item                JSON object to create iterator for
 *
 * @retval NULL    item is not a JSON object or @c item or @c decoder is NULL or
 *                 JSON object contains no elements
 * @retval !NULL   pointer to an iterator to iterate through the given object
 *
 * @see app_json_decode_object_iterator_next
 * @see app_json_decode_object_size
 */
IOT_API IOT_SECTION const app_json_object_iterator_t *
	app_json_decode_object_iterator(
		const app_json_decoder_t *decoder,
		const app_json_item_t *item );

/**
 * @brief Returns key for the object that an iterator currently points to
 *
 * @note The returned key is read-only and must not be modified or freed by
 * the user. It is valid as long as the @c decoder object remains in scope.
 *
 * @warning The returned string is not expected to be null-terminated,
 * to determine the length use the value returned via the @c key_len parameter
 *
 * @param[in]      decoder             JSON decoder object
 * @param[in]      item                JSON object to currently being iterated
 * @param[in]      iter                iterator to retieve key for
 * @param[out]     key                 returned string
 * @param[out]     key_len             length of the returned string
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_NOT_INITIALIZED  key is not set for the item
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see app_json_decode_object_iterator
 * @see app_json_decode_object_iterator_next
 * @see app_json_decode_object_iterator_value
 */
IOT_API IOT_SECTION iot_status_t app_json_decode_object_iterator_key(
	const app_json_decoder_t *decoder,
	const app_json_item_t *item,
	const app_json_object_iterator_t *iter,
	const char **key,
	size_t *key_len );

/**
 * @brief Returns an iterator pointing to the next item in a JSON object
 *
 * @param[in]      decoder             JSON decoder object
 * @param[in]      item                JSON object to currently being iterated
 * @param[in,out]  iter                current JSON iterator
 *
 * @note the object pointed to by @c iter is no longer valid after this call
 *
 * @retval NULL    an invalid parameter passed to the function or at the end of
 *                 the JSON object
 * @retval !NULL   pointer the next location in the given object
 *
 * @see app_json_decode_object_iterator
 * @see app_json_decode_object_iterator_key
 * @see app_json_decode_object_iterator_value
 * @see app_json_decode_object_size
 */
IOT_API IOT_SECTION const app_json_object_iterator_t *
	app_json_decode_object_iterator_next(
		const app_json_decoder_t *decoder,
		const app_json_item_t *item,
		const app_json_object_iterator_t *iter );

/**
 * @brief Returns value for the object that an iterator currently points to
 *
 * @param[in]      decoder             JSON decoder object
 * @param[in]      item                JSON object to currently being iterated
 * @param[in]      iter                iterator to retieve key for
 * @param[in,out]  out                 JSON object that is at the iterator
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see app_json_decode_object_iterator
 * @see app_json_decode_object_iterator_key
 * @see app_json_decode_object_iterator_next
 */
IOT_API IOT_SECTION iot_status_t app_json_decode_object_iterator_value(
	const app_json_decoder_t *decoder,
	const app_json_item_t *item,
	const app_json_object_iterator_t *iter,
	const app_json_item_t **out );

/**
 * @brief Returns the number of elements in an object
 *
 * @param[in]      decoder             JSON decoder object
 * @param[in]      object              JSON object
 *
 * @return Returns the number of elements in an array, or 0 if object is NULL
 * or not a JSON object
 *
 * @see app_json_decode_array_at
 * @see app_json_decode_object_iterator
 */
IOT_API IOT_SECTION size_t app_json_decode_object_size(
	const app_json_decoder_t *decoder,
	const app_json_item_t *object );

/**
 * @brief Parses a given JSON string and returns the root element
 *
 * @note the JSON string provided in the @c js parameter may not be checked for
 * the null-terminating character, always indicated the amount of valid
 * characters using the @c len parameter
 *
 * @param[in]      decoder             JSON decoder object
 * @param[in]      js                  pointer to a string containing JSON text
 * @param[in]      len                 length of the JSON string
 * @param[out]     root                the root element
 * @param[in,out]  error               error text on failure (optional)
 * @param[in]      error_len           size of the error string buffer
 *                                     (optional)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_NO_MEMORY        not enough memory available
 * @retval IOT_STATUS_PARSE_ERROR      invalid JSON string passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see app_json_decode_initialize
 */
IOT_API IOT_SECTION iot_status_t app_json_decode_parse(
	app_json_decoder_t *decoder,
	const char* js,
	size_t len,
	const app_json_item_t **root,
	char *error,
	size_t error_len );

/**
 * @brief Returns the associated real number value
 *
 * @param[in]      decoder             JSON decoder object
 * @param[in]      item                JSON real number
 * @param[in,out]  value               returned real number value
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      @c item does not point to a JSON real
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see app_json_decode_integer
 * @see app_json_decode_number
 * @see app_json_decode_type
 * @see app_json_encode_real
 */
IOT_API IOT_SECTION iot_status_t app_json_decode_real(
	const app_json_decoder_t *decoder,
	const app_json_item_t *item,
	iot_float64_t *value );

/**
 * @brief Returns the associated string value
 *
 * @note Returns the associated value of string as a null terminated UTF-8
 * encoded string, or NULL if string is not a JSON string.
 *
 * @note The returned value is read-only and must not be modified or freed by
 * the user. It is valid as long as the @c decoder object remains in scope.
 *
 * @warning The returned string not expected to be null-terminated, to determine
 * the length use the value returned via the @c value_len parameter
 *
 * @param[in]      decoder             JSON decoder object
 * @param[in]      item                JSON string
 * @param[out]     value               returned string
 * @param[out]     value_len           length of the returned string
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      @c item does not point to a JSON string
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see app_json_decode_type
 * @see app_json_encode_string
 */
IOT_API IOT_SECTION iot_status_t app_json_decode_string(
	const app_json_decoder_t *decoder,
	const app_json_item_t *item,
	const char **value,
	size_t *value_len );

/**
 * @brief Frees memory assocated with a JSON decoder
 *
 * @param[in]      decoder             JSON decoder object
 *
 * @see app_json_decode_initialize
 */
IOT_API IOT_SECTION void app_json_decode_terminate(
	app_json_decoder_t *decoder );

/**
 * @brief Returns the type pointed to by @c item
 *
 * @param[in]      decoder             JSON decoder object
 * @param[in]      item                JSON item
 *
 * @return the type of JSON object that @c item points to or APP_JSON_TYPE_NULL
 * if @c item or @c decoder is NULL or @c item is not a valid pointer
 *
 * @see app_json_decode_bool
 * @see app_json_decode_integer
 * @see app_json_decode_real
 * @see app_json_decode_string
 */
IOT_API IOT_SECTION app_json_type_t app_json_decode_type(
	const app_json_decoder_t *decoder,
	const app_json_item_t *item );


/* ENCODE SUPPORT */
/********************/
/** @brief Represents a JSON encoder object */
typedef struct app_json_encoder app_json_encoder_t;

/**
 * @brief Ends the encoding of a JSON array
 *
 * @param[in]      encoder             JSON encoder object
 *
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      not inside an JSON array object
 * @retval IOS_STATUS_SUCCESS          on success
 *
 * @see app_json_encode_array_start
 */
IOT_API IOT_SECTION iot_status_t app_json_encode_array_end(
	app_json_encoder_t *encoder );

/**
 * @brief Starts the encoding of a new JSON array
 *
 * @param[in]      encoder             JSON encoder object
 * @param[in]      key                 (optional) parent JSON object key
 *
 * @note @c key should be NULL when not inside a JSON object.  If defining a key
 * when not inside an JSON object a parent object is generated.  If NULL when
 * inside a JSON object, a blank key ("") will be used.  Adding an object with
 * the same key as another item in the object will result in undefined
 * behaviour.
 *
 * @retval IOT_STATUS_FULL             the maximum number of items for the
 *                                     buffer has been reached
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      adding item not inside an array or object
 * @retval IOT_STATUS_NO_MEMORY        no more memory available
 * @retval IOS_STATUS_SUCCESS          on success
 *
 * @see app_json_encode_array_end
 */
IOT_API IOT_SECTION iot_status_t app_json_encode_array_start(
	app_json_encoder_t *encoder,
	const char *key );

/**
 * @brief Encodes a boolean
 *
 * @param[in]      encoder             JSON encoder object
 * @param[in]      key                 (optional) parent JSON object key
 * @param[in]      value               boolean value
 *
 * @note @c key should be NULL when not inside a JSON object.  If defining a key
 * when not inside an JSON object a parent object is generated.  If NULL when
 * inside a JSON object, a blank key ("") will be used.  Adding an object with
 * the same key as another item in the object will result in undefined
 * behaviour.
 *
 * @retval IOT_STATUS_FULL             the maximum number of items for the
 *                                     buffer has been reached
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      adding item not inside an array or object
 * @retval IOT_STATUS_NO_MEMORY        no more memory available
 * @retval IOS_STATUS_SUCCESS          on success
 *
 * @see app_json_decode_bool
 */
IOT_API IOT_SECTION iot_status_t app_json_encode_bool(
	app_json_encoder_t *encoder,
	const char *key,
	iot_bool_t value );

/**
 * @brief Outputs a JSON encoded string produced by the JSON encoder
 *
 * @param[in]      encoder             JSON encoder object
 *
 * @return string in JSON format
 */
IOT_API IOT_SECTION const char *app_json_encode_dump(
	app_json_encoder_t *encoder );

/**
 * @brief Initializes the JSON encoding system
 *
 * @note specifying the flag APP_JSON_FLAG_DYNAMIC indicates to use dynamic
 * memory on the heap for allocating the JSON encoder object and JSON tokens.
 * In this case, the parameters @c buf and @c len are ignored.
 *
 * @param[in,out]  buf                 memory to use for the base parser
 * @param[in]      len                 amount of memory in the buf parameter
 * @param[in]      flags               flags for indicating parsing support
 *
 * @return a valid JSON encoder object
 *
 * @see app_json_encode_parse
 * @see app_json_encode_terminate
 */
IOT_API IOT_SECTION app_json_encoder_t *app_json_encode_initialize(
	void *buf,
	size_t len,
	unsigned int flags );

/**
 * @brief Encodes an integer number
 *
 * @param[in]      encoder             JSON encoder object
 * @param[in]      key                 (optional) parent JSON object key
 * @param[in]      value               integer number
 *
 * @note @c key should be NULL when not inside a JSON object.  If defining a key
 * when not inside an JSON object a parent object is generated.  If NULL when
 * inside a JSON object, a blank key ("") will be used.  Adding an object with
 * the same key as another item in the object will result in undefined
 * behaviour.
 *
 * @retval IOT_STATUS_FULL             the maximum number of items for the
 *                                     buffer has been reached
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      adding item not inside an array or object
 * @retval IOT_STATUS_NO_MEMORY        no more memory available
 * @retval IOS_STATUS_SUCCESS          on success
 *
 * @see app_json_decode_integer
 */
IOT_API IOT_SECTION iot_status_t app_json_encode_integer(
	app_json_encoder_t *encoder,
	const char *key,
	iot_int64_t value );

/**
 * @brief Cancels the encoding of a JSON object
 *
 * @param[in]      encoder             JSON encoder object
 *
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      not inside an JSON array object
 * @retval IOS_STATUS_SUCCESS          on success
 *
 * @see app_json_encode_object_end
 * @see app_json_encode_object_start
 */
IOT_API IOT_SECTION iot_status_t app_json_encode_object_cancel(
	app_json_encoder_t *encoder );

/**
 * @brief Removes any items previously added into an object
 *
 * @param[in]      encoder             JSON encoder object
 *
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      not inside an JSON array object
 * @retval IOS_STATUS_SUCCESS          on success
 *
 * @see app_json_encode_object_end
 * @see app_json_encode_object_start
 */
IOT_API IOT_SECTION iot_status_t app_json_encode_object_clear(
	app_json_encoder_t *encoder );

/**
 * @brief Ends the encoding of a JSON object
 *
 * @param[in]      encoder             JSON encoder object
 *
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      not inside an JSON array object
 * @retval IOS_STATUS_SUCCESS          on success
 *
 * @see app_json_encode_object_cancel
 * @see app_json_encode_object_start
 */
IOT_API IOT_SECTION iot_status_t app_json_encode_object_end(
	app_json_encoder_t *encoder );

/**
 * @brief Starts the encoding of a new JSON object
 *
 * @param[in]      encoder             JSON encoder object
 * @param[in]      key                 (optional) parent JSON object key
 *
 * @note @c key should be NULL when not inside a JSON object.  If defining a key
 * when not inside an JSON object a parent object is generated.  If NULL when
 * inside a JSON object, a blank key ("") will be used.  Adding an object with
 * the same key as another item in the object will result in undefined
 * behaviour.
 *
 * @retval IOT_STATUS_FULL             the maximum number of items for the
 *                                     buffer has been reached
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      adding item not inside an array or object
 * @retval IOT_STATUS_NO_MEMORY        no more memory available
 * @retval IOS_STATUS_SUCCESS          on success
 *
 * @see app_json_encode_object_cancel
 * @see app_json_encode_object_end
 */
IOT_API IOT_SECTION iot_status_t app_json_encode_object_start(
	app_json_encoder_t *encoder,
	const char *key );

/**
 * @brief Encodes a floating-point number
 *
 * @param[in]      encoder             JSON encoder object
 * @param[in]      key                 (optional) parent JSON object key
 * @param[in]      value               floating-point number
 *
 * @note @c key should be NULL when not inside a JSON object.  If defining a key
 * when not inside an JSON object a parent object is generated.  If NULL when
 * inside a JSON object, a blank key ("") will be used.  Adding an object with
 * the same key as another item in the object will result in undefined
 * behaviour.
 *
 * @retval IOT_STATUS_FULL             the maximum number of items for the
 *                                     buffer has been reached
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      adding item not inside an array or object
 * @retval IOT_STATUS_NO_MEMORY        no more memory available
 * @retval IOS_STATUS_SUCCESS          on success
 *
 * @see app_json_decode_real
 */
IOT_API IOT_SECTION iot_status_t app_json_encode_real(
	app_json_encoder_t *encoder,
	const char *key,
	iot_float64_t value );

/**
 * @brief Encodes a string
 *
 * @param[in]      encoder             JSON encoder object
 * @param[in]      key                 (optional) parent JSON object key
 * @param[in]      value               string value
 *
 * @note @c key should be NULL when not inside a JSON object.  If defining a key
 * when not inside an JSON object a parent object is generated.  If NULL when
 * inside a JSON object, a blank key ("") will be used.  Adding an object with
 * the same key as another item in the object will result in undefined
 * behaviour.
 *
 * @retval IOT_STATUS_FULL             the maximum number of items for the
 *                                     buffer has been reached
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      adding item not inside an array or object
 * @retval IOT_STATUS_NO_MEMORY        no more memory available
 * @retval IOS_STATUS_SUCCESS          on success
 *
 * @see app_json_decode_string
 */
IOT_API IOT_SECTION iot_status_t app_json_encode_string(
	app_json_encoder_t *encoder,
	const char *key,
	const char *value );

/**
 * @brief Frees memory assocated with a JSON encoder
 *
 * @param[in]      encoder             JSON encoder object
 *
 * @see app_json_encode_initialize
 */
IOT_API IOT_SECTION void app_json_encode_terminate(
	app_json_encoder_t *encoder );

#ifdef __cplusplus
};
#endif

#endif /* ifndef APP_JSON_H */
