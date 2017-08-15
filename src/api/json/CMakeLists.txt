#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

set( C_HDRS ${C_HDRS}
	"iot_json_base.h"
	"iot_json_schema.h"
)

set( C_SRCS ${C_SRCS}
	"iot_json_base.c"
	"iot_json_decode.c"
	"iot_json_encode.c"
	"iot_json_schema.c"
)

get_full_path( C_HDRS ${C_HDRS} )
set( API_HDRS_C ${API_HDRS_C} ${C_HDRS} CACHE INTERNAL "" FORCE )

get_full_path( C_SRCS ${C_SRCS} )
set( API_SRCS_C ${API_SRCS_C} ${C_SRCS} CACHE INTERNAL "" FORCE )
