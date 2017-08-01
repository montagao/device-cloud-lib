#
# Paho support
#
# If found the following will be defined:
# PAHO_FOUND, If false, do not try to use paho
# PAHO_INCLUDE_DIR, path where to find paho include files
# PAHO_LIBRARY_DIR, path where to find paho libraries
# PAHO_LIBRARIES, the library to link against
# PAHO_VERSION, update paho to display version
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

find_path( PAHO_INCLUDE_DIR
	NAMES "MQTTClient.h"
	DOC "paho include directory"
)

find_library( PAHO_LIBRARIES
	NAMES paho-mqtt3a paho-mqtt3a-static
	DOC "Required paho libraries"
)

get_filename_component( PAHO_LIBRARY_DIR "${PAHO_LIBRARIES}" PATH )

# determine version
if ( PAHO_INCLUDE_DIR AND PAHO_LIBRARIES )
	find_package( Threads )
	set( PAHO_TEST_OUT_FILE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/paho_test.c" )
	file( WRITE "${PAHO_TEST_OUT_FILE}"
		"#include <stdio.h>\n"
		"#include <MQTTClient.h>\n"
		"int main( void ) {\n"
		"  MQTTClient_nameValue nv = MQTTClient_getVersionInfo();\n"
		"  if ( nv ) printf( \"%s\\n\", nv->value );\n"
		"  printf( \"test test\\n\" );\n"
		"  return (nv != NULL) ? EXIT_SUCCESS : EXIT_FAILURE;\n"
		"}\n"
	)
	try_run( PAHO_VERSION_RUN_RESULT PAHO_VERSION_COMPILE_RESULT
		"${CMAKE_BINARY_DIR}" "${PAHO_TEST_OUT_FILE}"
		CMAKE_FLAGS
			"-DINCLUDE_DIRECTORIES:STRING=${PAHO_INCLUDE_DIR}"
			"-DLINK_LIBRARIES:STRING=${PAHO_LIBRARIES}"
		RUN_OUTPUT_VARIABLE PAHO_VERSION_OUTPUT )
	#message( FATAL_ERROR "-DLINK_LIBRARIES:STRING=${PAHO_LIBRARIES};${CMAKE_THREAD_LIBS_INIT}" )
	#if( PAHO_VERSION_COMPILE_RESULT AND PAHO_VERSION_RUN_RESULT )
	#	set( PAHO_VERSION "${PAHO_VERSION_OUTPUT}" )
	#endif( PAHO_VERSION_COMPILE_RESULT AND PAHO_VERSION_RUN_RESULT )
	#message( FATAL_ERROR "${PAHO_VERSION_COMPILE_RESULT}-->${PAHO_TEST_OUT_FILE}-->${PAHO_VERSION_RUN_RESULT}-->${PAHO_VERSION}" )
endif ( PAHO_INCLUDE_DIR AND PAHO_LIBRARIES )


find_package_handle_standard_args( paho
	FOUND_VAR PAHO_FOUND
	REQUIRED_VARS PAHO_LIBRARIES PAHO_INCLUDE_DIR
	VERSION_VAR PAHO_VERSION
	FAIL_MESSAGE DEFAULT_MSG
)

mark_as_advanced(
    PAHO_LIBRARY_DIR
    PAHO_INCLUDE_DIR
    PAHO_LIBRARIES
)

