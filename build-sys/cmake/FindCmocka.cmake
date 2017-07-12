#
# cmocka support
#
# If found the following will be defined:
# CMOCKA_FOUND - System has cmocka
# CMOCKA_INCLUDE_DIRS/CMOCKA_INCLUDES - Include directories for cmocka
# CMOCKA_LIBRARIES/CMOCKA_LIBS - Libraries needed for cmocka
# CMOCKA_DEFINITIONS - Compiler switches required for cmocka
#
#
# Copyright (C) 2015 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

find_package( PkgConfig )
include( FindPackageHandleStandardArgs )

find_path( CMOCKA_INCLUDE_DIR
	NAMES cmocka.h
	DOC "cmocka include directory"
)

find_library( CMOCKA_LIBRARY
	NAMES cmocka
	DOC "Required cmocka libraries"
)

set( CMOCKA_LIBRARIES ${CMOCKA_LIBRARY} )
set( CMOCKA_LIBS ${CMOCKA_LIBRARIES} )
set( CMOCKA_INCLUDE_DIRS ${CMOCKA_INCLUDE_DIR} )
set( CMOCKA_INCLUDES ${CMOCKA_INCLUDE_DIRS} )
set( CMOCKA_DEFINITIONS "" )

find_package_handle_standard_args( cmocka DEFAULT_MSG CMOCKA_LIBRARY CMOCKA_INCLUDE_DIR )
mark_as_advanced( CMOCKA_INCLUDE_DIR CMOCKA_LIBRARY )

