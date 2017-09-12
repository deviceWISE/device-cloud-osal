#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

find_package( Cmocka )
include_directories( SYSTEM ${CMOCKA_INCLUDES} )

# Add directory for find cmake configuration files for mocking libraries
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/mock" )

# Remove compiler flags
set_compiler_flag_if_supported( "-Wno-bad-function-cast" "-Wno-unused-parameter" )

# setup include directories
string( REPLACE "/test" "/src" CURRENT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" )
string( REPLACE "/test" "/src" CURRENT_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" )
include_directories( SYSTEM "${CURRENT_SOURCE_DIR}" )

# Supporting directories
if( MSVC )
	add_definitions( "/D_CRT_SECURE_NO_WARNINGS" )
endif( MSVC )
include_directories( "mock" "test-support" )
add_subdirectory( "mock" )
add_subdirectory( "test-support" )

# Directories containing tests
add_subdirectory( "src" )

