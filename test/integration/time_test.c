/**
 * @file
 * @brief source file containing integration tests for time related functions
 *
 * @copyright Copyright (C) 2016-2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include <os.h>

#include "test_support.h"
#include <time.h> /* for time() function */

/* test os_random */
static void test_os_random( void **state )
{
#define TEST_COUNT 100u
	unsigned int i = 0u;
	double results[TEST_COUNT];
	for ( i = 0u; i < TEST_COUNT; ++i )
	{
		unsigned int j;
		unsigned int same_count = 0u;
		const double max = (double)i * (double)i;
		const double min = max * -1;
		const double result = os_random( min, max );

		/* result is outside range */
		assert_false( result < min || result > max );

		/* check if same number generated */
		for ( j = 0u; j < i; ++j )
		{
			if ( results[j] <= result + 0.001 &&
			     results[j] >= result - 0.001 )
				++same_count;
		}

		/* > 50% of the numbers are the same,
		 * output is not random enough */
		assert_false( same_count > i / 2 );

		/* save result for next pass */
		results[i] = result;
	}
}

/* test os_time */
static void test_os_time( void **state )
{
	os_status_t result;
	os_timestamp_t ts = 0u;
	os_bool_t up_time = OS_FALSE;

	result = os_time( &ts, &up_time );
	assert_int_equal( result, OS_STATUS_SUCCESS );
#ifdef ARDUNIO
	assert_int_equal( up_time, OS_TRUE );
#else
	assert_int_equal( up_time, OS_FALSE );
	{
		os_timestamp_t time_now = 0u;
		time_t t = 0u;
		time( &t );
		time_now = (os_timestamp_t)t * 1000u;

		/* test if time is within 1 second of time from
		 * time() function */
		assert_false(
			( time_now > ts && time_now - ts > 1000u ) ||
			     ( ts - time_now > 1000u ) );
	}
#endif
}

/* test os_time_elapsed */
static void test_os_time_elapsed( void **state )
{
	unsigned int i;
	const os_millisecond_t test_times[] = {
		0u,       /* 0 milliseconds */
		500u,     /* 500 milliseconds */
		1000u,    /* 1 second */
		5000u };  /* 5 seconds */
	for ( i = 0u; i < (sizeof( test_times ) / sizeof( os_millisecond_t )); ++i )
	{
		os_status_t result;
		os_millisecond_t time_elapsed = 0u;
		os_timestamp_t time_stamp = 0u;
		result = os_time( &time_stamp, NULL );
		assert_int_equal( result, OS_STATUS_SUCCESS );

		os_time_sleep( test_times[i], OS_FALSE );
		os_time_elapsed( &time_stamp, &time_elapsed );

		/* depending on clock boundry time may be out by
		 * 1 second */
		assert_false( time_elapsed != test_times[i] &&
			time_elapsed != test_times[i] + 1u );
	}
}

