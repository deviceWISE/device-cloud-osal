#
# Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

include( CMakeParseArguments )

# Build a short name from a longer name
# Examples: Internet of Things->iot;Helix Device Cloud->hdc
# Arguments:
#  - _var          output variable
#  - _name ...     long name
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

# Function used to generate files from ".in" files.  The output files will have
# variables as specified between "@" symbols known by the build system replaced
# with their respective values.  The last extension is removed from the end of
# the file name if the file name ends with ".in"; i.e. "config.h.in" would become
# "config.h".
#
# Parameters:
#    OUTPUT_DIR    the directory where to output the files
#    OUTPUT_VAR    variable to save list of generated files ("" if not desired)
#    ...           list of input files
function( GENERATE_FILES OUTPUT_DIR OUTPUT_VAR )
	set( OUTPUT_FILES )
	foreach( IN_FILE_PATH ${ARGN} )
		# Determine the output file name
		get_filename_component( IN_FILE_NAME "${IN_FILE_PATH}" NAME )
		string( FIND "${IN_FILE_NAME}" "." EXT_LOC REVERSE )
		string( SUBSTRING "${IN_FILE_NAME}" ${EXT_LOC} -1 IN_FILE_EXT )
		set( OUT_FILE_NAME "${IN_FILE_NAME}" )
		if ( "${IN_FILE_EXT}" STREQUAL ".in" )
			string( SUBSTRING "${IN_FILE_NAME}" 0 ${EXT_LOC} OUT_FILE_NAME )
		endif ( "${IN_FILE_EXT}" STREQUAL ".in" )

		# Generate the new file
		set( OUT_FILE_PATH "${OUTPUT_DIR}/${OUT_FILE_NAME}" )
		if ( "${IN_FILE_PATH}" STREQUAL "${OUT_FILE_PATH}" )
			message( FATAL_ERROR "Unable to generated the file: ${OUT_FILE_PATH}.  "
			                     "The source file has the same path." )
		endif ( "${IN_FILE_PATH}" STREQUAL "${OUT_FILE_PATH}" )
		configure_file( "${IN_FILE_PATH}" "${OUT_FILE_PATH}" @ONLY )

		# Add the generated file to the list of files to be cleaned via "make clean"
		set( OUTPUT_FILES ${OUTPUT_FILES} "${OUT_FILE_PATH}" )
	endforeach( IN_FILE_PATH )
	# Output the list of files that were generated
	if ( NOT "x${OUTPUT_VAR}" STREQUAL "x" )
		set( "${OUTPUT_VAR}" ${OUTPUT_FILES} PARENT_SCOPE )
	endif ( NOT "x${OUTPUT_VAR}" STREQUAL "x" )
	set_source_files_properties( ${OUTPUT_FILES} PROPERTIES GENERATED TRUE )
endfunction( GENERATE_FILES )

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

# Determine if compiling in 64-bit mode
get_property( LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS )
if( LIB64 )
    set( LIB_SUFFIX 64 )
else()
    set( LIB_SUFFIX "" )
endif()

# Set default paths
function( SET_INSTALL_PATH _var _default _description )
	set( "${_var}" "${_default}" CACHE PATH "${_description}" )
	mark_as_advanced( "${_var}" )
	set( INSTALL_PATH "${${_var}}" )
	file( TO_NATIVE_PATH "${INSTALL_PATH}" INSTALL_PATH )
	if ( WIN32 )
		string( REPLACE "\\" "\\\\" INSTALL_PATH "${INSTALL_PATH}" )
	endif()
	set( "${_var}_PATH" "${INSTALL_PATH}" PARENT_SCOPE )
endfunction()

# The name for sub-directories for this project to use.
if ( WIN32 )
	set( DEFAULT_ETC_DIR "etc" )
	set( DEFAULT_EXAMPLES_DIR "examples" )
	set( DEFAULT_LIB_DIR "lib" )
	set( DEFAULT_PROTOCOL_DIR "protocols" )
	set( DEFAULT_VAR_DIR "%PROGRAMDATA%\\${PACKAGE_VENDOR}\\${PACKAGE_NAME}" )
