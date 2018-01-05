/* deviceCloudCfg.c - Device Cloud configlette */

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

const char *deviceCloudConfigDirGet (void)
    {
    return DEVICE_CLOUD_CONFIG_DIR;
    }

const char *deviceCloudRuntimeDirGet (void)
    {
    return DEVICE_CLOUD_RUNTIME_DIR;
    }

const char *deviceCloudRtpDirGet (void)
    {
    return DEVICE_CLOUD_RTP_DIR;
    }

unsigned int deviceCloudPriorityGet (void)
    {
    return DEVICE_CLOUD_PRIORITY;
    }

unsigned int deviceCloudStackSizeGet (void)
    {
    return DEVICE_CLOUD_STACK_SIZE;
    }
