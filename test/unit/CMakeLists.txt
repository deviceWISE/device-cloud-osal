#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

# Add directory for find cmake configuration files for mocking libraries
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/mock" )

# setup include directories
string( REPLACE "/test" "/src" CURRENT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" )
string( REPLACE "/test" "/src" CURRENT_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" )
include_directories( SYSTEM "${CURRENT_SOURCE_DIR}" )

# Supporting directories
include_directories( "mock" )
add_subdirectory( "mock" )

# Directories containing tests
add_subdirectory( "src" )