/* test os_time_format */
static void test_os_time_format( void **state )
{
	struct symbol_result_table {
		const char *symbol;
		const char *gmt_result;
		const char *local_result;
	};

	unsigned int i;
	struct symbol_result_table tests[] = {
		{ "%%a = %a", "%a = Thu",                      "%a = Wed" },
		{ "%%A = %A", "%A = Thursday",                 "%A = Wednesday" },
		{ "%%b = %b", "%b = Jan",                      "%b = Dec" },
		{ "%%B = %B", "%B = January",                  "%B = December" },
#ifdef _WIN32
		{ "%%c = %c", "%c = 1/1/1970",                 "%c = 12/31/1969" },
#else
		{ "%%c = %c", "%c = Thu Jan  1 00:00:00 1970", "%c = Wed Dec 31 19:00:00 1969" },
#endif
		{ "%%C = %C", "%C = 19",                       "%C = 19" },
		{ "%%d = %d", "%d = 01",                       "%d = 31" },
		{ "%%D = %D", "%D = 01/01/70",                 "%D = 12/31/69" },
		{ "%%e = %e", "%e =  1",                       "%e = 31" },
		/* { "%%E = %E", "%E = ",                         "%E = " }, - modifier symbol */
		{ "%%F = %F", "%F = 1970-01-01",               "%F = 1969-12-31" },
#ifdef _WIN32
		{ "%%G = %G", "%G = 1970",                     "%G = 1969" },
		{ "%%g = %g", "%g = 70",                       "%g = 69" },
#else
		{ "%%G = %G", "%G = 1970",                     "%G = 1970" },
		{ "%%g = %g", "%g = 70",                       "%g = 70" },
#endif
		{ "%%h = %h", "%h = Jan",                      "%h = Dec" },
		{ "%%H = %H", "%H = 00",                       "%H = 19" },
		{ "%%I = %I", "%I = 12",                       "%I = 07" },
		{ "%%j = %j", "%j = 001",                      "%j = 365" },
		{ "%%k = %k", "%k =  0",                       "%k = 19" },
		{ "%%l = %l", "%l = 12",                       "%l =  7" },
		{ "%%m = %m", "%m = 01",                       "%m = 12" },
		{ "%%M = %M", "%M = 00",                       "%M = 00" },
		{ "%%n = %n", "%n = \n",                       "%n = \n" },
		/* { "%%O = %O", "%O = ",                         "%O = " }, - modifier symbol */
		{ "%%p = %p", "%p = AM",                       "%p = PM" },
		{ "%%P = %P", "%P = am",                       "%P = pm" },
		{ "%%r = %r", "%r = 12:00:00 AM",              "%r = 07:00:00 PM" },
		{ "%%R = %R", "%R = 00:00",                    "%R = 19:00" },
		/*{ "%%s = %s", "%s = 18000",                     "%s = 18000" }, - system offset from epoch using current timestamp */
		{ "%%S = %S", "%S = 00",                       "%S = 00" },
		{ "%%t = %t", "%t = \t",                       "%t = \t" },
		{ "%%T = %T", "%T = 00:00:00",                 "%T = 19:00:00" },
		{ "%%u = %u", "%u = 4",                        "%u = 3" },
		{ "%%U = %U", "%U = 00",                       "%U = 52" },
		{ "%%V = %V", "%V = 01",                       "%V = 01" },
		{ "%%w = %w", "%w = 4",                        "%w = 3" },
		{ "%%W = %W", "%W = 00",                       "%W = 52" },
#ifdef _WIN32
		{ "%%x = %x", "%x = 1/1/1970",                 "%x = 12/31/1969" },
		{ "%%X = %X", "%X = 12:00:00 AM",              "%X = 7:00:00 PM" },
#else
		{ "%%x = %x", "%x = 01/01/70",                 "%x = 12/31/69" },
		{ "%%X = %X", "%X = 00:00:00",                 "%X = 19:00:00" },
#endif
		{ "%%y = %y", "%y = 70",                       "%y = 69" },
		{ "%%Y = %Y", "%Y = 1970",                     "%Y = 1969" },
		{ "%%z = %z", "%z = +0000",                    "%z = -0500" },
#ifdef _WIN32
		{ "%%Z = %Z", "%Z = GMT",                      "%Z = Eastern Standard Time" },
#else
		{ "%%Z = %Z", "%Z = GMT",                      "%Z = EST" },
#endif
		{ "%%%% = %%", "%% = %",                       "%% = %" },
	};
	os_timestamp_t ts = 0u;

	for ( i = 0u; i < sizeof( tests ) / sizeof( struct symbol_result_table ); ++i )
	{
		char buf[ 256u ];
		os_time_format( buf, 256u, tests[i].symbol, ts, OS_FALSE );
		assert_string_equal( buf, tests[i].gmt_result );
		os_time_format( buf, 256u, tests[i].symbol, ts, OS_TRUE );
		assert_string_equal( buf, tests[i].local_result );
	}
}

/* test os_time_remaining */
static void test_os_time_remaining( void **state )
{
	unsigned int i;
	os_status_t result;
	const os_millisecond_t test_times[] = {
		0u,       /* 0 milliseconds */
		500u,     /* 500 milliseconds */
		1000u,    /* 1 second */
		5000u,    /* 5 seconds */
		10000u }; /* 10 seconds */
	os_timestamp_t time_stamp = 0u;

	result = os_time( &time_stamp, NULL );
	assert_int_equal( result, OS_STATUS_SUCCESS );

	for ( i = 0u; i < (sizeof( test_times ) / sizeof( os_millisecond_t )); ++i )
	{
		os_millisecond_t time_remaining = 0u;
		os_time_remaining( &time_stamp, test_times[i], &time_remaining );
		assert_int_equal( time_remaining, test_times[i] );
	}
}

/* test os_time_sleep */
static void test_os_time_sleep( void **state )
{
	unsigned int i;
	const os_millisecond_t test_ms[] = {
		0u, /* 0 milliseconds */
		1u, /* 1 millisecond */
		500u, /* 500 milliseconds */
		1000u, /* 1 second */
		5000u }; /* 5 seconds */

	for ( i = 0u; i < (sizeof( test_ms ) / sizeof( os_millisecond_t )); ++i )
	{
		time_t time_start = 0u;
		time_t time_end = 0u;
		time( &time_start );
		os_time_sleep( test_ms[i], OS_FALSE );
		time( &time_end );

		/* if time not within (+1 second) of test time, then report
		 * failure */
		assert_false( time_end - time_start > test_ms[i] / 1000u + 1u );
	}
}

int main( int argc, char *argv[] )
{
	int result;
	const struct CMUnitTest tests[] = {
		cmocka_unit_test( test_os_random ),
		cmocka_unit_test( test_os_time ),
		cmocka_unit_test( test_os_time_elapsed ),
		cmocka_unit_test( test_os_time_format ),
		cmocka_unit_test( test_os_time_remaining ),
		cmocka_unit_test( test_os_time_sleep ),
	};

	test_initialize( argc, argv );
	result = cmocka_run_group_tests( tests, NULL, NULL );
	test_finalize( argc, argv );
	return result;
}
