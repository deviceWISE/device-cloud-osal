#
# cmocka support
#
# The following variables can be set to add additional find support:
# - CMOCKA_ROOT_DIR, specified an explicit root path to test
#
# If found the following will be defined:
# - CMOCKA_FOUND - System has cmocka
# - CMOCKA_INCLUDE_DIRS/CMOCKA_INCLUDES - Include directories for cmocka
# - CMOCKA_LIBRARIES/CMOCKA_LIBS - Libraries needed for cmocka
# - CMOCKA_DEFINITIONS - Compiler switches required for cmocka
#
# Copyright (C) 2015-2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

include( FindPackageHandleStandardArgs )

set( _PROGRAMFILES     "ProgramFiles" )
set( _PROGRAMFILES_X86 "ProgramFiles(x86)" )

# Try and find paths
set( LIB_SUFFIX "" )
get_property( LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS )
if( LIB64 )
	set( LIB_SUFFIX 64 )
endif()

# Allow the ability to specify a global dependency root directory
if ( NOT CMOCKA_ROOT_DIR )
	set( CMOCKA_ROOT_DIR ${DEPENDS_ROOT_DIR} )
endif()

find_path( CMOCKA_INCLUDE_DIR
	NAMES cmocka.h
	DOC "cmocka include directory"
	PATHS "${CMOCKA_ROOT_DIR}/include"
	      "$ENV{${_PROGRAMFILES}}/cmocka/include"
	      "$ENV{${_PROGRAMFILES_X86}}/cmocka/include"
)

find_library( CMOCKA_LIBRARY
	NAMES cmocka
	DOC "Required cmocka libraries"
	PATHS "${CMOCKA_ROOT_DIR}/lib${LIB_SUFFIX}"
	      "${CMOCKA_ROOT_DIR}/lib"
	      "$ENV{${_PROGRAMFILES}}/cmocka/lib"
	      "$ENV{${_PROGRAMFILES_X86}}/cmocka/lib"
)

set( CMOCKA_LIBRARIES ${CMOCKA_LIBRARY} )
set( CMOCKA_LIBS ${CMOCKA_LIBRARIES} )
set( CMOCKA_INCLUDE_DIRS ${CMOCKA_INCLUDE_DIR} )
set( CMOCKA_INCLUDES ${CMOCKA_INCLUDE_DIRS} )
set( CMOCKA_DEFINITIONS "" )

find_package_handle_standard_args( Cmocka
	FOUND_VAR CMOCKA_FOUND
	REQUIRED_VARS CMOCKA_INCLUDE_DIR CMOCKA_LIBRARY
	FAIL_MESSAGE DEFAULT_MSG )
mark_as_advanced( CMOCKA_INCLUDE_DIR CMOCKA_LIBRARY )

