/* deviceCloudManagerRtpCfg.c - Device Cloud configlette */

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

#include <ioLib.h>
#include <taskLib.h>
#include <rtpLib.h>

#define DEVICE_CLOUD_MANAGER_RTP_NAME "iot-device-manager"

/******************************************************************************
* 
* deviceCloudManagerRtpDelay() - spawns the RTP.
*
* This function spawns the RTP after a delay.
*
* RETURNS: N/A
*/

static void deviceCloudManagerRtpDelay (void)
    {
    int fd;
    const char *args[12];
    char priorityStr[32];
    char stackSizeStr[32];

    (void)sleep (DEVICE_CLOUD_AGENT_APP_DELAY);

    if (chdir (DEVICE_CLOUD_AGENT_BIN_DIR) != OK)
        {
        (void)fprintf (stderr, "bin directory %s chdir failed.\n", DEVICE_CLOUD_AGENT_BIN_DIR);
        return;
        }

    if ((fd = open (DEVICE_CLOUD_MANAGER_RTP_NAME, O_RDONLY, 0)) == -1)
        {
        (void)fprintf (stderr, "Open RTP file %s failed.\n", DEVICE_CLOUD_MANAGER_RTP_NAME);
        return;
        }
    (void)close (fd);

    (void)snprintf (priorityStr, 32, "%d", DEVICE_CLOUD_AGENT_PRIORITY);
    (void)snprintf (stackSizeStr, 32, "%d", DEVICE_CLOUD_AGENT_STACK_SIZE);

    args[0] = "";
    args[1] = "-d";
    args[2] = DEVICE_CLOUD_AGENT_CONFIG_DIR;
    args[3] = "-u";
    args[4] = DEVICE_CLOUD_AGENT_RUNTIME_DIR;
    args[5] = "-r";
    args[6] = DEVICE_CLOUD_AGENT_BIN_DIR;
    args[7] = "-p";
    args[8] = priorityStr;
    args[9] = "-t";
    args[10] = stackSizeStr;
    args[11] = NULL;

    if (rtpSpawn (DEVICE_CLOUD_MANAGER_RTP_NAME, args, NULL,
                  DEVICE_CLOUD_AGENT_PRIORITY,
                  DEVICE_CLOUD_AGENT_STACK_SIZE,
                  RTP_LOADED_WAIT, VX_FP_TASK) == RTP_ID_ERROR)
        {
        (void)fprintf (stderr, "RTP spawn %s error.\n", DEVICE_CLOUD_MANAGER_RTP_NAME);
        }
    }

/******************************************************************************
* 
* deviceCloudManagerRtp() - spawns a task
*
* This function spawns a task that will spawn the RTP after a delay.
*
* RETURNS: N/A
*/

void deviceCloudManagerRtp (void)
    {
    if (taskSpawn ("tManagerRtpDelay",
                   DEVICE_CLOUD_AGENT_PRIORITY,
                   0,
                   DEVICE_CLOUD_AGENT_STACK_SIZE,
                   (FUNCPTR) deviceCloudManagerRtpDelay,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == TASK_ID_ERROR)
        {
        (void)fprintf (stderr, "Task spawn error.\n");
        }
    }
