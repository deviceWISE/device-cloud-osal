#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

# Helper function to determine if a defintion is set in the list of definitions
#
# Parameters:
#	_def_list - list containing definitions
#	_def_to_find - definition to find
#	_output_var - output variable (-1 if not found)
function( _STRIP_C_FIND _def_list _def_to_find _output_var )
	set( _result -1 )
	set( _index 0 )
	foreach( _def ${${_def_list}} )
		string( REGEX MATCH "[a-zA-Z_][a-zA-Z0-9_]*" _key "${_def}" )
		if( _key STREQUAL _def_to_find )
			set( _result ${_index} )
			break()
		endif()
		math( EXPR _index "${_index} + 1" )
	endforeach( _def )
	set( ${_output_var} ${_result} PARENT_SCOPE )
endfunction( _STRIP_C_FIND )

# Helper function to find the value for a symbol
#
# Parameters:
#	_def_list - list containing definitions
#	_def_to_find - definition to find
#	_output_var - output variable (0 if not found)
function( _STRIP_C_FIND_VALUE _def_list _def_to_find _output_var )
	set( _result 0 )
	set( _value 1 ) # default value for a macro on command line
	foreach( _def ${${_def_list}} )
		string( REGEX MATCH "[a-zA-Z_][a-zA-Z0-9_]*" _key "${_def}" )
		string( REGEX REPLACE ".*=" "" _value "${_def}" )
		if( _key STREQUAL _def_to_find )
			set( _result "${_value}" )
			break()
		endif()
	endforeach( _def )
	set( ${_output_var} ${_result} PARENT_SCOPE )
endfunction( _STRIP_C_FIND_VALUE )

# Helper function to evaluate an expression
#
# Parameters:
#	_output_var - output variable (0 if not found)
macro( _STRIP_C_EVAL _output_var )
	if( ${ARGN} )
		set( ${_output_var} ON )
	else( ${ARGN} )
		set( ${_output_vaR} OFF )
	endif()
endmacro( _STRIP_C_EVAL )

