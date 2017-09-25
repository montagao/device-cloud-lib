/**
 * @file
 * @brief source file for calculating checksum of a file
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "iot_checksum.h"
#include "iot_checksum_crc32.h"

iot_status_t iot_checksum_file_get(
	iot_t *lib,
	os_file_t file,
	iot_checksum_type_t type,
	iot_uint64_t *checksum )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( file && checksum )
	{
		switch ( type )
		{
		case IOT_CHECKSUM_TYPE_CRC32:
			result = iot_checksum_crc32_file_get( file, checksum );
			break;
		case IOT_CHECKSUM_TYPE_MD5:
		case IOT_CHECKSUM_TYPE_SHA256:
		default:
			IOT_LOG( lib, IOT_LOG_ERROR, "%s",
				"Checksum algorithm not support" );
			break;
		}
	}
	return result;
}
