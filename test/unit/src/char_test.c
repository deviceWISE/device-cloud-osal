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
#include "test_support.h"

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
