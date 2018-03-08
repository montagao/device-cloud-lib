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

ifeq ($(SPACE), user)
#EXE = iot-app-complete
ADDED_LIBS += -liot -losal -lnet -larchive -lHASH -lOPENSSL -lmosquitto -ljson -lzlib -lcurl -lunix
else
#LIB_BASE_NAME = deviceCloudComplete
endif

BUILD_ALL_OBJS = TRUE

DEVICE_CLOUD_LIB_DIR = $(VSB_DIR)/3pp/HDC_DEVICE_CLOUD_LIB/device_cloud_lib_repo

SRC_DIRS = \
        $(DEVICE_CLOUD_LIB_DIR)/share/apps/complete

SRC_FILES = \
        $(DEVICE_CLOUD_LIB_DIR)/share/apps/complete/complete.c

OBJS = $(addsuffix .o, $(notdir $(basename $(SRC_FILES))))

EXTRA_INCLUDE +=

ADDED_CFLAGS += $(C_COMPILER_FLAGS_C99)

EXTRA_DEFINE += -DOSAL_STATIC -DOSAL_WRAP=1 -DIOT_THREAD_SUPPORT=1 \
                -DOPENSSL \
                -DIOT_PLUGIN_BUILTIN \
                -DIOT_MQTT_MOSQUITTO \
                -DIOT_JSON_JANSSON

VPATH = $(realpath $(SRC_DIRS))

ifeq ($(SPACE), user)
include $(WIND_USR_MK)/rules.rtp.mk
else
include $(WIND_KRNL_MK)/rules.library.mk
endif

#
# This code is 3rd party. Suppress all compiler warnings.
#
CC_WARNINGS = $(CC_WARNINGS_NONE)
