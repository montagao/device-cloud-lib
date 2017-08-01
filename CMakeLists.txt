#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-sys/cmake/modules" ${CMAKE_MODULE_PATH} )
set( IOT_CONFIG_FILE "build.yml" )
include ( ReadYamlFile )
read_yaml_file( ${IOT_CONFIG_FILE} )

cmake_minimum_required( VERSION 2.8.5 )
project( ${IOT_PRODUCT} )

# Project information
set( PROJECT_NAME                      "${CMAKE_PROJECT_NAME}" )
set( PROJECT_VERSION                   "${IOT_VERSION}" )
set( PROJECT_VENDOR                    "${IOT_VENDOR}" )

# Options
include( DefineCompilerFlags )
include( GetFullPath )
include( GitSha )
include( OptionSelect )
option_select( IOT_JSON_LIBRARY
	DESCRIPTION "JSON Library to use"
	DEFAULT "jsmn" "jansson" "jsmn"
)
option_select( IOT_MQTT_LIBRARY
	DESCRIPTION "MQTT Library to use"
	DEFAULT "paho" "mosquitto" "paho"
)
option( IOT_NO_THREAD_SUPPORT "do not support the use of threads" OFF )
option( IOT_STACK_ONLY "build library without the use of the heap" OFF )

# Base build configuration file
set( IOT_IN_FILE  "iot_build.h.in" )
set( IOT_OUT_FILE "iot_build.h" )
set( IOT_GIT_SHA  "${PROJECT_GIT_SHA}" )
configure_file( "${IOT_IN_FILE}" "${CMAKE_BINARY_DIR}/${IOT_OUT_FILE}" @ONLY )

# Setup Project output directories
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/lib" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/lib" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/bin" )
# Location to find public include files when building sample applications
set( CMAKE_INCLUDE_OUTPUT_DIRECTORY    "${CMAKE_SOURCE_DIR}/src/api/public" )

# Installation directories
set( INSTALL_BIN_DIR      "bin" )
set( INSTALL_EXAMPLES_DIR "share/iot" )
set( INSTALL_INCLUDE_DIR  "include" )
set( INSTALL_LIB_DIR      "lib" )
set( INSTALL_PLUGIN_DIR   "lib" )

# 3rd party libraries
if ( IOT_JSON_LIBRARY STREQUAL "jansson" )
	find_package( Jansson REQUIRED )
	add_definitions( "-DIOT_JSON_JANSSON" )
	set( JSON_INCLUDE_DIR "${JANSSON_INCLUDE_DIR}" )
	set( JSON_LIBRARIES   "${JANSSON_LIBRARIES}" )
else()
	find_package( Jsmn REQUIRED )
	set( JSON_INCLUDE_DIR "${JSMN_INCLUDE_DIR}" )
	set( JSON_LIBRARIES   "${JSMN_LIBRARIES}" )
	set( JSON_DEFS        "-DJSMN_PARENT_LINKS -DJSMN_STRICT" )
endif()

if ( IOT_MQTT_LIBRARY STREQUAL "mosquitto" )
	find_package( Mosquitto REQUIRED )
	add_definitions( "-DIOT_MQTT_MOSQUITTO" )
	set( MQTT_INCLUDE_DIR "${MOSQUITTO_INCLUDE_DIR}" )
	set( MQTT_LIBRARIES "${MOSQUITTO_LIBRARIES}" )
else()
	find_package( Paho REQUIRED )
	set( MQTT_INCLUDE_DIR "${PAHO_INCLUDE_DIR}" )
	set( MQTT_LIBRARIES "${PAHO_LIBRARIES}" )
endif()

find_package( Osal REQUIRED )

if( NOT OSAL_INCLUDE_DIR OR NOT MQTT_LIBRARIES OR NOT JSON_LIBRARIES )
	set( 3RDPARTY_DIR "${CMAKE_CURRENT_BINARY_DIR}/3rdparty" )
	file( MAKE_DIRECTORY "${3RDPARTY_DIR}" )
	execute_process( COMMAND "${CMAKE_COMMAND}" "-G${CMAKE_GENERATOR}"
			"${CMAKE_CURRENT_SOURCE_DIR}/3rdparty"
		WORKING_DIRECTORY "${3RDPARTY_DIR}"
		RESULT_VARIABLE CMD_RESULT
		OUTPUT_VARIABLE CMD_OUTPUT
		ERROR_VARIABLE CMD_ERROR )
	if( NOT CMD_RESULT STREQUAL "0" )
		message( FATAL_ERROR "Failed to obtain 3rd party libraries: ${CMD_ERROR}" )
	endif( NOT CMD_RESULT STREQUAL "0" )
	execute_process( COMMAND "make"
		WORKING_DIRECTORY "${3RDPARTY_DIR}"
		RESULT_VARIABLE CMD_RESULT )
	if( NOT CMD_RESULT STREQUAL "0" )
		message( FATAL_ERROR "Failed to build 3rd party libraries: ${CMD_ERROR}" )
	endif( NOT CMD_RESULT STREQUAL "0" )

	# jsmn
	set( JSMN_DIR "${3RDPARTY_DIR}/jsmn/out" )
	find_path( JSMN_INCLUDE_DIR NAMES jsmn.h
		PATHS "${JSMN_DIR}" )
	find_library( JSMN_LIBRARIES NAMES
		${CMAKE_STATIC_LIBRARY_PREFIX}jsmn${CMAKE_STATIC_LIBRARY_SUFFIX}
		PATHS "${JSMN_DIR}" )
	set( JSON_INCLUDE_DIR "${JSMN_INCLUDE_DIR}" )
	set( JSON_LIBRARIES   "${JSMN_LIBRARIES}" )
	set( JSON_DEFS        "-DJSMN_PARENT_LINKS" )

	# osal
	set( OSAL_DIR "${3RDPARTY_DIR}/osal/out" )
	find_path( OSAL_INCLUDE_DIR NAMES os.h
		PATHS "${OSAL_DIR}/include" )
	find_library( OSAL_LIBRARIES NAMES
		${CMAKE_STATIC_LIBRARY_PREFIX}osal${CMAKE_STATIC_LIBRARY_SUFFIX}
		${CMAKE_SHARED_LIBRARY_PREFIX}osal${CMAKE_SHARED_LIBRARY_SUFFIX}
		PATHS "${OSAL_DIR}/lib64" )

	# paho
	set( PAHO_DIR "${3RDPARTY_DIR}/paho/out" )
	find_path( PAHO_INCLUDE_DIR NAMES MQTTClient.h
		PATHS "${PAHO_DIR}/include" )
	find_library( PAHO_LIBRARIES NAMES
		"${CMAKE_STATIC_LIBRARY_PREFIX}paho-mqtt3cs-static${CMAKE_STATIC_LIBRARY_SUFFIX}"
		"${CMAKE_SHARED_LIBRARY_PREFIX}paho-mqtt3cs${CMAKE_SHARED_LIBRARY_SUFFIX}"
		PATHS "${PAHO_DIR}/lib64" )
	set( MQTT_INCLUDE_DIR "${PAHO_INCLUDE_DIR}" )
	set( MQTT_LIBRARIES "${PAHO_LIBRARIES}" )
endif( NOT OSAL_INCLUDE_DIR OR NOT MQTT_LIBRARIES OR NOT JSON_LIBRARIES )

set( CMAKE_POSITION_INDEPENDENT_CODE ON )
include_directories( "src/api/public" "${OSAL_INCLUDE_DIR}" "${CMAKE_BINARY_DIR}" )
add_subdirectory( "src" )
add_subdirectory( "share" )

include( TestSupport )
