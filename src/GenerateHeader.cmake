#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software  distributed
# under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
# OR CONDITIONS OF ANY KIND, either express or implied.
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

