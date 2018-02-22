/* deviceCloudCompleteRtpCfg.c - Device Cloud configlette */

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

#define DEVICE_CLOUD_COMPLETE_RTP_NAME "iot-app-complete"

/******************************************************************************
* 
* deviceCloudCompleteRtpDelay() - spawns the RTP.
*
* This function spawns the RTP after a delay.
*
* RETURNS: N/A
*/

static void deviceCloudCompleteRtpDelay (void)
    {
    int fd;
    const char *args[2];

    (void)sleep (DEVICE_CLOUD_APP_DELAY);

    if (chdir (DEVICE_CLOUD_RTP_DIR) != OK)
        {
        (void)fprintf (stderr, "RTP directory %s chdir failed.\n", DEVICE_CLOUD_RTP_DIR);
        return;
        }

    if ((fd = open (DEVICE_CLOUD_COMPLETE_RTP_NAME, O_RDONLY, 0)) == -1)
        {
        (void)fprintf (stderr, "Open RTP file %s failed.\n", DEVICE_CLOUD_COMPLETE_RTP_NAME);
        return;
        }
    (void)close (fd);

    args[0] = "";
    args[1] = NULL;

    if (rtpSpawn (DEVICE_CLOUD_COMPLETE_RTP_NAME, args, NULL,
                  DEVICE_CLOUD_PRIORITY,
                  DEVICE_CLOUD_STACK_SIZE,
                  RTP_LOADED_WAIT, VX_FP_TASK) == RTP_ID_ERROR)
        {
        (void)fprintf (stderr, "RTP spawn %s error.\n", DEVICE_CLOUD_COMPLETE_RTP_NAME);
        }
    }

/******************************************************************************
* 
* deviceCloudCompleteRtp() - spawns a task
*
* This function spawns a task that will spawn the RTP after a delay.
*
* RETURNS: N/A
*/

void deviceCloudCompleteRtp (void)
    {
    if (taskSpawn ("tCompleteRtpDelay",
                   DEVICE_CLOUD_PRIORITY,
                   0,
                   DEVICE_CLOUD_STACK_SIZE,
                   (FUNCPTR) deviceCloudCompleteRtpDelay,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == TASK_ID_ERROR)
        {
        (void)fprintf (stderr, "Task spawn error.\n");
        }
    }
