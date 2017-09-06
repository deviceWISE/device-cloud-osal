#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

# cmake script that will generate a C header file from the specified input files
# Example usage:
# cmake -DIN_FILE:PATH=header.h.in OUT_FILE:PATH=header.h \
#       -DDEFS:STRING="blah=1 blah=2" -DPROJECT_VERSION:STRING="0.0.1" \
#       -PGenerateHeader.cmake
# Required variables:
# - IN_FILE:PATH = input file
# - OUT_FILE:PATH = output file
# Optional variables:
# - DEFS:STRING = list of symbols to define (with -D seperated by spaces)
# - PROJECT_VERSION:STRING = project version

string( REPLACE "\\" "" PROJECT_VENDOR "${PROJECT_VENDOR}" )
set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../build-sys/cmake" )
include( BuildSupport )

if( NOT IN_FILE )
	message( FATAL_ERROR "IN_FILE must be defined on the command line" )
endif()
if ( NOT OUT_FILE )
	message( FATAL_ERROR "OUT_FILE must be defined on the command line" )
endif()

set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../build-sys/cmake" )
include( StripCDefines )

string( REPLACE " " ";" DEFS "${DEFS}" )
set( OS_FUNCTION_DEF )
foreach( HDR ${HDR_FILES} )
	strip_c_defines( "${HDR}" "${DEFS}" _stripped_hdr )
	set( OS_FUNCTION_DEF ${OS_FUNCTION_DEF} ${_stripped_hdr} )
endforeach( HDR )
configure_file( "${IN_FILE}" "${OUT_FILE}" )

