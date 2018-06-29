/**
 * @file
 * @brief source file containing integration tests for enviroment variable
 *        functions
 *
 * @copyright Copyright (C) 2018 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied."
 */

#include <os.h>

#include "test_support.h"

#if defined( _WIN32 )
#define WIN32_LEAN_AND_MEAN
#include <Windows.h> /* for SetEnvironmentVariable() function */
#else /* if defined( _WIN32 ) */
#include <stdlib.h> /* for system() */
#endif /* else if defined( _WIN32 ) */

/* internal main function to test @p fptr function */
static int test_run_main_func( int argc, char *argv[] )
{
	printf( "%s\n", argv[0] );
	return 255; /* should be a value: 0 - 255 */
}


/* test os_system_run */
static void test_os_system_run_complex_cmd_background( void **state )
{
	os_status_t rv;
	os_system_run_args_t args = OS_SYSTEM_RUN_ARGS_INIT;

	args.cmd = "echo \"this is a test\"";
	rv = os_system_run( &args );
	assert_int_equal( rv, OS_STATUS_INVOKED );
	assert_int_equal( args.return_code, 0 );
}

static void test_os_system_run_complex_cmd_exit_code( void **state )
{
	char std_out[64u];
	os_status_t rv;
	os_system_run_args_t args = OS_SYSTEM_RUN_ARGS_INIT;

	args.cmd = "echo this is a test";
	args.block = OS_TRUE;
	args.opts.block.std_out.buf = std_out;
	args.opts.block.std_out.len = 64u;
	rv = os_system_run( &args );
	assert_int_equal( rv, OS_STATUS_SUCCESS );
	assert_int_equal( args.return_code, 0 );
#if defined( _WIN32 )
	assert_string_equal( std_out, "this is a test\r\n" );
#else /* if defined( _WIN32 ) */
	assert_string_equal( std_out, "this is a test\n" );
#endif /* else if defined( _WIN32 ) */
}

static void test_os_system_run_complex_privileged_cmd( void **state )
{
	char std_out[64u];
	int expected_rc = 0;
	os_status_t rv;
	os_system_run_args_t args = OS_SYSTEM_RUN_ARGS_INIT;

	/* determine if we can run a priviledged command */
	/* priviledged is currently only supported on linux (via sudo) */
#if defined( __linux__ )
	if ( system( "sudo -v" ) != 0 )
		expected_rc = 1;
#endif /* if defined( __linux__ ) */

	args.cmd = "echo this is a test";
	args.block = OS_TRUE;
	args.privileged = OS_TRUE;
	args.opts.block.std_out.buf = std_out;
	args.opts.block.std_out.len = 64u;
	rv = os_system_run( &args );
	assert_int_equal( rv, OS_STATUS_SUCCESS );
	assert_int_equal( args.return_code, expected_rc );
	if ( expected_rc == 0 )
	{
#if defined( _WIN32 )
		assert_string_equal( std_out, "this is a test\r\n" );
#else /* if defined( _WIN32 ) */
		assert_string_equal( std_out, "this is a test\n" );
#endif /* else if defined( _WIN32 ) */
	}
}

static void test_os_system_run_fptr( void **state )
{
	char std_out[64u];
	os_status_t rv;
	os_system_run_args_t args = OS_SYSTEM_RUN_ARGS_INIT;

	args.fptr = test_run_main_func;
	args.cmd = "test_run_main_func running!";
	args.block = OS_TRUE;
	args.opts.block.std_out.buf = std_out;
	args.opts.block.std_out.len = 64u;
	rv = os_system_run( &args );

#if defined( _WIN32 )
	assert_int_equal( rv, OS_STATUS_NOT_SUPPORTED );
	assert_int_equal( args.return_code, -1 );
#else /* if defined( _WIN32 ) */
	assert_int_equal( rv, OS_STATUS_SUCCESS );
	assert_int_equal( args.return_code, 255 );
	assert_string_equal( std_out, "test_run_main_func running!\n" );
#endif /* else if defined( _WIN32 ) */
}