else( WIN32 )
	set( DEFAULT_ETC_DIR "/etc/${PROJECT_NAME_SHORT}" )
	set( DEFAULT_EXAMPLES_DIR "share/${PROJECT_NAME_SHORT}/examples" )
	set( DEFAULT_LIB_DIR "lib${LIB_SUFFIX}" )
	set( DEFAULT_PROTOCOL_DIR "${INSTALL_LIB_DIR}/${PROJECT_NAME_SHORT}/protocols" )
	set( DEFAULT_VAR_DIR "/var/lib/${PROJECT_NAME_SHORT}" )
endif( WIN32 )
set( DEFAULT_BIN_DIR     "bin" )
set( DEFAULT_INCLUDE_DIR "include" )
set( DEFAULT_SYSTEMD_DIR "/lib/systemd/system" )
set( DEFAULT_INITD_DIR   "/etc/init.d" )
set( DEFAULT_MUX_DIR     "/etc/${PROJECT_NAME_SHORT}" )
set_install_path( INSTALL_BIN_DIR      "${DEFAULT_BIN_DIR}"      "Installation directory for binaries" )
set_install_path( INSTALL_ETC_DIR      "${DEFAULT_ETC_DIR}"      "Installation configuration directory" )
set_install_path( INSTALL_EXAMPLES_DIR "${DEFAULT_EXAMPLES_DIR}" "Installation directory for example applications" )
set_install_path( INSTALL_INCLUDE_DIR  "${DEFAULT_INCLUDE_DIR}"  "Installation directory for header files" )
set_install_path( INSTALL_LIB_DIR      "${DEFAULT_LIB_DIR}"      "Installation directory for libraries" )
set_install_path( INSTALL_PROTOCOL_DIR "${DEFAULT_PROTOCOL_DIR}" "Installation directory for protocols" )
set_install_path( INSTALL_VAR_DIR      "${DEFAULT_VAR_DIR}"      "Installation directory for runtime data" )
if ( NOT WIN32 )
	set_install_path( INSTALL_SYSTEMD_DIR "${DEFAULT_SYSTEMD_DIR}" "Installation directory for systemd service files" )
	set_install_path( INSTALL_INITD_DIR    "${DEFAULT_INITD_DIR}"   "Installation directory for compatible initd scripts" )
	set_install_path( INSTALL_MUX_DIR      "${DEFAULT_MUX_DIR}"     "Installation prefix directory for MUX" )
endif ( NOT WIN32 )


