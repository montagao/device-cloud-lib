/**
 * @file
 * @brief Main source file for the Wind River IoT device manager
 *
 * @copyright Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#ifndef DEVICE_MANAGER_MAIN_H
#define DEVICE_MANAGER_MAIN_H

#include "os.h"
#include "device_manager_file.h"
#include <iot.h>

/**
 * @brief Main loop sleep interval in milliseconds
 */
#define POLL_INTERVAL_MSEC 2000u

/** @brief Flag to enable software update (manifest) action */
#define DEVICE_MANAGER_ENABLE_SOFTWARE_UPDATE         0x0001
/** @brief Flag to enable file transfers related actions */
#define DEVICE_MANAGER_ENABLE_FILE_TRANSFERS          0x0002
/** @brief Flag to enable decommission device action */
#define DEVICE_MANAGER_ENABLE_DECOMMISSION_DEVICE     0x0004
/** @brief Flag to enable restore factory image action */
#define DEVICE_MANAGER_ENABLE_RESTORE_FACTORY_IMAGES  0x0008
/** @brief Flag to enable dump log files action */
#define DEVICE_MANAGER_ENABLE_DUMP_LOG_FILES          0x0010
/** @brief Flag to enable device shutdown action */
#define DEVICE_MANAGER_ENABLE_DEVICE_SHUTDOWN         0x0020
/** @brief Flag to enable device reboot action */
#define DEVICE_MANAGER_ENABLE_DEVICE_REBOOT           0x0040
/** @brief Flag to enable agent reset action */
#define DEVICE_MANAGER_ENABLE_AGENT_RESET             0x0080
/** @brief Flag to enable remote login related actions */
#define DEVICE_MANAGER_ENABLE_REMOTE_LOGIN            0x0100

/** @brief Remote login protocol maximum length */
#define REMOTE_LOGIN_PROTOCOL_MAX 32

/** @brief Structure containing information for the device manager */
struct device_manager_info
{
	/** @brief Library handle */
	iot_t *iot_lib;
#ifndef NO_FILEIO_SUPPORT
	/** @brief structure used to support file input/output operations */
	struct device_manager_file_io_info file_io_info;
#endif
	/** @brief Mask of enabled actions */
	iot_uint16_t enabled_actions;
	/** @brief manifest action */
	iot_action_t *software_update;
	/** @brief decommission action */
	iot_action_t *decommission_device;
	/** @brief restore_factory_images action */
	iot_action_t *restore_factory_images;
	/** @brief dump_log_files action */
	iot_action_t *dump_log_files;
	/** @brief device_shutdown action */
	iot_action_t *device_shutdown;
	/** @brief device_reboot action */
	iot_action_t *device_reboot;
	/** @brief agent_reset action */
	iot_action_t *agent_reset;
	/** @brief remote_login action */
	iot_action_t *remote_login;
	/** @brief file_download action */
	iot_action_t *file_download;
	/** @brief file_upload action */
	iot_action_t *file_upload;
	/** @brief remote_login_protocol action */
	iot_action_t *remote_login_protocol;
	/** @brief agent_state */
	iot_bool_t agent_state;
	/** @brief application path */
	char app_path[PATH_MAX + 1u];
	/** @brief run time directory */
	char runtime_dir[PATH_MAX + 1u];
	/** @brief valid remote login protocols */
	char remote_login_protocols[REMOTE_LOGIN_PROTOCOL_MAX];
	/** @brief number of loops main loop has gone through */
	size_t loop_count;
	/** @brief log level */
	char log_level[16];
};

/**
 * @brief Main entry-point for the application
 *
 * @param[in]      argc                          number of arguments passed to
 *                                               the application
 * @param[in]      argv                          array of arguments passed to
 *                                               the application
 *
 * @retval EXIT_SUCCESS      application completed successfully
 * @retval EXIT_FAILURE      application encountered an error
 */
int device_manager_main( int argc, char* argv[] );

#endif /* ifndef DEVICE_MANAGER_H */

