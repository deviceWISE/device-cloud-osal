#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

# get header file full path
function( get_full_path OUT_VAR )
	set( SRCS )
	foreach ( SRC_FILE ${ARGN} )
		if ( IS_ABSOLUTE "${SRC_FILE}" )
			list( APPEND SRCS "${SRC_FILE}" )
		else()
			list( APPEND SRCS "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FILE}" )
		endif()
	endforeach( SRC_FILE )
	set( ${OUT_VAR} ${SRCS} PARENT_SCOPE )
endfunction( get_full_path OUT_VAR )

set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-sys" "${CMAKE_MODULE_PATH}" )

include( BuildSupport )
include( DefineCompilerFlags )

set( TARGET "osal" )

add_subdirectory( "os" )

set( C_HDRS "os.h" ${C_HDRS} )
set( C_SRCS "os.c" ${C_SRCS} )

get_full_path( C_HDRS ${C_HDRS} )
get_full_path( C_SRCS ${C_SRCS} )

set( API_HDRS_C ${API_HDRS_C} ${C_HDRS} )
set( API_HDRS_CXX ${API_HDRS_CXX} ${CXX_HDRS} )
set( API_SRCS_C ${API_SRCS_C} ${C_SRCS} )
set( API_SRCS_CXX ${API_SRCS_CXX} ${CXX_SRCS} )
set( API_INCS ${API_INCS} )
set( API_LIBS ${API_LIBS} )

set( OSAL_HDRS_C ${C_SRCS} ${API_HDRS_C} )
set( OSAL_SRCS_C ${C_SRCS} ${API_SRCS_C} )
add_library( ${TARGET} SHARED
       ${OSAL_HDRS_C}
       ${OSAL_HDRS_CXX}
       ${OSAL_SRCS_C}
       ${OSAL_SRCS_CXX}
)

if ( OS_API_THREAD_SUPPORT AND THREADS_FOUND )
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

install( TARGETS ${TARGET}
	RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT api
	ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT libs
	LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT ${LIB_COMPONENT}
	PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT headers
)