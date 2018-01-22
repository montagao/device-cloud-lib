/**
 * @file
 * @brief Header file defining internal variables
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
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
#ifndef IOT_BUILD_H
#define IOT_BUILD_H

/** @brief GIT sha of the project */
#define IOT_GIT_SHA                    ""

/** @brief Name of the product */
#define IOT_PRODUCT                    "Helix Device Cloud"
/** @brief Short name of the product */
#define IOT_PRODUCT_SHORT              "hdc"

/** @brief project version as string */
#define IOT_VERSION                    "17.12.01"
/** @brief project major version number */
#define IOT_VERSION_MAJOR              17
/** @brief project minor version number */
#define IOT_VERSION_MINOR              12
/** @brief project patch version number */
#define IOT_VERSION_PATCH              01
/** @brief project tweak version number */
#define IOT_VERSION_TWEAK              0


/** @brief Maximum number of supported actions */
#define IOT_ACTION_MAX                 255
/** @brief Maximum number of actions that can be queued */
#define IOT_ACTION_QUEUE_MAX           10
/** @brief Maximum number of actions in stack */
#define IOT_ACTION_STACK_MAX           3
/** @brief maximum number of alarm items reserved on the stack */
#define IOT_ALARM_STACK_MAX            3
/** @brief maximum number of alarm items allowed in an application */
#define IOT_ALARM_MAX                  255
/** @brief Maximum number of options */
#define IOT_OPTION_MAX                 20
/** @brief Maximum number of parameters per action */
#define IOT_PARAMETER_MAX              7
/** @brief maximum number of samples per telemetry item */
#define IOT_SAMPLE_MAX                 10
/** @brief maximum number of telemetry items reserved on the stack */
#define IOT_TELEMETRY_STACK_MAX        3
/** @brief maximum number of telemetry items allowed in an application */
#define IOT_TELEMETRY_MAX              255
/** @brief Number of "worker" threads */
#define IOT_WORKER_THREADS             5

/* DIRECTORIES */
/** @brief default configuration directory */
extern const char *deviceCloudConfigDirGet ( void );
#define IOT_DEFAULT_DIR_CONFIG         deviceCloudConfigDirGet()
/** @brief default runtime directory */
extern const char *deviceCloudRuntimeDirGet ( void );
#define IOT_DEFAULT_DIR_RUNTIME        deviceCloudRuntimeDirGet()

/* FILES */
/** @brief Connection configuration file name */
#define IOT_DEFAULT_FILE_CONFIG        "iot-connect"
/** @brief Configuration file extension (begin with '.') */
#define IOT_DEFAULT_FILE_CONFIG_EXT    ".cfg"
/** @brief File to store the generated device id */
#define IOT_DEFAULT_FILE_DEVICE_ID     "device_id"
/** @brief Device manager configuration file name */
#define IOT_DEFAULT_FILE_DEVICE_MANAGER "iot.cfg"

/* PATHS */
/** @brief Sub-directory that binary applications are installed within */
#define IOT_BIN_DIR                    "bin"

/* TARGETS */
/** @brief name of executable for performing updates */
#define IOT_TARGET_UPDATE              "iot-update"

/* PLUG-INS */
/** @brief maximum number of plug-ins that can be loaded */
#define IOT_PLUGIN_MAX                 5

/* INTERNAL APPLICATION INFORMATION */

/** @brief Device manager target name (executable name) */
#define IOT_DEVICE_MANAGER_TARGET      "iot-device-manager"
/** @brief Device manager name (short description) */
#define IOT_DEVICE_MANAGER_NAME        "Helix Device Cloud Device Manager"
/** @brief Device manager long description */
#define IOT_DEVICE_MANAGER_DESCRIPTION "Registers and handles device level requests for Helix Device Cloud devices. Client that registers with iot-service to handle over-the-air updates, device reboot/shutdown, agent reset etc."

/** @brief Control application target name (executable name) */
#define IOT_CONTROL_TARGET             "iot-control"
/** @brief Control application name (short description) */
#define IOT_CONTROL_NAME               ""
/** @brief Control application long description */
#define IOT_CONTROL_DESCRIPTION        ""


/** @brief Update application name (executable name) */
#define IOT_TARGET_UPDATE              "iot-update"
/** @brief Update application name (short description) */
#define IOT_UPDATE_NAME                ""
/** @brief Update appication long description */
#define IOT_UPDATE_DESCRIPTION         ""


/** @brief Websocket target name (executable name) */
#define IOT_TARGET_RELAY               "iot-relay"
/** @brief Websocket name (short description) */
#define IOT_RELAY_NAME                 ""
/** @brief Websocket long description */
#define IOT_RELAY_DESCRIPTION          ""

/** @brief Default state of agent reset action */
#define IOT_DEFAULT_ENABLE_AGENT_RESET                    1

/** @brief Default state of agent quit action */
#define IOT_DEFAULT_ENABLE_AGENT_QUIT                     1

/** @brief Default state of decommission device action */
#define IOT_DEFAULT_ENABLE_DECOMMISSION_DEVICE            1

/** @brief Default state of device reboot action */
#define IOT_DEFAULT_ENABLE_DEVICE_REBOOT                  1

/** @brief Default state of device shutdown action */
#define IOT_DEFAULT_ENABLE_DEVICE_SHUTDOWN                1

/** @brief Default state of dump log files action */
#define IOT_DEFAULT_ENABLE_DUMP_LOG_FILES                 1

/** @brief Default state of file transfer related actions */
#define IOT_DEFAULT_ENABLE_FILE_TRANSFERS                 1

/** @brief Default state of remote login related action */
#define IOT_DEFAULT_ENABLE_REMOTE_LOGIN                   1

/** @brief Default state of restore factory images action */
#define IOT_DEFAULT_ENABLE_RESTORE_FACTORY_IMAGES         0

/** @brief Default state of software update action */
#define IOT_DEFAULT_ENABLE_SOFTWARE_UPDATE                1

/** @brief Default state of enable offline action - do not unregister actions */
#define IOT_DEFAULT_ENABLE_PERSISTENT_ACTIONS             1

/** @brief Default state of enable automation test on Android */
#define IOT_DEFAULT_ENABLE_AUTOMATION_TEST_ON_ANDROID     0

/** @brief Default state of uploaded file removal upon success */
#define IOT_DEFAULT_UPLOAD_REMOVE_ON_SUCCESS              1

/** @brief Exectutable suffix */
#define IOT_EXE_SUFFIX                 ""

/** @brief Default user running services */
#define IOT_USER                       "iot"

/** @brief Default certification path */
#define IOT_DEFAULT_CERT_PATH          "/bd0:1/etc/ssl/certs/ca-certificates.crt"
/** @brief Default log file name when installing updates */
#define IOT_UPDATE_LOGFILE             "iot_install_updates.log"

/** @brief Default delay in seconds before rebooting */
#define IOT_REBOOT_DELAY               5

extern const char *deviceCloudRtpDirGet (void);
extern unsigned int deviceCloudPriorityGet (void);
extern unsigned int deviceCloudStackSizeGet (void);

#endif /* ifndef IOT_BUILD_H */
