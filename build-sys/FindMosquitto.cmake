#
# Mosquitto support
#
# If found the following will be defined:
# MOSQUITTO_FOUND - System has mosquitto
# MOSQUITTO_INCLUDE_DIRS/MOSQUITTO_INCLUDES - Include directories for mosquitto
# MOSQUITTO_LIBRARIES/MOSQUITTO_LIBS - Libraries needed for mosquitto
# MOSQUITTO_DEFINITIONS - Compiler switches required for mosquitto
# MOSQUITTO_VERSION - Library version
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

find_path( MOSQUITTO_INCLUDE_DIR
	NAMES mosquitto.h
	DOC "Mosquitto include directory"
)

find_library( MOSQUITTO_LIBRARY
	NAMES mosquitto
	DOC "Required mosquitto libraries"
)

# determine version
if ( MOSQUITTO_INCLUDE_DIR AND EXISTS "${MOSQUITTO_INCLUDE_DIR}/mosquitto.h" )
	set( _MOSQUITTO_VERSION_REGEX
		"^#define[ \t]+LIBMOSQUITTO_[A-Z]+[ \t]+([0-9]+).*$" )
	file( STRINGS "${MOSQUITTO_INCLUDE_DIR}/mosquitto.h"
		_MOSQUITTO_VERSION_STRINGS
		REGEX "${_MOSQUITTO_VERSION_REGEX}"
	)
	set( MOSQUITTO_VERSION )
	foreach( _MOSQUITTO_VERSION_STRING ${_MOSQUITTO_VERSION_STRINGS} )
		string( REGEX REPLACE "${_MOSQUITTO_VERSION_REGEX}"
			"${MOSQUITTO_VERSION}.\\1" MOSQUITTO_VERSION
			"${_MOSQUITTO_VERSION_STRING}")
		string( REGEX REPLACE "^\\." "" MOSQUITTO_VERSION
			"${MOSQUITTO_VERSION}" )
	endforeach( _MOSQUITTO_VERSION_STRING )
	unset( _MOSQUITTO_VERSION_REGEX )
	unset( _MOSQUITTO_VERSION_STRING )
	unset( _MOSQUITTO_VERSION_STRINGS )
endif ( MOSQUITTO_INCLUDE_DIR AND EXISTS "${MOSQUITTO_INCLUDE_DIR}/mosquitto.h" )

find_package_handle_standard_args( Mosquitto
	FOUND_VAR MOSQUITTO_FOUND
	REQUIRED_VARS MOSQUITTO_LIBRARY MOSQUITTO_INCLUDE_DIR
	VERSION_VAR MOSQUITTO_VERSION
	FAIL_MESSAGE DEFAULT_MSG
)

set( MOSQUITTO_LIBRARIES ${MOSQUITTO_LIBRARY} )
set( MOSQUITTO_INCLUDE_DIRS ${MOSQUITTO_INCLUDE_DIR} )
set( MOSQUITTO_DEFINITIONS "" )

set( MOSQUITTO_LIBS ${MOSQUITTO_LIBRARIES} )
set( MOSQUITTO_INCLUDES ${MOSQUITTO_INCLUDE_DIRS} )

mark_as_advanced( MOSQUITTO_INCLUDE_DIR MOSQUITTO_LIBRARY )

