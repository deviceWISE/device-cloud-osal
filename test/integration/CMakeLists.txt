#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
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
