#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

include( TestSupport )
#find_package( Jansson )

add_test_library( test-support
	"test_support.c"
	"test_support.h"
)

target_link_libraries( test-support ${CMOCKA_LIBRARIES} )

