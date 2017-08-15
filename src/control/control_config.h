/**
 * @file
 * @brief Main source file for the Wind River IoT control configuration files
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */
#ifndef CONTROL_CONFIG_FILE_H

#include "../api/json/iot_json_schema.h"

/**
 * @brief Generates a configuration file for the product by prompting for
 *        user input
 *
 * @retval IOT_STATUS_FAILURE          operating system operation failed
 * @retval IOT_STATUS_NO_MEMORY        not enough memory available
 * @retval IOT_STATUS_SUCCESS          on success
 */
iot_status_t control_config_generate( void );

#endif /* ifndef CONTROL_CONFIG_FILE_H */
