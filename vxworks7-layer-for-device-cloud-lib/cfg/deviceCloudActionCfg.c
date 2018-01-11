/* deviceCloudActionCfg.c - Device Cloud action configlette */

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

#define DEVICE_CLOUD_ACTION_TASK_NAME  "tDeviceCloudAction"

extern int app_action_main ( int argc, char *argv[] );

/******************************************************************************
*
* deviceCloudActionSpawn() - spawns the application task
*
* This function spawns the application task.
*
* RETURNS: N/A
*/

void deviceCloudActionSpawn (void)
    {
    static const char *argv[] = { "" };

    (void)sleep (DEVICE_CLOUD_APP_DELAY);

    if (taskSpawn (DEVICE_CLOUD_ACTION_TASK_NAME,
                   DEVICE_CLOUD_PRIORITY, 0,
                   DEVICE_CLOUD_STACK_SIZE,
                   (FUNCPTR) app_action_main,
                   (_Vx_usr_arg_t) 1, (_Vx_usr_arg_t) argv,
                   0, 0, 0, 0, 0, 0, 0, 0 ) == TASK_ID_ERROR)
        {
        (void)fprintf (stderr, "Task spawn error.\n");
        }
    }

/******************************************************************************
*
* deviceCloudActionStart() - spawns a task
*
* This function spawns a task that will spawn the application after a delay.
*
* RETURNS: N/A
*/

void deviceCloudActionStart (void)
    {
    if (taskSpawn ("tDeviceCloud",
                   DEVICE_CLOUD_PRIORITY,
                   0,
                   DEVICE_CLOUD_STACK_SIZE,
                   (FUNCPTR) deviceCloudActionSpawn,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == TASK_ID_ERROR)
        {
        (void)fprintf (stderr, "Task spawn error.\n");
        }
    }
