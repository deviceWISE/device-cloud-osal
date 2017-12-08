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

