/**
 * @file
 * @brief source file defining functions for POSIX systems
 *
 * @copyright Copyright (C) 2016-2018 Wind River Systems, Inc. All Rights Reserved.
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

#include "os_posix_private.h"

#include <ctype.h>       /* for isalpha, isalnum, isxdigit */
#include <errno.h>       /* for errno */
#include <ifaddrs.h>     /* for getifaddrs, freeifaddrs */
#include <stdarg.h>      /* for va_start, va_end, va_list */
#include <stdlib.h>      /* for getenv */
#include <stdio.h>       /* for snprintf */
#include <string.h>      /* for strncpy, strerror */
#include <arpa/inet.h>   /* for inet_ntop */
#include <net/if.h>      /* for if_nametoindex */
#include <netinet/in.h>  /* for AF_LINK (apple) */
#include <sys/ioctl.h>   /* for ioctl */
#include <sys/socket.h>  /* for setsockopt + AF_LINK (freebsd) */
#include <sys/stat.h>    /* for lstat */
#include <sys/time.h>    /* for gettimeofday */
#include <sys/types.h>   /* for uid_t and gid_t, + u_char, u_short (freebsd) */
#include <sys/utsname.h> /* for struct utsname */

#if defined( __linux__ )
#	include <linux/if_packet.h> /* for sockaddr_ll */
#elif defined( __VXWORKS__ )
#	include <net/if_ll.h>       /* for sockaddr_ll */
#elif defined( __APPLE__ )
#	include <mach/clock.h>      /* for clock_get_time */
#	include <mach/mach.h>       /* for mach_port_deallocate, mach_task_self */
#	include <net/if_dl.h>       /* for LLADDR definition */
#endif /* if defined( __linux__ ) */

#if !defined( ETHER_ADDR_LEN )
	/** @brief Ethernet (mac) address length */
#	define ETHER_ADDR_LEN 6u
#endif /* if !defined( ETHER_ADDR_LEN ) */

/* compiler flags to remove */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"

#if !defined(__VXWORKS__)
#include <sys/resource.h> /* for process priority support */
#include <sys/stat.h>    /* for struct filestat, stat */
#include <sys/wait.h>    /* for waitpid */
/**
 * @brief Base shell command for executing external processes with
 */
#define OS_COMMAND_SH                  "/bin/sh", "sh", "-c"
/**
 * @def OS_COMMAND_PREFIX
 * @brief Prefix to run the command in privileged mode
 *
 * @note on ANDROID, the "sudo" command is not installed.  So don't prepend
 * the string, even in priviledged mode.
 */
#if defined( __unix__ ) && !defined( __ANDROID__ )
#	define OS_COMMAND_PREFIX       "sudo "
#else
#	define OS_COMMAND_PREFIX       ""
#endif /* defined( __unix__ ) && !defined( __ANDROID__ ) */
#else /* if !defined(__VXWORKS__) */
typedef u_short in_port_t;
#endif /* else if !defined(__VXWORKS__) */

/**
 * @brief Time in milliseconds to wait between retrying an operation
 */
#define LOOP_WAIT_TIME                 100u

#if defined(OSAL_THREAD_SUPPORT) && OSAL_THREAD_SUPPORT
/**
 * @brief Returns the systems "best guess" at the actual time
 *
 * @param[out]     ts                  time stamp output
 *
 * @retval         -1                  on failure
 * @retval         0                   on success
 */
static int os_clock_realtime( struct timespec *ts );
#endif /* if defined(OSAL_THREAD_SUPPORT) && OSAL_THREAD_SUPPORT */

os_status_t os_adapters_address(
	os_adapter_address_t *address,
	unsigned int *index,
	os_address_family_t *family,
	char *address_out,
	size_t address_len )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( address && address->cur && address->cur->ifa_addr )
	{
		/* return address */
		struct ifaddrs *const ifa = address->cur;
		const int cur_family = ifa->ifa_addr->sa_family;

		result = OS_STATUS_NOT_FOUND;
		if ( index )
		{
#if defined( SIOCGIFINDEX )
			const int socket_fd =
				socket( AF_INET, SOCK_DGRAM, 0 );
			if ( socket_fd != OS_SOCKET_INVALID )
			{
				struct ifreq ifr;
				memset( &ifr, 0, sizeof( struct ifreq ) );
				strncpy( ifr.ifr_name, ifa->ifa_name,
					IFNAMSIZ - 1u );
				if ( ioctl( socket_fd, SIOCGIFINDEX, &ifr ) != -1 )
					*index = (unsigned int)ifr.ifr_ifindex;
				close( socket_fd );
			}
#else /* if defined( SIOCGIFINDEX ) */
			const unsigned int idx =
				if_nametoindex( ifa->ifa_name );
			if ( idx > 0u )
				*index = idx;
#endif /* else if defined( SIOCGIFINDEX ) */
		}

		if ( family )
		{
			*family = OS_FAMILY_UNSPEC;
			if ( cur_family == AF_INET )
				*family = OS_FAMILY_IPV4;
			else if ( cur_family == AF_INET6 )
				*family = OS_FAMILY_IPV6;
		}

		/* obtain address */
		if ( cur_family == AF_INET )
		{
			result = OS_STATUS_SUCCESS;
			if ( address_out )
			{
				const struct sockaddr_in *const in =
					(const struct sockaddr_in *)(const void *)ifa->ifa_addr;
				inet_ntop( cur_family, &(in->sin_addr),
					address_out, (socklen_t)address_len );
			}
		}
		else if ( cur_family == AF_INET6 )
		{
			result = OS_STATUS_SUCCESS;
			if ( address_out )
			{
				const struct sockaddr_in6 *const in =
					(const struct sockaddr_in6 *)(const void *)ifa->ifa_addr;
				inet_ntop( cur_family, &(in->sin6_addr),
					address_out, (socklen_t)address_len );
			}
		}
	}
	return result;
}

os_status_t os_adapters_address_first(
	const os_adapter_t *adapter,
	os_adapter_address_t *address )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( adapter && adapter->cur && address )
	{
		struct ifaddrs *ifa;

		/* find first network layer layer for the interface */
		result = OS_STATUS_NOT_FOUND;
		address->cur = NULL;
		for ( ifa = adapter->first; !address->cur &&
			ifa != NULL; ifa = ifa->ifa_next )
		{
			if ( ifa->ifa_addr )
			{
				const int family = ifa->ifa_addr->sa_family;
				if ( ( family == AF_INET || family == AF_INET6 ) &&
				     os_strcmp( adapter->cur->ifa_name,
					ifa->ifa_name ) == 0 )
				{
					address->cur = ifa;
					result = OS_STATUS_SUCCESS;
				}
			}
		}
	}
	return result;
}

os_status_t os_adapters_address_next(
	os_adapter_address_t *address )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( address && address->cur )
	{
		struct ifaddrs *ifa, *ifa_cur;

		/* find first network layer layer for the interface */
		result = OS_STATUS_NOT_FOUND;
		ifa_cur = address->cur;
		ifa = ifa_cur->ifa_next;
		address->cur = NULL;
		for ( ; !address->cur && ifa != NULL; ifa = ifa->ifa_next )
		{
			if ( ifa->ifa_addr )
			{
				const int family = ifa->ifa_addr->sa_family;
				if ( ( family == AF_INET || family == AF_INET6 ) &&
				     os_strcmp( ifa_cur->ifa_name,
					ifa->ifa_name ) == 0 )
				{
					address->cur = ifa;
					result = OS_STATUS_SUCCESS;
				}
			}
		}
	}
	return result;
}

