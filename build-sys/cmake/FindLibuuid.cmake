#
# Libuuid support
#
# If found the following will be defined:
# LIBUUID_FOUND - System has libuuid
# LIBUUID_INCLUDE_DIRS/LIBUUID_INCLUDES - Include directories for libuuid
# LIBUUID_LIBRARIES/LIBUUID_LIBS - Libraries needed for libuuid
# LIBUUID_DEFINITIONS - Compiler switches required for libuuid
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

find_path( LIBUUID_INCLUDE_DIR
	NAMES uuid/uuid.h
	DOC "Libuuid include directory"
)

find_library( LIBUUID_LIBRARY
	NAMES uuid
	DOC "Required Libuuid libraries"
)

find_package_handle_standard_args( libuuid DEFAULT_MSG LIBUUID_LIBRARY LIBUUID_INCLUDE_DIR )

set( LIBUUID_LIBRARIES ${LIBUUID_LIBRARY} )
set( LIBUUID_INCLUDE_DIRS ${LIBUUID_INCLUDE_DIR} )
set( LIBUUID_DEFINITIONS "" )

set( LIBUUID_LIBS ${LIBUUID_LIBRARIES} )
set( LIBUUID_INCLUDES ${LIBUUID_INCLUDE_DIRS} )

mark_as_advanced( LIBUUID_INCLUDE_DIR LIBUUID_LIBRARY )

