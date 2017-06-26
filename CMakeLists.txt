#
# Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#
set( TARGET "osal" )

add_subdirectory( "os" )

set( C_HDRS "os.h" ${C_HDRS} )
set( C_SRCS "os.c" ${C_SRCS} )

get_full_path( C_HDRS ${C_HDRS} )
get_full_path( C_SRCS ${C_SRCS} )

set( API_HDRS_C ${API_HDRS_C} ${C_HDRS} PARENT_SCOPE )
set( API_HDRS_CXX ${API_HDRS_CXX} ${CXX_HDRS} PARENT_SCOPE )
set( API_SRCS_C ${API_SRCS_C} ${C_SRCS} PARENT_SCOPE )
set( API_SRCS_CXX ${API_SRCS_CXX} ${CXX_SRCS} PARENT_SCOPE )
set( API_INCS ${API_INCS} PARENT_SCOPE )
set( API_LIBS ${API_LIBS} PARENT_SCOPE )

set( OSAL_HDRS_C ${C_SRCS} ${API_HDRS_C} )
set( OSAL_SRCS_C ${C_SRCS} ${API_SRCS_C} )
add_library( ${TARGET} SHARED
       ${OSAL_HDRS_C}
       ${OSAL_HDRS_CXX}
       ${OSAL_SRCS_C}
       ${OSAL_SRCS_CXX}
)

if ( IOT_API_THREAD_SUPPORT AND THREADS_FOUND )
	set( API_LIBS ${API_LIBS} ${CMAKE_THREAD_LIBS_INIT} )
endif()

if ( WIN32 )
	set( API_LIBS ${API_LIBS} Iphlpapi Rpcrt4 Shlwapi Ws2_32 )
else()
	find_package( Libuuid REQUIRED )
	set( API_LIBS ${API_LIBS} ${LIBUUID_LIBS} )
endif()
include_directories( SYSTEM ${API_INCS} )
target_link_libraries( ${TARGET} ${API_LIBS} )
