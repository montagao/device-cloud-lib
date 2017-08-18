#!/bin/bash
#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

# create iot-connect.schema.json.h and iot-connect.schema.json.c
# under "src/control/" based on upon the content from 
# "src/control/iot-connect.schema.json"
# refer to "build-sys/cmake/scripts/json_schema_to_c.cmake"
# for more information
#
pushd external/hdc/wr-iot-lib/src/control

CONFIG_FILE="./iot-connect.schema.json"
HEADER_FILE="./iot-connect.schema.json.h"
SOURCE_FILE="./iot-connect.schema.json.c"

if [ -f ${CONFIG_FILE} ]
then
	echo "#ifndef IOT_CONNECT_SCHEMA_JSON_H" > ${HEADER_FILE}
	echo "	extern const char *const IOT_CONNECT_SCHEMA;" >> ${HEADER_FILE}
	echo "#endif /* ifndef IOT_CONNECT_SCHEMA_JSON_H */" >> ${HEADER_FILE}

	echo "#include \"iot-connect.schema.json.h\" " > ${SOURCE_FILE}
	echo "" >> ${SOURCE_FILE}
	echo "const char *const IOT_CONNECT_SCHEMA =" >> ${SOURCE_FILE}
	while read -r line
	do
		new_line=$( echo ${line} | sed "s/\"/\\\\\"/g" )
		echo "\"${new_line}\"" >> ${SOURCE_FILE}
	done < ${CONFIG_FILE}
	echo ";" >> ${SOURCE_FILE}
else
	echo "Configration file doesn't exist!!!"
	exit 1
fi

popd

exit 0
