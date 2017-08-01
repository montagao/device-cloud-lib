#
# Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

find_program( GIT_EXECUTABLE git NO_CMAKE_FIND_ROOT_PATH )
mark_as_advanced( GIT_EXECUTABLE )
if( GIT_EXECUTABLE )
	execute_process( WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE PROJECT_GIT_SHA COMMAND ${GIT_EXECUTABLE} log -1 --format="%H" )
	string( REGEX REPLACE "\n" "" PROJECT_GIT_SHA ${PROJECT_GIT_SHA} )
	string( REGEX REPLACE "\"" "" PROJECT_GIT_SHA ${PROJECT_GIT_SHA} )
endif( GIT_EXECUTABLE )

