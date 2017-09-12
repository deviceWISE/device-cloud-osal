/**
 * @file
 * @brief Source file for common test support functions
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "test_support.h"

#include <stdlib.h> /* for srand, fileno */
#include <time.h>   /* for time() */

/**
 * @brief Global variable whether mock low-level system functionality is enabled
 */
int MOCK_SYSTEM_ENABLED = 0;

void test_initialize( int argc, char **argv )
{
	MOCK_SYSTEM_ENABLED = 1;
}

void test_generate_random_string( char *dest, size_t len )
{
	static const char *random_chars =
	    "abcdefghijklmnopqrstuvwxyz"
	    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	    "0123456789,.-#'?!";
	if ( dest && len > 1u )
	{
		char *cur_pos;
		const char *random_char;
		size_t i;
		size_t random_chars_len = 0u;

		/* (platform independent) strlen */
		random_char = random_chars;
		while ( *random_char != '\0' )
		{
			++random_chars_len;
			++random_char;
		}

		/* obtain random character */
		cur_pos = dest;
		srand( (unsigned int)( ( size_t )( time( NULL ) ) * len ) );
		for ( i = 0; i < len - 1u; ++i )
		{
			*cur_pos = random_chars[(size_t)rand() % random_chars_len];
			++cur_pos;
		}
		dest[len - 1u] = '\0'; /* ensure null-terminated */
	}
}

void test_finalize( int argc, char **argv )
{
	MOCK_SYSTEM_ENABLED = 0;
}

