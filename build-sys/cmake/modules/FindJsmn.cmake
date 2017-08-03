#
# Jsmn support
#
# The following variables can be set to add additional find support:
# - JSMN_ROOT_DIR, specified an explicit root path to test
#
# If found the following will be defined:
# - JSMN_DEFINES, definitions JSMN library was built with
# - JSMN_FOUND, If false, do not try to use jsmn
# - JSMN_INCLUDE_DIR, path where to find jsmn include files
# - JSMN_LIBRARIES, the library to link against
#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

include( FindPackageHandleStandardArgs )

find_path( JSMN_INCLUDE_DIR
	NAMES "jsmn.h"
	DOC "jsmn include directory"
	PATHS "${JSMN_ROOT_DIR}"
)

find_library( JSMN_LIBRARIES
	NAMES jsmn
	DOC "Required jsmn libraries"
	PATHS "${JSMN_ROOT_DIR}"
)

if( JSMN_INCLUDE_DIR AND JSMN_LIBRARIES )
	# Determine flags JSMN was compiled with
	set( JSMN_TEST_OUT_FILE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/jsmn_test.c" )
	file( WRITE "${JSMN_TEST_OUT_FILE}"
		"#define JSMN_PARENT_LINKS\n"
		"#include <jsmn.h>\n"
		"#include <stdio.h>\n"

		"int main( void ) {\n"
		"  const char *const js = \"{\\\"key\\\":bad_value}\";\n"
		"  jsmn_parser p;\n"
		"  jsmntok_t t[5];\n;"
		"  jsmn_init( &p );\n"
		"  if(jsmn_parse( &p, js, 15, &t[0], 5) == JSMN_ERROR_INVAL)\n"
		"    printf(\"JSMN_STRICT\\n\");\n"
		"  if(t[0].parent == -1)\n"
		"    printf(\"JSMN_PARENT_LINKS\\n\");\n"
		"  return 0;\n"
		"}\n"
	)
	try_run( JSMN_PARENT_LINKS_RUN_RESULT JSMN_PARENT_LINKS_COMPILE_RESULT
		"${CMAKE_BINARY_DIR}" "${JSMN_TEST_OUT_FILE}"
		CMAKE_FLAGS
			"-DINCLUDE_DIRECTORIES:STRING=${JSMN_INCLUDE_DIR}"
			"-DLINK_LIBRARIES:STRING=${JSMN_LIBRARIES}"
		RUN_OUTPUT_VARIABLE JSMN_DEFINES )
	string( STRIP "${JSMN_DEFINES}" JSMN_DEFINES )
	string( REPLACE "\n" ";" JSMN_DEFINES "${JSMN_DEFINES}" )
endif( JSMN_INCLUDE_DIR AND JSMN_LIBRARIES )

find_package_handle_standard_args( jsmn
	FOUND_VAR JSMN_FOUND
	REQUIRED_VARS JSMN_LIBRARIES JSMN_INCLUDE_DIR
	FAIL_MESSAGE DEFAULT_MSG
)

mark_as_advanced(
	JSMN_INCLUDE_DIR
	JSMN_LIBRARIES
)

