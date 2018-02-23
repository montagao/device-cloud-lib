/* 30deviceCloudComplete.cdf - Device Cloud component descriptor file */

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

Component INCLUDE_DEVICE_CLOUD_COMPLETE_KERNEL {
        NAME            Device Cloud complete kernel task
        SYNOPSIS        This component invokes the Device Cloud complete application as a kernel task
        _CHILDREN       SELECT_DEVICE_CLOUD_APP
        _DEFAULTS       SELECT_DEVICE_CLOUD_APP
        ARCHIVE         libdeviceCloudComplete.a
        CONFIGLETTES    deviceCloudCompleteCfg.c
        PROTOTYPE       void deviceCloudCompleteStart(void);
        INIT_RTN        deviceCloudCompleteStart();
        _INIT_ORDER     usrRoot
        INIT_AFTER      usrToolsInit
        INIT_BEFORE     INCLUDE_USER_APPL
        REQUIRES        INCLUDE_DEVICE_CLOUD
}

Component INCLUDE_DEVICE_CLOUD_COMPLETE_RTP {
        NAME            Device Cloud complete RTP
        SYNOPSIS        This component invokes the Device Cloud complete application as an RTP
        _CHILDREN       SELECT_DEVICE_CLOUD_APP
        CONFIGLETTES    deviceCloudCompleteRtpCfg.c
        PROTOTYPE       void deviceCloudCompleteRtp(void);
        INIT_RTN        deviceCloudCompleteRtp();
        _INIT_ORDER     usrRoot
        INIT_AFTER      usrToolsInit
        INIT_BEFORE     INCLUDE_USER_APPL
        REQUIRES        INCLUDE_DEVICE_CLOUD
}
