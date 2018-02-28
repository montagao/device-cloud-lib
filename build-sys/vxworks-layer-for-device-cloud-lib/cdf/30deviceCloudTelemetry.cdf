/* 30deviceCloudTelemetry.cdf - Device Cloud component descriptor file */

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

Component INCLUDE_DEVICE_CLOUD_TELEMETRY_KERNEL {
        NAME            Device Cloud telemetry kernel task
        SYNOPSIS        This component invokes the Device Cloud telemetry application as a kernel task
        _CHILDREN       SELECT_DEVICE_CLOUD_AGENT_APP
        ARCHIVE         libdeviceCloudTelemetry.a
        CONFIGLETTES    deviceCloudTelemetryCfg.c
        PROTOTYPE       void deviceCloudTelemetryStart(void);
        INIT_RTN        deviceCloudTelemetryStart();
        _INIT_ORDER     usrRoot
        INIT_AFTER      usrToolsInit
        INIT_BEFORE     INCLUDE_USER_APPL
        REQUIRES        INCLUDE_DEVICE_CLOUD
}

Component INCLUDE_DEVICE_CLOUD_TELEMETRY_RTP {
        NAME            Device Cloud telemetry RTP
        SYNOPSIS        This component invokes the Device Cloud telemetry application as an RTP
        _CHILDREN       SELECT_DEVICE_CLOUD_AGENT_APP
        CONFIGLETTES    deviceCloudTelemetryRtpCfg.c
        PROTOTYPE       void deviceCloudTelemetryRtp(void);
        INIT_RTN        deviceCloudTelemetryRtp();
        _INIT_ORDER     usrRoot
        INIT_AFTER      usrToolsInit
        INIT_BEFORE     INCLUDE_USER_APPL
        REQUIRES        INCLUDE_DEVICE_CLOUD
}
