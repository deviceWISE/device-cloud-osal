#
# SQLite3 support
#
# If found the following will be defined:
# SQLITE3_FOUND - System has SQLite3
# SQLITE3_INCLUDE_DIRS/SQLITE3_INCLUDES - Include directories for SQLite3
# SQLITE3_LIBRARIES/SQLITE3_LIBS - Libraries needed for SQLite3
# SQLITE3_DEFINITIONS - Compiler switches required for SQLite3
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

find_path( SQLITE3_INCLUDE_DIR
	NAMES sqlite3.h
	DOC "SQLite3 include directory"
)

find_library( SQLITE3_LIBRARY
	NAMES sqlite3
	DOC "Required SQLite3 libraries"
)

find_package_handle_standard_args( SQLite3 DEFAULT_MSG SQLITE3_LIBRARY SQLITE3_INCLUDE_DIR )

set( SQLITE3_LIBRARIES ${SQLITE3_LIBRARY} )
set( SQLITE3_INCLUDE_DIRS ${SQLITE3_INCLUDE_DIR} )
set( SQLITE3_DEFINITIONS "" )

set( SQLITE3_LIBS ${SQLITE3_LIBRARIES} )
set( SQLITE3_INCLUDES ${SQLITE3_INCLUDE_DIRS} )

mark_as_advanced( SQLITE3_INCLUDE_DIR SQLITE3_LIBRARY )

