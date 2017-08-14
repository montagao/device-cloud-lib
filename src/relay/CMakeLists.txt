#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

set( TARGET "${IOT_RELAY_TARGET}" )
set( TARGET_DESCRIPTION "${IOT_RELAY_DESCRIPTION}" )

find_package( LibWebSockets REQUIRED )
find_package( OpenSSL )
find_package( Threads )

set( IOT_HDRS_C ${IOT_HDRS_C}
	"relay_main.h"
)

set( IOT_SRCS_C ${IOT_SRCS_C}
	"relay.c"
	"relay_main.c"
)

add_executable( ${TARGET}
	${IOT_HDRS_C}
	${IOT_HDRS_CXX}
	${IOT_SRCS_C}
	${IOT_SRCS_CXX}
	${IOT_VERSION_FILES}
)

target_link_libraries( ${TARGET}
        "iotutils"
	${LIBWEBSOCKETS_LIBRARIES}
	${OPENSSL_LIBRARIES}
	${OSAL_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	dl
)

include_directories( SYSTEM
	${OSAL_INCLUDE_DIR}
	${LIBWEBSOCKETS_INCLUDE_DIR}
	${OPENSSL_INCLUDE_DIR}
	"${CMAKE_SOURCE_DIR}/src/api"
	"${CMAKE_SOURCE_DIR}/src/utilities"

)

# Installation instructions
install( TARGETS ${TARGET}
	RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT core
)
