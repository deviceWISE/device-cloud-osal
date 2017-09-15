#
# Copyright (C) 2015-2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

include( CMakeParseArguments )
find_package( Cmocka )
if ( CMOCKA_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test" )
	enable_testing()
	add_custom_target( tests
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
	)
	if ( NOT CMAKE_CROSSCOMPILING )
		add_custom_target( check
			DEPENDS tests
			COMMAND ${CMAKE_CTEST_COMMAND} -C ${CMAKE_CFG_INTDIR} --output-on-failure
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
		)

		# Setup wrapper scripts to help obtain unit test count numbers
		set( COUNT_SCRIPT "test-case-count.py" )
		set( COUNT_SCRIPT_DIR "${CMAKE_CURRENT_LIST_DIR}" )
		if( NOT EXISTS "${COUNT_SCRIPT_DIR}/${COUNT_SCRIPT}.in" AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/../scripts/${COUNT_SCRIPT}.in" )
			set( COUNT_SCRIPT_DIR "${CMAKE_CURRENT_LIST_DIR}/../scripts" )
		endif( NOT EXISTS "${COUNT_SCRIPT_DIR}/${COUNT_SCRIPT}.in" AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/../scripts/${COUNT_SCRIPT}.in" )
		# Create a temporary script by filing in the proper variable values
		configure_file( "${COUNT_SCRIPT_DIR}/${COUNT_SCRIPT}.in" "${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/${COUNT_SCRIPT}" )
		# Ensure that the script has executable permissions
		file( COPY
			"${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/${COUNT_SCRIPT}"
			DESTINATION "${CMAKE_BINARY_DIR}"
			FILE_PERMISSIONS
			OWNER_READ OWNER_WRITE OWNER_EXECUTE
			GROUP_READ GROUP_EXECUTE
			WORLD_READ WORLD_EXECUTE
		)
		add_custom_target( check_count
			DEPENDS tests
			COMMAND ${CMAKE_BINARY_DIR}/${COUNT_SCRIPT} --dir="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}"
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
		)

		# include code coverage support
		include( CodeCoverage )
	endif ( NOT CMAKE_CROSSCOMPILING )
	set( INT_TESTS "" CACHE INTERNAL "list of integration tests" )
	include_directories( SYSTEM ${CMOCKA_INCLUDES} )
	add_subdirectory( "test" )

	# add integration tests
	if ( INT_TESTS )
		add_custom_target( "integration-tests" )

		# On Windows, copy the CMocka .dll to working directory
		if ( MSVC )
			get_filename_component( CMOCKA_LIB_DIR
				"${CMOCKA_LIBRARY}" DIRECTORY )
			find_file( CMOCKA_DLL "cmocka.dll"
				HINTS "${CMOCKA_LIB_DIR}"
				      "{CMOCKA_LIB_DIR}/../bin"
				NO_DEFAULT_PATH
			)
			if ( CMOCKA_DLL )
				add_custom_command( TARGET "integration-tests"
					COMMAND ${CMAKE_COMMAND} -E copy_if_different
						"${CMOCKA_DLL}"
						"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/cmocka.dll"
				)
			endif( CMOCKA_DLL )
		endif( MSVC )

		# Add each test file
		foreach( INT_TEST ${INT_TESTS} )
			add_custom_command( TARGET "integration-tests"
				COMMAND ${INT_TEST} ${INT_TEST_${INT_TEST}_ARGS}
				DEPENDS "${INT_TEST}"
			)
		endforeach( INT_TEST )
	endif( INT_TESTS )
endif()

# Adds a new integration test
# Parameters:
# 	TEST_NAME - name of the test
# 	DEFS ... - list of definitions
# 	INCS ... - list of include directories
# 	LIBS ... - list of libraries for linking
# 	SRCS ... - list of source files
function( ADD_INTEGRATION_TEST TEST_NAME )
	# Loop to accept multiple test run variables (TEST1...TEST9)
	set( TEST_GENERATED_FILES )
	set( TEST_MAX 10 )
	set( TEST_MIN 1 )
	set( TEST_ARGS )
	while( ${TEST_MIN} LESS ${TEST_MAX} )
		set( TEST_ARGS ${TEST_ARGS} "TEST${TEST_MIN}" )
		math( EXPR TEST_MIN "${TEST_MIN} + 1" )
	endwhile( ${TEST_MIN} LESS ${TEST_MAX} )

	# Parse arguments passed to the function
	cmake_parse_arguments( INT_TEST "" "" "DEFS;INCS;LIBS;SRCS;${TEST_ARGS}" ${ARGN} )
	set( INT_TEST_SRCS ${INT_TEST_SRCS} ${INT_TEST_UNPARSED_ARGUMENTS} )

	# Add integration tests
	add_executable( "${TEST_NAME}" EXCLUDE_FROM_ALL ${INT_TEST_SRCS} )
	set( INT_TESTS ${INT_TESTS} "${TEST_NAME}" CACHE INTERNAL "list of integration tests" )

	# Add include directories
	if ( INT_TEST_INCS )
		target_include_directories( "${TEST_NAME}" SYSTEM
			PUBLIC ${INT_TEST_INCS} )
	endif( INT_TEST_INCS )

	# Add test library
	add_dependencies( tests "${TEST_NAME}" )
	target_link_libraries( "${TEST_NAME}" test-support ${INT_TEST_LIBS} )

	# Add defintions, if required
	if ( INT_TEST_DEFS )
		string( REGEX REPLACE "^-D" "" INT_TEST_DEFS ${INT_TEST_DEFS} )
		get_target_property( COMPILE_DEFS "${TEST_NAME}"
			COMPILE_DEFINITIONS )
		set( COMPILE_DEFS ${COMPILE_DEFS} "${INT_TEST_DEFS}" )
		set_target_properties( "${TEST_NAME}"
			PROPERTIES COMPILE_DEFINITIONS "${COMPILE_DEFS}" )
	endif ( INT_TEST_DEFS )
endfunction( ADD_INTEGRATION_TEST )

# Adds multiple integration tests
# Parameters:
# 	group - grouping name
# 	... - name of variables holding test information, in the form:
# 		TEST_${NAME}_ARGS - arguments to pass to each test
# 		TEST_${NAME}_DEFS - definitions to pass to each test
# 		TEST_${NAME}_INCS - system include directories to compile with
# 		TEST_${NAME}_LIBS - libraries to link the test executable with
# 		TEST_${NAME}_SRCS - test source files
# 		TEST_${NAME}_TEST1..9 - run a test multiple times, passing
# 		                        different arguments each time
macro( ADD_INTEGRATION_TESTS GROUP )
	foreach( TEST ${ARGN} )
		string( TOUPPER "${TEST}" TEST_UPPER )
		set( TEST_NAME "integration-${TEST}" )
		if ( NOT "x${GROUP}" STREQUAL "x" )
			set( TEST_NAME "integration-${GROUP}-${TEST}" )
		endif ( NOT "x${GROUP}" STREQUAL "x" )
		set( TEST_MAX 10 )
		set( TEST_MIN 1 )
		set( TEST_ARGS )
		while( ${TEST_MIN} LESS ${TEST_MAX} )
			if ( TEST_${TEST_UPPER}_TEST${TEST_MIN} )
				set ( TEST_ARGS ${TEST_ARGS} "TEST${TEST_MIN}" "${TEST_${TEST_UPPER}_TEST${TEST_MIN}}" )
			endif ( TEST_${TEST_UPPER}_TEST${TEST_MIN} )
			math( EXPR TEST_MIN "${TEST_MIN} + 1" )
		endwhile( ${TEST_MIN} LESS ${TEST_MAX} )
		add_integration_test( ${TEST_NAME}
			DEFS ${TEST_${TEST_UPPER}_DEFS}
			INCS ${TEST_${TEST_UPPER}_INCS}
			LIBS ${TEST_${TEST_UPPER}_LIBS}
			SRCS ${TEST_${TEST_UPPER}_SRCS}
			${TEST_ARGS}
		)
	endforeach( TEST )
endmacro( ADD_INTEGRATION_TESTS )

# Create a new library for testing functionality
# Parameters:
# 	LIB_NAME        - name of the mocking library
# 	INCLUDES ...    - additional include directories
# 	LIBS ...        - additional libraries to link with
#	SRCS ...        - source files to use to create the library (default)
function( ADD_TEST_LIBRARY LIB_NAME )
	cmake_parse_arguments( TEST "" "" "INCLUDES;LIBS;SRCS" ${ARGN} )
	set( TEST_SRCS ${TEST_SRCS} ${TEST_UNPARSED_ARGUMENTS} )

	# Register the library with cmake
	add_library( "${LIB_NAME}" STATIC EXCLUDE_FROM_ALL ${TEST_SRCS} )

	if ( TEST_INCLUDES )
		target_include_directories( "${LIB_NAME}" SYSTEM
			PUBLIC ${TEST_INCLUDES} )
	endif( TEST_INCLUDES )

	if ( TEST_LIBS )
		target_link_libraries( "${LIB_NAME}" ${TEST_LIBS} )
	endif( TEST_LIBS )

	# Register gcov generated files with cmake to be cleaned on "make clean"
	get_directory_property( MAKE_CLEAN_FILES ADDITIONAL_MAKE_CLEAN_FILES )
	foreach ( SRC_FILE ${TEST_SRCS} )
		if ( IS_ABSOLUTE "${SRC_FILE}" )
			file( RELATIVE_PATH SRC_FILE "${CMAKE_CURRENT_SOURCE_DIR}" "${SRC_FILE}" )
			string( REPLACE "../" "__/" SRC_FILE "${SRC_FILE}" )
		endif ( IS_ABSOLUTE "${SRC_FILE}" )
		set( MAKE_CLEAN_FILES ${MAKE_CLEAN_FILES} "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${LIB_NAME}.dir/${SRC_FILE}.gcda" )
		set( MAKE_CLEAN_FILES ${MAKE_CLEAN_FILES} "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${LIB_NAME}.dir/${SRC_FILE}.gcno" )
	endforeach( SRC_FILE )
	set_directory_properties( PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${MAKE_CLEAN_FILES}" )
endfunction( ADD_TEST_LIBRARY )

# Adds a new unit test
# Parameters:
# 	TEST_NAME - name of the test
# 	DEFS ... - list of definitions
# 	INCS ... - list of include directories
# 	LIBS ... - list of libraries for linking
# 	MOCK ... - list of functions to wrap
# 	SRCS ... - list of source files
# 	UNIT ... - list of source files from the unit under test
# 	TEST1 ... - arguments to pass when running the first test
# 	...
# 	TEST9 ... - arguments to pass when running the ninth test
function( ADD_UNIT_TEST TEST_NAME )
	# Loop to accept multiple test run variables (TEST1...TEST9)
	set( TEST_GENERATED_FILES )
	set( TEST_MAX 10 )
	set( TEST_MIN 1 )
	set( TEST_ARGS )
	while( ${TEST_MIN} LESS ${TEST_MAX} )
		set( TEST_ARGS ${TEST_ARGS} "TEST${TEST_MIN}" )
		math( EXPR TEST_MIN "${TEST_MIN} + 1" )
	endwhile( ${TEST_MIN} LESS ${TEST_MAX} )

	# Parse arguments passed to the function
	cmake_parse_arguments( UNIT_TEST "" "" "DEFS;INCS;LIBS;MOCK;SRCS;UNIT;${TEST_ARGS}" ${ARGN} )
	set( UNIT_TEST_SRCS ${UNIT_TEST_SRCS} ${UNIT_TEST_UNPARSED_ARGUMENTS} )

	# Build the test application
	string( REPLACE "/test/" "/src/" SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" )
	string( REPLACE "/src/unit/src" "/src" SOURCE_DIR "${SOURCE_DIR}" )
	foreach( TEST_SRC ${UNIT_TEST_UNIT} )
		if( NOT TEST_SRC MATCHES "[$]<.*>" )
			set( TEST_SRC "${SOURCE_DIR}/${TEST_SRC}" )
		endif( NOT TEST_SRC MATCHES "[$]<.*>" )
		set( UNIT_TEST_SRCS ${UNIT_TEST_SRCS} ${TEST_SRC} )
	endforeach( TEST_SRC )

	# Determine full paths to files that will be generated by gcov with CMake
	foreach( TEST_SRC ${UNIT_TEST_SRCS} )
		if ( IS_ABSOLUTE "${TEST_SRC}" )
			file( RELATIVE_PATH TEST_SRC "${CMAKE_CURRENT_SOURCE_DIR}" "${TEST_SRC}" )
			string( REPLACE "../" "__/" TEST_SRC "${TEST_SRC}" )
		endif ( IS_ABSOLUTE "${TEST_SRC}" )
		set( TEST_GENERATED_FILES ${TEST_GENERATED_FILES} "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TEST_NAME}.dir/${TEST_SRC}.gcda" )
		set( TEST_GENERATED_FILES ${TEST_GENERATED_FILES} "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TEST_NAME}.dir/${TEST_SRC}.gcno" )
	endforeach( TEST_SRC )

	include_directories( "${SOURCE_DIR}" )
	add_executable( "${TEST_NAME}" EXCLUDE_FROM_ALL ${UNIT_TEST_SRCS} )

	# Add include directories
	if ( UNIT_TEST_INCS )
		target_include_directories( "${TEST_NAME}" SYSTEM
			PUBLIC ${UNIT_TEST_INCS} )
	endif( UNIT_TEST_INCS )

	# Support test specific definitions
	if ( UNIT_TEST_DEFS )
		string( REGEX REPLACE "^-D" "" UNIT_TEST_DEFS ${UNIT_TEST_DEFS} )
		get_target_property( CURRENT_COMPILE_DEFINITIONS "${TEST_NAME}" COMPILE_DEFINITIONS )
		if ( NOT CURRENT_COMPILE_DEFINITIONS )
			set( CURRENT_COMPILE_DEFINITIONS "${UNIT_TEST_DEFS}" )
		else()
			set( CURRENT_COMPILE_DEFINITIONS "${CURRENT_COMPILE_DEFINITIONS};${UNIT_TEST_DEFS}" )
		endif ( NOT CURRENT_COMPILE_DEFINITIONS )
		set_target_properties( "${TEST_NAME}" PROPERTIES COMPILE_DEFINITIONS "${CURRENT_COMPILE_DEFINITIONS}" )
	endif ( UNIT_TEST_DEFS )

	# Add library dependencies
	add_dependencies( tests "${TEST_NAME}" )
	target_link_libraries( "${TEST_NAME}" test-support ${UNIT_TEST_LIBS} )

	# Support wrapping with mock objects
	get_target_property( CURRENT_COMPILE_FLAGS "${TEST_NAME}" COMPILE_FLAGS )
	if ( NOT CURRENT_COMPILE_FLAGS )
		set( CURRENT_COMPILE_FLAGS "" )
	endif ( NOT CURRENT_COMPILE_FLAGS )
	get_target_property( CURRENT_LINK_FLAGS "${TEST_NAME}" LINK_FLAGS )
	if ( NOT CURRENT_LINK_FLAGS )
		set( CURRENT_LINK_FLAGS "" )
	endif ( NOT CURRENT_LINK_FLAGS )

	set( CURRENT_COMPILE_FLAGS "-DIOT_STATIC" )
	foreach( MOCK_FUNC ${UNIT_TEST_MOCK} )
		if ( CMAKE_C_COMPILER_ID MATCHES "(Clang|GNU)" )
			set( CURRENT_LINK_FLAGS "${CURRENT_LINK_FLAGS} -Wl,--wrap=${MOCK_FUNC}" )
		elseif ( CMAKE_C_COMPILER_ID MATCHES "MSVC" )
			set( CURRENT_COMPILE_FLAGS "${CURRENT_COMPILE_FLAGS} /D${MOCK_FUNC}=__wrap_${MOCK_FUNC}" )
		else()
			set( CURRENT_COMPILE_FLAGS "${CURRENT_COMPILE_FLAGS} -D${MOCK_FUNC}=__wrap_${MOCK_FUNC}" )
		endif ( CMAKE_C_COMPILER_ID MATCHES "(Clang|GNU)" )
	endforeach( MOCK_FUNC )
	if ( CMAKE_C_COMPILER_ID MATCHES "(Clang|GNU)" )
		set( CURRENT_COMPILE_FLAGS "${COMPILE_FLAGS} -Wno-unused-function -Wno-missing-prototypes -Wno-unused-parameter" )
	endif ( CMAKE_C_COMPILER_ID MATCHES "(Clang|GNU)" )
	set_target_properties( "${TEST_NAME}" PROPERTIES LINK_FLAGS "${CURRENT_LINK_FLAGS}" )
	set_target_properties( "${TEST_NAME}" PROPERTIES COMPILE_FLAGS "${CURRENT_COMPILE_FLAGS}" )

	# Add the tests to be run
	set( TEST_MIN 1 )
	set( ATLEAST_ONE_TEST OFF )
	while( ${TEST_MIN} LESS ${TEST_MAX} )
		if( UNIT_TEST_TEST${TEST_MIN} )
			add_test( NAME "${TEST_NAME}${TEST_MIN}"
				COMMAND ${TEST_NAME} ${UNIT_TEST_TEST${TEST_MIN}}
				WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
			)
			set( ATLEAST_ONE_TEST ON )
			# Add 10 second delay
			set_tests_properties( "${TEST_NAME}${TEST_MIN}" PROPERTIES TIMEOUT 10 )
			set_tests_properties( "${TEST_NAME}${TEST_MIN}" PROPERTIES RUN_SERIAL ON )
		endif( UNIT_TEST_TEST${TEST_MIN} )
		math( EXPR TEST_MIN "${TEST_MIN} + 1" )
	endwhile( ${TEST_MIN} LESS ${TEST_MAX} )

	# If not option (TEST1...TEST9) given just add a default option
	if ( NOT ATLEAST_ONE_TEST )
		add_test( NAME "${TEST_NAME}"
			COMMAND ${TEST_NAME}
			WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
		)
		set_tests_properties( "${TEST_NAME}" PROPERTIES TIMEOUT 10 )
		set_tests_properties( "${TEST_NAME}" PROPERTIES RUN_SERIAL ON )
	endif ( NOT ATLEAST_ONE_TEST )

	# Inform cmake of the generated files, so that they are cleaned with "make clean"
	get_directory_property( GENERATED_FILES ADDITIONAL_MAKE_CLEAN_FILES )
	set( GENERATED_FILES ${GENERATED_FILES} ${TEST_GENERATED_FILES} )
	set_directory_properties( PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${GENERATED_FILES}" )
endfunction( ADD_UNIT_TEST )

# Adds multiple unit tests
# Parameters:
# 	group - grouping name
# 	... - name of variables holding unit test information, in the form:
# 		TEST_${NAME}_DEFS - definitions to pass to each test
# 		TEST_${NAME}_INCS - system include directories to compile with
# 		TEST_${NAME}_LIBS - libraries to link the test executable with
# 		TEST_${NAME}_UNIT - source files in the unit under test
# 		TEST_${NAME}_SRCS - test source files
# 		TEST_${NAME}_MOCK - functions that are mocked within the test
# 		TEST_${NAME}_TEST1..9 - run a test multiple times, passing
# 		                        different arguments each time
macro( ADD_UNIT_TESTS GROUP )
	foreach( TEST ${ARGN} )
		string( TOUPPER "${TEST}" TEST_UPPER )
		set( TEST_NAME "unit-${TEST}" )
		if ( NOT "x${GROUP}" STREQUAL "x" )
			set( TEST_NAME "unit-${GROUP}-${TEST}" )
		endif ( NOT "x${GROUP}" STREQUAL "x" )
		set( TEST_MAX 10 )
		set( TEST_MIN 1 )
		set( TEST_ARGS )
		while( ${TEST_MIN} LESS ${TEST_MAX} )
			if ( TEST_${TEST_UPPER}_TEST${TEST_MIN} )
				set ( TEST_ARGS ${TEST_ARGS} "TEST${TEST_MIN}" "${TEST_${TEST_UPPER}_TEST${TEST_MIN}}" )
			endif ( TEST_${TEST_UPPER}_TEST${TEST_MIN} )
			math( EXPR TEST_MIN "${TEST_MIN} + 1" )
		endwhile( ${TEST_MIN} LESS ${TEST_MAX} )
		add_unit_test( ${TEST_NAME}
			DEFS ${TEST_${TEST_UPPER}_DEFS}
			INCS ${TEST_${TEST_UPPER}_INCS}
			LIBS ${TEST_${TEST_UPPER}_LIBS}
			MOCK ${TEST_${TEST_UPPER}_MOCK}
			SRCS ${TEST_${TEST_UPPER}_SRCS}
			UNIT ${TEST_${TEST_UPPER}_UNIT}
			${TEST_ARGS}
		)
	endforeach( TEST )
endmacro( ADD_UNIT_TESTS )

