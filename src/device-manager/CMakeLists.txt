#
# Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

set( TARGET "${IOT_DEVICE_MANAGER_TARGET}" )
set( TARGET_DESCRIPTION "${IOT_DEVICE_MANAGER_DESCRIPTION}" )

set( IOT_DEVICE_MANAGER_DEST_COMPONENT "core" )
find_package( Osal REQUIRED )
find_package( Jsmn REQUIRED )

#add_subdirectory( "scripts" )

#if ( WIN32 )
	#set( PROJECT_ICON ${PROJECT_BUILD_WIN_PATH}/iot.ico )

	#configure_file(
		#${PROJECT_BUILD_WIN_PATH}/version.rc.in
		#${CMAKE_CURRENT_BINARY_DIR}/version.rc
		#@ONLY )
	#list( APPEND IOT_VERSION_FILES ${CMAKE_CURRENT_BINARY_DIR}/version.rc )
#endif()

set( IOT_HDRS_C ${IOT_HDRS_C}
	"device_manager_main.h"
	"device_manager_file.h"
	"device_manager_md5.h"
	"device_manager_sha256.h"
	"device_manager_ota.h"
	"app_config.h"
	"app_json.h"
)

set( IOT_SRCS_C ${IOT_SRCS_C}
	"device_manager.c"
	"device_manager_main.c"
	"device_manager_ota.c"
	"app_config.c"
	"app_json.c"
)

add_executable( ${TARGET}
	${IOT_HDRS_C}
	${IOT_HDRS_CXX}
	${IOT_SRCS_C}
	${IOT_SRCS_CXX}
	${IOT_VERSION_FILES}
)
include_directories( SYSTEM
        ${OSAL_INCLUDE_DIR}
        "${CMAKE_SOURCE_DIR}/src/api"
        "${CMAKE_SOURCE_DIR}/src/utilities"
	${JSMN_INCLUDE_DIR}


)

target_link_libraries( ${TARGET}
    	iotutils
	iot
        ${OSAL_LIBRARIES}
	${JSMN_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        dl
)
install( TARGETS ${TARGET}
	RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
		COMPONENT ${IOT_DEVICE_MANAGER_DEST_COMPONENT}
)

# create directories for file-io

if ( NOT WIN32 )
	# header file path for linux scripts name, aka .sh
	#include_directories( SYSTEM
		#"${CMAKE_SOURCE_DIR}/src/device-manager/scripts/linux"
	#)
	file( MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/upload" )
	install( DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/upload"
		DESTINATION "${INSTALL_VAR_DIR}"
		COMPONENT ${IOT_DEVICE_MANAGER_DEST_COMPONENT}
		DIRECTORY_PERMISSIONS
			OWNER_READ  OWNER_WRITE OWNER_EXECUTE
			GROUP_READ              GROUP_EXECUTE
			WORLD_READ              WORLD_EXECUTE
		)
	file( MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/download" )
	install( DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/download"
		DESTINATION "${INSTALL_VAR_DIR}"
		COMPONENT ${IOT_DEVICE_MANAGER_DEST_COMPONENT}
		DIRECTORY_PERMISSIONS
			OWNER_READ OWNER_WRITE OWNER_EXECUTE
			GROUP_READ             GROUP_EXECUTE
			WORLD_READ             WORLD_EXECUTE
		)
endif ()

