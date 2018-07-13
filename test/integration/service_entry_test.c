/**
 * @file
 * @brief source file containing integration tests for service entry functions
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

#if !defined(VERBOSE)
#define VERBOSE 0
#endif /* if !defined(VERBOSE) */

/**
 * @brief object to assoicate names and ports
 */
struct name_port
{
	const char *name;           /**< @brief service name */
	const char *proto;          /**< @brief service protocol (optional) */
	int port;                   /**< @brief service port */
};

/**
 * @brief contains a list of known ports for testing purposes
 */
static struct name_port known_services[] = {
	{ "http", "tcp", 80 },
	{ "ssh", NULL, 22 } ,
	{ "telnet", "tcp", 23 },
#if defined( __APPLE__ )
	{ "smtp", NULL, 25 },
#else /* if defined( __APPLE__ ) */
	{ "mail", NULL, 25 }, /* find via aliases (for stmp) */
#endif /* else if defined( __APPLE__ ) */
	{ NULL, NULL, 0 }
};

static struct name_port unknown_services[] = {
	{ "bad-service", "tcp", 8007 },
	{ "no-service", "udp", 8012 },
	{ "nothing-service", NULL, 8120 },
	{ NULL, NULL, 0 }
};

static void test_os_service_entry_find_by_name_known( void **state )
{
	const struct name_port *known_service = &known_services[0u];
	while ( known_service->name )
	{
		unsigned int i = 0u;
		const char *s_name;
		const os_uint16_t port_le =
			htons((uint16_t)known_service->port);
		os_service_entry_t *res =
			os_service_entry_by_name(
				known_service->name, known_service->proto );

		os_printf( "testing service: %s\n", known_service->name );
		assert_non_null( res );
		s_name = res->s_name;
		while ( res->s_aliases && res->s_aliases[i] )
		{
			if ( os_strcmp( res->s_aliases[i], known_service->name ) == 0 )
				s_name = res->s_aliases[i];
			++i;
		}
		assert_string_equal( s_name, known_service->name );
		assert_int_equal( res->s_port, port_le );

		++known_service;
	}

	os_service_entry_close();
}

static void test_os_service_entry_find_by_name_unknown( void **state )
{
	const struct name_port *unknown_service = &unknown_services[0u];
	while ( unknown_service->name )
	{
		os_service_entry_t *res =
			os_service_entry_by_name(
				unknown_service->name, unknown_service->proto );
		assert_null( res );
		++unknown_service;
	}
	os_service_entry_close();
}

static void test_os_service_entry_find_by_port_known( void **state )
{
	const struct name_port *known_service = &known_services[0u];
	while ( known_service->name )
	{
		unsigned int i = 0u;
		const char *s_name;
		const os_uint16_t port_le =
			htons((uint16_t)known_service->port);
		os_service_entry_t *const res =
			os_service_entry_by_port( port_le, known_service->proto );

		os_printf( "testing port: %d\n", known_service->port );
		assert_non_null( res );
		s_name = res->s_name;
		while ( res->s_aliases && res->s_aliases[i] )
		{
			if ( os_strcmp( res->s_aliases[i], known_service->name ) == 0 )
				s_name = res->s_aliases[i];
			++i;
		}
		assert_string_equal( s_name, known_service->name );
		assert_int_equal( res->s_port, port_le );

		++known_service;
	}
	os_service_entry_close();
}

static void test_os_service_entry_find_by_port_unknown( void **state )
{
	const struct name_port *unknown_service = &unknown_services[0u];
	while ( unknown_service->name )
	{
		const os_uint16_t port_le =
			htons((uint16_t)unknown_service->port);
		os_service_entry_t *const res =
			os_service_entry_by_port( port_le, unknown_service->proto );

		assert_null( res );
		++unknown_service;
	}
	os_service_entry_close();
}

static void test_os_service_entry_list_all( void **state )
{
	os_service_entry_t *cur;
	os_service_entry_open( 1 );

	cur = os_service_entry_get();
#if VERBOSE
	os_printf( "known services: \n" );
#endif /* if VERBOSE */
	while ( cur )
	{
		assert_non_null( cur->s_name );
		assert_non_null( cur->s_proto );
#if VERBOSE
		os_printf( " %d, %s, %s", ntohs((uint16_t)cur->s_port), cur->s_proto,
			cur->s_name );
		if ( cur->s_aliases )
		{
			size_t i = 0u;
			while ( cur->s_aliases[i] )
			{
				os_printf( ", %s", cur->s_aliases[i] );
				++i;
			}
		}
		os_printf( "\n" );
#endif /* if VERBOSE */
		assert_in_range( ntohs((uint16_t)cur->s_port), 1, 65535 );
		cur = os_service_entry_get();
	}

	os_service_entry_close();
}


int main( int argc, char *argv[] )
{
	int result;
	const struct CMUnitTest tests[] = {
		cmocka_unit_test( test_os_service_entry_find_by_name_known ),
		cmocka_unit_test( test_os_service_entry_find_by_name_unknown ),
		cmocka_unit_test( test_os_service_entry_find_by_port_known ),
		cmocka_unit_test( test_os_service_entry_find_by_port_unknown ),
		cmocka_unit_test( test_os_service_entry_list_all )
	};

	test_initialize( argc, argv );
	result = cmocka_run_group_tests( tests, NULL, NULL );
	test_finalize( argc, argv );
	return result;
}

