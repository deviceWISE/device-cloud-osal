/**
 * @file
 * @brief VxWorks operating system abstraction layer
 *
 * @copyright Copyright (C) 2016-2018 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied."
 */

#include "os.h"

#include <ioLib.h>
#include <pthread.h>
#include <semLib.h>
#include <stdlib.h> /* for EXIT_SUCCESS */
#include <string.h>
#include <taskLib.h>
#include <rtpLib.h>
#include <wait.h>
#include <version.h>

#if defined(_WRS_KERNEL)
#include <sysLib.h>
#include <vsbConfig.h>
#include <bootLib.h>
#include <shellLib.h>
#ifdef _WRS_CONFIG_SYS_PWR_OFF
#include <powerOffLib.h>
#endif /* _WRS_CONFIG_SYS_PWR_OFF */
extern BOOT_PARAMS sysBootParams;
#endif /* _WRS_KERNEL */

#define VX_RW_SEM_MAX_READERS (255)

#if defined(_WRS_KERNEL)
extern int control_main ( int argc, char* argv[] );
extern int iot_update_main ( int argc, char* argv[] );
#endif /* if(defined(_WRS_KERNEL) */

/* directory operations */
os_uint64_t os_directory_free_space( const char* path )
{
	return 0;
}

/** @todo fix later once it is needed in vxWorks */
os_status_t os_directory_delete(
	const char *path, const char *regex, os_bool_t recursive )
{
	os_status_t result = OS_STATUS_FAILURE;

	if (regex == NULL && rmdir (path) == OK)
		result = OS_STATUS_SUCCESS;

	return result;
}

/* file operations */
os_status_t os_file_chown(
	const char *path,
	const char *user )
{
	return OS_STATUS_FAILURE;
}

/* process functions */
os_status_t os_path_executable(
	char *path,
	size_t size )
{

	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( path )
	{
		const char *task_name = taskName(taskIdSelf());
		result = OS_STATUS_FAILURE;
		if (task_name && size > 2u)
		{
			strncpy(&path[0u], task_name, size - 1u);
			path[size - 1u] = '\0';
			result = OS_STATUS_SUCCESS;
		}
	}
	return result;
}

os_status_t os_process_cleanup( void )
{
	return OS_STATUS_FAILURE;
}

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

/** @todo fix later once it needs in vxWorks */
os_status_t os_stream_echo_set(
	os_file_t stream, os_bool_t enable )
{
	return OS_STATUS_FAILURE;
}

os_status_t os_system_info(
	os_system_info_t *sys_info )
{
	if ( sys_info )
	{
		memset( sys_info, 0, sizeof( struct os_system_info ) );

		/*
		* Populate reasonable values, knowing this is on
		* VxWorks, provided by Wind River Systems.
		*/

		strncpy( sys_info->vendor_name, "Wind River Systems",
			OS_SYSTEM_INFO_MAX_LEN );
		strncpy( sys_info->system_name, RUNTIME_NAME,
			OS_SYSTEM_INFO_MAX_LEN );
		strncpy( sys_info->system_version, VXWORKS_VERSION,
			OS_SYSTEM_INFO_MAX_LEN );
		strncpy( sys_info->system_release,
			_WRS_CONFIG_CORE_KERNEL_VERSION,
			OS_SYSTEM_INFO_MAX_LEN );
		strncpy( sys_info->system_platform, sysModel(),
			OS_SYSTEM_INFO_MAX_LEN );
#if defined(_WRS_KERNEL)
		strncpy( sys_info->host_name, sysBootParams.targetName,
			OS_SYSTEM_INFO_MAX_LEN );
#else /* if defined(_WRS_KERNEL) */
		strncpy( sys_info->host_name, "", OS_SYSTEM_INFO_MAX_LEN );
#endif /* else if defined(_WRS_KERNEL) */
		sys_info->system_flags = 0;
	}
	return OS_STATUS_SUCCESS;
}

#if defined(OSAL_THREAD_SUPPORT) && OSAL_THREAD_SUPPORT
os_status_t os_thread_rwlock_create(
	os_thread_rwlock_t *lock )
{
	SEM_ID semId;
	os_status_t result = OS_STATUS_BAD_PARAMETER;

	if ( lock )
	{
		result = OS_STATUS_FAILURE;
		semId = semRWCreate( 0, VX_RW_SEM_MAX_READERS );
		if ( SEM_ID_NULL != semId )
		{
			*lock = semId;
		}
	}

	return result;
}

