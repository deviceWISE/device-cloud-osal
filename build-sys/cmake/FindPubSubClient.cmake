#
# PubSubClient support
#
# If found the following will be defined:
# PUBSUBCLIENT_FOUND - System has PubSubClient
# PUBSUBCLIENT_INCLUDE_DIRS/PUBSUBCLIENT_INCLUDES - Include directories for PubSubClient
# PUBSUBCLIENT_LIBRARIES/PUBSUBCLIENT_LIBS - Libraries needed for PubSubClient
# PUBSUBCLIENT_DEFINITIONS - Compiler switches required for PubSubClient
#
#
# Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

find_package( PkgConfig )
include( FindPackageHandleStandardArgs )


find_path( PUBSUBCLIENT_INCLUDE_DIR
	NAMES PubSubClient.h
	DOC "PubSubClient include directory"
	PATHS
	${ARDUINO_SDK_PATH}/libraries/PubSubClient/src
)
if ( NOT CMAKE_SYSTEM_NAME MATCHES "Arduino" )
	find_library( PUBSUBCLIENT_LIBRARY
		NAMES PubSubClient
		DOC "Required PubSubClient libraries"
	)
endif()

find_package_handle_standard_args( PubSubClient DEFAULT_MSG PUBSUBCLIENT_INCLUDE_DIR )

set( PUBSUBCLIENT_LIBRARIES ${PUBSUBCLIENT_LIBRARY} )
set( PUBSUBCLIENT_INCLUDE_DIRS ${PUBSUBCLIENT_INCLUDE_DIR} )
set( PUBSUBCLIENT_DEFINITIONS "" )

set( PUBSUBCLIENT_LIBS ${PUBSUBCLIENT_LIBRARIES} )
set( PUBSUBCLIENT_INCLUDES ${PUBSUBCLIENT_INCLUDE_DIRS} )

mark_as_advanced( PUBSUBCLIENT_INCLUDE_DIR PUBSUBCLIENT_LIBRARY )

