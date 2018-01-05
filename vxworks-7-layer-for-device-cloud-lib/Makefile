# Makefile - Wind River Device Cloud C Library Layer Makefile
#
# Copyright 2017 Wind River Systems, Inc.
#      
# The right to copy, distribute, modify or otherwise make use
# of this software may be licensed only pursuant to the terms
# of an applicable Wind River license agreement.
#
# modification history
# -------------------- 
# 05oct17,yat  created

PRE_BUILD_DIRS = lib

SHARED_PUBLIC_H_DIRS = h

BUILD_DIRS = lib device-manager actions complete location telemetry control relay software-update
BUILD_USER_DIRS = lib
POSTBUILD_RTP_DIRS = device-manager actions complete location telemetry control relay software-update

POST_NOBUILD_CDFDIRS = cdf
POST_NOBUILD_CFGDIRS = cfg

include $(WIND_KRNL_MK)/rules.layers.mk
