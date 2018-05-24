/**
 * @file
 * @brief Android OS adaptation layer
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
#include "src/os_android.h"

os_status_t os_service_restart(
	const char *id,
	const char *exe,
	os_millisecond_t timeout
)
{
	os_status_t result;
	int exit_status;
	char service_cmd[ 256u ];
	char stdout_buf[COMMAND_OUTPUT_MAX_LEN] = "\0";
	char stderr_buf[COMMAND_OUTPUT_MAX_LEN] = "\0";
	char *out_buf[2u] = { stdout_buf, stderr_buf };
	size_t out_len[2u] = { COMMAND_OUTPUT_MAX_LEN, COMMAND_OUTPUT_MAX_LEN };

	if ( !exe )
		exe = id;

	snprintf( service_cmd, 255u, "ps | grep %s", exe );
	service_cmd[ 255u ] = '\0';
	result = os_system_run_wait( service_cmd, &exit_status,
		OS_TRUE, 0, 0u, out_buf, out_len, timeout );
	if ( result == OS_STATUS_SUCCESS )
	{
		if ( exit_status != 0 )
		{
			snprintf( service_cmd, 255u, "start %s", id );
			service_cmd[ 255u ] = '\0';
			result = os_system_run_wait( service_cmd, &exit_status,
				OS_TRUE, 0, 0u, out_buf, out_len, timeout );
		}
		else
		{
			os_bool_t first_space_found = OS_FALSE;
			os_bool_t pid_found = OS_FALSE;
			size_t i = 0u;
			char *pid = NULL;
			while ( pid_found == OS_FALSE &&
				i < COMMAND_OUTPUT_MAX_LEN )
			{
				/* search for the first space in stdout */
				if ( first_space_found == OS_FALSE &&
					stdout_buf[i] == ' ' )
					first_space_found = OS_TRUE;

				if ( first_space_found == OS_TRUE )
				{
					if ( pid == NULL &&
						stdout_buf[i] >= '0' &&
						stdout_buf[i] <= '9' )
						pid = &stdout_buf[i];

					if ( pid != NULL &&
						stdout_buf[i] == ' ' )
					{
						stdout_buf[i] = '\0';
						pid_found = OS_TRUE;
					}
				}
				++i;
			}

			if ( pid_found == OS_FALSE )
			{
				/* service is not found */
				result = OS_STATUS_NOT_FOUND;
			}
			else
			{
				/* kill the current process
				 * let the system start it again
				 */
				snprintf( service_cmd, 255u, "kill -9 %s", pid );
				service_cmd[ 255u ] = '\0';
				result = os_system_run_wait(
					service_cmd, &exit_status,
					OS_TRUE, 0, 0u, out_buf, out_len, timeout );
			}
		}
	}

	if ( result == OS_STATUS_SUCCESS && exit_status != 0 )
		result = OS_STATUS_FAILURE;
	return result;
}

