#
# Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

include( CMakeParseArguments )

### SET_COMPILER_FLAG
# checks to see if a particular compiler supports the given flag and
# added the definition to the compile if it is supported
# Arguments:
# - flag           name of first compiler flag to test
# - ...            additional compiler flags to test
macro( SET_COMPILER_FLAG_IF_SUPPORTED FLAG )
	foreach( _flag ${FLAG} ${ARGN} )
		check_c_compiler_flag( "${_flag}" _flag_exists )
		if ( _flag_exists )
			add_definitions( "${_flag}" )
		endif( _flag_exists )
	endforeach( _flag )
endmacro( SET_COMPILER_FLAG_IF_SUPPORTED )

### GET_SHORT_NAME
# Build a short name from a longer name
# Examples:
# - Internet of Things->iot;
# - Helix Device Cloud->hdc;
# - Operating System Abstraction Layer->osal
# Arguments:
# - _var          output variable
# - _name ...     long name
function( GET_SHORT_NAME _var _name )
	set( _name ${_name} ${ARGN} )
	string( TOLOWER "${_name}" NAME_SPLIT )
	string( REGEX REPLACE "[ -_:.]" ";" NAME_SPLIT "${NAME_SPLIT}" )
	set( NAME_SHORT "" )
	foreach( WORD ${NAME_SPLIT} )
		string( REGEX REPLACE "([a-z0-9]).+" "\\1" WORD "${WORD}" )
		set( NAME_SHORT "${NAME_SHORT}${WORD}" )
	endforeach( WORD )
	string( LENGTH "${NAME_SHORT}" NAME_SHORT_LENGTH )
	if ( NAME_SHORT_LENGTH LESS 2 )
		set( NAME_SHORT "${_name}" )
	endif ( NAME_SHORT_LENGTH LESS 2 )
	string( REGEX REPLACE "[^a-z0-9]" "" NAME_SHORT "${NAME_SHORT}" )
	set( ${_var} "${NAME_SHORT}" PARENT_SCOPE )
endfunction( GET_SHORT_NAME )

get_short_name( PACKAGE_NAME_SHORT "${PACKAGE_NAME}" )
get_short_name( PROJECT_NAME_SHORT "${PROJECT_NAME}" )

### Parse version information
# Support function for extracting a version number from a string, if the
# index is not specified it will return a value of "0" for that version part
# Arguments:
# - _out           output variable name
# - _list_name     list containing version seperated
# - _index         index to extract
function( VERSION_PART_FROM_LIST _out _list_name _index )
	set( VERSION "0" )
	list( LENGTH ${_list_name} VERSION_ITEMS )
	if( ${VERSION_ITEMS} GREATER ${_index} )
		list( GET ${_list_name} ${_index} VERSION )
	endif( ${VERSION_ITEMS} GREATER ${_index} )
	set( "${_out}" ${VERSION} PARENT_SCOPE )
endfunction( VERSION_PART_FROM_LIST )

# Extracts the various version parts (major, minor, patch, tweak) from a
# version string The values will be saved as:
# - ${_out}_COUNT: number of version parts set
# - ${_out}_MAJOR: major (1st) version number
# - ${_out}_MINOR: minor (2nd) version number
# - ${_out}_PATCH: patch (3rd) version number
# - ${_out}_TWEAK: tweak (4th) version number
#
# Arugments:
# - _out           output variable name
# - _ver ...       version to split
function( VERSION_SPLIT _out _ver )
	set( _ver ${_ver} ${ARGN} )
	string( REPLACE "." ";" VERSION_SPLIT "${_ver}" )

	list( LENGTH VERSION_SPLIT "VERSION_COUNT" )

	version_part_from_list( VERSION_MAJOR VERSION_SPLIT 0 )
	version_part_from_list( VERSION_MINOR VERSION_SPLIT 1 )
	version_part_from_list( VERSION_PATCH VERSION_SPLIT 2 )
	version_part_from_list( VERSION_TWEAK VERSION_SPLIT 3 )

	set( "${_out}_COUNT" "${VERSION_COUNT}" PARENT_SCOPE )
	set( "${_out}_MAJOR" "${VERSION_MAJOR}" PARENT_SCOPE )
	set( "${_out}_MINOR" "${VERSION_MINOR}" PARENT_SCOPE )
	set( "${_out}_PATCH" "${VERSION_PATCH}" PARENT_SCOPE )
	set( "${_out}_TWEAK" "${VERSION_TWEAK}" PARENT_SCOPE )
endfunction( VERSION_SPLIT )
version_split( PROJECT_VERSION "${PROJECT_VERSION}" )

### Generate Copyright String ###
if( WIN32 )
	execute_process( COMMAND "cmd" "/C" "date" "/T" OUTPUT_VARIABLE CURRENT_YEAR )
	# Windows 10 format
	string( REGEX REPLACE "([0-9]+)-[0-9]+-[0-9]+" "\\1" CURRENT_YEAR "${CURRENT_YEAR}" )
	# Windows XP format
	string( REGEX REPLACE "[0-9]+/[0-9]+/([0-9]+)" "\\1" CURRENT_YEAR "${CURRENT_YEAR}" )
	string( REPLACE "\n" "" CURRENT_YEAR "${CURRENT_YEAR}" )
	string( STRIP "${CURRENT_YEAR}" CURRENT_YEAR )
else()
	execute_process( COMMAND "date" "+%Y" OUTPUT_VARIABLE CURRENT_YEAR )
	string( REGEX REPLACE "(....).*" "\\1" CURRENT_YEAR "${CURRENT_YEAR}" )
endif()
set( COPYRIGHT_RANGE "2017" )
if( NOT "${COPYRIGHT_RANGE}" STREQUAL "${CURRENT_YEAR}" )
	set( COPYRIGHT_RANGE "${COPYRIGHT_RANGE}-${CURRENT_YEAR}" )
endif()
set( PROJECT_COPYRIGHT "Copyright (C) ${COPYRIGHT_RANGE} ${PROJECT_VENDOR}, All Rights Reserved." )

