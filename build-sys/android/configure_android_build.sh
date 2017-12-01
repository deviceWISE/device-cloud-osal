#!/bin/bash
#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software  distributed
# under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
# OR CONDITIONS OF ANY KIND, either express or implied.
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
