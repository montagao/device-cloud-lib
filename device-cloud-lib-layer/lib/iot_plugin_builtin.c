/**
 * @file
 * @brief Source file for loading built-in plugins
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software may
 * be licensed only pursuant to the terms of an applicable Wind River license
 * agreement.  No license to Wind River intellectual property rights is granted
 * herein.  All rights not licensed by Wind River are reserved by Wind River.
 */

#include "src/api/shared/iot_types.h"

/**
 @brief internal function to load the tr50 plug-in
 @param[out]       p                   location to load plug-in to
 @retval           IOT_TRUE            successfully loaded plug-in
 @retval           IOT_FALSE           failed to load plug-in
 */
iot_bool_t tr50_load( iot_plugin_t *p );


unsigned int iot_plugin_builtin_load( iot_t* lib, unsigned int max )
{
	unsigned int result = 0u;
	if ( lib )
	{
		/* tr50 */
		if ( (lib->plugin_count + result < max) && tr50_load( lib->plugin_ptr[lib->plugin_count + result] ) ) { ++result; }
		lib->plugin_count = result;
	}
	return result;
}

iot_bool_t iot_plugin_builtin_enable( iot_t *lib )
{
	iot_bool_t result = IOT_TRUE;
	if ( lib )
	{
		/* tr50 */
		if ( iot_plugin_enable( lib, "tr50" ) != IOT_STATUS_SUCCESS ) result = IOT_FALSE;
	}
	return result;
}

