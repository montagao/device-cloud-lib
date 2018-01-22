/**
 * @file
 * @brief VxWorks 7 configlette file
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied."
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
* deviceCloudCompleteSpawn() - spawns the application task
*
* This function spawns the application task.
*
* RETURNS: N/A
*/

void deviceCloudCompleteSpawn (void)
    {
    static const char *argv[] = { "" };

    (void)sleep (DEVICE_CLOUD_APP_DELAY);

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

/******************************************************************************
*
* deviceCloudCompleteStart() - spawns a task
*
* This function spawns a task that will spawn the application after a delay.
*
* RETURNS: N/A
*/

void deviceCloudCompleteStart (void)
    {
    if (taskSpawn ("tDeviceCloud",
                   DEVICE_CLOUD_PRIORITY,
                   0,
                   DEVICE_CLOUD_STACK_SIZE,
                   (FUNCPTR) deviceCloudCompleteSpawn,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == TASK_ID_ERROR)
        {
        (void)fprintf (stderr, "Task spawn error.\n");
        }
    }
