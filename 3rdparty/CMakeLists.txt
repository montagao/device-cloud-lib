#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

cmake_minimum_required( VERSION 2.8.5 )

include( ExternalProject )
set( GIT_PATCH_FLAGS "--ignore-space-change" "--ignore-whitespace" "-p1"
	"--whitespace=nowarn" "--unsafe-paths"
)
build_command( CMAKE_BUILD_COMMAND )
separate_arguments( CMAKE_BUILD_COMMAND )

# jsmn
set( JSMN_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/jsmn" )
set( JSMN_INSTALL_DIR "${JSMN_BUILD_DIR}/out" )
ExternalProject_Add( "jsmn"
	GIT_REPOSITORY         "https://github.com/zserge/jsmn.git"
	GIT_TAG                "master"
	PREFIX                 "${JSMN_BUILD_DIR}"
	CONFIGURE_COMMAND      "make" "clean"
	BUILD_COMMAND          "make" "CFLAGS=-DJSMN_PARENT_LINKS=1 -DJSMN_STRICT=1 -fPIC"
	BUILD_IN_SOURCE        1
	INSTALL_DIR            "${JSMN_INSTALL_DIR}" # creates empty directory
	INSTALL_COMMAND        "${CMAKE_COMMAND}" "-E" "copy" "${JSMN_BUILD_DIR}/src/jsmn/libjsmn.a" "${JSMN_INSTALL_DIR}"
	        COMMAND        "${CMAKE_COMMAND}" "-E" "copy" "${JSMN_BUILD_DIR}/src/jsmn/jsmn.h" "${JSMN_INSTALL_DIR}"
	TEST_BEFORE_INSTALL    1
	TEST_COMMAND           "make" "test"
)

# paho
set( PAHO_BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/paho" )
set( PAHO_BINARY_DIR "${PAHO_BASE_DIR}/src/paho-build" )
set( PAHO_INSTALL_DIR  "${PAHO_BASE_DIR}/out" )
set( PAHO_SOURCE_DIR "${PAHO_BASE_DIR}/src/paho" )
string( REPLACE ";.;" ";${PAHO_SOURCE_DIR};" PAHO_BUILD_COMMAND "${CMAKE_BUILD_COMMAND}" )
ExternalProject_Add( "paho"
	GIT_REPOSITORY         "https://github.com/eclipse/paho.mqtt.c.git"
	GIT_TAG                "master" #"v1.1.0"
	PREFIX                 "${PAHO_BASE_DIR}"
	CMAKE_ARGS             "-DCMAKE_INSTALL_PREFIX:PATH=${PAHO_INSTALL_DIR}"
	                       "-DPAHO_WITH_SSL:BOOL=TRUE"
	                       "-DPAHO_BUILD_STATIC:BOOL=TRUE"
	                       "-DCMAKE_C_FLAGS:STRING=-fPIC"
	#BINARY_DIR             "${PAHO_BINARY_DIR}"
	BUILD_COMMAND          ${PAHO_BUILD_COMMAND}
	BUILD_IN_SOURCE        1
	INSTALL_DIR            "${PAHO_INSTALL_DIR}" # creates empty directory
	INSTALL_COMMAND        ${CMAKE_MAKE_PROGRAM} "install"
	#TEST_BEFORE_INSTALL    1
	#TEST_COMMAND            ${CMAKE_MAKE_PROGRAM} "test"
)

set( OSAL_BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/osal" )
set( OSAL_BINARY_DIR "${OSAL_BASE_DIR}/src/osal-build" )
set( OSAL_INSTALL_DIR  "${OSAL_BASE_DIR}/out" )
set( OSAL_SOURCE_DIR "${OSAL_BASE_DIR}/src/osal" )
string( REPLACE ";.;" ";${OSAL_SOURCE_DIR};" OSAL_BUILD_COMMAND "${CMAKE_BUILD_COMMAND}" )
ExternalProject_Add( "osal"
	GIT_REPOSITORY         "http://stash.wrs.com/scm/hpr/wr-osal.git"
	GIT_TAG                "master"
	PREFIX                 "${OSAL_BASE_DIR}"
	CMAKE_ARGS             "-DCMAKE_INSTALL_PREFIX:PATH=${OSAL_INSTALL_DIR}"
	#BINARY_DIR             "${OSAL_BINARY_DIR}"
	BUILD_COMMAND          ${OSAL_BUILD_COMMAND}
	BUILD_IN_SOURCE        1
	INSTALL_DIR            "${OSAL_INSTALL_DIR}" # creates empty directory
	INSTALL_COMMAND        ${CMAKE_MAKE_PROGRAM} "install"
	#TEST_BEFORE_INSTALL    1
	#TEST_COMMAND            ${CMAKE_MAKE_PROGRAM} "test"
)

