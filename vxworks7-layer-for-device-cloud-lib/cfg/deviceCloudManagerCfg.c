/* deviceCloudManagerCfg.c - Device Cloud manager configlette */

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

#define DEVICE_CLOUD_MANAGER_TASK_NAME  "tDeviceCloudManager"

extern int device_manager_main ( int argc, char *argv[] );

/******************************************************************************
*
* deviceCloudManagerSpawn() - spawns the application task
*
* This function spawns the application task.
*
* RETURNS: N/A
*/

void deviceCloudManagerSpawn (void)
    {
    static const char *argv[] = { "" };

    (void)sleep (DEVICE_CLOUD_APP_DELAY);

    if (taskSpawn (DEVICE_CLOUD_MANAGER_TASK_NAME,
                   DEVICE_CLOUD_PRIORITY, 0,
                   DEVICE_CLOUD_STACK_SIZE,
                   (FUNCPTR) device_manager_main,
                   (_Vx_usr_arg_t) 1, (_Vx_usr_arg_t) argv,
                   0, 0, 0, 0, 0, 0, 0, 0 ) == TASK_ID_ERROR)
        {
        (void)fprintf (stderr, "Task spawn error.\n");
        }
    }

/******************************************************************************
*
* deviceCloudManagerStart() - spawns a task
*
* This function spawns a task that will spawn the application after a delay.
*
* RETURNS: N/A
*/

void deviceCloudManagerStart (void)
    {
    if (taskSpawn ("tDeviceCloud",
                   DEVICE_CLOUD_PRIORITY,
                   0,
                   DEVICE_CLOUD_STACK_SIZE,
                   (FUNCPTR) deviceCloudManagerSpawn,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == TASK_ID_ERROR)
        {
        (void)fprintf (stderr, "Task spawn error.\n");
        }
    }
