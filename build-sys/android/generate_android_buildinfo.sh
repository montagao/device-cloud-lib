#!/bin/bash
#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

# collect hdc build infomation
pushd external/hdc

BUILD_INFO_FILE="wr-iot-lib/build-sys/android/buildinfo_hdc_android"
touch ${BUILD_INFO_FILE}
BUILD_INFO_FILE_ABS_PATH=$( readlink -e ${BUILD_INFO_FILE} )

if [ ! -z ${BUILD_INFO_FILE_ABS_PATH} ]
then
	echo "USER: ${USER}" > ${BUILD_INFO_FILE_ABS_PATH}
	echo "BUILD_SERVER: $(uname -a)" >> ${BUILD_INFO_FILE_ABS_PATH}
	echo "BUILD_TIME: $(date -d now +'%F %H-%M-%S %A week %W %Z %::z')" >> ${BUILD_INFO_FILE_ABS_PATH}
	for i in *
	do
		echo "------------------" >> ${BUILD_INFO_FILE_ABS_PATH}
		echo "repo: ${i}" >> ${BUILD_INFO_FILE_ABS_PATH}
		pushd ${i}
		echo "git describe --tags --abbrev=0" >> ${BUILD_INFO_FILE_ABS_PATH}
		git describe --tags --abbrev=0 >> ${BUILD_INFO_FILE_ABS_PATH}
		echo "git log -1" >> ${BUILD_INFO_FILE_ABS_PATH}
		git log -1 >> ${BUILD_INFO_FILE_ABS_PATH}
		echo "git status" >> ${BUILD_INFO_FILE_ABS_PATH}
		git status >> ${BUILD_INFO_FILE_ABS_PATH}
		popd
		echo "" >> ${BUILD_INFO_FILE_ABS_PATH}
	done
fi
popd # external/hdc

exit 0
