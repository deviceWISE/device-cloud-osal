#
# Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

if( WIN32 )
	set( C_HDRS "os_win.h" )
	set( C_SRCS "os_win.c" )
	set( C_LIBS "version" )
else()
	set( C_HDRS "os_posix.h" )
	set( C_SRCS "os_posix.c" )
	set( C_LIBS "dl" )
endif()

get_full_path( C_HDRS ${C_HDRS} )
get_full_path( C_SRCS ${C_SRCS} )

set( API_HDRS_C ${API_HDRS_C} ${C_HDRS} PARENT_SCOPE )
set( API_HDRS_CXX ${API_HDRS_CXX} ${CXX_HDRS} PARENT_SCOPE )
set( API_SRCS_C ${API_SRCS_C} ${C_SRCS} PARENT_SCOPE )
set( API_SRCS_CXX ${API_SRCS_CXX} ${CXX_SRCS} PARENT_SCOPE )
set( API_INCS ${API_INCS} PARENT_SCOPE )
set( API_LIBS ${API_LIBS} ${C_LIBS} PARENT_SCOPE )

install( FILES ${C_HDRS} DESTINATION "${INSTALL_INCLUDE_DIR}/os/" COMPONENT headers )
