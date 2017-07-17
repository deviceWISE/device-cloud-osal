#!/bin/bash
#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

# configuring header files for android build

EXEC_PATH=$( dirname $0 )
OSAL_DIR=$( readlink -e "${EXEC_PATH}/../.." )
OSAL_CMAKE_BUILD_DIR="osal-cmake"

pushd ${OSAL_DIR}

mkdir -p $OSAL_CMAKE_BUILD_DIR
cd $OSAL_CMAKE_BUILD_DIR
cmake -DOSAL_WRAP=0 -DOSAL_TARGET=android ..
make osal-pre
cp "os.h" ..

exit 0
