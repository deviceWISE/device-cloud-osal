#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

set( TARGET "os" )
set( TESTS
	"char"
)

if ( WIN32 )
	set( OS_SRC "os_win.c" )
	set( OS_LIBS version Iphlpapi Rpcrt4 Shlwapi Ws2_32 )
else()
	set( OS_SRC "os_posix.c" ) #$<TARGET_OBJECTS:uuid> )
	set( OS_LIBS "dl" )
endif()

#include( "mock_api" )
#include( "mock_osal" )

include_directories( "${CMAKE_BINARY_DIR}/out" )
set( TEST_CHAR_MOCK ${MOCK_API_PART} ${MOCK_OSAL_FUNC} )
set( TEST_CHAR_SRCS ${MOCK_API_SRCS} ${MOCK_OSAL_SRCS} "char_test.c" )
set( TEST_CHAR_LIBS ${MOCK_API_LIBS} ${MOCK_OSAL_LIBS} ${OS_LIBS} )
set( TEST_CHAR_UNIT ${OS_SRC} "os.c" )

include( TestSupport )
add_tests( ${TARGET} ${TESTS} )

