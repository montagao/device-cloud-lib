#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

set( C_HDRS
	"iot_base64.h"
	"iot_defs.h"
	"iot_types.h"
)

if ( API_MINIMUM_SUPPORTED_VERSION VERSION_LESS "2.1" )
	set( C_HDRS ${C_HDRS}
		"wra_connection.h"
		"wra_internal.h"
	)
endif()

get_full_path( C_HDRS ${C_HDRS} )
set( API_HDRS_C ${API_HDRS_C} ${C_HDRS} CACHE INTERNAL "" FORCE )

