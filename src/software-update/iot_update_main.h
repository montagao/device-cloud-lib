/**
 * @file
 * @brief Main source file for the Wind River IoT software update application
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#ifndef IOT_UPDATE_MAIN_H
#define IOT_UPDATE_MAIN_H

/**
 * @brief Main entry-point for the application
 *
 * @param[in]      argc                number of arguments passed to the
 *                                     application
 * @param[in]      argv                array of arguments passed to the
 *                                     application
 *
 * @retval EXIT_SUCCESS      application completed successfully
 * @retval EXIT_FAILURE      application encountered an error
 */
int iot_update_main( int argc, char* argv[] );

#endif /* ifndef IOT_UPDATE_MAIN_H */

