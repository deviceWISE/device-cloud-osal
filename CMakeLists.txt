#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#
cmake_minimum_required( VERSION 2.8.5 )
project( "OSAL" )

### Definitions ###
set( PACKAGE_NAME                "OSAL" )
set( PACKAGE_VERSION             "0.0.1" )
set( PACKAGE_DESCRIPTION_SUMMARY "Operating System Abstraction Library" )
set( PACKAGE_VENDOR              "Wind River Systems" )

set( PROJECT_VERSION             "${PACKAGE_VERSION}" )
set( PROJECT_VENDOR              "${PACKAGE_VENDOR}" )

set( CMAKE_MODULE_PATH           "${CMAKE_SOURCE_DIR}/build-sys" "${CMAKE_MODULE_PATH}" )

### Functions ###

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

### General Build Config ###

include( BuildSupport )
include( DefineCompilerFlags )

set_target_properties( ${TARGET} PROPERTIES
	SOVERSION ${PACKAGE_VERSION}
	VERSION ${PACKAGE_VERSION}
	)

### CPack Config ###
set( CPACK_PACKAGE_VERSION ${PACKAGE_VERSION} )
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "Operating System Abstraction Library" )
set( CPACK_PACKAGE_CONTACT "bwiebe" )

if ( WIN32 )
	set( CPACK_GENERATOR "ZIP" )
else()
	set( CPACK_GENERATOR "DEB" )
	find_program( RPMBUILD_EXECUTABLE rpmbuild
		DOC "path to rpmbuild executable" )
	mark_as_advanced( RPMBUILD_EXECUTABLE )
	if ( RPMBUILD_EXECUTABLE )
		set( GENERATOR_DEFAULT "RPM" )
	endif ( RPMBUILD_EXECUTABLE )
	set( GENERATOR_LIST "RPM" "DEB" )
	set( CPACK_DEBIAN_PACKAGE_DEPENDS "libuuid1, uuid-dev, sudo, libc6" )
endif()

include( CPack )

### Doxygen Config ###
find_package( Doxygen )

if( DOXYGEN_FOUND )
	set( DOXYFILE_SOURCE_DIR "build-sys/doxygen" )
	string( TIMESTAMP DATE_YEAR "%Y" UTC )
	string( TIMESTAMP DATE_MONTH "%m" UTC )
	string( TIMESTAMP DATE_DAY "%d" UTC )
	string( TIMESTAMP DATE_TIME "%H:%M" UTC )
	configure_file( "${DOXYFILE_SOURCE_DIR}/Doxyfile.in" "${CMAKE_BINARY_DIR}/Doxyfile" @ONLY )
	configure_file( "${DOXYFILE_SOURCE_DIR}/footer.html.in" "${CMAKE_BINARY_DIR}/footer.html" @ONLY )

	add_custom_target( doc
		COMMAND "${CMAKE_COMMAND}" -E echo "Generating documentation..."
		COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_BINARY_DIR}/doxygen"
		COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_BINARY_DIR}/Doxyfile"
		COMMAND "${CMAKE_COMMAND}" -E echo_append "Complete"
		DEPENDS "${CMAKE_BINARY_DIR}/Doxyfile"
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		COMMENT "Generating Documentation" VERBATIM
	)
endif( DOXYGEN_FOUND )

### Build Library ###
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

set( LIB_COMPONENT "api" )
if ( WIN32 )
	set( LIB_COMPONENT "libs" )
endif( WIN32 )

install( TARGETS ${TARGET}
	RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT api
	ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT libs
	LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT ${LIB_COMPONENT}
	PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT headers
)

install( FILES ${C_HDRS} 
	DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT headers 
)