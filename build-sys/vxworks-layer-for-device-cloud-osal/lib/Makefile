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

include $(WIND_KRNL_MK)/defs.ver.mk

PKG_NAME := device_cloud_osal_repo
PKG_URL = https://github.com/Wind-River/device-cloud-osal.git $(PKG_NAME); cd $(PKG_NAME) && git checkout vxworks_layer_test && git checkout -b vxworks && git reset HEAD --hard
PKG_FILE_TYPE = git

PKG_BUILD_TARGETS := src
PKG_BUILD_DIR := $(PKG_NAME)/src
PKG_PATCH_DIR := $(PKG_NAME)

PKG_EXTRA_SRC :=

include $(WIND_KRNL_MK)/rules.unix.ports.mk
