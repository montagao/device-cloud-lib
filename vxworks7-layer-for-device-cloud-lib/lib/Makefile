# Makefile - for Device Cloud C Library
#
# Copyright  2017 Wind River Systems, Inc.
#
# The right to copy, distribute, modify, or otherwise make use
# of this software may be licensed only pursuant to the terms
# of an applicable Wind River license agreement.
#
# Modification history
# --------------------
# 05oct17,yat  created

include $(WIND_KRNL_MK)/defs.ver.mk

PKG_NAME := device_cloud_lib_repo
PKG_URL = https://github.com/Wind-River/device-cloud-lib.git $(PKG_NAME); cd $(PKG_NAME) && git checkout vxworks_layer && git checkout -b vxworks && git reset HEAD --hard
PKG_FILE_TYPE = git

PKG_BUILD_TARGETS := src
PKG_BUILD_DIR := $(PKG_NAME)/src
PKG_PATCH_DIR := $(PKG_NAME)

PKG_EXTRA_SRC := iot_build.h iot_plugin_builtin.c iot-connect.schema.json.h iot-connect.schema.json.c

KERNEL_PUBLIC_H_FILES = api/public/iot.h \
                        api/public/iot_checksum.h \
                        api/public/iot_json.h \
                        api/public/iot_mqtt.h \
                        api/public/iot_plugin.h
USER_PUBLIC_H_FILES =   api/public/iot.h \
                        api/public/iot_checksum.h \
                        api/public/iot_json.h \
                        api/public/iot_mqtt.h \
                        api/public/iot_plugin.h

include $(WIND_KRNL_MK)/rules.unix.ports.mk
