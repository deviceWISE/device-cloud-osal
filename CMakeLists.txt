#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#
cmake_minimum_required( VERSION 2.8.5 )
project( "osal" )

### Definitions ###
set( PACKAGE_NAME                "Operating System Abstraction Library" )
set( PACKAGE_VERSION             "0.9.0" )
set( PACKAGE_DESCRIPTION_SUMMARY "Operating System Abstraction Library" )
set( PACKAGE_VENDOR              "Wind River Systems" )

set( PROJECT_VERSION             "${PACKAGE_VERSION}" )
set( PROJECT_VENDOR              "${PACKAGE_VENDOR}" )
set( CMAKE_MODULE_PATH           "${CMAKE_SOURCE_DIR}/build-sys/cmake" "${CMAKE_MODULE_PATH}" )

### General Build Config ###
include( BuildSupport )
include( DefineCompilerFlags )

set( TARGET                      "${PACKAGE_NAME_SHORT}" )
set( TARGET_STATIC_SUFFIX        "-static" )
set( TARGET_SHARED_SUFFIX        "-shared" )

### Output Directories ###
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/out" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/out" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/out" )
set( CMAKE_INCLUDE_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/out" )

### Operating System Specific Options ###
if( APPLE )
	set( CMAKE_MACOSX_RPATH ON )
endif( APPLE )

### Build Library ###
option( OSAL_THREAD_SUPPORT "enable multi-thread support" ON )
option( OSAL_WRAP "provide wrappers for simple functions, this is useful for mocking and unit testing" OFF )

add_subdirectory( "src" )

### Doxygen ###
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

### CPack Config ###
set( CPACK_PACKAGE_VERSION ${PACKAGE_VERSION} )
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PACKAGE_DESCRIPTION_SUMMARY}" )
set( CPACK_PACKAGE_CONTACT "support@windriver.com" )

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
include( TestSupport )

