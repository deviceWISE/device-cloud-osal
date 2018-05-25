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

$( info ($(shell (cd ${LOCAL_PATH}; ./build-sys/generate_header.sh -DOSAL_WRAP=1 -DOSAL_THREAD_SUPPORT=1 > os.h ))))

osal_c_includes := \
    external/e2fsprogs/lib

osal_src_files := \
    src/os_android.c \
    src/os_linux.c \
    src/os_posix.c \
    src/os.c

include $(CLEAR_VARS)
LOCAL_CFLAGS := -DOSAL_WRAP=1 -DOSAL_THREAD_SUPPORT=1
LOCAL_C_INCLUDES := $(osal_c_includes)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_SHARED_LIBRARIES := libdl libext2_uuid
LOCAL_STATIC_LIBRARIES := libandroidifaddrs
LOCAL_MODULE := libosal
LOCAL_SRC_FILES := $(osal_src_files)
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CFLAGS := -DOSAL_WRAP=1 -DOSAL_THREAD_SUPPORT=1
LOCAL_C_INCLUDES := $(osal_c_includes)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_STATIC_LIBRARIES := libandroidifaddrs
LOCAL_MODULE := libosal
LOCAL_SRC_FILES := $(osal_src_files)
include $(BUILD_STATIC_LIBRARY)
