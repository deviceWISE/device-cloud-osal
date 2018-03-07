# Makefile - VxWorks 7 Layer Makefile
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

include $(WIND_KRNL_MK)/defs.ver.mk

PKG_NAME := device_cloud_osal_repo
PKG_URL = https://github.com/Wind-River/device-cloud-osal.git $(PKG_NAME); cd $(PKG_NAME) && git checkout vxworks_layer && git checkout -b vxworks && git reset HEAD --hard
PKG_FILE_TYPE = git

PKG_BUILD_TARGETS := src
PKG_BUILD_DIR := $(PKG_NAME)/src
PKG_PATCH_DIR := $(PKG_NAME)

KERNEL_PUBLIC_H_FILES = vxworks/os.h
USER_PUBLIC_H_FILES = vxworks/os.h

PKG_EXTRA_SRC :=

include $(WIND_KRNL_MK)/rules.unix.ports.mk
