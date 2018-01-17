/* 20deviceCloudLib.cdf - Device Cloud library component descriptor file */

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

Component INCLUDE_DEVICE_CLOUD_LIBRARY {
    NAME            Device Cloud library
    SYNOPSIS        Device Cloud library
    _CHILDREN       FOLDER_HDC
    REQUIRES        INCLUDE_POSIX_PTHREAD_SCHEDULER \
                    INCLUDE_POSIX_PIPES \
                    INCLUDE_IPSSL \
                    INCLUDE_IPDNSC \
                    INCLUDE_IPSNTPC \
                    INCLUDE_IPTELNETS \
                    INCLUDE_DOSFS \
                    INCLUDE_ARCHIVE \
                    INCLUDE_CIVETWEB
    CFG_PARAMS      DEVICE_CLOUD_CONFIG_DIR \
                    DEVICE_CLOUD_RUNTIME_DIR \
                    DEVICE_CLOUD_RTP_DIR \
                    DEVICE_CLOUD_PRIORITY \
                    DEVICE_CLOUD_STACK_SIZE \
                    DEVICE_CLOUD_APP_DELAY
    CONFIGLETTES    deviceCloudCfg.c
    ARCHIVE         libiot.a
}

Parameter DEVICE_CLOUD_CONFIG_DIR {
    NAME            Device Cloud configuration directory
    SYNOPSIS        This value points to the configuration directory where \
                    the Device Cloud library will look for \
                    the configuration files.
    TYPE            string
    DEFAULT         "/bd0:1/etc/iot"
}

Parameter DEVICE_CLOUD_RUNTIME_DIR {
    NAME            Device Cloud runtime directory
    SYNOPSIS        This value points to the runtime directory where \
                    the Device Cloud library will look for \
                    the runtime files.
    TYPE            string
    DEFAULT         "/bd0:1/var/lib/iot"
}

Parameter DEVICE_CLOUD_RTP_DIR {
    NAME            Device Cloud RTP directory
    SYNOPSIS        This value points to the RTP directory where \
                    the Device Cloud library will look for \
                    the RTP binary.
    TYPE            string
    DEFAULT         "/bd0:1/bin"
}

Parameter DEVICE_CLOUD_PRIORITY {
    NAME            Device Cloud task and thread priority
    SYNOPSIS        This parameter specifies the task and thread priority.
    TYPE            uint
    DEFAULT         100
}

Parameter DEVICE_CLOUD_STACK_SIZE {
    NAME            Device Cloud task and thread stack size
    SYNOPSIS        This parameter specifies the task and thread stack size.
    TYPE            uint
    DEFAULT         0x10000
}

Parameter DEVICE_CLOUD_APP_DELAY {
    NAME            Device Cloud application delay
    SYNOPSIS        This parameter specifies the start up delay in seconds before launching the application.
    TYPE            uint
    DEFAULT         10
}

Selection SELECT_DEVICE_CLOUD_APP {
        NAME            Select Device Cloud application
        SYNOPSIS        This selection selects a choice of application.
        _CHILDREN       FOLDER_HDC
        _DEFAULTS       FOLDER_HDC
        COUNT           1-1
}
