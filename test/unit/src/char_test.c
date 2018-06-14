/**
 * @file
 * @brief unit testing for OSAL library (character related functions)
 *
 * @copyright Copyright (C) 2017-2018 Wind River Systems, Inc. All Rights Reserved.
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
#include <float.h> /* for DBL_EPSILON */
#include "test_support.h"

static void _assert_float_equal( const double a, const double b,
	const char *const file, const int line )
{
#define PRECISION (DBL_EPSILON)
	if ( !(((a) - PRECISION <= (b)) && ((a) + PRECISION >= (b))) )
	{
		/* error message */
		char err_msg[64u];
		sprintf( err_msg, "%.12f == %.12f", a, b );
		_assert_true( 0, err_msg, file, line );
	}
}

#define assert_float_equal(a, b) _assert_float_equal( a, b, __FILE__, __LINE__ )

static void test_atoi( void **state )
{
	/* zero */
	assert_int_equal( os_atoi( NULL ), 0 );
	assert_int_equal( os_atoi( "0" ), 0 );
	assert_int_equal( os_atoi( "abcdefghi" ), 0 );
	assert_int_equal( os_atoi( "abcd-10efghi" ), 0 );

	/* negative numbers */
	assert_int_equal( os_atoi( "-1" ), -1 );
	assert_int_equal( os_atoi( "-1234567890" ), -1234567890 );
	assert_int_equal( os_atoi( "1" ), 1 );
	assert_int_equal( os_atoi( "1234567890" ), 1234567890 );

	/* positive numbers */
	assert_int_equal( os_atoi( "-1.1234" ), -1 );
	assert_int_equal( os_atoi( "-1234567890.0" ), -1234567890 );
	assert_int_equal( os_atoi( "1.56789" ), 1 );
	assert_int_equal( os_atoi( "1234567890.43234" ), 1234567890 );
}

static void test_atof( void **state )
{
	/* zero */
	assert_float_equal( os_atof( NULL ), 0.0 );
	assert_float_equal( os_atof( "0" ), 0.0 );
	assert_float_equal( os_atof( "abcdefghi" ), 0.0 );
	assert_float_equal( os_atof( "abcd-10efghi" ), 0.0 );

	/* negative numbers */
	assert_float_equal( os_atof( "-1.0" ), -1.0 );
	assert_float_equal( os_atof( "-1234567890" ), -1234567890.0 );
	assert_float_equal( os_atof( "1.0" ), 1.0 );
	assert_float_equal( os_atof( "1234567890" ), 1234567890.0 );

	/* positive numbers */
	assert_float_equal( os_atof( "-1.1234" ), -1.1234 );
	assert_float_equal( os_atof( "-1234567890.0" ), -1234567890.0 );
	assert_float_equal( os_atof( "1.56789" ), 1.56789 );
	assert_float_equal( os_atof( "1234567890.25" ), 1234567890.25 );

	/* strange values */
	assert_float_equal( os_atof( "-1..1234" ), -1.0 );
	assert_float_equal( os_atof( "-1.12.34" ), -1.12 );
	assert_float_equal( os_atof( "--1.12" ), 0.0 );
}

static void test_char_isalnum_false( void **state )
{
	const char *c;
	static const char *const TEST_CHARS =
		"~!@#$%^&*()_+`-={}[]|\\;:\"'<,>.?/";
	c = TEST_CHARS;
	while ( *c )
		assert_false( os_char_isalnum( *c++ ) );
}

static void test_char_isalnum_true( void **state )
{
	const char *c;
	static const char *const TEST_CHARS =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	c = TEST_CHARS;
	while ( *c )
		assert_true( os_char_isalnum( *c++ ) );
}

static void test_char_isxdigit_false( void **state )
{
	const char *c;
	static const char *const TEST_CHARS =
		"~!@#$%^&*()_+`-={}[]|\\;:\"'<,>.?/GHIJKLMNOPQRSTUVWXYZghijklmnopqrstuvwxyz";
	c = TEST_CHARS;
	while ( *c )
		assert_false( os_char_isxdigit( *c++ ) );
}

static void test_char_isxdigit_true( void **state )
{
	const char *c;
	static const char *const TEST_CHARS =
		"ABCDEFabcdef0123456789";
	c = TEST_CHARS;
	while ( *c )
		assert_true( os_char_isxdigit( *c++ ) );
}

int main( int argc, char *argv[] )
{
	int result;
	const struct CMUnitTest tests[] = {
		cmocka_unit_test( test_atoi ),
		cmocka_unit_test( test_atof ),
		cmocka_unit_test( test_char_isalnum_false ),
		cmocka_unit_test( test_char_isalnum_true ),
		cmocka_unit_test( test_char_isxdigit_false ),
		cmocka_unit_test( test_char_isxdigit_true ),
	};

	test_initialize( argc, argv );
	result = cmocka_run_group_tests( tests, NULL, NULL );
	test_finalize( argc, argv );
	return result;
}
