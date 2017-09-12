/**
 * @file
 * @brief unit testing for IoT library (action source file)
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
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
