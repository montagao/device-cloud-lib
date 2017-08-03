#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

find_package( Threads )
if ( MQTT_SSL_SUPPORT )
	find_package( OpenSSL )
endif()

set( API_HDRS_C "" CACHE INTERNAL "api header files" FORCE )
set( API_SRCS_C "" CACHE INTERNAL "api source files" FORCE )

set( API_HDRS_C ${API_HDRS_C}
	"iot_common.h"
	CACHE INTERNAL "" FORCE
)

set( API_SRCS_C ${API_SRCS_C}
	"iot_action.c"
	"iot_base.c"
	"iot_base64.c"
	"iot_common.c"
	"iot_location.c"
	"iot_mqtt.c"
	"iot_plugin.c"
	"iot_telemetry.c"
	CACHE INTERNAL "" FORCE
)

if ( IOT_API_MINIMUM_SUPPORTED_VERSION VERSION_LESS "2.1" )
	set( API_SRCS_C ${API_SRCS_C}
		"wra_command.c"
		"wra_common.c"
		"wra_location.c"
		"wra_metric.c"
		"wra_service.c"
		"wra_source.c"
		CACHE INTERNAL "" FORCE
	)
endif()

if ( IOT_API_MINIMUM_SUPPORTED_VERSION VERSION_LESS "2.0" )
	set( API_SRCS_C ${API_SRCS_C}
		"wra_compat.c"
		CACHE INTERNAL "" FORCE
	)
	# Version 1.0 compatibility is built into a source file called "wra_compat.c"
	# this file contains deprecated declarations within it to warn customers using
	# the old API.  However, this flag will cause the build the fail when using
	# GNU/Clang. This fix suppresses this warning for the offending files.
	set_source_files_properties( ${API_HDRS_CXX} ${API_SRCS_CXX} PROPERTIES LANGUAGE CXX )
	if ( CMAKE_C_COMPILER_ID MATCHES "(Clang|GNU)" )
		set_source_files_properties( "wra_compat.c" PROPERTIES COMPILE_FLAGS
			"-Wno-deprecated-declarations" )
	endif ( CMAKE_C_COMPILER_ID MATCHES "(Clang|GNU)" )
endif()

include_directories( SYSTEM "${JSON_INCLUDE_DIR}" "${MQTT_INCLUDE_DIR}" )
if( JSON_DEFINES )
	foreach( JSON_DEFINE ${JSON_DEFINES} )
		set( JSON_DEFINES_ "-D${JSON_DEFINE}=1" )
	endforeach( JSON_DEFINE )
	add_definitions( ${JSON_DEFINES_} )
endif( JSON_DEFINES )

set( LIB_OPTIONS "IOT_NO_THREAD_SUPPORT" "IOT_STACK_ONLY" )
foreach( LIB_OPTION ${LIB_OPTIONS} )
	if ( ${LIB_OPTION} )
		add_definitions( "-D${LIB_OPTION}" )
	endif( ${LIB_OPTION} )
endforeach( LIB_OPTION )
add_subdirectory( "json" )
add_subdirectory( "plugin" )
add_subdirectory( "public" )

set( IOT_API_LIBRARY_TYPE "SHARED" )
add_library( "${IOT_LIBRARY_NAME}"
	${IOT_API_LIBRARY_TYPE}
	${API_HDRS_C}
	${API_SRCS_C}
)
target_link_libraries( "${IOT_LIBRARY_NAME}" "dl"
	${JSON_LIBRARIES}
	${MQTT_LIBRARIES}
	${OSAL_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	${PLUGIN_BUILTIN_LIBS}
)

if ( MQTT_SSL_SUPPORT )
	target_link_libraries( "${IOT_LIBRARY_NAME}" ${OPENSSL_LIBRARIES} )
endif()

# Installation instructions
# LIBRARY DESTINATION (static library for Arduino: iot.lib, iot.a);
#                     (static library for Windows: iot.lib)
#                     (shared library for Non-Windows: iot.so)
# RUNTIME DESTINATION (shared library for Windows: iot.dll)
# ARCHIVE DESTINATION (not used)
set( LIB_COMPONENT "api" )
if ( WIN32 )
	set( LIB_COMPONENT "libs" )
endif( WIN32 )
install( TARGETS ${IOT_LIBRARY_NAME}
	RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT api
	ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT libs
	LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT ${LIB_COMPONENT}
	PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT headers
)

