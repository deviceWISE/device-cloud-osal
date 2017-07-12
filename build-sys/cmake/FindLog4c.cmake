#
# Log4c support
#
# If found the following will be defined:
# LOG4C_FOUND - System has log4c
# LOG4C_INCLUDE_DIRS/LOG4C_INCLUDES - Include directories for log4c
# LOG4C_LIBRARIES/LOG4C_LIBS - Libraries needed for log4c
# LOG4C_DEFINITIONS - Compiler switches required for log4c
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

find_path( LOG4C_INCLUDE_DIR
	NAMES log4c.h
	DOC "Log4c include directory"
	HINTS "${WRLINUX_IOT_EXTRAS}/log4c/build/include"
)

find_library( LOG4C_LIBRARY
	NAMES log4c
	DOC "Required Log4c libraries"
	HINTS "${WRLINUX_IOT_EXTRAS}/log4c/build/lib"
)

set( LOG4C_LIBRARIES ${LOG4C_LIBRARY} )
set( LOG4C_LIBS ${LOG4C_LIBRARIES} )
set( LOG4C_INCLUDE_DIRS ${LOG4C_INCLUDE_DIR} )
set( LOG4C_INCLUDES ${LOG4C_INCLUDE_DIRS} )
set( LOG4C_DEFINITIONS "" )

find_package_handle_standard_args( Log4c DEFAULT_MSG LOG4C_LIBRARY LOG4C_INCLUDE_DIR )
mark_as_advanced( LOG4C_INCLUDE_DIR LOG4C_LIBRARY )

