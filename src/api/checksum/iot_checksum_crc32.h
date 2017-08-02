/**
 * @file
 * @brief source file for calculating crc32 checksum of a file
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#ifndef IOT_CHECKSUM_CRC32_H
#define IOT_CHECKSUM_CRC32_H

#include <iot.h>
#include <os.h>

iot_status_t iot_checksum_crc32_file_get(
	os_file_t file,
	iot_uint64_t *checksum );

#endif /* IOT_CHECKSUM_CRC32_H */
