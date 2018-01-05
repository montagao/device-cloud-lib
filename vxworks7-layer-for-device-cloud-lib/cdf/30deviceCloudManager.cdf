/* 30deviceCloudManager.cdf - Device Cloud manager component descriptor file */

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

Component INCLUDE_DEVICE_CLOUD_MANAGER_KERNEL {
        NAME            Device Cloud manager kernel task
        SYNOPSIS        This component invokes the manager application as kernel task
        _CHILDREN       SELECT_DEVICE_CLOUD_APP
        _DEFAULTS       SELECT_DEVICE_CLOUD_APP
        ARCHIVE         libdeviceCloudManager.a
        CONFIGLETTES    deviceCloudManagerCfg.c
        PROTOTYPE       void deviceCloudManagerStart(void);
        INIT_RTN        deviceCloudManagerStart();
        _INIT_ORDER     usrRoot
        INIT_AFTER      usrAppInit
        REQUIRES        INCLUDE_DEVICE_CLOUD_LIBRARY
}

Component INCLUDE_DEVICE_CLOUD_MANAGER_RTP {
        NAME            Device Cloud manager RTP
        SYNOPSIS        This component invokes the manager application as RTP
        _CHILDREN       SELECT_DEVICE_CLOUD_APP
        CONFIGLETTES    deviceCloudManagerRtpCfg.c
        PROTOTYPE       void deviceCloudManagerRtp(void);
        INIT_RTN        deviceCloudManagerRtp();
        _INIT_ORDER     usrRoot
        INIT_AFTER      usrRtpAppInit
        REQUIRES        INCLUDE_DEVICE_CLOUD_LIBRARY
}
