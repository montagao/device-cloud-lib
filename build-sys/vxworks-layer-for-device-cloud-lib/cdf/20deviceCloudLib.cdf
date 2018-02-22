/* 20deviceCloudLib.cdf - Device Cloud component descriptor file */

/*
 * Copyright (c) 2017 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
05oct17,yat  created
*/

Component INCLUDE_DEVICE_CLOUD {
    NAME            Device Cloud
    SYNOPSIS        Device Cloud
    _CHILDREN       FOLDER_HDC
    REQUIRES        INCLUDE_POSIX_PTHREAD_SCHEDULER \
                    INCLUDE_POSIX_PIPES \
                    INCLUDE_IPSSL \
                    INCLUDE_IPDNSC \
                    INCLUDE_IPSNTPC \
                    INCLUDE_DOSFS \
                    INCLUDE_UNIX \
                    INCLUDE_ARCHIVE \
                    INCLUDE_MOSQUITTO \
                    INCLUDE_WEBCLI_CURL
    CFG_PARAMS      DEVICE_CLOUD_CONFIG_DIR \
                    DEVICE_CLOUD_RUNTIME_DIR \
                    DEVICE_CLOUD_RTP_DIR \
                    DEVICE_CLOUD_PRIORITY \
                    DEVICE_CLOUD_STACK_SIZE \
                    DEVICE_CLOUD_APP_DELAY
    CONFIGLETTES    deviceCloudCfg.c
    ARCHIVE         libiot.a libosal.a
    LINK_SYMS       iot_initialize iot_terminate os_system_run
}

Parameter DEVICE_CLOUD_CONFIG_DIR {
    NAME            Device Cloud application connection configuration directory
    SYNOPSIS        The directory for the Device Cloud application connection \
                    configuration files.
    TYPE            string
    DEFAULT         "/bd0:1/etc/iot"
}

Parameter DEVICE_CLOUD_RUNTIME_DIR {
    NAME            Device Cloud runtime directory
    SYNOPSIS        The directory for the run-time files and the base path of \
                    the upload and download directories used for file transfer.
    TYPE            string
    DEFAULT         "/bd0:1/var/lib/iot"
}

Parameter DEVICE_CLOUD_RTP_DIR {
    NAME            Device Cloud RTP directory
    SYNOPSIS        The directory for the Device Cloud RTP applications.
    TYPE            string
    DEFAULT         "/bd0:1/bin"
}

Parameter DEVICE_CLOUD_PRIORITY {
    NAME            Device Cloud task and thread priority
    SYNOPSIS        The task and thread priority of the application.
    TYPE            uint
    DEFAULT         100
}

Parameter DEVICE_CLOUD_STACK_SIZE {
    NAME            Device Cloud task and thread stack size
    SYNOPSIS        The task and thread stack size of the application.
    TYPE            uint
    DEFAULT         0x10000
}

Parameter DEVICE_CLOUD_APP_DELAY {
    NAME            Device Cloud application delay
    SYNOPSIS        The start up delay in seconds before launching \
                    the application.
    TYPE            uint
    DEFAULT         10
}

Selection SELECT_DEVICE_CLOUD_APP {
    NAME            Select Device Cloud application
    SYNOPSIS        The selection of the application.
    _CHILDREN       FOLDER_HDC
    _DEFAULTS       FOLDER_HDC
    COUNT           1-
}
