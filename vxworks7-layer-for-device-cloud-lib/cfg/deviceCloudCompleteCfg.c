/* deviceCloudCompleteCfg.c - Device Cloud complete configlette */

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

#include <taskLib.h>

#define DEVICE_CLOUD_COMPLETE_TASK_NAME  "tDeviceCloudComplete"

extern int app_complete_main ( int argc, char *argv[] );

/******************************************************************************
*
* deviceCloudCompleteStart() - spawns the application task
*
* This function spawns the application task.
*
* RETURNS: N/A
*/

void deviceCloudCompleteStart (void)
    {
    static const char *argv[] = { "" };

    if (taskSpawn (DEVICE_CLOUD_COMPLETE_TASK_NAME,
                   DEVICE_CLOUD_PRIORITY, 0,
                   DEVICE_CLOUD_STACK_SIZE,
                   (FUNCPTR) app_complete_main,
                   (_Vx_usr_arg_t) 1, (_Vx_usr_arg_t) argv,
                   0, 0, 0, 0, 0, 0, 0, 0 ) == TASK_ID_ERROR)
        {
        (void)fprintf (stderr, "Task spawn error.\n");
        }
    }
