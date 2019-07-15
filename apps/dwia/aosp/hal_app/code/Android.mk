# Copyright (c) 2015 Intel Corporation
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

# Ultra Wide Band (UWB) virtual sensor HAL module implementation, 
# compiled as hw/uwb-virtual-sensor-hal.so

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

src_path := .
src_files := $(src_path)/entry.c

LOCAL_C_INCLUDES += $(LOCAL_PATH) $(LOCAL_PATH)/linux $(LOCAL_PATH)/../../../hardware/libhardware/include/
LOCAL_MODULE := uwb-virtual-sensor-hal
LOCAL_MODULE_OWNER := pathpartner
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := -DLOG_TAG=\"Sensors\" -fvisibility=hidden
LOCAL_LDFLAGS := -Wl,--gc-sections
LOCAL_SHARED_LIBRARIES := liblog libcutils libdl
LOCAL_PRELINK_MODULE := false
LOCAL_SRC_FILES := $(src_files)
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(LOCAL_PATH) $(LOCAL_PATH)/linux $(LOCAL_PATH)/../../../hardware/libhardware/include/
LOCAL_MODULE := uwbvirtualsens
LOCAL_CFLAGS := -DLOG_TAG=\"Sensors\" -fvisibility=hidden
LOCAL_SHARED_LIBRARIES := liblog libcutils libdl
LOCAL_SRC_FILES := uwbvirtualsens.c
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)
include $(BUILD_EXECUTABLE)

