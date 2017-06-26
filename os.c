/**
 * @file
 * @brief Function definitions that work on all operating systems
 *
 * @copyright Copyright (C) 2016-2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "os.h"

#include <stdarg.h>

iot_status_t os_time_elapsed(
	iot_timestamp_t *start_time,
	iot_millisecond_t *elapsed_time )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( start_time && elapsed_time )
	{
		iot_timestamp_t end_time;
		os_time( &end_time, NULL );
		if( end_time >= *start_time )
		{
			*elapsed_time =
				(iot_millisecond_t)( end_time - *start_time );
			result = IOT_STATUS_SUCCESS;
		}
		else
		{
			*elapsed_time = 0u;
			result = IOT_STATUS_BAD_REQUEST;
		}
	}
	return result;
}

iot_status_t os_time_remaining(
	iot_timestamp_t *start_time,
	iot_millisecond_t time_out,
	iot_millisecond_t *remaining )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( start_time && remaining )
	{
		if ( time_out == 0u )
		{
			*remaining = 0u;
			/* if time_out is 0u; indicates forever: so
			 * don't return IOT_STATUS_TIMED_OUT */
			result = IOT_STATUS_SUCCESS;
		}
		else
		{
			iot_millisecond_t elapsed_time = 0u;
			result = os_time_elapsed(
				start_time, &elapsed_time );
			if ( result == IOT_STATUS_SUCCESS )
			{
				if ( elapsed_time < time_out )
					*remaining = time_out - elapsed_time;
				else
				{
					*remaining = 0u;
					result = IOT_STATUS_TIMED_OUT;
				}
			}
		}
	}
	return result;
}

#ifndef IOT_API_ONLY
char *os_string_tolower(
	char *out,
	const char *in,
	size_t len )
{
	if ( in && out )
	{
		size_t i;
		for ( i = 0; i < len && in[i] != '\0'; i++ )
			out[i] = os_char_tolower( in[i] );
	}
	return out;
}

char *os_string_toupper(
	char *out,
	const char *in,
	size_t len )
{
	if ( in && out )
	{
		size_t i;
		for ( i = 0; i < len && in[i] != '\0'; i++ )
			out[i] = os_char_toupper( in[i] );
	}
	return out;
}

iot_status_t os_make_path( char *path, size_t len, ... )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( path && len > 0u )
	{
		va_list args;
		const char *dir;
		unsigned int dir_sep_len = 0u;
		size_t idx = 0u;
		va_start( args, len );
		dir = va_arg( args, const char * );
		result = IOT_STATUS_FULL;
		while ( dir && idx < ( len - dir_sep_len ) )
		{
			const size_t req_len = os_strlen( dir );
			if ( ( req_len + dir_sep_len ) < ( len - idx ) )
			{
				if ( idx > 0u && path[idx - 1u] != OS_DIR_SEP )
				{
					path[idx] = OS_DIR_SEP;
					++idx;
				}
				os_strncpy( &path[idx], dir, req_len + 1u );
			}
			idx += req_len;
			dir_sep_len = 1u;
			dir = va_arg( args, const char * );
		}
		va_end( args );

		if ( idx + dir_sep_len < len )
			result = IOT_STATUS_SUCCESS;
		else
			path[0] = '\0';
	}
	return result;
}

size_t os_strspn(
	const char *str1,
	const char *str2 )
{
	size_t result = 0u;
	if ( str1 && str2 )
	{
		iot_bool_t done;
		iot_bool_t match;
		const char *s1;
		const char *s2;

		for ( s1 = str1, done = IOT_FALSE;
			*s1 != '\0' && done == IOT_FALSE;
			s1++ )
		{
			for ( s2 = str2, match = IOT_FALSE;
				*s2 != '\0' && match == IOT_FALSE;
				s2++ )
				if ( *s1 == *s2 )
					match = IOT_TRUE;
			if ( match )
				++result;
			else
				done = IOT_TRUE;
		}
	}
	return result;
}

size_t os_strcspn(
	const char *str1,
	const char *str2 )
{
	size_t result = 0u;
	if ( str1 && str2 )
	{
		iot_bool_t match = IOT_FALSE;
		const char *s1;
		const char *s2;

		for ( s1 = str1;
			*s1 != '\0' && match == IOT_FALSE;
			s1++ )
		{
			for ( s2 = str2;
				*s2 != '\0' && match == IOT_FALSE;
				s2++ )
				if ( *s1 == *s2 )
					match = IOT_TRUE;
			if ( !match )
				++result;
		}
	}
	return result;
}

char *os_strtok(
	char *str,
	const char *delimiters )
{
	static char *old = NULL;
	char *result = NULL;

	if ( str == NULL )
		str = old;
	if ( str && *str != '\0' )
		str += os_strspn( str, delimiters );
	if ( str && *str != '\0' )
	{
		result = str;
		str = os_strpbrk( result, delimiters );
		if ( str )
		{
			*str = '\0';
			old = str + 1u;
		}
		else
			old = NULL;
	}
	return result;
}

int os_strcasecmp(
	const char *s1,
	const char *s2 )
{
	int result = 0;
	if ( s1 && s2 )
	{
		while( ( *s1 ) && ( *s2 ) &&
			( os_char_toupper( *s1 ) ==
			os_char_toupper( *s2 ) ) )
			s1++, s2++;
		result = *( const char* )s1 - *( const char* )s2;
	}
	else
	{
		if ( s1 )
			result = 1;
		else if ( s2 )
			result = -1;
	}
	return result;
}

int os_strncasecmp(
	const char *s1,
	const char *s2,
	size_t len )
{
	size_t i = 0u;
	int result = 0;
	if ( s1 && s2 )
	{
		while( ( *s1 ) && ( *s2 ) &&
			( os_char_toupper( *s1 ) ==
			os_char_toupper( *s2 ) ) &&
			( i < len ) )
			s1++, s2++, i++;
		result = *( const char* )s1 - *( const char* )s2;
	}
	else
	{
		if ( s1 )
			result = 1;
		else if ( s2 )
			result = -1;
	}
	return result;
}
#endif /* ifndef IOT_API_ONLY */