os_status_t os_adapters_mac(
	const os_adapter_t *adapter,
	char *mac,
	size_t mac_len )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( adapter && adapter->cur )
	{
		const unsigned char *id = NULL;
		unsigned int i;
		const unsigned int id_len = ETHER_ADDR_LEN;
		os_bool_t good_mac = OS_FALSE;
#if defined( __VXWORKS__ )
		struct ifreq ifr;
		const int socket_fd =
			socket( adapter->cur->ifa_addr->sa_family,
				SOCK_DGRAM, 0 );
		if ( socket_fd != OS_SOCKET_INVALID )
		{
			memset( &ifr, 0, sizeof( struct ifreq ) );
			strncpy( ifr.ifr_name, adapter->cur->ifa_name,
				IFNAMSIZ - 1u );
			if ( ioctl( socket_fd, SIOCGIFHWADDR, &ifr ) == 0 )
			{
#if !defined( __VXWORKS__ )
				id = (const unsigned char *)
					(const void *)ifr.ifr_hwaddr.sa_data;
#else /* if !defined( __VXWORKS__ ) */
				id = (const unsigned char *)
					(const void *)ifr.ifr_addr.sa_data;
#endif /* else if !defined( __VXWORKS__ ) */
			}
		}
#elif defined( __linux__ )
		const struct sockaddr_ll *s = (const struct sockaddr_ll *)
			(const void *)adapter->cur->ifa_addr;
		id = (const unsigned char *)(s->sll_addr);
#else /* if defined( __linux__ ) */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
		id = (const unsigned char *)
			LLADDR((const struct sockaddr_dl*)(const void*)adapter->cur->ifa_addr);
#pragma GCC diagnostic pop
#endif /* else if defined( __linux__ ) */

		result = OS_STATUS_NOT_FOUND;
		/* loop through to produce mac address */
		for ( i = 0u; i < id_len && i * 3u < mac_len; ++i )
		{
			int m = 0;
			if ( id && id[ i ] > 0u )
			{
				good_mac = OS_TRUE;
				m = id[i];
			}
			snprintf( &mac[ i * 3u ], 4, "%2.2x:", m );
		}

		/* null-terminate mac */
		if ( i > 0u && i * 3u < mac_len )
			mac_len = (i * 3u);

		if ( mac_len > 0u )
			mac[ mac_len - 1u ] = '\0';

		/* mac contained at least 1 non-zero value */
		if ( good_mac != OS_FALSE )
			result = OS_STATUS_SUCCESS;

#if defined( __VXWORKS__ )
		if ( socket_fd != OS_SOCKET_INVALID )
			close( socket_fd );
#endif /* if defined( __VXWORKS__ ) */
	}
	return result;
}

os_status_t os_adapters_next(
	os_adapter_t *adapter )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( adapter && adapter->cur )
	{
		struct ifaddrs *ifa;

		/* find next data-layer (physical) link */
		result = OS_STATUS_NOT_FOUND;
		ifa = adapter->cur;
		adapter->cur = NULL;
		for ( ifa = ifa->ifa_next; !adapter->cur &&
			ifa != NULL; ifa = ifa->ifa_next )
		{
#if defined( __linux__ )
			if (ifa->ifa_addr->sa_family == AF_PACKET)
#else /* if defined( __linux__ ) */
			if (ifa->ifa_addr->sa_family == AF_LINK)
#endif /* else if defined( __linux__ ) */
			{
				adapter->cur = ifa;
				result = OS_STATUS_SUCCESS;
			}
		}
	}
	return result;
}

os_status_t os_adapters_name(
	const os_adapter_t *adapter,
	char *name,
	size_t name_len )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( adapter && adapter->cur )
	{
		if ( adapter->cur->ifa_name )
			os_strncpy( name, adapter->cur->ifa_name, name_len );
		else if ( name_len )
			*name = '\0';
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_adapters_obtain(
	os_adapter_t *adapter )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( adapter )
	{
		result = OS_STATUS_FAILURE;
		if ( getifaddrs( &adapter->first ) == 0 )
		{
			struct ifaddrs *ifa;

			result = OS_STATUS_SUCCESS;
			adapter->cur = NULL;

			/* loop through all adapters finding all data-layer
			 * (physical) links */
			for ( ifa = adapter->first; ifa != NULL;
				ifa = ifa->ifa_next )
			{
#if defined(__linux__)
				if (ifa->ifa_addr->sa_family == AF_PACKET)
#else /* if defined(__linux__) */
				if (ifa->ifa_addr->sa_family == AF_LINK)
#endif
				{
					if ( !adapter->cur )
						adapter->cur = adapter->first;
				}
			}
		}
	}
	return result;
}

