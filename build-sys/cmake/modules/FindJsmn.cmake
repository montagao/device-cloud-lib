#
# Jsmn support
#
# If found the following will be defined:
# JSMN_FOUND, If false, do not try to use jsmn
# JSMN_INCLUDE_DIR, path where to find jsmn include files
# JSMN_LIBRARY_DIR, path where to find jsmn libraries
# JSMN_LIBRARIES, the library to link against
# JSMN_VERSION, update jsmn to display version
#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

#find_package( PkgConfig )
include( FindPackageHandleStandardArgs )

find_path( JSMN_INCLUDE_DIR
	NAMES "jsmn.h"
	DOC "jsmn include directory"
)

find_library( JSMN_LIBRARIES
	NAMES jsmn
	DOC "Required jsmn libraries"
)

get_filename_component( JSMN_LIBRARY_DIR "${JSMN_LIBRARIES}" PATH )

find_package_handle_standard_args( jsmn
	FOUND_VAR JSMN_FOUND
	REQUIRED_VARS JSMN_LIBRARIES JSMN_INCLUDE_DIR
	FAIL_MESSAGE DEFAULT_MSG
)

mark_as_advanced(
    JSMN_LIBRARY_DIR
    JSMN_INCLUDE_DIR
    JSMN_LIBRARIES
)

