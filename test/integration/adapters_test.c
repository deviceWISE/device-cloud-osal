/**
 * @file
 * @brief source file containing integration tests for network adapters
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
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#	include <Winsock2.h>       /* for socket functions and structures */
#	include <ws2tcpip.h>       /* for SOCKETADDR_IN6 structure */
#	include <Iphlpapi.h>       /* for GetAdaptersAddresses */
#	include <Strsafe.h>        /* for StringCbLengthA, StringCbCopyA */
#else /* if defined( _WIN32 ) */
#	include <net/if.h>         /* for if_nametoindex */
#	if defined( __linux__ )
#		include <linux/if_packet.h> /* for sockaddr_ll */
#	else
#		include <net/if_dl.h>       /* for LLADDR definition */
#	endif /* if defined( __linux__ ) */
#	include <arpa/inet.h>      /* for inet_ntop */
#	include <ifaddrs.h>        /* for getifaddrs, freeifaddrs */
#	include <netinet/in.h>     /* for AF_LINK (apple) */
#	include <sys/socket.h>     /* for AF_LINK (freebsd) */
#	include <sys/types.h>      /* for u_char, u_short (freebsd) */
#endif /* else if defined( _WIN32 ) */

/** @brief structure for information about an ip address */
struct ip_address
{
	int found;                  /**< whether address was found */
	int is_ipv6;                /**< address is an ipv6 address */
	char *address;              /**< string representation of the address */
	struct ip_address *next;    /**< pointer to the next address */
};

/** @brief structure containing information about a network adapter */
struct adapter
{
	int found;                  /**< whether adapter was found */
	unsigned int idx;           /**< index of the adapter */
	unsigned int idx6;          /**< index of the adapter (ipv6) */
	unsigned int mac_len;       /**< length of the mac address */
	unsigned char *mac;         /**< mac address */
	char *name;                 /**< name of the interface */
	struct ip_address *ip;      /**< pointer to the first assigned ip */
	struct adapter *next;       /**< pointer to the next adapter */
};

static void free_adapters( struct adapter *adapter )
{
	while ( adapter )
	{
		struct adapter *const adapter_next = adapter->next;
		struct ip_address *ip = adapter->ip;

		if ( adapter->mac ) test_free( adapter->mac );
		if ( adapter->name ) test_free( adapter->name );
		while ( ip )
		{
			struct ip_address *const ip_next = ip->next;
			if ( ip->address ) test_free( ip->address );
			test_free( ip );
			ip = ip_next;
		}

		test_free( adapter );
		adapter = adapter_next;
	}
}

