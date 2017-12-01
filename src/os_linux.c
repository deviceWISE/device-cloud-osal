/**
 * @file
 * @brief source file defining functions for linux systems
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
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

#include "os.h"

#include <stdlib.h>
#include <string.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"

#define COMMAND_PREFIX           "sudo "
#define SERVICE_START_CMD        "systemctl start %s"
#define SERVICE_STATUS_CMD       "systemctl status %s"
#define SERVICE_STOP_CMD         "systemctl stop %s"

/* service functions */
os_status_t os_service_run(
	const char *id,
	os_service_main_t service_function,
	int argc,
	char *argv[],
	int remove_argc,
	const char *remove_argv[],
	os_sighandler_t UNUSED(handler),
	const char *UNUSED(logdir) )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( id && service_function )
	{
		int i;
		char** good_argv = (char**)malloc( (unsigned long)argc * sizeof( char* ) );
		result = OS_STATUS_FAILURE;
		/* remove bad arguments */
		if ( good_argv )
		{
			int good_argc = 0u;
			for ( i = 0; i < argc; ++i )
			{
				int is_good_arg = 1; /* true */
				if ( remove_argc > 0 && remove_argv && argv[i] )
				{
					int j;
					for ( j = 0;
						j < remove_argc && is_good_arg; ++j )
					{
						if ( remove_argv[j] )
						{
							size_t arg_len =
								strlen( remove_argv[j] );
							is_good_arg = strncmp( argv[i],
								remove_argv[j], arg_len );
						}
					}
				}

				if ( is_good_arg )
				{
					good_argv[good_argc] = argv[i];
					++good_argc;
				}
			}

			if ( ( *service_function )( good_argc, good_argv ) == EXIT_SUCCESS )
				result = OS_STATUS_SUCCESS;

			free( good_argv );
		}
	}
	return result;
}

os_status_t os_service_install(
	const char *UNUSED(id),
	const char *UNUSED(executable),
	const char *UNUSED(args),
	const char *UNUSED(name),
	const char *UNUSED(description),
	const char *UNUSED(dependencies),
	os_millisecond_t UNUSED(timeout)
)
{
	return OS_STATUS_NOT_SUPPORTED;
}

os_status_t os_service_uninstall(
	const char *UNUSED(id),
	os_millisecond_t UNUSED(timeout)
)
{
	return OS_STATUS_NOT_SUPPORTED;
}

os_status_t os_service_start(
	const char *id,
	os_millisecond_t timeout
)
{
	os_status_t result;
	int exit_status;
	char *out_buf[2u] = { NULL, NULL };
	size_t out_len[2u] = { 0u, 0u };
	char service_cmd[ 256u ];
	snprintf( service_cmd, 255u, SERVICE_START_CMD, id );
	service_cmd[ 255u ] = '\0';
	result = os_system_run_wait( service_cmd, &exit_status,
		out_buf, out_len, timeout );
	if ( result == OS_STATUS_SUCCESS && exit_status != 0 )
		result = OS_STATUS_FAILURE;
	return result;
}

os_status_t os_service_stop(
	const char *id,
	const char *exe,
	os_millisecond_t timeout
)
{
	os_status_t result;
	int exit_status;
	char *out_buf[2u] = { NULL, NULL };
	size_t out_len[2u] = { 0u, 0u };
	char service_cmd[ 256u ];

	if ( !exe )
		exe = id;

	/* if the service is not installed or not running, skip stop */
#	ifndef __ANDROID__
	snprintf( service_cmd, 255u, SERVICE_STATUS_CMD, id );
#	else
	snprintf( service_cmd, 255u, SERVICE_STATUS_CMD, exe );
#	endif
	service_cmd[ 255u ] = '\0';
	result = os_system_run_wait( service_cmd, &exit_status,
		out_buf, out_len, timeout );
	if ( result == OS_STATUS_SUCCESS && exit_status != 0 )
		result = OS_STATUS_NOT_FOUND;
	if ( result != OS_STATUS_NOT_FOUND )
	{
		snprintf( service_cmd, 255u, SERVICE_STOP_CMD, id );
		service_cmd[ 255u ] = '\0';
		result = os_system_run_wait( service_cmd, &exit_status,
			out_buf, out_len, timeout );
		if ( result == OS_STATUS_SUCCESS && exit_status != 0 )
			result = OS_STATUS_FAILURE;
	}
	return result;
}

os_status_t os_service_query(
	const char *id,
	os_millisecond_t timeout
)
{
	size_t i;
	os_status_t result = OS_STATUS_SUCCESS;

#	ifndef __ANDROID__
	const char *status_cmds[] = { "show", "is-active", "is-failed" };
	const char *operation_cmd = "systemctl %s %s";
#	else /* __ANDROID__ */
	const char *status_cmds[] = { "ps | grep" };
	const char *operation_cmd = "%s %s";
#	endif /* __ANDROID__ */
	for ( i = 0u; result == OS_STATUS_SUCCESS &&
		i < sizeof( status_cmds ) / sizeof( const char * ); ++i )
	{
		int exit_status = 0;
		char *out_buf[2u] = { NULL, NULL };
		size_t out_len[2u] = { 0u, 0u };
		char service_cmd[ 256u ];
		snprintf( service_cmd, 255u, operation_cmd,
			status_cmds[i], id );
		service_cmd[ 255u ] = '\0';
		result = os_system_run_wait( service_cmd, &exit_status,
			out_buf, out_len, timeout );
#	ifndef __ANDROID__
		if ( result == OS_STATUS_SUCCESS )
		{
			/* is-failed returns 0 if it's failed */
			if ( ( exit_status != 0 && i != 2u ) ||
			     ( exit_status == 0 && i == 2u ) )
				result = OS_STATUS_FAILURE;
		}

		if ( result == OS_STATUS_FAILURE )
		{
			if ( i == 0u )
				result = OS_STATUS_NOT_FOUND;
			else if ( i == 1u )
				result = OS_STATUS_NOT_INITIALIZED;
		}
#	else /* __ANDROID__ */
		if ( result != OS_STATUS_SUCCESS || exit_status != 0 )
			result = OS_STATUS_NOT_INITIALIZED;
#	endif /* __ANDROID__ */
	}
	return result;
}

os_status_t os_service_restart(
	const char *id,
	const char *UNUSED(exe),
	os_millisecond_t timeout
)
{
	os_status_t result;
	int exit_status;
	char service_cmd[ 256u ];
	
	char *out_buf[2u] = { NULL, NULL };
	size_t out_len[2u] = { 0u, 0u };
	snprintf( service_cmd, 255u, COMMAND_PREFIX "systemctl restart %s", id );
	service_cmd[ 255u ] = '\0';
	result = os_system_run_wait( service_cmd, &exit_status,
		out_buf, out_len, timeout );

	if ( result == OS_STATUS_SUCCESS && exit_status != 0 )
		result = OS_STATUS_FAILURE;
	return result;
}

#pragma clang diagnostic pop

