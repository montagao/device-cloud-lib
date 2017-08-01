#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.

add_sample_app( old-api
	DISPLAY_NAME "Older API example"
	DESCRIPTION "Application demonstrating using the old api"
	README README.md
	SCRIPTS_UNIX old_api_script.sh
	SCRIPTS_WIN32 old_api_script.cmd
	SRCS old_api.c
	TARGET iot-app-old-api
)

