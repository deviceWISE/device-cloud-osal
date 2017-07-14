#include "../os.h"

#include <ctype.h>       /* for isalha, isalnum */
#include <errno.h>       /* for errno */
#include <stdarg.h>      /* for va_start, va_end, va_list */
#include <stdlib.h>      /* for getenv */
#include <stdio.h>       /* for snprintf */
#include <string.h>      /* for strncpy, strerror */
#include <unistd.h>      /* for close */
#include <sys/socket.h>  /* for setsockopt */
#include <sys/stat.h>    /* for struct filestat, stat */
#include <sys/time.h>    /* for gettimeofday */
#include <sys/types.h>   /* for uid_t and gid_t */
#include <sys/wait.h>    /* for waitpid */
#ifndef _WRS_KERNEL
#include <dlfcn.h>       /* for dlclose, dlopen, dlsym */
#include <pwd.h>         /* for getpwnam */
#include <regex.h>       /* for regular expression support */
#include <sys/statvfs.h> /* for struct statsfs */
#include <termios.h>     /* for terminal input */
#endif /* _WRS_KERNEL */

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

char *os_file_fgets(
	char *str,
	size_t size,
	os_file_t stream )
{
	return fgets( str, (int)size, stream );
}

size_t os_file_fputs(
	char *str,
	os_file_t stream )
{
	return (size_t)fputs( str, stream );
}

size_t os_file_fread(
	void *ptr,
	size_t size,
	size_t nmemb,
	os_file_t stream )
{
	return fread( ptr, size, nmemb, stream );
}

size_t os_file_fwrite(
	const void *ptr,
	size_t size,
	size_t nmemb,
	os_file_t stream )
{
	return fwrite( ptr, size, nmemb, stream );
}

void *os_library_find(
	os_lib_handle lib,
	const char *function )
{
	return dlsym( lib, function );
}

os_lib_handle os_library_open(
	const char *path )
{
	return dlopen( path, RTLD_LAZY );
}

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
	char *destination,
	const char *source,
	size_t num )
{
	return strncpy( destination, source, num );
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

void os_memcpy(
	void *dest,
	const void *src,
	size_t len )
{
	memcpy( dest, src, len );
}

void os_memmove(
	void *dest,
	const void *src,
	size_t len )
{
	memmove( dest, src, len );
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
	memset( dest, 0, len );
}


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

int os_snprintf(
	char *str,
	size_t size,
	const char *format,
	... )
{
	int result;
	va_list args;
	va_start( args, format );
	result = os_vsnprintf( str, size, format, args );
	va_end( args );
	return result;
}
int os_vfprintf(
	os_file_t stream,
	const char *format,
	va_list args )
{
	return vfprintf( stream, format, args );
}

int os_vsnprintf(
	char *str,
	size_t size,
	const char *format,
	va_list args )
{
	int result = vsnprintf( str, size, format, args );
	if ( (size_t)result >= size )
		result = -1;
	return result;
}

void *os_heap_calloc( size_t nmemb, size_t size )
{
	return calloc( nmemb, size );
}

void os_heap_free( void **ptr )
{
	if ( ptr && *ptr )
	{
		free( *ptr );
		*ptr = NULL;
	}
}

void *os_heap_malloc( size_t size )
{
	return malloc( size );
}

void *os_heap_realloc( void *ptr, size_t size )
{
	return realloc( ptr, size );
}

/* operating system specific */
int os_system_error_last( void )
{
	return errno;
}

os_uint32_t os_system_pid( void )
{
	return (os_uint32_t)getpid();
}


os_status_t os_thread_condition_wait(
	os_thread_condition_t *cond,
	os_thread_mutex_t *lock )
{
	return os_thread_condition_timed_wait( cond, lock, 0 );
}