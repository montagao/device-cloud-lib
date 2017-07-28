#
# Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

set( TARGET "app-common" )

set( IOT_HDRS_C ${IOT_HDRS_C}
	"app_arg.h"
)

set( IOT_SRCS_C ${IOT_SRCS_C}
	"app_arg.c"
)

add_definitions( ${IOT_DEFS} )

option( IOT_LOG_TIMESTAMP "Option to add time stamps in the log" OFF )
if( IOT_LOG_TIMESTAMP )
	add_definitions( "-DIOT_LOG_TIMESTAMP" )
endif( IOT_LOG_TIMESTAMP )

add_library( ${TARGET} STATIC
	${IOT_HDRS_C}
	${IOT_HDRS_CXX}
	${IOT_SRCS_C}
	${IOT_SRCS_CXX}
)

if( MSVC )
	# Compile MSVC runtime libraries into the output binary
	target_compile_options( "${TARGET}"
		PRIVATE "/MT$<$<CONFIG:Debug>:d>" )
endif( MSVC )

include_directories( SYSTEM
	${JSMN_INCLUDE_DIR}
	${IOT_INCS}
	"${CMAKE_SOURCE_DIR}/src/api"
	"${CMAKE_SOURCE_DIR}/src/app-common"
)

#target_link_libraries( ${TARGET}
	#"jsmn"
#)

