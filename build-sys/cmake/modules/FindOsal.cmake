#
# Operating System Abstraction Library
#
# If found the following will be defined:
# OSAL_FOUND, If false, do not try to use osal
# OSAL_INCLUDE_DIR, path where to find osal include files
# OSAL_LIBRARY_DIR, path where to find osal libraries
# OSAL_LIBRARIES, the library to link against
# OSAL_VERSION, update osal to display version
#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

include( FindPackageHandleStandardArgs )

find_path( OSAL_INCLUDE_DIR
	NAMES "os.h"
	DOC "osal include directory"
)

find_library( OSAL_LIBRARIES
	NAMES osal
	DOC "Required osal libraries"
)

get_filename_component( OSAL_LIBRARY_DIR "${OSAL_LIBRARIES}" PATH )

find_package_handle_standard_args( osal
	FOUND_VAR OSAL_FOUND
	REQUIRED_VARS OSAL_LIBRARIES OSAL_INCLUDE_DIR
	FAIL_MESSAGE DEFAULT_MSG
)

mark_as_advanced(
	OSAL_LIBRARY_DIR
	OSAL_INCLUDE_DIR
	OSAL_LIBRARIES
)

