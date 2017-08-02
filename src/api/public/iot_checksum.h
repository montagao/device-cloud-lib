/**
 * @file
 * @brief Header file for calculating checksums within the IoT library
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#ifndef IOT_CHECKSUM_H
#define IOT_CHECKSUM_H

#include <iot.h>
#include <os.h>

/** @brief type checksum algorithm supported */
typedef enum iot_checksum_type
{
	IOT_CHECKSUM_TYPE_CRC32,
	IOT_CHECKSUM_TYPE_MD5,
	IOT_CHECKSUM_TYPE_SHA256
} iot_checksum_type_t;

iot_status_t iot_checksum_file_get(
	os_file_t file,
	iot_checksum_type_t type,
	iot_uint64_t *checksum );

#endif /* IOT_CHECKSUM_H */
