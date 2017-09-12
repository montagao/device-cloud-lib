#
# Operating System Abstraction Library
#
# The following variables can be set to add additional find support:
# - OSAL_PREFER_STATIC, If true, tries to find static library versions
# - OSAL_ROOT_DIR, specified an explicit root path to test
#
# If found the following will be defined:
# - OSAL_FOUND, If false, do not try to use osal
# - OSAL_INCLUDE_DIR, path where to find osal include files
# - OSAL_LIBRARIES, the library to link against
# - OSAL_VERSION, update osal to display version
#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

include( FindPackageHandleStandardArgs )

set( OSAL_LIB_NAME "osal" )
set( OSAL_LIBS
	${CMAKE_SHARED_LIBRARY_PREFIX}${OSAL_LIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}
	${CMAKE_STATIC_LIBRARY_PREFIX}${OSAL_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}
)

if ( OSAL_PREFER_STATIC )
	list( REVERSE OSAL_LIBS )
endif( OSAL_PREFER_STATIC )

# Try and find paths
set( LIB_SUFFIX "" )
get_property( LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS )
if( LIB64 )
	set( LIB_SUFFIX 64 )
endif()

find_path( OSAL_INCLUDE_DIR
	NAMES "os.h"
	DOC "osal include directory"
	PATHS "${OSAL_ROOT_DIR}/include"
)

find_library( OSAL_LIBRARIES
	NAMES ${OSAL_LIBS}
	DOC "Required osal libraries"
	PATHS "${OSAL_ROOT_DIR}/lib${LIB_SUFFIX}"
)

# determine version
if ( OSAL_INCLUDE_DIR AND EXISTS "${OSAL_INCLUDE_DIR}/os.h" )
	set( _OSAL_VERSION_REGEX
		"^[ \t]*#[ \t]*define[ \t]+OS_LIB_VERSION[ \t]+\"([0-9a-zA-Z][0-9a-zA-Z_.]*[0-9a-zA-Z])\".*$")
	file( STRINGS "${OSAL_INCLUDE_DIR}/os.h"
		_OSAL_VERSION_STRING
		LIMIT_COUNT 1
		REGEX "${_OSAL_VERSION_REGEX}"
	)
	if ( _OSAL_VERSION_STRING )
		string( REGEX REPLACE "${_OSAL_VERSION_REGEX}"
			"\\1" OSAL_VERSION
			"${_OSAL_VERSION_STRING}")
	endif ( _OSAL_VERSION_STRING )
	unset( _OSAL_VERSION_REGEX )
	unset( _OSAL_VERSION_STRING )
endif ( OSAL_INCLUDE_DIR AND EXISTS "${OSAL_INCLUDE_DIR}/os.h" )

find_package_handle_standard_args( osal
	FOUND_VAR OSAL_FOUND
	REQUIRED_VARS OSAL_LIBRARIES OSAL_INCLUDE_DIR
	VERSION_VAR OSAL_VERSION
	FAIL_MESSAGE DEFAULT_MSG
)

mark_as_advanced(
	OSAL_LIBRARY_DIR
	OSAL_INCLUDE_DIR
	OSAL_LIBRARIES
)

