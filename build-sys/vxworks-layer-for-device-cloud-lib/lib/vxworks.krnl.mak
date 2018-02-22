# Makefile - VxWorks 7 Makefile
#
# Copyright (c) 2017 Wind River Systems, Inc.
#
# The right to copy, distribute, modify, or otherwise make use
# of this software may be licensed only pursuant to the terms
# of an applicable Wind River license agreement.
#
# Modification history
# --------------------
# 05oct17,yat  created

LIB_BASE_NAME = iot

BUILD_ALL_OBJS = TRUE

DEVICE_CLOUD_LIB_DIR = $(VSB_DIR)/3pp/HDC_DEVICE_CLOUD_LIB/device_cloud_lib_repo

SRC_DIRS = \
        $(DEVICE_CLOUD_LIB_DIR) \
        $(DEVICE_CLOUD_LIB_DIR)/src/vxworks \
        $(DEVICE_CLOUD_LIB_DIR)/src/api \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/json \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/checksum \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/plugin/tr50 \
        $(DEVICE_CLOUD_LIB_DIR)/src/utilities \
        $(DEVICE_CLOUD_LIB_DIR)/src/software-update

SRC_FILES = \
        $(DEVICE_CLOUD_LIB_DIR)/src/vxworks/iot_plugin_builtin.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_action.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_alarm.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_attribute.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_base64.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_base.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_common.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_event.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_file.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_location.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_mqtt.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_option.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_plugin.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_telemetry.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/json/iot_json_base.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/json/iot_json_decode.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/json/iot_json_encode.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/json/iot_json_schema.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/checksum/iot_checksum.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/checksum/iot_checksum_crc32.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/plugin/tr50/tr50.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/utilities/app_arg.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/utilities/app_path.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/utilities/app_log.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/software-update/iot_update.c

OBJS = $(addsuffix .o, $(notdir $(basename $(SRC_FILES))))

EXTRA_INCLUDE += -I$(DEVICE_CLOUD_LIB_DIR) \
                 -I$(DEVICE_CLOUD_LIB_DIR)/src \
                 -I$(DEVICE_CLOUD_LIB_DIR)/src/api \
                 -I$(DEVICE_CLOUD_LIB_DIR)/src/api/public \
                 -I$(DEVICE_CLOUD_LIB_DIR)/src/utilities

ADDED_CFLAGS += $(C_COMPILER_FLAGS_C99)

EXTRA_DEFINE += -DOSAL_STATIC -DOSAL_WRAP=1 -DIOT_THREAD_SUPPORT=1 \
                -DOPENSSL \
                -DIOT_PLUGIN_BUILTIN \
                -DIOT_MQTT_MOSQUITTO \
                -DIOT_JSON_JANSSON

VPATH = $(realpath $(SRC_DIRS))

ifeq ($(SPACE), user)
include $(WIND_USR_MK)/rules.library.mk
else
include $(WIND_KRNL_MK)/rules.library.mk
endif

#
# This code is 3rd party. Suppress all compiler warnings.
#
CC_WARNINGS = $(CC_WARNINGS_NONE)