# Installs a shortcut to the file into the start menu if the specified files
# are installed on the target system.
# ARGS args        (optional) arguments for the shortcut
# DESCRIPTION desc (optional) description for the shortcut
# ADD_TO_DESKTOP   (optional) add as a desktop icon
# DESTINATION dest (optional) desination for the shortcut under the start menu
# FILES ...        list of files to add shortcuts for
# TARGETS ...      list of targets to add shortcuts for
# ...              defaults to the TARGETS list
macro( INSTALL_SHORTCUT )
	cmake_parse_arguments( INSTALL "ADD_TO_DESKTOP" "ARGS;DESCRIPTION;DESTINATION"
		"FILES;TARGETS" ${ARGN} )
	set( INSTALL_TARGETS ${INSTALL_TARGETS} ${INSTALL_UNPARSED_ARGUMENTS} )

	set( INSTALL_JUMP 2 )
	set( INSTALL_DIR "$SMPROGRAMS\\$STARTMENU_FOLDER" )
	set( UNINSTALL_DIR "$SMPROGRAMS\\$MUI_TEMP" )
	if ( INSTALL_DESTINATION )
		file( TO_NATIVE_PATH    "${INSTALL_DESTINATION}" INSTALL_DESTINATION )
		set( INSTALL_DIR        "${INSTALL_DIR}\\${INSTALL_DESTINATION}" )
		set( UNINSTALL_DIR      "${UNINSTALL_DIR}\\${INSTALL_DESTINATION}" )
		set( DIR_INSTALL_CMD    "CreateDirectory \"${INSTALL_DIR}\"\n    " )
		set( DIR_UNINSTALL_CMD  "RMDir /r \"${UNINSTALL_DIR}\"" )
		math( EXPR INSTALL_JUMP "${INSTALL_JUMP} + 1" )
	endif( INSTALL_DESTINATION )

	# Support for specifying targets instead of just files
	foreach ( TARGET ${INSTALL_TARGETS} )
		# Obtain the file name of the target (i.e. ${TARGET}.exe or
		#  ${TARGET}.dll or lib${TARGET}.so)
		get_target_property( TARGET_TYPE "${TARGET}" TYPE )
		set( TARGET_FILE_NAME "${INSTALL_BIN_DIR}/${CMAKE_${TARGET_TYPE}_PREFIX}${TARGET}${CMAKE_${TARGET_TYPE}_SUFFIX}" )
		set( INSTALL_FILES ${INSTALL_FILES} "${TARGET_FILE_NAME}" )
	endforeach( TARGET )

	foreach ( FILE_PATH ${INSTALL_FILES} )
		get_filename_component( FILE_NAME_NO_EXT "${FILE_PATH}" NAME_WE )
		file( TO_NATIVE_PATH "${FILE_PATH}" FILE_PATH )

		# Add installation commands
		set( CPACK_NSIS_CREATE_ICONS_EXTRA
			${CPACK_NSIS_CREATE_ICONS_EXTRA}
			"  IfFileExists \"$INSTDIR\\${FILE_PATH}\" 0 +${INSTALL_JUMP}"
			"    ${DIR_INSTALL_CMD}CreateShortCut \"${INSTALL_DIR}\\${FILE_NAME_NO_EXT}.lnk\" \"$INSTDIR\\${FILE_PATH}\" \"${INSTALL_ARGS}\" \"\" 0 \"\" \"\" \"${INSTALL_DESCRIPTION}\""
			)
		set( CPACK_NSIS_DELETE_ICONS_EXTRA
			"  Delete \"${UNINSTALL_DIR}\\${FILE_NAME_NO_EXT}.lnk\""
			"  ${DIR_UNINSTALL_CMD}"
			${CPACK_NSIS_DELETE_ICONS_EXTRA}
			)
		if ( INSTALL_ADD_TO_DESKTOP )
			set( CPACK_NSIS_CREATE_ICONS_EXTRA
				${CPACK_NSIS_CREATE_ICONS_EXTRA}
				"  StrCmp $INSTALL_DESKTOP \"1\" 0 +3"
				"    IfFileExists \"$INSTDIR\\${FILE_PATH}\" 0 +2"
				"      CreateShortCut \"$DESKTOP\\${FILE_NAME_NO_EXT}.lnk\" \"$INSTDIR\\${FILE_PATH}\" \"${INSTALL_ARGS}\" \"\" 0 \"\" \"\" \"${INSTALL_DESCRIPTION}\""
			)
			set( CPACK_NSIS_DELETE_ICONS_EXTRA
				${CPACK_NSIS_DELETE_ICONS_EXTRA}
				"  StrCmp $INSTALL_DESKTOP \"1\" 0 +2"
				"    Delete \"$DESKTOP\\${FILE_NAME_NO_EXT}.lnk\""
			)
		endif( INSTALL_ADD_TO_DESKTOP )
	endforeach( FILE_PATH )
endmacro( INSTALL_SHORTCUT )