static int get_adapters( struct adapter **adapter )
{
	int rv = -1;
	struct adapter *first_adapter = NULL;
	struct adapter *last_adapter = NULL;
#if defined(_WIN32)
	ULONG buf_len = 0u;
	const ULONG family = AF_UNSPEC;
	const ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST |
		GAA_FLAG_SKIP_DNS_SERVER;

	if ( GetAdaptersAddresses( family, flags,
		NULL, NULL, &buf_len ) == ERROR_BUFFER_OVERFLOW )
	{
		IP_ADAPTER_ADDRESSES *aa =
			HeapAlloc( GetProcessHeap(), 0, buf_len );
		if ( aa &&  GetAdaptersAddresses( family, flags,
			NULL, aa, &buf_len ) == NO_ERROR )
		{
			IP_ADAPTER_ADDRESSES *cur_aa = aa;
			rv = 0;
			while ( cur_aa )
			{
				struct adapter *cur_adapter =
					test_malloc( sizeof( struct adapter ) );
				IP_ADAPTER_UNICAST_ADDRESS *cur_address;
				size_t name_len;
				size_t fname_len;

				ZeroMemory( cur_adapter,
					sizeof( struct adapter ) );
				cur_adapter->idx = cur_aa->IfIndex;
				cur_adapter->idx6 = cur_aa->Ipv6IfIndex;

				/* mac address */
				cur_adapter->mac_len =
					cur_aa->PhysicalAddressLength;
				if ( cur_aa->PhysicalAddressLength == 0u )
					cur_adapter->mac_len = 6u;

				cur_adapter->mac = test_malloc(
					cur_adapter->mac_len );
				if ( cur_aa->PhysicalAddressLength == 0u )
					ZeroMemory( cur_adapter->mac,
						cur_adapter->mac_len );
				else
					CopyMemory( cur_adapter->mac,
						cur_aa->PhysicalAddress,
						cur_adapter->mac_len );

				/* adapter name */
				StringCbLengthW( cur_aa->FriendlyName,
					MAX_ADAPTER_NAME_LENGTH, &fname_len );
				name_len = WideCharToMultiByte( CP_UTF8, 0,
					cur_aa->FriendlyName, fname_len,
					NULL, 0, NULL, NULL );
				cur_adapter->name = test_malloc(
					sizeof( char ) * ( name_len + 1u ) );
				WideCharToMultiByte( CP_UTF8, 0,
					cur_aa->FriendlyName, fname_len,
					cur_adapter->name, name_len,
					NULL, NULL );

				/* assigned ip addresses for interface */
				cur_address = cur_aa->FirstUnicastAddress;
				while ( cur_address )
				{
					struct ip_address *ip =
						test_malloc( sizeof( struct ip_address ) );
					const short af_family =
						(short)(cur_address->Address.lpSockaddr->sa_family);

					ZeroMemory( ip,
						sizeof( struct ip_address ) );

					switch (af_family)
					{
					case AF_INET:
						{
							SOCKADDR_IN *in = (SOCKADDR_IN *)(cur_address->Address.lpSockaddr);
							ip->address = test_malloc( sizeof( char * ) * ( INET_ADDRSTRLEN + 1u ) );
							InetNtop(AF_INET, &(in->sin_addr), ip->address, INET_ADDRSTRLEN);
							break;
						}
					case AF_INET6:
						{
							SOCKADDR_IN6 *in = (SOCKADDR_IN6 *)(cur_address->Address.lpSockaddr);
							ip->address = test_malloc( sizeof( char * ) * ( INET6_ADDRSTRLEN + 1u ) );
							InetNtop(AF_INET6, &(in->sin6_addr), ip->address, INET6_ADDRSTRLEN );
							ip->is_ipv6 = 1;
						}
					}

					/* add to end of list */
					if ( !cur_adapter->ip )
						cur_adapter->ip = ip;
					else
					{
						struct ip_address *cur_ip =
							cur_adapter->ip;
						struct ip_address *last_ip;
						do {
							last_ip = cur_ip;
							cur_ip = cur_ip->next;
						} while ( cur_ip );
						last_ip->next = ip;
					}

					/* go to next address */
					cur_address = cur_address->Next;
				}

				/* add to end of linked list */
				if ( last_adapter )
					last_adapter->next = cur_adapter;
				else
					first_adapter = cur_adapter;
				last_adapter = cur_adapter;
				++rv;

				/* go to next adapter */
				cur_aa = cur_aa->Next;

			}
		}

		if ( aa )
			HeapFree( GetProcessHeap(), 0, aa );
	}
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
	struct ifaddrs *ifap;
	if ( getifaddrs( &ifap ) == 0 )
	{
		struct ifaddrs *ifa;
		rv = 0;
		for ( ifa = ifap; ifa != NULL; ifa = ifa->ifa_next )
		{
			struct adapter *cur_adapter = NULL;
			int family;

			/* find the adapter matching the name */
			if ( ifa->ifa_name )
			{
				cur_adapter = first_adapter;
				while ( cur_adapter &&
					os_strcmp( cur_adapter->name, ifa->ifa_name ) != 0 )
					cur_adapter = cur_adapter->next;
			}

			/* not adapter matching name, so let's create one */
			if ( !cur_adapter )
			{
				size_t name_len = 0u;
				cur_adapter =
					test_malloc( sizeof( struct adapter ) );
				os_memset( cur_adapter, 0,
					sizeof( struct adapter ) );
				if ( ifa->ifa_name )
				{
					name_len = os_strlen( ifa->ifa_name );
					cur_adapter->idx = if_nametoindex( ifa->ifa_name );
				}
				else
					cur_adapter->idx = (unsigned int)(rv + 1);

				cur_adapter->idx6 =cur_adapter->idx;
				cur_adapter->name = test_malloc( name_len + 1u );
				os_strncpy( cur_adapter->name, ifa->ifa_name, name_len );
				cur_adapter->name[name_len] = '\0';
				++rv;

				/* add to end of linked list */
				if ( last_adapter )
					last_adapter->next = cur_adapter;
				else
					first_adapter = cur_adapter;
				last_adapter = cur_adapter;
			}

			family = ifa->ifa_addr->sa_family;
			switch ( family )
			{
#if defined(__linux__)
				case AF_PACKET:
				{
					const unsigned char *mac;
					unsigned int mac_len = 6u;
					const struct sockaddr_ll *s =
						(const struct sockaddr_ll *)
						(const void *)ifa->ifa_addr;
					mac = (const unsigned char *)(s->sll_addr);
#else /* if defined(__linux__) */
				case AF_LINK:
				{
					const unsigned char *mac;
					unsigned int mac_len = 6u;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
					mac = (const unsigned char *)
						LLADDR((const struct sockaddr_dl*)(const void*)ifa->ifa_addr);
#pragma GCC diagnostic pop
#endif /* else if defined(__linux__) */
					/* copy mac address */
					cur_adapter->mac = test_malloc( mac_len );
					os_memcpy(cur_adapter->mac, mac, mac_len );
					cur_adapter->mac_len = mac_len;
					break;
				}
				case AF_INET:
				case AF_INET6:
				{
					struct ip_address *ip = test_malloc( sizeof( struct ip_address ) );
					os_memset( ip, 0, sizeof( struct ip_address ) );
					if ( family == AF_INET )
					{
						const struct sockaddr_in *const in =
							(const struct sockaddr_in *)
							(const void *)ifa->ifa_addr;
						ip->address = test_malloc( sizeof( char * ) * ( INET_ADDRSTRLEN + 1u ) );
						inet_ntop(family, &(in->sin_addr), ip->address, INET_ADDRSTRLEN);
					}
					else
					{
						const struct sockaddr_in6 *const in =
							(const struct sockaddr_in6 *)
							(const void *)ifa->ifa_addr;
						ip->address = test_malloc( sizeof( char * ) * ( INET6_ADDRSTRLEN + 1u ) );
						inet_ntop(family, &(in->sin6_addr), ip->address, INET6_ADDRSTRLEN );
						ip->is_ipv6 = 1;
					}

					/* add to end of list */
					if ( !cur_adapter->ip )
						cur_adapter->ip = ip;
					else
					{
						struct ip_address *cur_ip =
							cur_adapter->ip;
						struct ip_address *last_ip;
						do {
							last_ip = cur_ip;
							cur_ip = cur_ip->next;
						} while ( cur_ip );
						last_ip->next = ip;
					}
					break;
				}
			}
		}
		freeifaddrs( ifap );
	}
#else
	rv = 0; /* no results */
#endif
	/* return results */
	if ( adapter )
		*adapter = first_adapter;
	else
		free_adapters( first_adapter );
	return rv;
}

static void print_adapters( struct adapter *adapter )
{
	while ( adapter )
	{
		struct ip_address *ip = adapter->ip;
		const char *name = "";
		if ( adapter->name )
			name = adapter->name;

		os_printf( "idx : %d\n", adapter->idx );
		os_printf( "idx6: %d\n", adapter->idx6 );
		os_printf( "name: %s\n", name );
		if ( adapter->mac )
		{
			unsigned int i;
			os_printf( "mac : " );
			for ( i = 0u; i < adapter->mac_len; ++i )
			{
				if ( i > 0u )
					os_printf( ":" );
				os_printf("%02x", adapter->mac[i] & 0xff);
			}
			os_printf("\n");
		}

		while ( ip )
		{
			if ( ip->address && ip->is_ipv6) os_printf( "ipv6: %s\n", ip->address );
			else if ( ip->address) os_printf( "ipv4: %s\n", ip->address );
			ip = ip->next;
		}
		os_printf("\n");
		adapter = adapter->next;
	}
}

/* test obtaining network addresses of network adapters */
static void test_os_adapters_address( void **state )
{
	struct adapter *adapter = (struct adapter *)*state;
	os_adapter_t adapters;

	if ( os_adapters_obtain( &adapters ) == OS_STATUS_SUCCESS )
	{
		do {
			struct adapter *ad = adapter;
			char name[128u] = {0};
			os_adapters_name( &adapters, name, sizeof(name) );

			while( ad && os_strcmp( ad->name, name ) != 0 )
				ad = ad->next;

			if ( ad )
			{
				os_adapter_address_t address;
				if ( os_adapters_address_first( &adapters,
					&address ) == OS_STATUS_SUCCESS )
				do {
					struct ip_address *ip;
					unsigned int idx = 0u;
					char address_str[128u] = {0};
					os_address_family_t family =
						OS_FAMILY_UNSPEC;
					os_adapters_address( &address, &idx,
						&family, address_str,
						sizeof(address_str) );

					ip = ad->ip;
					/* find matching ip */
					while ( ip && os_strcmp( ip->address,
						address_str ) != 0 )
						ip = ip->next;

					if ( ip )
						ip->found = 1;
					else
					{
						os_printf( "ERROR: test setup "
							"failed to find address "
							"%s (for adapter: %s)\n",
							address_str, name );
					}
					assert_true( ip != NULL );
				} while ( os_adapters_address_next( &address )
					== OS_STATUS_SUCCESS );
			}
			else
			{
				os_printf(
					"ERROR: test setup failed to find adapter:"
					" %s\n", name );
			}
			assert_true( ad != NULL );
		} while( os_adapters_next( &adapters ) == OS_STATUS_SUCCESS );
		os_adapters_release( &adapters );
	}

	/* check all adapters were found */
	while ( adapter )
	{
		struct ip_address *ip = adapter->ip;
		while ( ip )
		{
			if ( !ip->found )
			{
				os_printf( "ERROR: failed to find address (%s) "
					"for adapter: %s\n",
					ip->address, adapter->name );
			}
			assert_true( ip->found );
			ip = ip->next;
		}
		adapter = adapter->next;
	}
}

/* test obtaining mac addresses of network adapters */
static void test_os_adapters_mac( void **state )
{
	struct adapter *adapter = (struct adapter *)*state;
	os_adapter_t adapters;

	if ( os_adapters_obtain( &adapters ) == OS_STATUS_SUCCESS )
	{
		do {
			struct adapter *ad = adapter;
			int ad_found = 0;
			char mac[24u] = {0};
			char name[128u] = {0};
			os_adapters_mac( &adapters, mac, sizeof(mac) );
			os_adapters_name( &adapters, name, sizeof(name) );

			while( ad && !ad_found )
			{
				if ( os_strcmp( ad->name, name ) == 0 )
				{
					char mac_str[24u] = {0};
					unsigned int i;
					ad->found = ad_found = 1;
					for ( i = 0u; i < ad->mac_len; ++i )
					{
						os_snprintf( &mac_str[ i * 3u ], 4,
							"%2.2x:",
							ad->mac[ i ] );
					}
					mac_str[i * 3u - 1u] = '\0';
					assert_string_equal( mac, mac_str );
				}
				ad = ad->next;
			}

			if ( !ad_found )
			{
				os_printf(
					"ERROR: test setup failed to find adapter:"
					" %s [%s]\n", name, mac );
			}
			assert_true( ad_found );
		} while( os_adapters_next( &adapters ) == OS_STATUS_SUCCESS );
		os_adapters_release( &adapters );
	}

	/* check all adapters were found */
	while ( adapter )
	{
		if ( !adapter->found )
		{
			os_printf( "ERROR: failed to find adapter: %s\n",
				adapter->name );
			assert_true( adapter->found );
		}
		adapter = adapter->next;
	}
}

int main( int argc, char *argv[] )
{
	int result;
	struct adapter *adapters = NULL;

	get_adapters( &adapters );
	print_adapters( adapters );

	{
		const struct CMUnitTest tests[] = {
			cmocka_unit_test_prestate( test_os_adapters_address, adapters ),
			cmocka_unit_test_prestate( test_os_adapters_mac, adapters ),
		};

		test_initialize( argc, argv );
		result = cmocka_run_group_tests( tests, NULL, NULL );
		test_finalize( argc, argv );
	}
	free_adapters( adapters );
	return result;
}

