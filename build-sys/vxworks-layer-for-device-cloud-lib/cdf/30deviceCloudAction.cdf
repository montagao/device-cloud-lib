/* 30deviceCloudAction.cdf - Device Cloud component descriptor file */

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

Component INCLUDE_DEVICE_CLOUD_ACTION_KERNEL {
        NAME            Device Cloud actions kernel task
        SYNOPSIS        This component invokes the Device Cloud actions application as a kernel task
        _CHILDREN       SELECT_DEVICE_CLOUD_AGENT_APP
        ARCHIVE         libdeviceCloudAction.a
        CONFIGLETTES    deviceCloudActionCfg.c
        PROTOTYPE       void deviceCloudActionStart(void);
        INIT_RTN        deviceCloudActionStart();
        _INIT_ORDER     usrRoot
        INIT_AFTER      usrToolsInit
        INIT_BEFORE     INCLUDE_USER_APPL
        REQUIRES        INCLUDE_DEVICE_CLOUD_AGENT
}

Component INCLUDE_DEVICE_CLOUD_ACTION_RTP {
        NAME            Device Cloud actions RTP
        SYNOPSIS        This component invokes the Device Cloud actions application as an RTP
        _CHILDREN       SELECT_DEVICE_CLOUD_AGENT_APP
        CONFIGLETTES    deviceCloudActionRtpCfg.c
        PROTOTYPE       void deviceCloudActionRtp(void);
        INIT_RTN        deviceCloudActionRtp();
        _INIT_ORDER     usrRoot
        INIT_AFTER      usrToolsInit
        INIT_BEFORE     INCLUDE_USER_APPL
        REQUIRES        INCLUDE_DEVICE_CLOUD_AGENT
}
