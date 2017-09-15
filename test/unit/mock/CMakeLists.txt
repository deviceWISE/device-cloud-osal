#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

include( TestSupport )

if ( CMAKE_C_COMPILER_ID MATCHES "(Clang|GNU)" )
	add_definitions( "-Wno-unused-parameter" )
endif ( CMAKE_C_COMPILER_ID MATCHES "(Clang|GNU)" )

#add_test_library( mock_api             "mock_api.c" )
#add_test_library( mock_osal            "mock_osal.c" )

