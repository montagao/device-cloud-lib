/* 30deviceCloudActions.cdf - Device Cloud actions component descriptor file */

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

Component INCLUDE_DEVICE_CLOUD_ACTIONS_KERNEL {
        NAME            Device Cloud actions kernel task
        SYNOPSIS        This component invokes the actions application as kernel task
        _CHILDREN       SELECT_DEVICE_CLOUD_APP
        ARCHIVE         libdeviceCloudActions.a
        CONFIGLETTES    deviceCloudActionsCfg.c
        PROTOTYPE       void deviceCloudActionsStart(void);
        INIT_RTN        deviceCloudActionsStart();
        _INIT_ORDER     usrRoot
        INIT_AFTER      usrToolsInit
        INIT_BEFORE     INCLUDE_USER_APPL
        REQUIRES        INCLUDE_DEVICE_CLOUD_LIBRARY
}

Component INCLUDE_DEVICE_CLOUD_ACTIONS_RTP {
        NAME            Device Cloud actions RTP
        SYNOPSIS        This component invokes the actions application as RTP
        _CHILDREN       SELECT_DEVICE_CLOUD_APP
        CONFIGLETTES    deviceCloudActionsRtpCfg.c
        PROTOTYPE       void deviceCloudActionsRtp(void);
        INIT_RTN        deviceCloudActionsRtp();
        _INIT_ORDER     usrRoot
        INIT_AFTER      usrToolsInit
        INIT_BEFORE     INCLUDE_USER_APPL
        REQUIRES        INCLUDE_DEVICE_CLOUD_LIBRARY
}
