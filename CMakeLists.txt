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

set( CMAKE_MODULE_PATH           "${CMAKE_SOURCE_DIR}/build-sys/cmake" "${CMAKE_MODULE_PATH}" )

set( OSAL_INSTALL_HEADER_PATH    "${CMAKE_BINARY_DIR}/install/os.h" )

set( TARGET                      "osal" )

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
	set( CPACK_DEBIAN_PACKAGE_DEPENDS "sudo, libc6" )
endif()

include( CPack )

### Header Generation Config ###
# Create empty file. CMake needs this to exist, but the real file will be
# generated at compile-time (below, in the ${TARGET}-pre target)
file(WRITE ${CMAKE_BINARY_DIR}/os.h "")

add_custom_target(${TARGET}-pre
		COMMAND "${CMAKE_COMMAND}" -E echo "Generating library header file..."
		COMMAND "${CMAKE_COMMAND}" -DOSAL_TARGET=${OSAL_TARGET} -DOSAL_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
			-DOSAL_WRAP=${OSAL_WRAP} -P "${CMAKE_SOURCE_DIR}/header/GenerateHeader.cmake"
		COMMAND "${CMAKE_COMMAND}" -E echo "Complete"
		)
include_directories(${CMAKE_BINARY_DIR})

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
		COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_BINARY_DIR}/os.h" "${CMAKE_SOURCE_DIR}"
		COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_BINARY_DIR}/Doxyfile"
		COMMAND "${CMAKE_COMMAND}" -E remove "${CMAKE_SOURCE_DIR}/os.h"
		COMMAND "${CMAKE_COMMAND}" -E echo_append "Complete"
		DEPENDS "${CMAKE_BINARY_DIR}/Doxyfile"
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		COMMENT "Generating Documentation" VERBATIM
	)

	add_dependencies(doc ${TARGET}-pre)

endif( DOXYGEN_FOUND )

### Build Library ###

add_subdirectory( "os" )
add_subdirectory( "uuid" )

set( C_HDRS "${CMAKE_BINARY_DIR}/os.h" ${C_HDRS} )
set( C_SRCS "os.c" ${C_SRCS}  )

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


# For uuid, we need to add defines from config.h, since this was an
# autoconf project
add_definitions( -DHAVE_ERR_H=1 -DHAVE_USLEEP=1 -DHAVE_SYS_FILE_H
-DHAVE_DECL__SC_HOST_NAME_MAX=1 -DHAVE_DLFCN_H=1 -DHAVE_FCNTL_H=1
-DHAVE_FTRUNCATE=1 -DHAVE_GETTIMEOFDAY=1 -DHAVE_INTTYPES_H=1
-DHAVE_LIMITS_H=1 -DHAVE_MEMORY_H=1 -DHAVE_MEMSET=1
-DHAVE_NETINET_IN_H=1 -DHAVE_SOCKET=1 -DHAVE_SRANDOM=1
-DHAVE_STDINT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRINGS_H=1
-DHAVE_STRING_H=1 -DHAVE_STRTOUL=1 -DHAVE_SYS_FILE_H=1
-DHAVE_SYS_IOCTL_H=1 -DHAVE_SYS_SOCKET_H=1 -DHAVE_SYS_STAT_H=1
-DSTDC_HEADERS=1 -DHAVE_SYS_TIME_H=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_UNISTD_H=1 )

# On Windows, .lib files are for both Shared and Static libraries. For clarity,
# no renaming of this target is done; the library will exist as libosalStatic
# and osalStatic.lib on POSIX and Windows, respectively, so as to be consistent.
add_library( ${TARGET}Static STATIC
       ${OSAL_HDRS_C}
       ${OSAL_HDRS_CXX}
       ${OSAL_SRCS_C}
       ${OSAL_SRCS_CXX}
)

target_compile_definitions( ${TARGET}Static PRIVATE OSAL_STATIC=1 )
add_dependencies(${TARGET}Static ${TARGET}-pre)
add_library( ${TARGET}Shared SHARED
       ${OSAL_HDRS_C}
       ${OSAL_HDRS_CXX}
       ${OSAL_SRCS_C}
       ${OSAL_SRCS_CXX}
)

set_target_properties( ${TARGET}Shared PROPERTIES
	OUTPUT_NAME ${TARGET}
	SOVERSION ${PACKAGE_VERSION}
	VERSION ${PACKAGE_VERSION}
	)

target_compile_definitions( ${TARGET}Shared PRIVATE OSAL_EXPORTS=1 )

add_dependencies(${TARGET}Shared ${TARGET}-pre)

if ( OS_API_THREAD_SUPPORT AND THREADS_FOUND )
	set( API_LIBS ${API_LIBS} ${CMAKE_THREAD_LIBS_INIT} )
endif()

if ( WIN32 )
	set( API_LIBS ${API_LIBS} Iphlpapi Rpcrt4 Shlwapi Ws2_32 )
else()
	set( API_LIBS ${API_LIBS} uuid )
endif()
include_directories( SYSTEM ${API_INCS} )

target_link_libraries( ${TARGET}Static ${API_LIBS} )
target_link_libraries( ${TARGET}Shared ${API_LIBS} )

set( LIB_COMPONENT "api" )
if ( WIN32 )
	set( LIB_COMPONENT "libs" )
endif( WIN32 )

install( TARGETS ${TARGET}Shared
	RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT api
	ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT libs
	LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT ${LIB_COMPONENT}
	PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT headers
)

install( TARGETS ${TARGET}Static
	RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT api
	ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT libs
	LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT ${LIB_COMPONENT}
	PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT headers
)

install( FILES ${C_HDRS}
	DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT headers 
)
