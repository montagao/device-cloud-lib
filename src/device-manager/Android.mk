#
# Copyright 2013 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/scripts/linux/ \
    $(LOCAL_PATH)/../ \
    $(LOCAL_PATH)/../../ \
    $(LOCAL_PATH)/../../build-sys/android/ \
    external/e2fsprogs/lib \
    external/hdc/libarchive/contrib/android/include \

LOCAL_SHARED_LIBRARIES := libcutils libdl libjansson libmosquitto libiot libext2_uuid libcurl libz
LOCAL_STATIC_LIBRARIES := libapp-common libapp-json libandroidifaddrs libarchive

LOCAL_MODULE := iot-device-manager

LOCAL_SRC_FILES := \
    ./file-io/device_manager_md5.c \
    ./file-io/device_manager_sha256.c \
    ./file-io/device_manager_file.c \
    ./ota/device_manager_ota.c \
    ./device_manager.c \
    ./device_manager_main.c \

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := buidinfo_hdc_android
LOCAL_SRC_FILES := scripts/linux/actions/buildinfo_hdc_android
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/iot
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := config_hdc_android.sh
LOCAL_SRC_FILES := scripts/linux/actions/config_hdc_android.sh
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := wr_restore_factory_images_android.sh
LOCAL_SRC_FILES := scripts/linux/actions/wr_restore_factory_images_android.sh
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
include $(BUILD_PREBUILT)
