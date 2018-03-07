/* 30deviceCloudLocation.cdf - Device Cloud component descriptor file */

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

Component INCLUDE_DEVICE_CLOUD_LOCATION_KERNEL {
        NAME            Device Cloud location kernel task
        SYNOPSIS        This component invokes the Device Cloud location application as a kernel task
        _CHILDREN       SELECT_DEVICE_CLOUD_AGENT_APP
        ARCHIVE         libdeviceCloudLocation.a
        CONFIGLETTES    deviceCloudLocationCfg.c
        PROTOTYPE       void deviceCloudLocationStart(void);
        INIT_RTN        deviceCloudLocationStart();
        _INIT_ORDER     usrRoot
        INIT_AFTER      usrToolsInit
        INIT_BEFORE     INCLUDE_USER_APPL
        REQUIRES        INCLUDE_DEVICE_CLOUD_AGENT
}

#if 0
Component INCLUDE_DEVICE_CLOUD_LOCATION_RTP {
        NAME            Device Cloud location RTP
        SYNOPSIS        This component invokes the Device Cloud location application as an RTP
        _CHILDREN       SELECT_DEVICE_CLOUD_AGENT_APP
        CONFIGLETTES    deviceCloudLocationRtpCfg.c
        PROTOTYPE       void deviceCloudLocationRtp(void);
        INIT_RTN        deviceCloudLocationRtp();
        _INIT_ORDER     usrRoot
        INIT_AFTER      usrToolsInit
        INIT_BEFORE     INCLUDE_USER_APPL
        REQUIRES        INCLUDE_DEVICE_CLOUD_AGENT
}
#endif