# Support function for adding rules to an installer to replace text inside an
# installed file after installation
#
# Parameters:
#    _FILE         installed file to perform replacement within
#    _STR          string to replace
#    _REPLACE      replacement string (use CMAKE_INSTALL_PREFIX for
#                  to replace with the user-specifed installation path)
#    ...           (optional) list of components that will include this rule
#
function( INSTALL_REPLACE_IN_FILE _FILE _STR _REPLACE )
	set( _NSIS_FILE "${_FILE}" )
	set( _RPM_FILE "${_FILE}" )
	set( _COMPONENTS ${ARGN} )

	# If not an absolute installation, then make it one
	if ( NOT IS_ABSOLUTE "${_FILE}" )
		set( _NSIS_FILE "$INSTDIR\\${_FILE}" )
		set( _RPM_FILE "$RPM_INSTALL_PREFIX/${_FILE}" )
	endif ( NOT IS_ABSOLUTE "${_FILE}" )

	if ( CMAKE_INSTALL_PREFIX )
		string( REPLACE "${CMAKE_INSTALL_PREFIX}" "$INSTDIR" _NSIS_REPLACE "${_REPLACE}" )
		string( REPLACE "${CMAKE_INSTALL_PREFIX}" "$RPM_INSTALL_PREFIX" _RPM_REPLACE "${_REPLACE}" )
	else()
		set( _NSIS_REPLACE "${_REPLACE}" )
		set( _RPM_REPLACE "${_REPLACE}" )
	endif ( CMAKE_INSTALL_PREFIX )

	# NSIS Support
	if ( NOT "${_STR}" STREQUAL "${_NSIS_REPLACE}" )
		file( TO_NATIVE_PATH "${_NSIS_FILE}" _NSIS_FILE )
		string( REGEX REPLACE "[^A-Za-z0-9]" "_" NSIS_FILE_TAG "${_FILE}" )
		set( CPACK_NSIS_EXTRA_INSTALL_COMMANDS
			"  IfFileExists \"${_NSIS_FILE}\" 0 ${NSIS_FILE_TAG}_not_found"
			"    !insertMacro _ReplaceInFile \"${_NSIS_FILE}\" \"${_STR}\" \"${_NSIS_REPLACE}\""
			"    Delete \"${_NSIS_FILE}.old\""
			"${NSIS_FILE_TAG}_not_found:"
			${CPACK_NSIS_EXTRA_INSTALL_COMMANDS} PARENT_SCOPE )
	endif ( NOT "${_STR}" STREQUAL "${_NSIS_REPLACE}" )

	# Relocatable RPM support
	if ( NOT "${_STR}" STREQUAL "${_RPM_REPLACE}" )
		if ( NOT _COMPONENTS )
			set( _COMPONENTS "@" )
		endif( NOT _COMPONENTS )
		foreach( _COMPONENT ${_COMPONENTS} )
			set( COMPONENT "_${_COMPONENT}" )
			string( REPLACE "_@" "" COMPONENT "${COMPONENT}" )
			file( APPEND "${CMAKE_BINARY_DIR}/CMakeFiles/postinst${COMPONENT}_rpm"
				"sed -i \"${_RPM_FILE}\" -e \"s|${_STR}|${_RPM_REPLACE}|g\"\n" )
		endforeach( _COMPONENT )
	endif ( NOT "${_STR}" STREQUAL "${_RPM_REPLACE}" )
endfunction( INSTALL_REPLACE_IN_FILE )

# The installer commands sets various variables that must be promoted up the
# CMakeLists.txt directory chain.  This macro supports setting the required
# variables up the chain.
# Example usage:
#   install_set_variables( PARENT_SCOPE )
macro( INSTALL_SET_VARIABLES )
	set( CPACK_NSIS_CREATE_ICONS_EXTRA
		${CPACK_NSIS_CREATE_ICONS_EXTRA} ${ARGN} )
	set( CPACK_NSIS_DELETE_ICONS_EXTRA
		${CPACK_NSIS_DELETE_ICONS_EXTRA} ${ARGN} )
	set( CPACK_NSIS_EXTRA_INSTALL_COMMANDS
		${CPACK_NSIS_EXTRA_INSTALL_COMMANDS} ${ARGN} )
	set( CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS
		${CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS} ${ARGN} )
	set( CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
		${CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS} ${ARGN} )
endmacro( INSTALL_SET_VARIABLES )

