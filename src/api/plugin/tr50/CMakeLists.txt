#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

set( TARGET "tr50" )
set( TARGET_DESCRIPTION "${TARGET} api plugin" )

add_iot_plugin( "${TARGET}" BUILTIN ENABLED
	tr50.c
)
