#
# Copyright (C) 2017-2018 Wind River Systems, Inc. All Rights Reserved.
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
	"adapters"
	"env"
	"run"
	"service_entry"
	"time"
)

# Use static library version
add_definitions( "-DOSAL_STATIC=1" )
set( OS_LIB ${TARGET}${TARGET_STATIC_SUFFIX} )

include_directories( "${CMAKE_BINARY_DIR}/out" )

# adapter tests
set( TEST_ADAPTERS_SRCS "adapters_test.c" )
set( TEST_ADAPTERS_LIBS ${OS_LIB} )

# env tests
set( TEST_ENV_SRCS "env_test.c" )
set( TEST_ENV_LIBS ${OS_LIB} )

# system run tests
set( TEST_RUN_SRCS "run_test.c" )
set( TEST_RUN_LIBS ${OS_LIB} )

# service entry tests
set( TEST_SERVICE_ENTRY_SRCS "service_entry_test.c" )
set( TEST_SERVICE_ENTRY_LIBS ${OS_LIB} )

# time tests
set( TEST_TIME_SRCS "time_test.c" )
set( TEST_TIME_LIBS ${OS_LIB} )

add_integration_tests( "" ${TESTS} )