os_status_t os_thread_rwlock_read_lock(
	os_thread_rwlock_t *lock )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;

	if ( lock )
	{
		result = OS_STATUS_FAILURE;
		if ( OK == semRTake( *lock, WAIT_FOREVER ) )
			result = OS_STATUS_SUCCESS;
	}

	return result;
}

os_status_t os_thread_rwlock_read_unlock(
	os_thread_rwlock_t *lock )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;

	if ( lock )
	{
		result = OS_STATUS_FAILURE;
		if (  OK == semGive( *lock ) )
			result = OS_STATUS_SUCCESS;
	}

	return result;
}

os_status_t os_thread_rwlock_write_lock(
	os_thread_rwlock_t *lock )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;

	if ( lock )
	{
		result = OS_STATUS_FAILURE;
		if ( OK == semWTake( *lock, WAIT_FOREVER ) )
			result = OS_STATUS_SUCCESS;
	}

	return result;
}

os_status_t os_thread_rwlock_write_unlock(
	os_thread_rwlock_t *lock )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;

	if ( lock )
	{
		result = OS_STATUS_FAILURE;
		if (  OK == semGive( *lock ) )
			result = OS_STATUS_SUCCESS;
	}

	return result;
}

os_status_t os_thread_rwlock_destroy(
	os_thread_rwlock_t *lock )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;

	if ( lock )
	{
		result = OS_STATUS_FAILURE;
		if ( OK == semDelete( *lock ) )
			result = OS_STATUS_SUCCESS;
	}

	return result;
}
#endif /* if defined(OSAL_THREAD_SUPPORT) && OSAL_THREAD_SUPPORT */

#if defined(_WRS_KERNEL)
static void os_vxworks_reboot( void )
{
	/* Wait 5 seconds for messages to propagate */

	sleep (5);

	/* Force a cold reboot - We do not return */

	sysToMonitor(2);
}

#if defined(_WRS_CONFIG_SYS_PWR_OFF)
static void os_vxworks_shutdown( void )
{
	/* Wait 5 seconds for messages to propagate */

	sleep (5);

	powerOff();
}
#endif /* _WRS_CONFIG_SYS_PWR_OFF */

static os_status_t os_vxworks_script(
	const char * script )
{
	os_status_t result = OS_STATUS_FAILURE;
	char * shell_task_name;
	int fd;

	if ( ( fd = open( script, O_RDONLY, 0 ) ) == ERROR ) {
		return OS_STATUS_FAILURE;
	}

	if ( shellGenericInit( "INTERPRETER=C", 0, NULL, &shell_task_name,i
		FALSE, FALSE, fd, STD_OUT, STD_ERR ) == OK ) {
		result = OS_STATUS_SUCCESS;
	}

	do {
		taskDelay( sysClkRateGet() );
	} while ( taskNameToId( shell_task_name ) != TASK_ID_ERROR );

	close( fd );

	return result;
}
#endif /* _WRS_KERNEL */

