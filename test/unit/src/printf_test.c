/**
 * @file
 * @brief unit testing for OSAL library (printf related functions)
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

static void test_printf( void **state )
{
	/** @todo implement this with mocking */
}

static void test_fprintf( void **state )
{
	/** @todo implement this with mocking */
}

static void test_snprintf( void **state )
{
	char buf[512u];
	int result;

	result = os_snprintf( NULL, 0u, NULL );
	assert_int_equal( result, -1 );

	result = os_snprintf( NULL, 0u, "this is a test" );
	assert_int_equal( result, -1 );

	result = os_snprintf( buf, sizeof(buf), "replacement %s", "test" );
	assert_int_equal( result, 16 );
	assert_string_equal( buf, "replacement test" );

	result = os_snprintf( buf, 10, "replacement %s", "test" );
	assert_int_equal( result, -1 );
	assert_string_equal( buf, "replaceme" );

	result = os_snprintf( buf, 10, "%u != %d", 1u, -2 );
	assert_int_equal( result, 7 );
	assert_string_equal( buf, "1 != -2" );
}

static void test_vfprintf( void **state )
{
	/** @todo implement this with mocking */
}

static void test_vsnprintf( void **state )
{
	/** @todo implement this with mocking */
}

int main( int argc, char *argv[] )
{
	int result;
	const struct CMUnitTest tests[] = {
		cmocka_unit_test( test_printf ),
		cmocka_unit_test( test_fprintf ),
		cmocka_unit_test( test_snprintf ),
		cmocka_unit_test( test_vfprintf ),
		cmocka_unit_test( test_vsnprintf ),
	};

	test_initialize( argc, argv );
	result = cmocka_run_group_tests( tests, NULL, NULL );
	test_finalize( argc, argv );
	return result;
}
