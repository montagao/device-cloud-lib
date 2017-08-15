/**
 * @file
 * @brief header file for base IoT library json functionality
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */
#ifndef JSON_BASE_H
#define JSON_BASE_H

#ifdef IOT_JSON_JANSSON
#include <jansson.h>
#else /* ifdef IOT_JSON_JANSSON */
#include <jsmn.h>
#endif /* else IOT_JSON_JANSSON */

#ifdef IOT_JSON_JANSSON
	/** @brief base structure used for decoding with JANSSON */
	struct iot_json_decoder
	{
		/** @brief output flags */
		unsigned int flags;
		/** @brief pointer to the root object */
		json_t *j_root;
	};
#else /* ifdef IOT_JSON_JANSSON */
	/** @brief base structure used for decoding with JSMN */
	struct iot_json_decoder
	{
		/** @brief start of the buffer */
		const char *buf;
		/** @brief size of json buffer */
		size_t len;
		/** @brief output flags */
		unsigned int flags;
		/** @brief current number of objects */
		unsigned int objs;
		/** @brief maximum number of objects */
		unsigned int size;
		/** @brief pointer to first token */
		jsmntok_t *tokens;
	};
#endif

/* functions */
#ifndef IOT_STACK_ONLY
/**
 * @brief internal dynamic memory allocation function
 *
 * @param[in]      ptr                 currently allocated buffer (optional)
 * @param[in]      size                new size to allocate buffer for
 *
 * @return a pointer to the new allocated memory large enough to hold atleast
 * @c size in bytes, NULL if memory is unable to be allocated
 *
 * @see iot_json_free
 */
void *iot_json_realloc( void *ptr, size_t size );

/**
 * @brief internal dynamic memory deallocation function
 *
 * @param[in]      ptr                 pointer of memory to free
 *
 * @see iot_json_realloc
 */
void iot_json_free( void* ptr );
#endif /* ifndef IOT_STACK_ONLY */

#endif

