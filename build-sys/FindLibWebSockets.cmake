#
# LibWebSockets support
#
# If found the following will be defined:
# LIBWEBSOCKETS_FOUND, If false, do not try to use libWebSockets
# LIBWEBSOCKETS_INCLUDE_DIR, path where to find libwebsockets.h
# LIBWEBSOCKETS_LIBRARY_DIR, path where to find libwebsockets.so
# LIBWEBSOCKETS_LIBRARIES, the library to link against
# LIBWEBSOCKETS_VERSION, update libwebsockets to display version
#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

find_package( PkgConfig )
include( FindPackageHandleStandardArgs )

find_path( LIBWEBSOCKETS_INCLUDE_DIR
	NAMES libwebsockets.h
	DOC "libwebsockets include directory"
)

find_library( LIBWEBSOCKETS_LIBRARIES
	NAMES websockets
	DOC "Required libwebsockets libraries"
)

get_filename_component( LIBWEBSOCKETS_LIBRARY_DIR
	${LIBWEBSOCKETS_LIBRARIES} PATH )

# determine version
if ( LIBWEBSOCKETS_INCLUDE_DIR AND EXISTS "${LIBWEBSOCKETS_INCLUDE_DIR}/lws_config.h" )
	set( _LIBWEBSOCKETS_VERSION_REGEX
		"^#define[ \t]+LWS_LIBRARY_VERSION[ \t]+\"([0-9]+)\\.([0-9]+)\\.([0-9]+)\".*$")
	file( STRINGS "${LIBWEBSOCKETS_INCLUDE_DIR}/lws_config.h"
		_LIBWEBSOCKETS_VERSION_STRING
		LIMIT_COUNT 1
		REGEX "${_LIBWEBSOCKETS_VERSION_REGEX}"
	)
	if ( _LIBWEBSOCKETS_VERSION_STRING )
		string( REGEX REPLACE "${_LIBWEBSOCKETS_VERSION_REGEX}"
			"\\1.\\2.\\3" LIBWEBSOCKETS_VERSION
			"${_LIBWEBSOCKETS_VERSION_STRING}")
	endif ( _LIBWEBSOCKETS_VERSION_STRING )
	unset( _LIBWEBSOCKETS_VERSION_REGEX )
	unset( _LIBWEBSOCKETS_VERSION_STRING )
endif ( LIBWEBSOCKETS_INCLUDE_DIR AND EXISTS "${LIBWEBSOCKETS_INCLUDE_DIR}/lws_config.h" )

find_package_handle_standard_args( libwebsockets
	FOUND_VAR LIBWEBSOCKETS_FOUND
	REQUIRED_VARS LIBWEBSOCKETS_LIBRARIES LIBWEBSOCKETS_INCLUDE_DIR
	VERSION_VAR LIBWEBSOCKETS_VERSION
	FAIL_MESSAGE DEFAULT_MSG
)

mark_as_advanced(
    LIBWEBSOCKETS_LIBRARY_DIR
    LIBWEBSOCKETS_INCLUDE_DIR
    LIBWEBSOCKETS_LIBRARIES
)

