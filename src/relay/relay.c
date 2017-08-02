/**
 * @file
 * @brief Main source file for the Wind River IoT relay application
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "relay_main.h"

/**
 * @brief Main entry-point for the application
 *
 * The body of this function is actually in another function, this is to allow
 * for full unit testing of the application.  The unit test would not work if
 * there were multiple "main" functions; i.e. if there was one for the entry
 * point and another for the unit test entry point.  Thus, this function is
 * just a wrapper around a starting block named other than "main".
 *
 * @param[in]      argc                          number of arguments passed to
 *                                               the application
 * @param[in]      argv                          array of arguments passed to
 *                                               the application
 *
 * @retval EXIT_SUCCESS      application completed successfully
 * @retval EXIT_FAILURE      application encountered an error
 */
int main( int argc, char* argv[] )
{
	return relay_main( argc, argv );
}

