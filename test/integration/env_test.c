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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h> /* for SetEnvironmentVariable() function */
#else /* ifdef _WIN32 */
#include <stdlib.h> /* for setenv(), unsetenv() functions */
#endif /* else ifdef _WIN32 */
#include <string.h> /* for strlen() */

/* test os_env_expand */
static void test_os_env_expand( void **state )
{
	/* size_t os_env_expand( char *src, size_t in_len, size_t out_len ); */
	char buffer[1024u];
	size_t i;
	char out[1024u];
	size_t result;
	char var[3u][5u];
	char value[3u][10u];

	for ( i = 0u; i < 3u; ++i )
	{
		snprintf( &var[i][0u], 5u, "VAR%u", (unsigned int)(i+1u) );
		test_generate_random_string( &value[i][0u], 10u );
#ifdef _WIN32
		SetEnvironmentVariableA( &var[i][0u], &value[i][0u] );
#else /* ifdef _WIN32 */
		setenv( &var[i][0u], &value[i][0u], 1 );
#endif /* else ifdef _WIN32 */
	}

	/* null string */
	result = os_env_expand( NULL, sizeof(buffer), sizeof(buffer) );
	assert_int_equal( result, 0u );

	/* obtain output size only */
#ifdef _WIN32
	strncpy( buffer, "%VAR1%%VAR2%%VAR3%", sizeof( buffer ) );
#else /* ifdef _WIN32 */
	strncpy( buffer, "$VAR1$VAR2$VAR3", sizeof( buffer ) );
#endif /* else ifdef _WIN32 */
	snprintf( out, sizeof(out), "%s%s%s",
		&value[0u][0u], &value[1u][0u], &value[2u][0u] );
	result = os_env_expand( buffer, 0u, 0u );
	assert_int_equal( result, strlen(out) );

	/* just variables */
#ifdef _WIN32
	strncpy( buffer, "%VAR1%%VAR2%%VAR3%", sizeof( buffer ) );
#else /* ifdef _WIN32 */
	strncpy( buffer, "$VAR1$VAR2$VAR3", sizeof( buffer ) );
#endif /* else ifdef _WIN32 */
	snprintf( out, sizeof(out), "%s%s%s",
		&value[0u][0u], &value[1u][0u], &value[2u][0u] );
	result = os_env_expand( buffer, sizeof(buffer) / 2u, sizeof(buffer) - 3u);
	assert_int_equal( result, strlen(out) );
	assert_string_equal( buffer, out );

	/* variables & text */
#ifdef _WIN32
	strncpy( buffer, "first: %VAR1%, second: %VAR2%, third: %VAR3%",
		sizeof( buffer ) );
#else /* ifdef _WIN32 */
	strncpy( buffer, "first: $VAR1, second: $VAR2, third: $VAR3",
		sizeof( buffer ) );
#endif /* else ifdef _WIN32 */
	snprintf( out, sizeof(out), "first: %s, second: %s, third: %s",
		&value[0u][0u], &value[1u][0u], &value[2u][0u] );
	result = os_env_expand( buffer, sizeof(buffer), sizeof(buffer) );
	assert_int_equal( result, strlen(out) );
	assert_string_equal( buffer, out );

	/* in > out */
#ifdef _WIN32
	strncpy( buffer, "first: %VAR1%, second: %VAR2%, third: %VAR3%",
		sizeof( buffer ) );
#else /* ifdef _WIN32 */
	strncpy( buffer, "first: $VAR1, second: $VAR2, third: $VAR3",
		sizeof( buffer ) );
#endif /* else ifdef _WIN32 */
	result = os_env_expand( buffer, sizeof(buffer), 25u );
	assert_int_equal( result, 0u );

	/* var not found */
#ifdef _WIN32
	strncpy( buffer, "first: %VAR1%, second: %BAD_VAR_HERE%, third: %VAR3%",
		sizeof( buffer ) );
	snprintf( out, sizeof(out), "first: %s, second: %%BAD_VAR_HERE%%, third: %s",
		&value[0u][0u], &value[2u][0u] );
#else /* ifdef _WIN32 */
	strncpy( buffer, "first: $VAR1, second: $BAD_VAR_HERE, third: $VAR3",
		sizeof( buffer ) );
	snprintf( out, sizeof(out), "first: %s, second: $BAD_VAR_HERE, third: %s",
		&value[0u][0u], &value[2u][0u] );
#endif /* else ifdef _WIN32 */
	result = os_env_expand( buffer, sizeof(buffer), sizeof(buffer) );
	assert_int_equal( result, strlen(out) );
	assert_string_equal( buffer, out );

	/* clean up */
	for ( i = 0u; i < 3u; ++i )
	{
#ifdef _WIN32
		SetEnvironmentVariableA( &var[i][0u], NULL );
#else /* ifdef _WIN32 */
		unsetenv( &var[i][0u] );
#endif /* else ifdef _WIN32 */
	}
}

/* test os_env_get */
static void test_os_env_get( void **state )
{
	/* size_t os_env_get( const char *env, char *dest, size_t len ); */
	char buffer[1024u];
	size_t result;

	const char *env_var = "TEST_VAR";
	const char *env_value = "a really long environment variable value";
	const size_t env_value_len = strlen( env_value );
#ifdef _WIN32
		SetEnvironmentVariableA( env_var, env_value );
#else /* ifdef _WIN32 */
		setenv( env_var, env_value, 1 );
#endif /* else ifdef _WIN32 */

	/* null variable */
	result = os_env_get( NULL, buffer, sizeof(buffer) );
	assert_int_equal( result, 0u );

	/* 0 buffer */
	result = os_env_get( env_var, buffer, 0u );
	assert_int_equal( result, 0u );

	/* enough space */
	assert_true( sizeof(buffer) > env_value_len );
	result = os_env_get( env_var, buffer, sizeof(buffer) );
	assert_int_equal( result, env_value_len );
	assert_string_equal( buffer, env_value );

	/* don't write past buffer */
	strncpy( &buffer[env_value_len], "DONTWRITEHERE", 14 );
	result = os_env_get( env_var, buffer, env_value_len );
	assert_int_equal( result, env_value_len );
	assert_string_equal( &buffer[env_value_len], "DONTWRITEHERE" );

	/* 1 past buffer */
	strncpy( &buffer[env_value_len], "DONTWRITEHERE", 14 );
	result = os_env_get( env_var, buffer, env_value_len + 1u );
	assert_int_equal( result, env_value_len );
	assert_int_equal( buffer[env_value_len], '\0' );
	assert_string_equal( &buffer[env_value_len + 1u], "ONTWRITEHERE" );

	/* environment variable not found */
	result = os_env_get( "AFDAFafdadfa232424234", buffer, sizeof(buffer));
	assert_int_equal( result, 0u );

	/* clean up */
#ifdef _WIN32
		SetEnvironmentVariableA( env_var, NULL );
#else /* ifdef _WIN32 */
		unsetenv( env_var );
#endif /* else ifdef _WIN32 */
}

int main( int argc, char *argv[] )
{
	int result;
	const struct CMUnitTest tests[] = {
		cmocka_unit_test( test_os_env_expand ),
		cmocka_unit_test( test_os_env_get ),
	};

	test_initialize( argc, argv );
	result = cmocka_run_group_tests( tests, NULL, NULL );
	test_finalize( argc, argv );
	return result;
}

