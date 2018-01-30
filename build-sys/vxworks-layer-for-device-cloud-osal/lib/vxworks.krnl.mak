# Makefile - VxWorks 7 Makefile
#
# @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# @license Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software distributed
# under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
# OR CONDITIONS OF ANY KIND, either express or implied."
#
# Modification history
# --------------------
# 05oct17,yat  created

LIB_BASE_NAME = osal

BUILD_ALL_OBJS = TRUE

DEVICE_CLOUD_OSAL_DIR = $(VSB_DIR)/3pp/HDC_DEVICE_CLOUD_OSAL/device_cloud_osal_repo

SRC_DIRS = \
	$(DEVICE_CLOUD_OSAL_DIR)/src \
	$(DEVICE_CLOUD_OSAL_DIR)/src/uuid

SRC_FILES = \
	$(DEVICE_CLOUD_OSAL_DIR)/src/os.c \
	$(DEVICE_CLOUD_OSAL_DIR)/src/os_linux.c \
	$(DEVICE_CLOUD_OSAL_DIR)/src/os_posix.c \
	$(DEVICE_CLOUD_OSAL_DIR)/src/os_vxworks.c \
	$(DEVICE_CLOUD_OSAL_DIR)/src/uuid/gen_uuid.c \
        $(DEVICE_CLOUD_OSAL_DIR)/src/uuid/parse.c \
	$(DEVICE_CLOUD_OSAL_DIR)/src/uuid/unparse.c \
	$(DEVICE_CLOUD_OSAL_DIR)/src/uuid/pack.c \
	$(DEVICE_CLOUD_OSAL_DIR)/src/uuid/unpack.c \
	$(DEVICE_CLOUD_OSAL_DIR)/src/uuid/randutils.c

OBJS = $(addsuffix .o, $(notdir $(basename $(SRC_FILES))))

EXTRA_INCLUDE += -I. -Iuuid

ADDED_CFLAGS += $(C_COMPILER_FLAGS_C99)

EXTRA_DEFINE += -DOSAL_STATIC -DOSAL_WRAP=1 -DOSAL_THREAD_SUPPORT=1 \
                -DHAVE_USLEEP \
                -DHAVE_STDLIB_H -DHAVE_STRING_H -DHAVE_UNISTD_H \
                -DHAVE_SYS_TIME_H -DHAVE_GETTIMEOFDAY

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