# Function to support stripping C preprocessor conditional statements from a
# file based on the (-D definitions) passed to the file.
#
# Note: This function does not attempt to open files defined by #include
# statements to find additional definitions
#
# Parameters:
#	IN_FILE - path to the source file
#	DEFS    - definitions to use (without -D)... usually obtained from
#	          get_directory_property( var DIRECTORY "." COMPILE_DEFINITIONS )
#	OUTPUT_VAR - path to the variable to save the output to
function( STRIP_C_DEFINES IN_FILE DEFS OUTPUT_VAR )
	# Get list of definitions
	set( _defs ${DEFS} )

	set( _if_match_0 ON ) # Enable root depth level
	set( _if_level 0 ) # current if level
	set( _if_pass 0 ) # last if depth level that passed
	set( _output_line ON )
	set( _output_next_line ON )

	# Read the file
	file( STRINGS "${IN_FILE}" _contents )
	string( REPLACE ";;" ";\n;" _contents "${_contents}" )
	foreach( _line ${_contents} )
		# handle #define statement
		if ( _line MATCHES "^[ \t]*#[ \t]*define[ \t]+([A-Za-z0-9_]*)" )
			if ( ${_if_level} EQUAL ${_if_pass} )
				list( APPEND _defs "${CMAKE_MATCH_1}" )
			endif()
		# handle ifdef/ifndef statements
		elseif ( _line MATCHES "^[ \t]*#[ \t]*if(n)?def[ \t]+([a-zA-Z0-9_]+)" )
			if ( ${_if_level} EQUAL ${_if_pass} )
				set( _output_line OFF )
				if ( _if_match_${_if_level} )
					set( _if_true OFF )
					if ( CMAKE_MATCH_1 STREQUAL "n" )
						set( _if_true ON )
					endif()
					_strip_c_find( _defs "${CMAKE_MATCH_2}" _def_found )
					if ( _def_found GREATER -1 )
						if ( CMAKE_MATCH_1 STREQUAL "n" )
							set( _if_true OFF )
						else()
							set( _if_true ON )
						endif()
					endif()

					if ( _if_true )
						math( EXPR _if_pass "${_if_level} + 1" )
						if ( _if_pass EQUAL 0 )
							message( FATAL_ERROR "Internal error: _if_match being set for root" )
						endif()
						set( "_if_match_${_if_pass}" ON )
					endif()
					set( _output_next_line ${_if_true} )
				endif()
			endif()
			math( EXPR _if_level "${_if_level} + 1" )
			set( "_if_match_${_if_level}" OFF )

		# handle if/elif statements
		elseif ( _line MATCHES "^[ \t]*#[ \t]*(if|elif)(.*)$" )
			set( _if_type "${CMAKE_MATCH_1}" )
			set( _if_expr "${CMAKE_MATCH_2}" )

			# Remove any trailing comments from line
			string( REGEX REPLACE "/[*/](.*)$" "" _if_expr "${_if_expr}" )

			# handle "defined( ) && 1 || defined( .. )" syntax
			set( _output_line OFF )
			math( EXPR _if_level1 "${_if_level} + 1" )
			set( _if_true OFF )
			if ( ${_if_level} EQUAL ${_if_pass} )
				if ( _if_match_${_if_level} )
					# split at "||" marks
					string( REPLACE "||" ";" _if_exprs "${_if_expr}" )
					foreach( _expr ${_if_exprs} )
						set( _if_good ON )
						string( REPLACE "&&" ";" _ex "${_expr}" )
						foreach( _e ${_ex} )
							string( STRIP "${_e}" _e )
							set( _if_not OFF )
							if ( _e MATCHES "^!" )
								set( _if_not ON )
								string( REGEX REPLACE "![ \t]*" "" _e "${_e}" )
							endif()

							if ( _e STREQUAL "0" )
								# handle "if 0" case
								set( _if_good OFF )
							elseif( _e MATCHES "defined[( \t][ \t]*([a-zA-Z_][a-zA-Z0-9_]*)[ \t]*[ \t)]" )
								# handle "if defined(...)" or "if defined ..." statements
								set( _if_good OFF )
								_strip_c_find( _defs "${CMAKE_MATCH_1}" _def_found )
								if ( _def_found GREATER -1 )
									set( _if_good ON )
								endif()
							else()
								# handle compound "if VAR > 255" statements
								string( REGEX REPLACE "[ \t]+" ";" _e_split "${_e}" )
								set( _e )
								set( _add_or OFF )
								foreach( _count 0 1 )
									foreach( _e_pos ${_e_split} )
										if( _e_pos MATCHES "([a-zA-Z_][a-zA-Z0-9_]*)" )
											_strip_c_find_value( _defs "${CMAKE_MATCH_0}" _def_value )
											string( REGEX REPLACE "[a-zA-Z_][a-zA-Z0-9_]*" "${_def_value}" _e_pos "${_e_pos}" )
										elseif ( _e_pos STREQUAL ">" )
											set( _e_pos "GREATER" )
										elseif ( _e_pos STREQUAL "<" )
											set( _e_pos "LESS" )
										elseif( _e_pos STREQUAL "==" )
											set( _e_pos "EQUAL" )
										elseif( _e_pos STREQUAL ">=" OR _e_pos STREQUAL "<=" )
											if( _count EQUAL 1 ) # Handle 2nd part for >= and <= case
												set( _e_pos "EQUAL" )
											elseif( _e_pos STREQUAL ">=" )
												set( _e_pos "GREATER" )
												set( _add_or ON )
											elseif( _e_pos STREQUAL "<=" )
												set( _e_pos "LESS" )
												set( _add_or ON )
											endif()
										endif()
										list( APPEND _e "${_e_pos}" )
									endforeach( _e_pos )
									if ( _add_or )
										list( APPEND _e "OR" )
										set( _add_or OFF )
									else()
										break()
									endif()
								endforeach( _count )
								set( _if_good OFF )
								_strip_c_eval( _if_good ${_e} )
							endif()

							# not (!) expression, so let's reverse result
							if( _if_not )
								if ( _if_good )
									set( _if_good OFF )
								else()
									set( _if_good ON )
								endif()
							endif()
						endforeach( _e )

						if ( _if_good )
							set( _if_true ON )
						endif()
					endforeach( _expr )

					if ( _if_true )
						math( EXPR _if_pass "${_if_level} + 1" )
					endif()
				endif()
			endif()
			set( _output_next_line ${_if_true} )
			if ( _if_type STREQUAL "if" )
				math( EXPR _if_level "${_if_level} + 1" )
				set( _if_match_${_if_level} ${_if_true} )
			endif()

		# handle else condition
		elseif ( _line MATCHES "^[ \t]*#[ \t]*else" )
			math( EXPR _if_pass_1 "${_if_pass} + 1" )
			set( _output_line OFF )
			if ( ${_if_level} EQUAL ${_if_pass_1} )
				if ( NOT _if_match_${_if_level} )
					set( _if_pass "${_if_level}" )
					set( _if_match_${_if_level} ON )
					set( _output_next_line ON )
				else()
					set( _output_next_line OFF )
				endif()
			else()
				set( _output_next_line OFF )
			endif()

		# handle endif condition
		elseif ( _line MATCHES "^[ \t]*#[ \t]*endif" )
			set( _output_line OFF )
			if ( _if_level EQUAL 0 )
				message( FATAL_ERROR "Internal error: _if_level already at root" )
			endif()
			set( _if_match_${_if_level} OFF )
			math( EXPR _if_level "${_if_level} - 1" )
			if ( _if_level EQUAL _if_pass OR _if_pass GREATER _if_level )
				set( _output_next_line ON )
				math( EXPR _if_pass "${_if_level}" )
			elseif ( _if_level EQUAL 0 )
				message( FATAL_ERROR "Internal error: at root level and inside if statement" )
			endif()
		endif()

		if ( _output_line )
			if ( _line STREQUAL "\n" )
				set( _line "" )
			endif()
			set( _output "${_output}${_line}\n" )
		endif( _output_line )
		set( _output_line ${_output_next_line} )
	endforeach( _line )
	set( "${OUTPUT_VAR}" "${_output}" PARENT_SCOPE )
endfunction( STRIP_C_DEFINES )