os_status_t os_system_run(
	os_system_run_args_t *args )
{
	unsigned int i;
	char **argv = NULL;
	int argc = 0, argc_total;
	os_bool_t is_executable = OS_FALSE;

	if ( !args || (!args->cmd && !args->fptr) )
		return OS_STATUS_BAD_PARAMETER;

	/* set a default exit status */
	args->return_code = -1;

	/* tokenize the command */
	/* converts: "a.out \"param 1\" param2 param3"
	 * to: argv[0] = "a.out"
	 *     argv[1] = "param 1"
	 *     argv[2] = "param2"
	 *     argv[3] = "param3"
	 *     argv[4] = NULL
	 *
	 * 2 passes: - 1) calculate amount of memory to dynamically allocate
	 *           - 2) store values in allocated memory
	 */
	for ( i = 0u; i < 2u; ++i )
	{
		size_t cmd_len = 0u;
		char in_quote = '\0';
		const char *cmd_pos, *cmd_start = args->cmd;
		int ignore_next = 0;
		argc = 0;
		cmd_pos = args->cmd;
		while ( cmd_pos )
		{
			/* handle quoted arguments */
			if ( !ignore_next && ((in_quote == '\0' && (*cmd_pos == '\"' || *cmd_pos == '\'')) ||
				(in_quote != '\0' && *cmd_pos == in_quote)))
			{
				if ( in_quote == '\0' )
					in_quote = *cmd_pos;
				else
					in_quote = '\0';
			}
			else if ( (!ignore_next && in_quote == '\0' && *cmd_pos == ' ') || *cmd_pos == '\0')
			{
				size_t arg_len = cmd_pos - cmd_start;
				if ( i > 0u )
				{
					char *d;
					argv[argc] = ((char *)argv + (sizeof(char*) * (argc_total + 1)) + cmd_len);
					strncpy(argv[argc], cmd_start, arg_len);
					argv[argc][arg_len] = '\0';
					argv[argc + 1] = NULL;
				}
				cmd_start = cmd_pos + 1;
				cmd_len += arg_len + 1;
				++argc;
			}

			if ( ignore_next )
				ignore_next = 0;
			else if ( *cmd_pos == '\\' )
				ignore_next = 1;

			if ( *cmd_pos == '\0' )
				cmd_pos = NULL;
			else
				++cmd_pos;
		}

		/* create array storage location */
		if ( i == 0u )
		{
			int j;
			argv = (char**)malloc((sizeof(char *) * (argc + 1)) +
				              (sizeof(char) * (cmd_len + 1)));
			argc_total = argc;
			if ( argv )
				argv[0] = NULL;
			else
				return OS_STATUS_FAILURE;
		}
	}

	/* determine if file points to an executable file */
	if ( argv && argv[0]
#if defined( _WRS_KERNEL )
		&& !args->fptr
#endif /* if defined( _WRS_KERNEL ) */
	   )
	{
		FILE *fstream = fopen( argv[0], "r" );
		if ( fstream )
		{
			/* test for ELF file format */
			char buf[4u];
			char elf_magic[4u] = { 0x7F, 'E', 'L', 'F' };
			if ( fread( buf, sizeof(char), sizeof(buf), fstream)
				== sizeof(buf) && memcmp(buf, elf_magic,
				sizeof(elf_magic)) == 0)
			{
				is_executable = OS_TRUE;
			}
			fclose( fstream );
		}
	}

	if ( is_executable != OS_FALSE )
	{
		int i;
		const char *app_name = strrchr(argv[0], '/');
		if ( !app_name )
			app_name = argv[0];
		else
			++app_name;

		/* rtpSpawn: Spawn Real-Time Process (task)
		 * - process executable path
		 * - command line array (must be NULL terminated)
		 * - environment variables
		 * - priority
		 * - stack size
		 * - RTP options
		 * - task options
		 */
		if ( rtpSpawn( argv[0], argv, NULL,
			args->priority, args->stack_size,
			RTP_LOADED_WAIT, VX_FP_TASK) == RTP_ID_ERROR)
		{
			free( argv );
			return OS_STATUS_FAILURE;
		}
	}
#if defined( _WRS_KERNEL )
	else if ( args->fptr )
	{
		const char *task_name = "fPtr";
		if ( argv && argv[0] )
			task_name = argv[0];
		if (taskSpawn (task_name, args->priority, 0, args->stack_size,
			(FUNCPTR)args->fptr,
			argc, argv, 0, 0, 0, 0, 0, 0, 0, 0) == TASK_ID_ERROR)
		{
			if ( argv )
				free( argv );
			return OS_STATUS_FAILURE;
		}
	}
	else if ( argv && argv[0] && strncmp (argv[0], "sh", 2) == 0 )
	{
		if (os_vxworks_script(argv[1]) == OS_STATUS_FAILURE)
		{
			free( argv );
			return OS_STATUS_FAILURE;
		}
	}
	else if ( argv && os_vxworks_script(argv[0]) == OS_STATUS_FAILURE )
	{
		free( argv );
		return OS_STATUS_FAILURE;
	}
#else /* if defined( _WRS_KERNEL ) */
	else
	{
		printf( "Invalid command: %s\n", args->cmd );
		if ( argv )
			free( argv );
		return OS_STATUS_FAILURE;
	}
#endif /* else if defined( _WRS_KERNEL ) */

	args->return_code = 0;
	if ( argv )
		free( argv );
	return OS_STATUS_SUCCESS;
}

os_status_t os_system_shutdown(
	os_bool_t reboot, unsigned int delay)
{
	os_status_t result = OS_STATUS_FAILURE;
#if defined(_WRS_KERNEL)
	if (reboot != OS_FALSE)
	{
		if (taskSpawn ("tReboot", 10, 0, 0x1000,
			(FUNCPTR) os_vxworks_reboot,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0) != TASK_ID_ERROR) {
			result = OS_STATUS_SUCCESS;
		}
	}
#if defined(_WRS_CONFIG_SYS_PWR_OFF)
	else
	{
		if (taskSpawn ("tShutdown", 10, 0, 0x1000,
			(FUNCPTR) os_vxworks_shutdown,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0) != TASK_ID_ERROR) {
			result = OS_STATUS_SUCCESS;
		}
	}
#endif /* if defined(_WRS_CONFIG_SYS_PWR_OFF) */
#endif /* if defined(_WRS_KERNEL) */
	return result;
}
