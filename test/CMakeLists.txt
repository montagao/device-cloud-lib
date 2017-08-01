#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

find_package( Cmocka )
include_directories( SYSTEM ${CMOCKA_INCLUDES} )

# Add directory for find cmake configuration files for mocking libraries
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/mock" )

# Fixes some warnings with using CMocka with clang compiler
if ( CMAKE_C_COMPILER_ID MATCHES "Clang" )
	add_definitions( "-Wno-bad-function-cast" )
endif ( CMAKE_C_COMPILER_ID MATCHES "Clang" )

# setup include directories
string( REPLACE "/test" "/src" CURRENT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" )
string( REPLACE "/test" "/src" CURRENT_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" )
include_directories( "${CURRENT_SOURCE_DIR}" )
include_directories( "${CURRENT_BINARY_DIR}/api/protocol-client/out" )
include_directories( "${CURRENT_BINARY_DIR}/build-info/out" )

# Supporting directories
if( MSVC )
	add_definitions( "/D_CRT_SECURE_NO_WARNINGS" )
endif( MSVC )
include_directories( "mock" "test-support" )
add_subdirectory( "mock" )
add_subdirectory( "test-support" )

# Directories containing tests
add_subdirectory( "api" )

