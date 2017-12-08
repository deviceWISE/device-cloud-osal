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

set( TESTS
	"time"
)

# Use static library version
add_definitions( "-DOSAL_STATIC=1" )
set( OS_LIB ${TARGET}${TARGET_STATIC_SUFFIX} )

include_directories( "${CMAKE_BINARY_DIR}/out" )

# time test
set( TEST_TIME_SRCS "time_test.c" )
set( TEST_TIME_LIBS ${OS_LIB} )

add_integration_tests( "" ${TESTS} )
