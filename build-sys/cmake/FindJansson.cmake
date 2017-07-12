#
# Jansson support
#
# If found the following will be defined:
# JANSSON_FOUND - System has jansson
# JANSSON_INCLUDE_DIRS/JANSSON_INCLUDES - Include directories for jansson
# JANSSON_LIBRARIES/JANSSON_LIBS - Libraries needed for jansson
# JANSSON_DEFINITIONS - Compiler switches required for jansson
# JANSSON_VERSION - Library version
#
#
# Copyright (C) 2014-2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

find_package( PkgConfig )
include( FindPackageHandleStandardArgs )

find_path( JANSSON_INCLUDE_DIR
	NAMES jansson.h
	DOC "Jansson include directory"
)

find_library( JANSSON_LIBRARY
	NAMES jansson
	DOC "Required jansson libraries"
)

# determine version
if ( JANSSON_INCLUDE_DIR AND EXISTS "${JANSSON_INCLUDE_DIR}/jansson.h" )
	set( _JANSSON_VERSION_REGEX
		"^#define[ \t]+JANSSON_VERSION[ \t]+\"([0-9]+)\\.([0-9]+)\\.?([0-9]*)\".*$")
	file( STRINGS "${JANSSON_INCLUDE_DIR}/jansson.h"
		_JANSSON_VERSION_STRING
		LIMIT_COUNT 1
		REGEX "${_JANSSON_VERSION_REGEX}"
	)
	if ( _JANSSON_VERSION_STRING )
		string( REGEX REPLACE "${_JANSSON_VERSION_REGEX}"
			"\\1.\\2.\\3" JANSSON_VERSION
			"${_JANSSON_VERSION_STRING}")
		string( REGEX REPLACE "\\.$" ".0" JANSSON_VERSION
			"${JANSSON_VERSION}" )
	endif ( _JANSSON_VERSION_STRING )
	unset( _JANSSON_VERSION_REGEX )
	unset( _JANSSON_VERSION_STRING )
endif ( JANSSON_INCLUDE_DIR AND EXISTS "${JANSSON_INCLUDE_DIR}/jansson.h" )

find_package_handle_standard_args( Jansson
	FOUND_VAR JANSSON_FOUND
	REQUIRED_VARS JANSSON_LIBRARY JANSSON_INCLUDE_DIR
	VERSION_VAR JANSSON_VERSION
	FAIL_MESSAGE DEFAULT_MSG
)

set( JANSSON_LIBRARIES ${JANSSON_LIBRARY} )
set( JANSSON_INCLUDE_DIRS ${JANSSON_INCLUDE_DIR} )
set( JANSSON_DEFINITIONS "" )

set( JANSSON_LIBS ${JANSSON_LIBRARIES} )
set( JANSSON_INCLUDES ${JANSSON_INCLUDE_DIRS} )

mark_as_advanced( JANSSON_INCLUDE_DIR JANSSON_LIBRARY )