os_status_t os_adapters_release(
	os_adapter_t *adapter )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( adapter )
	{
		if ( adapter->first )
			freeifaddrs( adapter->first );
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

double os_atof( const char *str )
{
	double result = 0.0;
	if ( str )
		result = atof( str );
	return result;
}

int os_atoi( const char *str )
{
	int result = 0;
	if ( str )
		result = atoi( str );
	return result;
}

long int os_atol( const char *str )
{
	long int result = 0l;
	if ( str )
		result = atol( str );
	return result;
}

/* character testing support */
os_bool_t os_char_isalnum(
	char c )
{
	os_bool_t result = OS_FALSE;
	if ( isalnum( c ) )
		result = OS_TRUE;
	return result;
}

os_bool_t os_char_isxdigit(
	char c )
{
	os_bool_t result = OS_FALSE;
	if ( isxdigit( c ) )
		result = OS_TRUE;
	return result;
}

#if defined(OSAL_WRAP) && OSAL_WRAP
char os_char_tolower(
	char c )
{
	return (char)tolower( c );
}

char os_char_toupper(
	char c )
{
	return (char)toupper( c );
}
#endif /* if defined(OSAL_WRAP) && OSAL_WRAP */

#if defined(OSAL_THREAD_SUPPORT) && OSAL_THREAD_SUPPORT
int os_clock_realtime( struct timespec *ts )
{
#ifdef CLOCK_REALTIME
	return clock_gettime( CLOCK_REALTIME, ts );
#else
	clock_serv_t cclock;
	mach_timespec_t mts;
	host_get_clock_service( mach_host_self(), CALENDAR_CLOCK, &cclock );
	clock_get_time( cclock, &mts );
	mach_port_deallocate( mach_task_self(), cclock );
	ts->tv_sec = mts.tv_sec;
	ts->tv_nsec = mts.tv_nsec;
	return 0;
#endif
}
#endif /* defined(OSAL_THREAD_SUPPORT) && OSAL_THREAD_SUPPORT */

/* directory support */
os_status_t os_directory_create(
		const char *path,
		os_millisecond_t timeout )
{
	os_status_t result;
	os_timestamp_t start_time;
	os_millisecond_t time_elapsed = 0u;

	os_time( &start_time, NULL );
	do {
		result = os_directory_create_nowait( path );
		if ( result != OS_STATUS_SUCCESS )
		{
			os_time_elapsed( &start_time, &time_elapsed );
			os_time_sleep( LOOP_WAIT_TIME, OS_TRUE );
		}
	} while ( result != OS_STATUS_SUCCESS &&
		( timeout == 0u || time_elapsed < timeout ) );

	return result;
}

os_status_t os_directory_create_nowait(
		const char *path )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( path )
	{
		size_t path_len = 0;
		char *p = NULL;
		char temp_path[ PATH_MAX + 1u ];
		result = OS_STATUS_FAILURE;

		if ( os_directory_exists( path ) )
			return OS_STATUS_SUCCESS;

		/* Get the parent dir and check if it exists */
		os_strncpy( temp_path, path, PATH_MAX );
		path_len = os_strlen( temp_path );
		for ( p = &temp_path[ path_len - 1u ];
			  p > temp_path && *p != OS_DIR_SEP;
			  p--);
		if ( p > temp_path )
		{
			*p = '\0';
			if ( os_directory_exists( temp_path ) == OS_FALSE )
				os_directory_create_nowait( temp_path );
		}

#if !defined(_WRS_KERNEL)
		if ( mkdir( path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH ) == 0 )
#else
		if ( ( mkdir( path ) == OK ) &&
			( chmod( path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH ) == OK ) )
#endif
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_directory_current(
		char *buffer,
		size_t size )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( buffer )
	{
		result = OS_STATUS_FAILURE;
		if ( getcwd( buffer, size ) != NULL )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_directory_change(const char *path)
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( path )
	{
		result = OS_STATUS_SUCCESS;
		if ( chdir ( path ) != 0 )
			result = OS_STATUS_FAILURE;
	}
	return result;
}

os_status_t os_directory_close(
	os_dir_t *dir )
{
	os_status_t result = OS_STATUS_FAILURE;
	if( dir && dir->dir && closedir( dir->dir ) == 0 )
	{
		dir->dir = NULL;
		free( dir );
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_bool_t os_directory_exists(
	const char *dir_path )
{
	DIR *dir;
	os_bool_t result = OS_FALSE;

	dir = opendir( dir_path );
	if ( dir )
	{
		result = OS_TRUE;
		if ( closedir( dir ) != 0 )
			os_fprintf( OS_STDERR,
				"Failed to close dir %s due to %s\n",
				dir_path,
				os_system_error_string( errno ) );
	}
	return result;
}

const char *os_directory_get_temp_dir( char *dest, size_t size )
{
	const char *result = NULL;
	const char *tmp_dir = getenv( "TMPDIR" );
#if defined( P_tmpdir )
	if ( !tmp_dir )
		tmp_dir = P_tmpdir;
#endif /* if defined( P_tmpdir ) */
	if ( !tmp_dir )
		tmp_dir = "/tmp";

	if ( dest && strlen( tmp_dir ) < size )
	{
		os_strncpy( dest, tmp_dir, size );
		result = dest;
	}
	return ( result );
}

os_status_t os_directory_next(
	os_dir_t *dir,
	os_bool_t files_only,
	char *path,
	size_t path_len )
{
	os_status_t result = OS_STATUS_FAILURE;
	if ( dir && dir->dir && path && path_len > 0u )
	{
		struct dirent *d = NULL;
		while ( ( d = readdir( dir->dir ) ) )
		{
			if ( strncmp( d->d_name, ".", 1 ) != 0 &&
				strncmp( d->d_name, "..", 2 ) != 0 )
			{
				os_make_path( path, path_len, dir->path,
					d->d_name, NULL );
				path[ path_len - 1 ] = '\0';
				if ( files_only != OS_FALSE )
				{
#if !defined(__VXWORKS__)
					if ( d->d_type == DT_UNKNOWN )
#endif /* if !defined(__VXWORKS__) */
					{
						struct stat s;
						if ( ( lstat( path, &s ) == 0 ) &&
							!S_ISREG( s.st_mode ) )
							continue;
					}
#if !defined(__VXWORKS__)
					else if ( d->d_type != DT_REG )
						continue;
#endif /* if !defined(__VXWORKS__) */
				}
				break;
			}
		}

		if ( d != NULL )
			result = OS_STATUS_SUCCESS;
		else
			path[ 0 ] = '\0';
	}
	return result;
}

os_status_t os_directory_rewind(
	os_dir_t *dir )
{
	os_status_t result = OS_STATUS_FAILURE;
	if( dir && dir->dir )
	{
		rewinddir( dir->dir );
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_dir_t *os_directory_open(
	const char *dir_path )
{
	os_dir_t *out = malloc( sizeof( struct os_dir ) );
	if ( dir_path && out )
	{
		out->path = dir_path;
		out->dir = opendir( dir_path );
		if ( !out->dir )
		{
			free( out );
			out = NULL;
		}
	}
	return out;
}

/* file support */
os_status_t os_file_close(
	os_file_t handle )
{
	os_status_t result = OS_STATUS_FAILURE;
	if ( handle && fflush( handle ) == 0 && fclose( handle ) == 0 )
		result = OS_STATUS_SUCCESS;
	return result;
}

os_status_t os_file_copy(
	const char *old_path,
	const char *new_path )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( old_path && new_path )
	{
		int fd_from;

		result = OS_STATUS_FAILURE;
#ifndef _WRS_KERNEL
		fd_from = open( old_path, O_RDONLY );
#else
		fd_from = open( old_path, O_RDONLY, 0 );
#endif
		if ( fd_from >= 0 )
		{
			char buf[4096];
			int fd_to;
			ssize_t nread;
			struct stat file_stats;
			memset( &file_stats, 0, sizeof( struct stat ) );
			file_stats.st_mode = 0666;
			fstat( fd_from, &file_stats );

			if ( os_file_exists( new_path ) != OS_FALSE )
				os_file_delete( new_path );
			fd_to = open( new_path, O_WRONLY | O_CREAT | O_EXCL,
				file_stats.st_mode );
			if ( fd_to >= 0 )
			{
				result = OS_STATUS_SUCCESS;
				while ( result == OS_STATUS_SUCCESS &&
					( nread = read( fd_from, &buf[0],
					  sizeof( buf ) ) ) > 0 )
				{
					char *out_ptr = buf;
					ssize_t nwritten;
					do {
						nwritten = write( fd_to,
							out_ptr, (size_t)nread );
						if ( nwritten >= 0 )
						{
							nread -= nwritten;
							out_ptr += nwritten;
						}
						else if ( errno != EINTR )
							result = OS_STATUS_FAILURE;
					} while ( result == OS_STATUS_SUCCESS
						&& nread > 0 );
				}
				if ( close( fd_to ) < 0 )
					result = OS_STATUS_FAILURE;
			}
			close( fd_from );
		}
	}
	return result;
}

os_status_t os_file_delete(
	const char *path )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( path )
	{
		result = OS_STATUS_FAILURE;
		if ( unlink( path ) == 0 )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

#if defined(OSAL_WRAP) && OSAL_WRAP
os_bool_t os_file_eof(
	os_file_t stream )
{
	return feof( stream ) == 0 ? OS_FALSE : OS_TRUE;
}
#endif /* if defined(OSAL_WRAP) && OSAL_WRAP */

os_bool_t os_file_exists(
	const char *file_path )
{
	os_bool_t result = OS_FALSE;
	struct stat file_stat;
	if ( access( file_path, F_OK ) == 0 &&
		stat( file_path, &file_stat ) == 0 )
	{
		if ( S_ISREG( file_stat.st_mode ) ||
			S_ISLNK( file_stat.st_mode ) )
			result = OS_TRUE;
	}
	return result;
}

#if defined(OSAL_WRAP) && OSAL_WRAP
char *os_file_gets(
	char *str,
	size_t size,
	os_file_t stream )
{
	return fgets( str, (int)size, stream );
}
#endif /* if defined(OSAL_WRAP) && OSAL_WRAP */

os_status_t os_file_move(
	const char *old_path,
	const char *new_path
)
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( old_path && new_path )
	{
		result = OS_STATUS_FAILURE;
		if ( rename( old_path, new_path ) == 0 )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_file_t os_file_open(
	const char *file_path,
	int flags )
{
	os_file_t result = NULL;
	if ( file_path )
	{
		int fd;
		int oflags = 0;
		char mode[3u];
		memset( mode, 0, sizeof( mode ) );

		if ( ( flags & OS_READ ) && ( flags & OS_WRITE ) )
		{
			oflags |= O_RDWR;
			if ( ( flags & OS_APPEND ) && !( flags & OS_TRUNCATE ) )
			{
				oflags |= O_APPEND;
				strncpy( mode, "a+", sizeof( mode ) );
			}
			else if ( ( flags & OS_CREATE ) || ( flags & OS_TRUNCATE ) )
			{
				oflags |= O_TRUNC;
				strncpy( mode, "w+", sizeof( mode ) );
			}
			else
				strncpy( mode, "r+", sizeof( mode ) );
		}
		else if ( flags & OS_READ )
		{
			oflags |= O_RDONLY;
			strncpy( mode, "r", sizeof( mode ) );
		}
		else if ( flags & OS_WRITE )
		{
			oflags |= O_WRONLY;
			strncpy( mode, "w", sizeof( mode ) );
			if ( ( flags & OS_APPEND ) && !( flags & OS_TRUNCATE ) )
			{
				oflags |= O_APPEND;
				strncpy( mode, "a", sizeof( mode ) );
			}
			else if ( ( flags & OS_CREATE ) || ( flags & OS_TRUNCATE ) )
				oflags |= O_TRUNC;
		}
		else if ( flags & OS_CREATE )
			strncpy( mode, "r", sizeof( mode ) );

		/* create and exclusive flags */
		if ( flags & OS_CREATE )
		{
			oflags |= O_CREAT;
			if ( flags & OS_EXCLUSIVE )
				oflags |= O_EXCL;
		}

		fd = open( file_path, oflags,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
		if ( fd != -1 )
			result = fdopen( fd, mode );
	}
	return result;
}

#if defined(OSAL_WRAP) && OSAL_WRAP
size_t os_file_puts(
	char *str,
	os_file_t stream )
{
	return (size_t)fputs( str, stream );
}

size_t os_file_read(
	void *ptr,
	size_t size,
	size_t nmemb,
	os_file_t stream )
{
	return fread( ptr, size, nmemb, stream );
}
#endif /* if defined(OSAL_WRAP) && OSAL_WRAP */

os_status_t os_file_seek(
	os_file_t stream,
	long offset,
	int whence
)
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( stream != OS_FILE_INVALID )
	{
		result = OS_STATUS_FAILURE;
		if ( fseek( stream, offset, whence ) == 0 )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_uint64_t os_file_size(
	const char *file_path )
{
	struct stat file_stat;

	if ( stat( file_path, &file_stat ) != 0 )
		file_stat.st_size = 0;

	return (os_uint64_t)file_stat.st_size;
}

os_uint64_t os_file_size_handle(
	os_file_t file_handle )
{
	long int file_size = 0L;

	if ( file_handle )
	{
		long int cur_pos = ftell( file_handle );
		if ( fseek( file_handle, 0, SEEK_END ) == 0 )
		{
			file_size = ftell( file_handle );
			if ( cur_pos != file_size )
				fseek( file_handle, cur_pos, SEEK_SET );
		}
	}
	return (os_uint64_t)file_size;
}

os_status_t os_file_sync(
	const char *file_path )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( file_path )
	{
		int fd;
		result = OS_STATUS_FAILURE;
#if !defined(__VXWORKS__)
		fd = open( file_path, O_RDONLY );
#else /* if !defined(__VXWORKS__) */
		fd = open( file_path, O_RDONLY, 0 );
#endif /* else if !defined(__VXWORKS__) */
		if ( fd >= 0 )
		{
			if ( fsync( fd ) == 0 )
				result = OS_STATUS_SUCCESS;
			close( fd );
		}
	}
	else
	{
#if !defined(__VXWORKS__)
		sync();
#endif /* !defined(__VXWORKS__) */
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

#if defined(OSAL_WRAP) && OSAL_WRAP
long int os_file_tell(
	os_file_t stream )
{
	return ftell( stream );
}
#endif /* if defined(OSAL_WRAP) && OSAL_WRAP */

os_status_t os_file_temp(
	char *prototype,
	size_t suffix_len )
{
	os_status_t result = OS_STATUS_FAILURE;
	if ( prototype )
	{
		int fd = mkstemps( prototype, (int)suffix_len );
		close( fd );
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

#if defined(OSAL_WRAP) && OSAL_WRAP
size_t os_file_write(
	const void *ptr,
	size_t size,
	size_t nmemb,
	os_file_t stream )
{
	return fwrite( ptr, size, nmemb, stream );
}
#endif /* if defined(OSAL_WRAP) && OSAL_WRAP */

/* memory functions */
#if defined(OSAL_WRAP) && OSAL_WRAP
void *os_calloc( size_t nmemb, size_t size )
{
	return calloc( nmemb, size );
}

void os_free( void *ptr )
{
	free( ptr );
}

void os_free_null( void **ptr )
{
	if ( ptr && *ptr )
	{
		free( *ptr );
		*ptr = NULL;
	}
}

void *os_malloc( size_t size )
{
	return malloc( size );
}

void *os_realloc( void *ptr, size_t size )
{
	return realloc( ptr, size );
}

int os_memcmp(
	const void *ptr1,
	const void *ptr2,
	size_t num )
{
	return memcmp( ptr1, ptr2, num );
}

void *os_memcpy(
	void *dest,
	const void *src,
	size_t len )
{
	memcpy( dest, src, len );
	return dest;
}

void *os_memmove(
	void *dest,
	const void *src,
	size_t len )
{
	memmove( dest, src, len );
	return dest;
}

void os_memset(
	void *dest,
	int c,
	size_t len )
{
	memset( dest, c, len );
}

void os_memzero(
	void *dest,
	size_t len )
{
	bzero( dest, len );
}
#endif /* if defined(OSAL_WRAP) && OSAL_WRAP */

/* print functions */
size_t os_env_expand(
	char *src,
	size_t in_len,
	size_t out_len )
{
	size_t result = 0u;
	if ( src && in_len <= out_len )
	{
		char *dest = src;
		char *src_end = NULL;
		if ( in_len > 0u )
			src_end = src + in_len;
		while ( *src && ( !src_end || src < src_end ) )
		{
			if ( *src == '$' )
			{
				const char *env_start;
				const char *env_value = NULL;
				++src;
				env_start = src;
				if ( isalpha( *src ) || *src == '_' )
				{
					++src;
					while( ( isalnum( *src ) || *src == '_' ) &&
						( !src_end || src < src_end ) )
					{
						++src;
					}
				}
				if ( src != env_start )
				{
					char env_name[256u];
					size_t name_len = (size_t)(src-env_start);
					if ( name_len < 256u )
					{
						strncpy( env_name, env_start,
							name_len );
						env_name[name_len] = '\0';
						env_value = getenv( env_name );
						name_len++; /* for '$' char */
						if ( env_value )
						{
							const size_t val_len =
								strlen( env_value );
							size_t var_name_offset = 0u;
							if ( val_len >= name_len )
								var_name_offset = (val_len-name_len) + 1u;
							if ( out_len > result && val_len < out_len - result - var_name_offset - name_len - 1u )
							{
								memmove( dest + val_len,
									src,
									out_len - result - name_len - 1u - var_name_offset );
								strncpy( dest,
									env_value,
									val_len );
								dest += val_len;
								src += val_len - name_len;
								if ( src_end )
									src_end += val_len  - name_len;
							}
							result += val_len;
						}
						else
						{
							/* not found, so skip */
							dest = src;
							result += name_len;
						}
					}
					else
					{
						/* variable name is too long */
						dest = src;
						result += name_len + 1u;
					}
				}
			}
			else
			{
				/* handle escaped character in path */
				if ( *src == '\\' )
					++src;
				if ( *src != '\0' )
				{
					if ( result < out_len )
					{
						*dest = *src;
						++dest;
					}
					++result;
					++src;
				}
			}
		}

		/* add null-terminator */
		if ( result < out_len )
			*dest = '\0';
	}
	return result;
}

size_t os_env_get(
	const char *env,
	char *dest,
	size_t len )
{
	size_t result = 0u;
	if ( dest && len > 0u )
		dest[0] = '\0';
	if ( env && dest && len > 0u )
	{
		const char *value = getenv( env );
		if ( value )
		{
			strncpy( dest, value, len );
			result = strlen( value );
			if ( result > len )
			{
				dest[len - 1u] = '\0';
				result = len;
			}
		}
	}
	return result;
}

#if defined(OSAL_WRAP) && OSAL_WRAP
int os_fprintf(
	os_file_t stream,
	const char *format,
	... )
{
	int result;
	va_list args;
	va_start( args, format );
	result = vfprintf( stream, format, args );
	va_end( args );
	return result;
}

int os_printf(
	const char *format,
	... )
{
	int result;
	va_list args;
	va_start( args, format );
	result = vprintf( format, args );
	va_end( args );
	return result;
}
#endif /* if defined(OSAL_WRAP) && OSAL_WRAP */

int os_snprintf(
	char *str,
	size_t size,
	const char *format,
	... )
{
	int result;
	va_list args;
	va_start( args, format );
#if defined( __APPLE__ )
	if ( !format )
		result = -1;
	else
#endif /* if defined( __APPLE__ ) */
		result = os_vsnprintf( str, size, format, args );
	if ( result < 0 || (size_t)result > size )
		result = -1;
	va_end( args );
	return result;
}

#if defined(OSAL_WRAP) && OSAL_WRAP
int os_vfprintf(
	os_file_t stream,
	const char *format,
	va_list args )
{
	return vfprintf( stream, format, args );
}
#endif /* if defined(OSAL_WRAP) && OSAL_WRAP */

int os_vsnprintf(
	char *str,
	size_t size,
	const char *format,
	va_list args )
{
	int result = vsnprintf( str, size, format, args );
	return result;
}

int os_sprintf(
	char *str,
	const char *format,
	... )
{
	int result;
	va_list args;
	va_start( args, format );
	result = vsprintf( str, format, args );
	va_end( args );
	return result;
}

os_bool_t os_flush( os_file_t stream )
{
	os_bool_t result = OS_FALSE;
	if ( fflush( stream ) == 0 )
		result = OS_TRUE;
	return result;
}

os_bool_t os_path_is_absolute( const char *path )
{
	os_bool_t result = OS_FALSE;
	if ( path && *path == OS_DIR_SEP )
		result = OS_TRUE;
	return result;
}

/* socket functions */
int os_get_host_address(
	const char *host,
	const char *service,
	char *address,
	size_t address_len,
	int family
	)
{
	int result;
	struct addrinfo *address_list = NULL;
	struct addrinfo hints;

	os_memzero( &hints, sizeof( hints ) );

	hints.ai_family = family;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	result = getaddrinfo( host, service, &hints, &address_list );

	if ( result == 0 && address_list )
	{
		if ( address_list->ai_family == AF_INET )
		{
			/* cast to void*, to suppress the clang warning:
			 *   cast from 'struct sockaddr *' to
			 *   'struct sockaddr_in *' increases required alignment
			 *   from 2 to 4
			 */
			struct sockaddr_in *address_struct =
				(struct sockaddr_in *)((void *)address_list->ai_addr);
			inet_ntop( AF_INET, &(address_struct->sin_addr), address, (socklen_t)address_len );
		}
		else if ( address_list->ai_family == AF_INET6 )
		{
			/* cast to void*, to suppress the clang warning:
			 *   cast from 'struct sockaddr *' to
			 *   'struct sockaddr_in *' increases required alignment
			 *   from 2 to 4
			 */
			struct sockaddr_in6 *address_struct =
				(struct sockaddr_in6 *)((void *)address_list->ai_addr);
			inet_ntop( AF_INET6, &(address_struct->sin6_addr), address, (socklen_t)address_len );
		}
	}
	if ( address_list )
		freeaddrinfo( address_list );
	return result;
}

os_status_t os_socket_accept(
	const os_socket_t *socket,
	os_socket_t **out,
	os_millisecond_t max_time_out )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( socket && out )
	{
		os_socket_t *s = malloc( sizeof( struct os_socket ) );
		result = OS_STATUS_NO_MEMORY;
		if ( s )
		{
			result = OS_STATUS_FAILURE;
			if ( socket->fd != OS_SOCKET_INVALID )
			{
				int select_result = 1;
				if ( max_time_out > 0u )
				{
					struct timeval ts;
					fd_set rfds;

					ts.tv_sec = max_time_out / OS_MILLISECONDS_IN_SECOND;
					ts.tv_usec = ( max_time_out % OS_MILLISECONDS_IN_SECOND ) *
						OS_MICROSECONDS_IN_MILLISECOND;

					FD_ZERO( &rfds );
					FD_SET( socket->fd, &rfds );
					select_result = select( socket->fd + 1,
						&rfds, NULL, NULL, &ts );
					if ( select_result == 0 )
						result = OS_STATUS_TIMED_OUT;
				}
				if ( select_result > 0 )
				{
					socklen_t sock_len = sizeof( struct sockaddr );
					memcpy( s, socket, sizeof( struct os_socket ) );
					s->fd = accept( socket->fd, &s->addr, &sock_len );
					if ( s->fd != OS_SOCKET_INVALID )
						result = OS_STATUS_SUCCESS;
				}
			}

			if ( result == OS_STATUS_SUCCESS )
				*out = s;
			else
				free( s );
		}
	}
	return result;
}

os_status_t os_socket_bind(
	const os_socket_t *socket,
	int queue_size )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( socket )
	{
		result = OS_STATUS_FAILURE;
		if ( socket->fd != OS_SOCKET_INVALID && bind( socket->fd,
			&socket->addr, sizeof( struct sockaddr ) ) == 0 )
		{
			if ( listen( socket->fd, queue_size ) == 0 )
				result = OS_STATUS_SUCCESS;
		}
	}
	return result;
}

os_status_t os_socket_broadcast(
	const os_socket_t *socket,
	const void *buf,
	size_t len,
	int ttl,
	size_t *bytes_written,
	os_millisecond_t max_time_out )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	const int broadcast_enable = 1;
	if ( socket && socket->fd != OS_SOCKET_INVALID )
	{
		ssize_t retval;
		result = OS_STATUS_FAILURE;
		retval = setsockopt( socket->fd, SOL_SOCKET, SO_BROADCAST,
			&broadcast_enable, sizeof( broadcast_enable ) );
		if ( retval == 0 && ttl > 1 )
			retval = setsockopt( socket->fd, IPPROTO_IP,
				IP_MULTICAST_TTL, &ttl, sizeof( ttl ) );
		if ( retval == 0 )
			result = os_socket_write( socket, buf, len,
				bytes_written, max_time_out );
	}
	return result;
}

os_status_t os_socket_close(
	os_socket_t *socket )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( socket )
	{
		result = OS_STATUS_FAILURE;
		if ( socket->fd != OS_SOCKET_INVALID &&
			close( socket->fd ) == 0 )
		{
			free( socket );
			result = OS_STATUS_SUCCESS;
		}
	}
	return result;
}

os_status_t os_socket_connect(
	const os_socket_t *socket )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( socket )
	{
		result = OS_STATUS_FAILURE;
		if ( socket->fd != OS_SOCKET_INVALID &&
			connect( socket->fd, &socket->addr,
				sizeof( struct sockaddr ) ) == 0 )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_socket_initialize( void )
{
	return OS_STATUS_SUCCESS;
}

os_status_t os_socket_open(
	os_socket_t **out,
	const char *address,
	os_uint16_t port,
	int type,
	int protocol,
	os_millisecond_t max_time_out )
{
	os_millisecond_t time_elapsed = 0u;
	os_status_t result = OS_STATUS_BAD_PARAMETER;

	if ( out && address && port > 0u )
	{
		os_socket_t *s = malloc( sizeof( os_socket_t ) );
		result = OS_STATUS_NO_MEMORY;
		*out = NULL;
		if ( s )
		{
			/* cast to void* removes erroneous warning in clang */
			void *const addr_ptr = &s->addr;
			struct sockaddr_in  *const addr4 =
				(struct sockaddr_in *)addr_ptr;
			struct sockaddr_in6 *const addr6 =
				(struct sockaddr_in6 *)addr_ptr;
			result = OS_STATUS_FAILURE;
			memset( s, 0, sizeof( os_socket_t ) );
			if ( inet_pton( AF_INET, address,
				&(addr4->sin_addr) ) == 1 )
			{
				addr4->sin_family = AF_INET;
				addr4->sin_port = (in_port_t)htons( port );
				result = OS_STATUS_SUCCESS;
			}
			else if ( inet_pton( AF_INET6, address,
				&(addr6->sin6_addr) ) == 1 )
			{
				addr6->sin6_family = AF_INET6;
				addr6->sin6_port = (in_port_t)htons( port );
				result = OS_STATUS_SUCCESS;
			}

			if ( result == OS_STATUS_SUCCESS )
			{
				s->type = type;
				s->protocol = protocol;
				s->fd = socket( s->addr.sa_family, type,
					protocol );
				while ( s->fd == OS_SOCKET_INVALID &&
					errno == EAGAIN &&
					( max_time_out == 0u || time_elapsed < max_time_out ) )
				{
					struct timeval ts;

					os_millisecond_t wait_time = 2000u;
					if ( max_time_out > 0u && max_time_out - time_elapsed <  wait_time )
						wait_time = max_time_out - time_elapsed;
					ts.tv_sec = wait_time / OS_MILLISECONDS_IN_SECOND;
					ts.tv_usec = ( wait_time % OS_MILLISECONDS_IN_SECOND ) *
						OS_MICROSECONDS_IN_MILLISECOND;

					/* keep trying to obtain a socket until one if available,
					 * this condition may be hit when running in a service, and
					 * the client application is started before network services
					 * are available */
					select( 0, NULL, NULL, NULL, &ts );
					s->fd = socket( s->addr.sa_family,
						type, protocol );
				}

				if ( s->fd == OS_SOCKET_INVALID )
					result = OS_STATUS_TIMED_OUT;
			}

			if ( result == OS_STATUS_SUCCESS )
				*out = s;
			else
				free( s );
		}
	}
	return result;
}

os_status_t os_socket_option(
	const os_socket_t *socket,
	int level,
	int optname,
	const void *optval,
	size_t optlen )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( socket && socket->fd != OS_SOCKET_INVALID && optval )
	{
		result = OS_STATUS_FAILURE;
		if ( setsockopt( socket->fd, level, optname, optval,
			(socklen_t)optlen ) == 0 )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_socket_read(
	const os_socket_t *socket,
	void *buf,
	size_t len,
	size_t *bytes_read,
	os_millisecond_t max_time_out )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( bytes_read )
		*bytes_read = 0u;
	if ( socket && socket->fd != OS_SOCKET_INVALID )
	{
		ssize_t retval = 0;
		result = OS_STATUS_FAILURE;
		if ( max_time_out > 0u )
		{
			struct timeval tv;
			tv.tv_sec = max_time_out / OS_MILLISECONDS_IN_SECOND;
			tv.tv_usec = ( max_time_out % OS_MILLISECONDS_IN_SECOND ) *
				OS_MICROSECONDS_IN_MILLISECOND;
			retval = setsockopt( socket->fd, SOL_SOCKET,
				SO_RCVTIMEO, &tv, sizeof( struct timeval ) );
		}
		if ( retval >= 0 )
		{
			retval = read( socket->fd, buf, len );
			if ( retval > 0 )
			{
				if ( bytes_read )
					*bytes_read = (size_t)retval;
				result = OS_STATUS_SUCCESS;
			}
			else if ( errno == ETIMEDOUT )
				result = OS_STATUS_TIMED_OUT;
			else if ( retval == 0 )
				result = OS_STATUS_TRY_AGAIN;
		}
	}
	return result;
}

ssize_t os_socket_receive(
	const os_socket_t *socket,
	void *buf,
	size_t len,
	char *src_addr,
	size_t src_addr_len,
	os_uint16_t *port,
	os_millisecond_t max_time_out )
{
	ssize_t result = -1;
	if ( socket && socket->fd != OS_SOCKET_INVALID )
	{
		result = 0;
		if ( max_time_out > 0u )
		{
			struct timeval tv;
			tv.tv_sec = max_time_out / OS_MILLISECONDS_IN_SECOND;
			tv.tv_usec = ( max_time_out % OS_MILLISECONDS_IN_SECOND ) *
				OS_MICROSECONDS_IN_MILLISECOND;
			result = setsockopt( socket->fd, SOL_SOCKET, SO_RCVTIMEO, &tv,
				sizeof( struct timeval ) );
		}
		if ( result >= 0 )
		{
			struct sockaddr peer_addr;
			socklen_t peer_addr_len = sizeof( struct sockaddr );
			result = recvfrom( socket->fd, buf, len, 0, &peer_addr,
				&peer_addr_len );
			if ( result >= 0 && ( src_addr || port ) )
			{
				if ( peer_addr.sa_family == AF_INET )
				{
					/* cast to void* removes erroneous warning in clang */
					void *const addr_ptr = &peer_addr;
					struct sockaddr_in *sa =
						(struct sockaddr_in *)addr_ptr;
					if ( src_addr )
						inet_ntop( AF_INET, &(sa->sin_addr),
							src_addr, (socklen_t)src_addr_len );
					if ( port )
						*port = ntohs( sa->sin_port );
				}
				else if ( peer_addr.sa_family == AF_INET6 )
				{
					/* cast to void* removes erroneous warning in clang */
					void *const addr_ptr = &peer_addr;
					struct sockaddr_in6 *sa =
						(struct sockaddr_in6 *)addr_ptr;
					if ( src_addr )
						inet_ntop( AF_INET6, &(sa->sin6_addr),
							src_addr, (socklen_t)src_addr_len );
					if ( port )
						*port = ntohs( sa->sin6_port );
				}
			}
		}
	}
	return result;
}

ssize_t os_socket_send(
	const os_socket_t *socket,
	const void *buf,
	size_t len,
	const char *dest_addr,
	os_uint16_t port,
	os_millisecond_t max_time_out )
{
	ssize_t result = -1;
	if( socket && socket->fd != OS_SOCKET_INVALID && dest_addr )
	{
		result = 0;
		if ( max_time_out > 0u )
		{
			struct timeval tv;
			tv.tv_sec = max_time_out / OS_MILLISECONDS_IN_SECOND;
			tv.tv_usec = ( max_time_out % OS_MILLISECONDS_IN_SECOND )
				* OS_MICROSECONDS_IN_MILLISECOND;
			result = setsockopt( socket->fd, SOL_SOCKET, SO_SNDTIMEO,
				&tv, sizeof( struct timeval ) );
		}
		if ( result >= 0 )
		{
			struct sockaddr addr;
			/* cast to void* removes erroneous warning in clang */
			void *const addr_ptr = &addr;
			struct sockaddr_in  *const addr4 =
				(struct sockaddr_in *)addr_ptr;
			struct sockaddr_in6 *const addr6 =
				(struct sockaddr_in6 *)addr_ptr;
			memset( &addr, 0, sizeof( struct sockaddr ) );
			result = -1;
			if ( inet_pton( AF_INET, dest_addr,
				&(addr4->sin_addr) ) == 1 )
			{
				addr4->sin_family = AF_INET;
				addr4->sin_port = (in_port_t)htons( port );
				result = 0;
			}
			else if ( inet_pton( AF_INET6, dest_addr,
				&(addr6->sin6_addr) ) == 1 )
			{
				addr6->sin6_family = AF_INET6;
				addr6->sin6_port = (in_port_t)htons( port );
				result = 0;
			}
			if ( result >= 0 )
				result = sendto( socket->fd, buf, len, 0,
					(struct sockaddr*)&addr,
					sizeof( struct sockaddr ) );
		}
	}
	return result;
}

os_status_t os_socket_terminate( void )
{
	return OS_STATUS_SUCCESS;
}

os_status_t os_socket_write(
	const os_socket_t *socket,
	const void *buf,
	size_t len,
	size_t *bytes_written,
	os_millisecond_t max_time_out )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( bytes_written )
		*bytes_written = 0u;
	if ( socket && socket->fd != OS_SOCKET_INVALID )
	{
		ssize_t retval = 0;
		result = OS_STATUS_FAILURE;
		if ( max_time_out > 0u )
		{
			struct timeval tv;
			tv.tv_sec = max_time_out / OS_MILLISECONDS_IN_SECOND;
			tv.tv_usec = ( max_time_out % OS_MILLISECONDS_IN_SECOND )
				* OS_MICROSECONDS_IN_MILLISECOND;
			retval = setsockopt( socket->fd, SOL_SOCKET, SO_SNDTIMEO,
				&tv, sizeof( struct timeval ) );
		}
		if ( retval >= 0 )
		{
			retval = write( socket->fd, buf, len );
			if ( retval >= 0 )
			{
				if ( bytes_written )
					*bytes_written = (size_t)retval;
				result = OS_STATUS_SUCCESS;
			}
			else if ( errno == ETIMEDOUT )
				result = OS_STATUS_TIMED_OUT;
		}
	}
	return result;
}

#if defined(OSAL_WRAP) && OSAL_WRAP
char *os_strchr(
	const char *s,
	char c )
{
	return strchr( s, (int)c );
}

int os_strcmp(
	const char *s1,
	const char *s2
)
{
	return strcmp( s1, s2 );
}

size_t os_strlen(
	const char *s )
{
	return strlen( s );
}

int os_strncmp(
	const char *s1,
	const char *s2,
	size_t len
)
{
	return strncmp( s1, s2, len );
}

char *os_strncpy(
	char *dest,
	const char *src,
	size_t num )
{
	return strncpy( dest, src, num );
}

char *os_strpbrk(
	const char *str1,
	const char *str2 )
{
	return strpbrk( str1, str2 );
}

char *os_strrchr(
	const char *s,
	char c )
{
	return strrchr( s, (int)c );
}

char *os_strstr(
	const char *str1,
	const char *str2 )
{
	return strstr( str1, str2 );
}

double os_strtod(
	const char *str,
	char **endptr )
{
	return strtod( str, endptr );
}

long os_strtol(
	const char *str,
	char **endptr )
{
	return strtol( str, endptr, 10 );
}

unsigned long os_strtoul(
	const char *str,
	char **endptr )
{
	return strtoul( str, endptr, 10 );
}
#endif /* if defined( OSAL_WRAP ) */

#if defined(OSAL_WRAP) && OSAL_WRAP
/* operating system specific */
int os_system_error_last( void )
{
	return errno;
}
#endif /* if defined(OSAL_WRAP) && OSAL_WRAP */

const char *os_system_error_string(
	int error_number )
{
	if ( error_number == -1 )
		error_number = errno;

	return strerror( error_number );
}

#if defined(OSAL_WRAP) && OSAL_WRAP
os_uint32_t os_system_pid( void )
{
	return (os_uint32_t)getpid();
}
#endif /* if defined(OSAL_WRAP) && OSAL_WRAP */

#if !defined(__VXWORKS__)
os_status_t os_system_run(
	os_system_run_args_t *args )
{
	size_t i;
	const int output_fd[2u] = { STDOUT_FILENO, STDERR_FILENO };
	int command_output_fd[2u][2u] =
		{ { -1, -1 }, { -1, -1 } };
	os_status_t result = OS_STATUS_SUCCESS;
	os_timestamp_t start_time;

	if ( !args || (!args->cmd && !args->fptr) )
		return OS_STATUS_BAD_PARAMETER;

	/* set a default exit status */
	args->return_code = -1;

	os_time( &start_time, NULL );

	/* capture the stdout & stderr of the command and send it back
	 * as the response (if this is a waiting process call) */
	for ( i = 0u; i < 2u && result == OS_STATUS_SUCCESS; ++i )
	{
		/* if either the output pipes are set or we need a return code,
		 * then we block */
		if ( args->block == OS_FALSE )
		{
			if ( i == 0u && args->opts.nonblock.std_out != OS_FILE_INVALID )
				command_output_fd[i][0] =
					fileno( args->opts.nonblock.std_out );
			else if ( i == 1u && args->opts.nonblock.std_err != OS_FILE_INVALID )
				command_output_fd[i][0] =
					fileno( args->opts.nonblock.std_err );
		}
		else
		{
			if ( pipe( command_output_fd[i] ) != 0 )
				result = OS_STATUS_IO_ERROR;
		}
	}

	if ( result == OS_STATUS_SUCCESS )
	{
		const pid_t pid = fork();
		result = OS_STATUS_NOT_EXECUTABLE;
		if ( pid != -1 )
		{
			/* pid == 0 for child... */
			args->return_code = 0; /* success */
			if ( pid == 0 )
			{
				/* Create a new session for the child process.
				 */
				pid_t sid = setsid();
				if ( sid < 0 )
					exit( errno );

				/* redirect child stdout/stderr to the pipe */
				for ( i = 0u; i < 2u; ++i )
				{
					dup2( command_output_fd[i][1], output_fd[i] );
					close( command_output_fd[i][0] );
				}

				/* set priority */
				if ( args->priority != 0 )
				{
					int cur_priority = 0;
					cur_priority = getpriority(
						PRIO_PROCESS, (id_t)sid );
					args->priority += cur_priority;
					if ( args->priority < -20 )
						args->priority = -20;
					else if ( args->priority > 20 )
						args->priority = 20;
					setpriority( PRIO_PROCESS,
						(id_t)sid, args->priority );
				}

				/* set stack size */
				if ( args->stack_size > 0u )
				{
					struct rlimit lim;
					lim.rlim_cur = args->stack_size;
					lim.rlim_max = args->stack_size;
					setrlimit( RLIMIT_STACK, &lim );
				}

				if ( args->fptr )
				{
					int argc = 0;
					char **argv;
					size_t cmd_len = 0u;
					int rc;

					if ( args->cmd )
						cmd_len = strlen( args->cmd );

					argv = malloc( sizeof(char *) * 1u );
					argv[argc++] = malloc(cmd_len + 1u);
					if ( args->cmd )
						strncpy( argv[0],
							args->cmd, cmd_len );
					argv[0][cmd_len] = '\0';

					rc = args->fptr( argc, argv );
					free( argv[0] );
					free( argv );
					exit( rc );
				}
				else
				{
					if ( args->privileged == OS_FALSE )
						execl( OS_COMMAND_SH, args->cmd,
							(char *)NULL );
					else
						execl( OS_COMMAND_SH, OS_COMMAND_PREFIX,
							args->cmd, (char *)NULL );

					/* Process failed to be replaced, return failure */
					exit( errno );
				}
			}

			for ( i = 0u; i < 2u; ++i )
				close( command_output_fd[i][1] );

			result = OS_STATUS_INVOKED;
			if ( args->block != OS_FALSE )
			{
				int system_result = -1;
				os_millisecond_t time_elapsed;
				errno = 0;
				do {
					waitpid( pid, &system_result, WNOHANG );
					os_time_elapsed( &start_time, &time_elapsed );
					os_time_sleep( LOOP_WAIT_TIME, OS_FALSE );
				} while ( ( errno != ECHILD ) &&
					( !WIFEXITED( system_result ) ) &&
					( !WIFSIGNALED( system_result ) ) &&
					( args->opts.block.max_wait_time == 0u ||
						time_elapsed < args->opts.block.max_wait_time) );

				if ( ( errno != ECHILD ) &&
					!WIFEXITED( system_result ) &&
					!WIFSIGNALED( system_result ) )
				{
					kill( pid, SIGTERM );
					waitpid( pid, &system_result, WNOHANG );
					result = OS_STATUS_TIMED_OUT;
					/* MacOSX waitpid will sometimes return
					 * 255 here instead of -1.  Explicitly
					 * set it to -1 for consistency across
					 * platforms. */
					system_result = -1;
				}
				else
					result = OS_STATUS_SUCCESS;

				fflush( stdout );
				fflush( stderr );

				for ( i = 0u; i < 2u; ++i )
				{
					char *buf = args->opts.block.std_out.buf;
					size_t buf_len = args->opts.block.std_out.len;
					if ( i != 0u )
					{
						buf = args->opts.block.std_err.buf;
						buf_len = args->opts.block.std_err.len;
					}
					if ( buf && buf_len > 0u )
					{
						buf[0] = '\0';
						/* if we are able to read from pipe */
						if ( command_output_fd[i][0] != -1 )
						{
							const ssize_t output_size =
								read( command_output_fd[i][0],
								buf, buf_len - 1u );
							if ( output_size >= 0 )
								buf[ output_size ] = '\0';
						}
					}
				}

				if ( result != OS_STATUS_TIMED_OUT )
				{
					if ( WIFEXITED( system_result ) )
						system_result = WEXITSTATUS( system_result );
					else if ( WIFSIGNALED( system_result ) )
						system_result = WTERMSIG( system_result );
					else
						system_result = WIFEXITED( system_result );
				}

				args->return_code = system_result;
			}
		}
	}
	return result;
}
#endif /* if !defined(__VXWORKS__) */

os_bool_t os_terminal_vt100_support(
	os_file_t stream
)
{
	int fd;
	os_bool_t result = OS_FALSE;
	fd = fileno( stream );
	if ( isatty( fd ) )
		result = OS_TRUE;
	else
	{
		struct stat file_stat;
		if ( fstat( fd, &file_stat ) == 0 &&
			S_ISFIFO( file_stat.st_mode ) )
			result = OS_TRUE;
	}
	return result;
}

os_status_t os_terminate_handler(
	os_sighandler_t signal_handler )
{
	struct sigaction new_action;
	memset( &new_action, 0, sizeof( new_action ) );
	new_action.sa_handler = signal_handler;
	sigemptyset( &new_action.sa_mask );
	sigaction( SIGINT, &new_action, NULL );
	sigaction( SIGTERM, &new_action, NULL );
	sigaction( SIGCHLD, &new_action, NULL );

	return OS_STATUS_SUCCESS;
}

static __thread int rand_init = 0;
/* time functions */
double os_random(
	double  min,
	double  max )
{
	/* take a time seed to get better randomness */
	if (!rand_init )
	{
		srand( (unsigned int)time( NULL ) );
		rand_init = 1;
	}
	return min + (rand() / (double)RAND_MAX) * ( max - min );
}

os_status_t os_time(
	os_timestamp_t *time_stamp,
	os_bool_t *up_time )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( time_stamp )
	{
		struct timeval tv;

		result = OS_STATUS_FAILURE;
		if ( gettimeofday( &tv, NULL ) == 0 )
		{
			*time_stamp = (os_timestamp_t)tv.tv_sec *
				OS_MILLISECONDS_IN_SECOND +
				(os_timestamp_t)tv.tv_usec /
				OS_MICROSECONDS_IN_MILLISECOND;
			result = OS_STATUS_SUCCESS;
		}
	}
	if ( up_time )
		*up_time = OS_FALSE;
	return result;
}

size_t os_time_format(
	char *buf,
	size_t len,
	const char *format,
	os_timestamp_t time_stamp,
	os_bool_t to_local_time )
{
	size_t result = 0u;
	if ( buf && format )
	{
		const time_t raw_time = time_stamp / OS_MILLISECONDS_IN_SECOND;
		struct tm *t;

		if ( to_local_time == OS_FALSE )
			t = gmtime( &raw_time );
		else
			t = localtime( &raw_time );

		result = strftime( buf, len, format, t );
	}

	/* handle error case */
	if ( result == 0u && buf && len > 0u )
		*buf = '\0';
	return result;
}

os_status_t os_time_sleep(
	os_millisecond_t ms,
	os_bool_t allow_interrupts )
{
	struct timespec rem;
	os_status_t result = OS_STATUS_FAILURE;
	struct timespec s;
	int sleep_result;
	s.tv_sec = ms / OS_MILLISECONDS_IN_SECOND;
	s.tv_nsec = ( ms % OS_MILLISECONDS_IN_SECOND ) *
		OS_NANOSECONDS_IN_MILLISECOND;
	sleep_result = nanosleep( &s, &rem );

	/* continue sleeping if an interrupt is received */
	while( allow_interrupts == OS_FALSE && sleep_result == -1 &&
		errno == EINTR )
	{
		memcpy( &s, &rem, sizeof( struct timespec ) );
		sleep_result = nanosleep( &s, &rem );
	}

	if ( sleep_result == 0 )
		result = OS_STATUS_SUCCESS;
	return result;
}

/* threads & lock support */
#if defined(OSAL_THREAD_SUPPORT) && OSAL_THREAD_SUPPORT
os_status_t os_thread_condition_broadcast(
	os_thread_condition_t *cond )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( cond )
	{
		result = OS_STATUS_FAILURE;
		if ( pthread_cond_broadcast( cond ) == 0 )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_condition_create(
	os_thread_condition_t *cond )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( cond )
	{
		result = OS_STATUS_FAILURE;
		if ( pthread_cond_init( cond, NULL ) == 0 )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_condition_destroy(
	os_thread_condition_t *cond )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( cond )
	{
		result = OS_STATUS_FAILURE;
		if ( pthread_cond_destroy( cond ) == 0 )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_condition_signal(
	os_thread_condition_t *cond,
	os_thread_mutex_t *lock )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( cond && lock )
	{
		result = OS_STATUS_FAILURE;
		if (  pthread_mutex_lock( lock ) == 0 )
		{
			if ( pthread_cond_signal( cond ) == 0 )
				result = OS_STATUS_SUCCESS;
			pthread_mutex_unlock( lock );
		}
	}
	return result;
}

os_status_t os_thread_condition_timed_wait(
	os_thread_condition_t *cond,
	os_thread_mutex_t *lock,
	os_millisecond_t max_time_out )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( cond && lock )
	{
		result = OS_STATUS_FAILURE;
		if ( max_time_out > 0u )
		{
			int error_number;
			struct timespec abs_time_out;
			os_clock_realtime( &abs_time_out );
			abs_time_out.tv_nsec +=
				( max_time_out % OS_MILLISECONDS_IN_SECOND ) *
				OS_NANOSECONDS_IN_MILLISECOND;
			abs_time_out.tv_sec +=
				( max_time_out / OS_MILLISECONDS_IN_SECOND ) +
				( (unsigned long)abs_time_out.tv_nsec /
					OS_NANOSECONDS_IN_SECOND );
			abs_time_out.tv_nsec %= OS_NANOSECONDS_IN_SECOND;
			error_number = pthread_cond_timedwait( cond, lock,
				&abs_time_out );
			if ( error_number == 0 )
				result = OS_STATUS_SUCCESS;
			else if ( error_number == ETIMEDOUT )
				result = OS_STATUS_TIMED_OUT;
		}
		else if ( pthread_cond_wait( cond, lock ) == 0 )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

#if defined(OSAL_WRAP) && OSAL_WRAP
os_status_t os_thread_condition_wait(
	os_thread_condition_t *cond,
	os_thread_mutex_t *lock )
{
	return os_thread_condition_timed_wait( cond, lock, 0u );
}
#endif /* if defined(OSAL_WRAP) && OSAL_WRAP */

os_status_t os_thread_create(
	os_thread_t *thread,
	os_thread_main_t main,
	void *arg,
	size_t stack_size )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( main )
	{
		pthread_attr_t attr;
		pthread_attr_t *pattr = NULL;

		/* initialize attributes */
		if ( pthread_attr_init( &attr ) == 0 )
		{
			pattr = &attr;
			if ( stack_size > 0 )
				pthread_attr_setstacksize( &attr, (size_t)stack_size );
		}

		result = OS_STATUS_FAILURE;
		if ( pthread_create( thread, pattr, main, arg ) == 0 )
			result = OS_STATUS_SUCCESS;

		/* it's safe to destory, pthread_attr structure after using */
		if ( pattr )
			pthread_attr_destroy( pattr );
	}
	return result;
}

os_status_t os_thread_destroy(
	os_thread_t *thread )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( thread )
	{
		result = OS_STATUS_FAILURE;
#ifndef __ANDROID__
		if ( !(*thread) || pthread_cancel( *thread ) == 0 )
			result = OS_STATUS_SUCCESS;
#endif
	}
	return result;
}

os_status_t os_thread_wait(
	os_thread_t *thread )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( thread )
	{
		result = OS_STATUS_FAILURE;
		if ( !(*thread) || pthread_join( *thread, NULL ) == 0 )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_mutex_create(
	os_thread_mutex_t *lock )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( lock )
	{
		result = OS_STATUS_FAILURE;
		if ( pthread_mutex_init( lock, NULL ) == 0 )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_mutex_lock(
	os_thread_mutex_t *lock )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( lock )
	{
		result = OS_STATUS_FAILURE;
		if ( pthread_mutex_lock( lock ) == 0 )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_mutex_unlock(
	os_thread_mutex_t *lock )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( lock )
	{
		result = OS_STATUS_FAILURE;
		if ( pthread_mutex_unlock( lock ) == 0 )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_thread_mutex_destroy(
	os_thread_mutex_t *lock )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( lock )
	{
		result = OS_STATUS_FAILURE;
		if ( pthread_mutex_destroy( lock ) == 0 )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}
#endif /* if defined(OSAL_THREAD_SUPPORT) && OSAL_THREAD_SUPPORT */

/* uuid support */
os_status_t os_uuid_generate(
	os_uuid_t *uuid )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( uuid )
	{
		uuid_t out;
		uuid_generate( out );
		memcpy( uuid, &out, sizeof( os_uuid_t ) );
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_uuid_to_string_lower(
	os_uuid_t *uuid,
	char *dest,
	size_t len )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( uuid && dest )
	{
		result = OS_STATUS_NO_MEMORY;
		if ( len >= 36u )
		{
			uuid_t in;
			memcpy( &in, uuid, sizeof( uuid_t ) );
			uuid_unparse_lower( in, dest );
			result = OS_STATUS_SUCCESS;
		}
	}
	return result;
}

#pragma clang diagnostic pop

