#
# json-c support
#
# If found the following will be defined:
# JSONC_FOUND - System has json-c
# JSONC_INCLUDE_DIRS/JSONC_INCLUDES - Include directories for json-c
# JSONC_LIBRARIES/JSONC_LIBS - Libraries needed for json-c
# JSONC_DEFINITIONS - Compiler switches required for json-c
# JSONC_VERSION - Library version
#
#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

include( FindPackageHandleStandardArgs )

find_path( JSONC_INCLUDE_DIR
	NAMES json-c/json.h
	DOC "json-c include directory"
)

find_library( JSONC_LIBRARY
	NAMES json-c
	DOC "Required json-c libraries"
)

# determine version
if ( JSONC_INCLUDE_DIR AND EXISTS "${JSONC_INCLUDE_DIR}/json-c/json_c_version.h" )
	set( _JSONC_VERSION_REGEX
		"^[ \t]*#define[ \t]+JSON_C_VERSION[ \t]+\"([0-9]+)\\.([0-9]+)\\.?([0-9]*)\".*$")
	file( STRINGS "${JSONC_INCLUDE_DIR}/json-c/json_c_version.h"
		_JSONC_VERSION_STRING
		LIMIT_COUNT 1
		REGEX "${_JSONC_VERSION_REGEX}"
	)
	if ( _JSONC_VERSION_STRING )
		string( REGEX REPLACE "${_JSONC_VERSION_REGEX}"
			"\\1.\\2.\\3" JSONC_VERSION
			"${_JSONC_VERSION_STRING}")
		string( REGEX REPLACE "\\.$" ".0" JSONC_VERSION
			"${JSONC_VERSION}" )
	endif ( _JSONC_VERSION_STRING )
	unset( _JSONC_VERSION_REGEX )
	unset( _JSONC_VERSION_STRING )
endif ( JSONC_INCLUDE_DIR AND EXISTS "${JSONC_INCLUDE_DIR}/json-c/json_c_version.h" )

find_package_handle_standard_args( jsonc
	FOUND_VAR JSONC_FOUND
	REQUIRED_VARS JSONC_LIBRARY JSONC_INCLUDE_DIR
	VERSION_VAR JSONC_VERSION
	FAIL_MESSAGE DEFAULT_MSG
)

set( JSONC_LIBRARIES ${JSONC_LIBRARY} )
set( JSONC_INCLUDE_DIRS ${JSONC_INCLUDE_DIR} )
set( JSONC_DEFINITIONS "" )

set( JSONC_LIBS ${JSONC_LIBRARIES} )
set( JSONC_INCLUDES ${JSONC_INCLUDE_DIRS} )

mark_as_advanced( JSONC_INCLUDE_DIR JSONC_LIBRARY )
