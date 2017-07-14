/**
 * @file
 * @brief Function definitions for Windows wrapper functions used instead of 
 * macros when building a "wrapped" version of the library
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */


#include "../os.h"
#pragma warning( push, 1 )
#include <float.h>  /* for DBL_MAX */
#include <signal.h> /* for SIGINT & SIGTERM */
#include <Shlwapi.h> /* for StrStr */
#if _MSC_VER > 1700
#include <VersionHelpers.h> /* for IsWindowsVersionOrGreater */
#endif
#pragma warning( pop )

void *os_library_find(
	os_lib_handle lib,
	const char *function )
{
	return GetProcAddress( lib, function );
}

os_lib_handle os_library_open(
	const char *path )
{
	return LoadLibrary( path );
}

char *os_strstr(
	const char *str1,
	const char *str2 )
{
	return StrStr( str1, str2 );
}

void os_memcpy(
	void *dest,
	const void *src,
	size_t len )
{
	CopyMemory( dest, src, len );
}

void os_memmove(
	void *dest,
	const void *src,
	size_t len )
{
	MoveMemory( dest, src, len );
}

void os_memset(
	void *dest,
	int c,
	size_t len )
{
	FillMemory( dest, len, c );
}

void os_memzero(
	void *dest,
	size_t len )
{
	ZeroMemory( dest, len );
}

/* memory functions */
void *os_heap_calloc( size_t nmemb, size_t size )
{
	return HeapAlloc( GetProcessHeap(), 0, nmemb * size );
}

/* operating system specific */
int os_system_error_last( void )
{
	return (int)GetLastError();
}

os_status_t os_thread_condition_wait(
	os_thread_condition_t *cond,
	os_thread_mutex_t *lock )
{
	return os_thread_condition_timed_wait( cond, lock, 0 );
}