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

set( TARGET "os" )
set( TESTS
	"char"
)

# Remove some compiler warnings for clang
set_compiler_flag_if_supported(
	"-Wno-conversion"
	"-Wno-reserved-id-macro"
	"-Wno-shorten-64-to-32"
	"-Wno-sign-conversion"
	"-Wno-undef"
	"-Wno-unused-macros" )

add_definitions( "-DOSAL_STATIC=1" )
if ( WIN32 )
	set( OS_SRC "os_win.c" )
	set( OS_LIBS version Iphlpapi Rpcrt4 Shlwapi Ws2_32 )
else()
	set( OS_SRC "os_posix.c" )
	set( OS_LIBS "uuid-unit-test" "dl" ${CMAKE_THREAD_LIBS_INIT} )
endif()

#include( "mock_api" )
#include( "mock_osal" )

include_directories( "${CMAKE_BINARY_DIR}/out" )
set( TEST_CHAR_MOCK ${MOCK_API_PART} ${MOCK_OSAL_FUNC} )
set( TEST_CHAR_SRCS ${MOCK_API_SRCS} ${MOCK_OSAL_SRCS} "char_test.c" )
set( TEST_CHAR_LIBS ${MOCK_API_LIBS} ${MOCK_OSAL_LIBS} ${OS_LIBS} )
set( TEST_CHAR_UNIT ${OS_SRC} "os.c" )

add_unit_tests( ${TARGET} ${TESTS} )

