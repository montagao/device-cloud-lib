#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.

add_sample_app( location
	DISPLAY_NAME "Simple Location"
	DESCRIPTION "Simple application demonstrating location"
	README README.md
	SRCS location.c
	TARGET iot-app-simple-location
)

