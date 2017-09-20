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

#include <stdlib.h> /* for getenv(), setenv(), unsetenv() functions */
#include <time.h> /* for localtime(), time(), tzet() functions */

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

		/* depending on clock boundry and system calls, time may be
		 * out by as much as 5% (+ 1 millisecond) */
		assert_in_range( time_elapsed,
			test_times[i],
			(os_millisecond_t)((double)test_times[i] * 1.05) + 1u );
	}
}

/* test os_time_format */
static void test_os_time_format( void **state )
{
	struct symbol_result_table {
		const char *symbol;
		const char *result;
	};
	unsigned int i = 0u;
	time_t tz_offset = 0; /* time zone offset in seconds */
	int to_local_time = 0;
	os_timestamp_t ts = 0u;

	struct symbol_result_table tests[] = {
		{ "%%a = %a", "%a = Thu" },
		{ "%%A = %A", "%A = Thursday" },
		{ "%%b = %b", "%b = Jan" },
		{ "%%B = %B", "%B = January" },
#ifdef _WIN32
		{ "%%c = %c", "%c = 1/1/1970" },
#else
		{ "%%c = %c", "%c = Thu Jan  1 00:00:00 1970" },
#endif
		{ "%%C = %C", "%C = 19" },
		{ "%%d = %d", "%d = 01" },
		{ "%%D = %D", "%D = 01/01/70" },
		{ "%%e = %e", "%e =  1" },
		/* { "%%E = %E", "%E = " }, - modifier symbol */
		{ "%%F = %F", "%F = 1970-01-01" },
		{ "%%G = %G", "%G = 1970" },
		{ "%%g = %g", "%g = 70" },
		{ "%%h = %h", "%h = Jan" },
		{ "%%H = %H", "%H = 00" },
		{ "%%I = %I", "%I = 12" },
		{ "%%j = %j", "%j = 001" },
		{ "%%k = %k", "%k =  0" },
		{ "%%l = %l", "%l = 12" },
		{ "%%m = %m", "%m = 01" },
		{ "%%M = %M", "%M = 00" },
		{ "%%n = %n", "%n = \n" },
		/* { "%%O = %O", "%O = " }, - modifier symbol */
		{ "%%p = %p", "%p = AM" },
#ifdef __APPLE__
		{ "%%P = %P", "%P = P" },
#else
		{ "%%P = %P", "%P = am" },
#endif
		{ "%%r = %r", "%r = 12:00:00 AM" },
		{ "%%R = %R", "%R = 00:00" },
		/*{ "%%s = %s", "%s = 18000" }, - system offset from epoch using current timestamp */
		{ "%%S = %S", "%S = 00" },
		{ "%%t = %t", "%t = \t" },
		{ "%%T = %T", "%T = 00:00:00" },
		{ "%%u = %u", "%u = 4" },
		{ "%%U = %U", "%U = 00" },
		{ "%%V = %V", "%V = 01" },
		{ "%%w = %w", "%w = 4" },
		{ "%%W = %W", "%W = 00" },
#ifdef _WIN32
		{ "%%x = %x", "%x = 1/1/1970" },
		{ "%%X = %X", "%X = 12:00:00 AM" },
#else
		{ "%%x = %x", "%x = 01/01/70" },
		{ "%%X = %X", "%X = 00:00:00" },
#endif
		{ "%%y = %y", "%y = 70" },
		{ "%%Y = %Y", "%Y = 1970" },
		{ "%%z = %z", "%z = +0000" },
		/* { "%%Z = %Z", "%Z = GMT" }, - not consistent on all platforms */
		{ "%%%% = %%", "%% = %" }
	};

#ifdef _WIN32
	{
		TIME_ZONE_INFORMATION tzi;
		if ( GetTimeZoneInformationForYear( 1970 + (USHORT)(ts / 3.154e+10), NULL, &tzi ) )
		{
			tz_offset = tzi.Bias * -60;
		}
	}
#else
	{
		time_t gmt_time = (time_t)ts;
		struct tm *local;
		char *tz;

		/* not thread safe (but this is okay) as this program runs in a
		 * single thread.  This calculates the offset for the time zon
		 */
		local = localtime( &gmt_time );
		tz = getenv( "TZ" );
		setenv( "TZ", "UTC", 1 );
		tzset();
		tz_offset = mktime( local );
		if ( tz )
			setenv( "TZ", tz, 1 );
		else
			unsetenv("TZ");
		tzset();
	}
#endif

	for ( to_local_time = 0; to_local_time < 2; ++to_local_time )
	{
		if ( to_local_time )
		{
			unsigned int j = 0;
			if ( tz_offset < 0 )
			{
				tests[j++].result = "%a = Wed";
				tests[j++].result = "%A = Wednesday";
				tests[j++].result = "%b = Dec";
				tests[j++].result = "%B = December";
#ifdef _WIN32
				tests[j++].result = "%c = 12/31/1969";
#else
				tests[j++].result = "%c = Wed Dec 31 19:00:00 1969";
#endif
				tests[j++].result = "%C = 19";
				tests[j++].result = "%d = 31";
				tests[j++].result = "%D = 12/31/69";
				tests[j++].result = "%e = 31";
				/* tests[j++].result = "%E = "; - modifier symbol */
				tests[j++].result = "%F = 1969-12-31";
#ifdef _WIN32
				tests[j++].result = "%G = 1969";
				tests[j++].result = "%g = 69";
#else
				/** @todo fix this bug */
				tests[j++].result = "%G = 1970";
				tests[j++].result = "%g = 70";
#endif
				tests[j++].result = "%h = Dec";
				tests[j++].result = "%H = 19";
				tests[j++].result = "%I = 07";
				tests[j++].result = "%j = 365";
				tests[j++].result = "%k = 19";
				tests[j++].result = "%l =  7";
				tests[j++].result = "%m = 12";
				tests[j++].result = "%M = 00";
				tests[j++].result = "%n = \n";
				/* tests[j++].result = "%O = "; - modifier symbol */
				tests[j++].result = "%p = PM";
#ifdef __APPLE__
				tests[j++].result = "%P = P";
#else
				tests[j++].result = "%P = pm";
#endif
				tests[j++].result = "%r = 07:00:00 PM";
				tests[j++].result = "%R = 19:00";
				/* tests[j++].result = "%s = 18000"; - modifier symbol */
				tests[j++].result = "%S = 00";
				tests[j++].result = "%t = \t";
				tests[j++].result = "%T = 19:00:00";
				tests[j++].result = "%u = 3";
				tests[j++].result = "%U = 52";
				tests[j++].result = "%V = 01";
				tests[j++].result = "%w = 3";
				tests[j++].result = "%W = 52";
#ifdef _WIN32
				tests[j++].result = "%x = 12/31/1969";
				tests[j++].result = "%X = 7:00:00 PM";
#else
				tests[j++].result = "%x = 12/31/69";
				tests[j++].result = "%X = 19:00:00";
#endif
				tests[j++].result = "%y = 69";
				tests[j++].result = "%Y = 1969";
				tests[j++].result = "%z = -0500";
/* not consistent on all platforms
#ifdef _WIN32
				tests[j++].result = "%Z = Eastern Standard Time";
#else
				tests[j++].result = "%Z = EST";
#endif
*/
				tests[j++].result = "%% = %";
			}
			else
			{
				tests[j++].result = "%a = Thu";
				tests[j++].result = "%A = Thursday";
				tests[j++].result = "%b = Jan";
				tests[j++].result = "%B = January";
#ifdef _WIN32
				tests[j++].result = "%c = 1/1/1970";
#else
				tests[j++].result = "%c = Thu Jan  1 00:00:00 1970";
#endif
				tests[j++].result = "%C = 19";
				tests[j++].result = "%d = 01";
				tests[j++].result = "%D = 01/01/70";
				tests[j++].result = "%e =  1";
				/* tests[j++].result = "%E = "; - modifier symbol */
				tests[j++].result = "%F = 1970-01-01";
				tests[j++].result = "%G = 1970";
				tests[j++].result = "%g = 70";
				tests[j++].result = "%h = Jan";
				tests[j++].result = "%H = 00";
				tests[j++].result = "%I = 12";
				tests[j++].result = "%j = 001";
				tests[j++].result = "%k =  0";
				tests[j++].result = "%l = 12";
				tests[j++].result = "%m = 01";
				tests[j++].result = "%M = 00";
				tests[j++].result = "%n = \n";
				/* tests[j++].result = "%O = "; - modifier symbol */
				tests[j++].result = "%p = AM";
#ifdef __APPLE__
				tests[j++].result = "%P = P";
#else
				tests[j++].result = "%P = am";
#endif
				tests[j++].result = "%r = 12:00:00 AM";
				tests[j++].result = "%R = 00:00";
				/* tests[j++].result = "%s = 0"; - modifier symbol */
				tests[j++].result = "%S = 00";
				tests[j++].result = "%t = \t";
				tests[j++].result = "%T = 00:00:00";
				tests[j++].result = "%u = 4";
				tests[j++].result = "%U = 00";
				tests[j++].result = "%V = 01";
				tests[j++].result = "%w = 4";
				tests[j++].result = "%W = 00";
#ifdef _WIN32
				tests[j++].result = "%x = 1/1/1970";
				tests[j++].result = "%X = 12:00:00 AM";
#else
				tests[j++].result = "%x = 01/01/70";
				tests[j++].result = "%X = 00:00:00";
#endif
				tests[j++].result = "%y = 70";
				tests[j++].result = "%Y = 1970";
				tests[j++].result = "%z = +0000";
				/* tests[j++].result = "%Z = GMT"; not consistent on all platforms */
				tests[j++].result = "%% = %";
			}
		}

		for ( i = 0u; i < sizeof( tests ) / sizeof( struct symbol_result_table ); ++i )
		{
				char buf[ 256u ];
				os_time_format( buf, 256u,
					tests[i].symbol, ts, to_local_time );
				assert_string_equal( buf, tests[i].result );
			}
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
