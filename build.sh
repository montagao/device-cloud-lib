#!/usr/bin/env bash
#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

CFG_FILE="build.yml"
SRC_DIR=$(pwd)
DEST_DIR="."
PREFIX="IOT"
IN_EXT=".in"
OUT_FILES=("iot_build.h" "src/api/plugin/iot_plugin_builtin.c")

# Determine GIT SHA
GIT_PATH=`which git`
GIT_FLAGS="log -1 --format=%H"
if [ -n "${GIT_PATH}" ]; then
	export ${PREFIX}_GIT_SHA=`${GIT_PATH} ${GIT_FLAGS} 2>/dev/null`
fi

# read yaml file
# derived from https://gist.github.com/epiloque/8cf512c6d64641bde388
# works for arrays of hashes, as long as the hashes do not have arrays
parse_yaml() {
	local prefix=$2
	local s
	local w
	local fs
	s='[[:space:]]*'
	w='[a-zA-Z0-9_]*'
	fs="$(echo @|tr @ '\034')"
	sed -n -e "s|^\($s\)\($w\)$s:$s\"\(.*\)\"$s\$|\1$fs\2$fs\3|p" \
		-e "s|^\($s\)\($w\)$s[:-]$s\(.*\)$s\$|\1$fs\2$fs\3|p" "$1" |
	awk -F"$fs" '{
		indent = length($1)/2;
		if (length($2) == 0) { conj[indent]="+";} else {conj[indent]="";}
		vname[indent] = $2;
		for (i in vname) {if (i > indent) {delete vname[i]}}
		if (length($3) > 0) {
			vn=""; for (i=0; i<indent; i++) {vn=(vn)(vname[i])("_")}
			printf("%s%s%s%s=(\"%s\")\n", "'"$prefix"'",vn, $2, conj[indent-1],$3);
		}
	}' | sed 's/_=/+=/g'
}

# parse config file and save into variables
eval $(parse_yaml "${CFG_FILE}" "")

# set variables for generating built-in plugin support
for i in ${!IOT_PLUGIN_BUILTIN[@]}; do
	PLUGIN_BUILTIN_NAME=$(echo "${IOT_PLUGIN_BUILTIN[${i}]}" | awk -F ":" '{print $1}' | tr -d '[:space:]')
	PLUGIN_BUILTIN_ENABLE=$(echo "${IOT_PLUGIN_BUILTIN[${i}]}" | awk -F ":" '{print $2}' | tr -d '[:space:]')
	case ${PLUGIN_BUILTIN_ENABLE} in
		no | No | NO | false | False | FALSE | off | Off | OFF)
			PLUGIN_BUILTIN_ENABLE=false;;
		*)
			PLUGIN_BUILTIN_ENABLE=true
	esac

	IOT_PLUGIN_BUILTIN_INCS="${IOT_PLUGIN_BUILTIN_INCS}
/**
 * @brief internal function to load the ${PLUGIN_BUILTIN_NAME} plug-in
 * @param[out]       p                   location to load plug-in to
 * @retval           IOT_TRUE            successfully loaded plug-in
 * @retval           IOT_FALSE           failed to load plug-in
 */
iot_bool_t ${PLUGIN_BUILTIN_NAME}_load( iot_plugin_t *p );"

	IOT_PLUGIN_BUILTIN_IMPL="${IOT_PLUGIN_BUILTIN_IMPL}\n
		/* ${PLUGIN_BUILTIN_NAME} */
		if ( (lib->plugin_count + result < max) \\\\&\\\\& ${PLUGIN_BUILTIN_NAME}_load( lib->plugin_ptr[lib->plugin_count + result] ) ) { ++result; }"

	if [ "${PLUGIN_BUILTIN_ENABLE}" = true ]; then
		IOT_PLUGIN_BUILTIN_ENABLE="${IOT_PLUGIN_BUILTIN_ENABLE}
		/* ${PLUGIN_BUILTIN_NAME} */
		if ( iot_plugin_enable( lib, \"${PLUGIN_BUILTIN_NAME}\" ) != IOT_STATUS_SUCCESS ) result = IOT_FALSE;"
	fi
done

# For each configuration file replace any matching variables
for i in ${OUT_FILES[@]}; do
	# Replace matching variables from in-file
	cp "${SRC_DIR}/${i}${IN_EXT}" "${DEST_DIR}/${i}"
	set | awk -F "=" '{print $1}' | grep "${PREFIX}_" | while read -r var; do
		awk -v value="${!var}" "{ sub(/@${var}@/, value); sub(/\${${var}}/, value); print; }" "${DEST_DIR}/${i}" > "${DEST_DIR}/${i}.tmp"
		mv "${DEST_DIR}/${i}.tmp" "${DEST_DIR}/${i}"
	done
	# Replace any remaining variables with blanks
	sed -e "s|@CMAKE_SOURCE_DIR@|${SRC_DIR}|g" -e "s|\${CMAKE_SOURCE_DIR}}|${SRC_DIR}|g" -i "${DEST_DIR}/${i}"
	sed -e "s|@CMAKE_BINARY_DIR@|${DEST_DIR}|g" -e "s|\${CMAKE_BINARY_DIR}}|${DEST_DIR}|g" -i "${DEST_DIR}/${i}"
	sed -e "s|@[a-zA-Z0-9_]*@||g" -e "s|\${[a-zA-Z0-9_]*}||g" -i "${DEST_DIR}/${i}"
done

exit 0