static void test_os_system_run_simple_cmd_background( void **state )
{
	os_status_t rv;
	os_system_run_args_t args = OS_SYSTEM_RUN_ARGS_INIT;

	args.cmd = "echo test";
	rv = os_system_run( &args );
	assert_int_equal( rv, OS_STATUS_INVOKED );
	assert_int_equal( args.return_code, 0 );
}

static void test_os_system_run_simple_cmd_exit_code( void **state )
{
	char std_out[64u];
	os_status_t rv;
	os_system_run_args_t args = OS_SYSTEM_RUN_ARGS_INIT;

	args.cmd = "echo test";
	args.block = OS_TRUE;
	args.opts.block.std_out.buf = std_out;
	args.opts.block.std_out.len = 64u;
	rv = os_system_run( &args );
	assert_int_equal( rv, OS_STATUS_SUCCESS );
	assert_int_equal( args.return_code, 0 );
#if defined( _WIN32 )
	assert_string_equal( std_out, "test\r\n" );
#else /* if defined( _WIN32 ) */
	assert_string_equal( std_out, "test\n" );
#endif /* else if defined( _WIN32 ) */
}

static void test_os_system_run_max_wait_timeout_exceeded( void **state )
{
	os_status_t rv;
	os_system_run_args_t args = OS_SYSTEM_RUN_ARGS_INIT;

	/* create a process that will take 5 seconds */
#if defined( _WIN32 )
	args.cmd = "timeout 5";
#else /* if defined( _WIN32 ) */
	args.cmd = "sleep 5";
#endif /* else if defined( _WIN32 ) */
	args.block = OS_TRUE;
	args.opts.block.max_wait_time = 2000u; /* kill after 2000 milliseconds (2 secs) */
	rv = os_system_run( &args );
	assert_int_equal( rv, OS_STATUS_TIMED_OUT );
	assert_int_equal( args.return_code, -1 );
}

static void test_os_system_run_max_wait_timeout_not_hit( void **state )
{
	os_status_t rv;
	os_system_run_args_t args = OS_SYSTEM_RUN_ARGS_INIT;

	/* create a process that will take 5 seconds */
#if defined( _WIN32 )
	args.cmd = "timeout 1";
#else /* if defined( _WIN32 ) */
	args.cmd = "sleep 1";
#endif /* else if defined( _WIN32 ) */
	args.block = OS_TRUE;
	args.opts.block.max_wait_time = 2000u; /* kill after 2000 milliseconds (2 secs) */
	rv = os_system_run( &args );
	assert_int_equal( rv, OS_STATUS_SUCCESS );
	assert_int_equal( args.return_code, 0 );
}

static void test_os_system_run_parameter_blank( void **state )
{
	os_status_t rv;
	os_system_run_args_t args = OS_SYSTEM_RUN_ARGS_INIT;

	rv = os_system_run( &args );
	assert_int_equal( rv, OS_STATUS_BAD_PARAMETER );
}

static void test_os_system_run_parameter_none( void **state )
{
	os_status_t rv;

	rv = os_system_run( NULL );
	assert_int_equal( rv, OS_STATUS_BAD_PARAMETER );
}

int main( int argc, char *argv[] )
{
	int result;
	const struct CMUnitTest tests[] = {
		cmocka_unit_test( test_os_system_run_complex_cmd_background ),
		cmocka_unit_test( test_os_system_run_complex_cmd_exit_code ),
		cmocka_unit_test( test_os_system_run_complex_privileged_cmd ),
		cmocka_unit_test( test_os_system_run_fptr ),
		cmocka_unit_test( test_os_system_run_simple_cmd_background ),
		cmocka_unit_test( test_os_system_run_simple_cmd_exit_code ),
		cmocka_unit_test( test_os_system_run_max_wait_timeout_exceeded ),
		cmocka_unit_test( test_os_system_run_max_wait_timeout_not_hit ),
		cmocka_unit_test( test_os_system_run_parameter_blank ),
		cmocka_unit_test( test_os_system_run_parameter_none )
	};

	test_initialize( argc, argv );
	result = cmocka_run_group_tests( tests, NULL, NULL );
	test_finalize( argc, argv );
	return result;
}

