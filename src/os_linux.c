/**
 * @file
 * @brief source file defining functions for linux systems
 *
 * @copyright Copyright (C) 2017-2018 Wind River Systems, Inc. All Rights Reserved.
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

#include <dirent.h>      /* for closedir */
#include <dlfcn.h>       /* for dlclose, dlopen, dlsym */
#include <errno.h>       /* for ENOTEMPTY */
#include <pwd.h>         /* for getpwnam */
#include <regex.h>       /* for regex */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      /* for close */
#include <sys/resource.h> /* for process priority support */
#include <sys/stat.h>    /* for struct filestat, stat */
#include <sys/statvfs.h> /* for struct statvfs */
#include <sys/wait.h>    /* for waitpid */
#include <sys/utsname.h> /* for struct utsname */
#include <termios.h>     /* for terminal input */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"

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
#	define OS_COMMAND_PREFIX       "sudo"
#else
#	define OS_COMMAND_PREFIX       ""
#endif /* defined( __unix__ ) && !defined( __ANDROID__ ) */
/**
 * @brief Operating system reboot command
 */
#define OS_REBOOT_CMD                  "/sbin/shutdown -r"
/**
 * @brief Operating system shutdown command
 */
#define OS_SHUTDOWN_CMD                "/sbin/shutdown -h"
/**
 * @brief Time in milliseconds to wait between retrying an operation
 */
#define LOOP_WAIT_TIME 100u

#define OS_SERVICE_START_CMD           "systemctl start %s"
#define OS_SERVICE_STATUS_CMD          "systemctl status %s"
#define OS_SERVICE_STOP_CMD            "systemctl stop %s"

/* directory functions */
os_uint64_t os_directory_free_space( const char *path )
{
	os_uint64_t free_space = 0u;
	struct statvfs sfs;

	if ( statvfs( path, &sfs ) != -1 )
		free_space = (os_uint64_t)sfs.f_bsize *
			(os_uint64_t)sfs.f_bavail;
	return free_space;
}

os_status_t os_directory_delete(
	const char *path, const char *regex, os_bool_t recursive )
{
/** @brief maximum regular expression string length */
#define REGEX_MAX_LEN   64u
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( path )
	{
		regex_t regex_obj;
		const char *regex_pos = regex;
		char regex_str[REGEX_MAX_LEN];
		result = OS_STATUS_SUCCESS;
		if ( regex == NULL || *regex == '\0' )
			regex_pos = "*";
		else if ( os_strncmp( regex, ".", 2u ) == 0 ||
			os_strncmp( regex, "..", 3u ) == 0 ||
			os_strstr( regex, "/" ) != NULL )
			result = OS_STATUS_BAD_REQUEST;

		if ( result == OS_STATUS_SUCCESS )
		{
			/* convert file regular expressionto POSIX regular
			 * expression (i.e. "*.txt" -> "^.*\.txt$") */
			size_t i = 0u;
			regex_str[i++] = '^';
			/* REGEX_MAX_LEN - 3u: 1 spot for '$' character and
			 * 1 spot for '\0'. Which are after the loop to prevent
			 * going past the end of the array.  But inside array
			 * 'i' may increase by 2. */
			while ( *regex_pos && i < REGEX_MAX_LEN - 3u )
			{
				if ( *regex_pos == '.' || *regex_pos == '|' ||
					*regex_pos == '^' || *regex_pos == '$' ||
					*regex_pos == '[' || *regex_pos == ']' ||
					*regex_pos == '{' || *regex_pos == '}' ||
					*regex_pos == '(' || *regex_pos == ')' ||
					*regex_pos == '\\' || *regex_pos == '/' )
					regex_str[i++] = '\\';
				else if ( *regex_pos == '*' ||
					*regex_pos == '+' || *regex_pos == '?' )
					regex_str[i++] = '.';
				regex_str[i++] = *regex_pos;
				++regex_pos;
			}
			regex_str[i++] = '$';
			regex_str[i++] = '\0';
		}

		/* compile regular expression */
		if ( result == OS_STATUS_SUCCESS &&
			regcomp( &regex_obj, regex_str, REG_NOSUB ) )
				result = OS_STATUS_BAD_REQUEST;

		if ( result == OS_STATUS_SUCCESS )
		{
			DIR *d = opendir( path );
			if ( d )
			{
				struct dirent *p;
				/* loop through all files */
				while ( result == OS_STATUS_SUCCESS &&
					( p = readdir( d ) ) )
				{
					if ( strncmp( p->d_name, ".", 2u ) != 0 &&
					     strncmp( p->d_name, "..", 3u ) != 0 )
					{
						char *buf;
						size_t buf_size =
							strlen( path ) +
							strlen( p->d_name ) + 2u;
						buf = (char *)malloc( buf_size );
						if ( buf )
						{
							struct stat st;
							snprintf( buf, buf_size,
								"%s/%s", path,
								p->d_name );

							if ( stat( buf, &st ) == 0 )
							{
								/* check for matching
								 * files in sub-directory */
								if ( recursive != OS_FALSE &&
									S_ISDIR( st.st_mode ) )
									result = os_directory_delete(
										buf, regex, recursive );

								/* name matches regular
								 * expression */
								if ( regexec( &regex_obj,
									p->d_name, 0,
									NULL, 0 ) == 0 )
								{
									if ( S_ISDIR( st.st_mode ) )
									{
										/* delete all files
										 * within sub-directory */
										result = os_directory_delete(
											buf, NULL, OS_TRUE );
									}
									else if ( unlink( buf ) != 0 )
										result = OS_STATUS_FAILURE;
								}
							}
							else
								result = OS_STATUS_FAILURE;
							free( buf );
						}
						else
							result = OS_STATUS_NO_MEMORY;
					}
				}
				closedir( d );
			}
			else
				result = OS_STATUS_FAILURE;
			regfree( &regex_obj );

			/* delete the directory */
			if ( result == OS_STATUS_SUCCESS && regex == NULL )
			{
				int retval = rmdir( path );
				if ( retval == ENOTEMPTY )
					result = OS_STATUS_TRY_AGAIN;
				else if ( retval != 0 )
					result = OS_STATUS_FAILURE;
			}
		}
	}
	return result;
}

/* file operations */
os_status_t os_file_chown(
	const char *path,
	const char *user )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( path && user && *path != '\0' && *user != '\0' )
	{
		int sys_result = -1;
		struct passwd const *pwd = getpwnam( user );
		if ( pwd )
			sys_result = chown( path, pwd->pw_uid, pwd->pw_gid );
		if ( sys_result == 0 )
			result = OS_STATUS_SUCCESS;
		else
			result = OS_STATUS_FAILURE;
	}
	return result;
}

char os_key_wait( void )
{
	char result = '\0';
	struct termios new, old;
	tcgetattr( 0, &old ); /* grab old terminal i/o settings */
	new = old; /* make new settings same as old settings */
	new.c_lflag &= (unsigned int)~ICANON; /* disable buffered i/o */
	new.c_lflag &= (unsigned int)~ECHO; /* disable echo mode */
	tcsetattr( 0, TCSANOW, &new );
	result = (char)getchar();
	tcsetattr( 0, TCSANOW, &old );
	return result;
}

os_status_t os_library_close(
	os_lib_handle lib )
{
	os_status_t result = OS_STATUS_FAILURE;
	if ( lib && dlclose( lib ) == 0 )
		result = OS_STATUS_SUCCESS;
	return result;
}

#if defined(OSAL_WRAP) && OSAL_WRAP
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
#endif /* if defined(OSAL_WRAP) && OSAL_WRAP */

/* process functions */
os_status_t os_path_executable(
	char *path,
	size_t size )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( path )
	{
		result = OS_STATUS_FAILURE;
		if ( readlink( "/proc/self/exe", path, size ) > 0 )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_process_cleanup( void )
{
	os_status_t result = OS_STATUS_FAILURE;
	if ( waitpid( -1, NULL, WNOHANG ) > 0 )
		result = OS_STATUS_SUCCESS;
	return result;
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
	snprintf( service_cmd, 255u, OS_SERVICE_START_CMD, id );
	service_cmd[ 255u ] = '\0';
	result = os_system_run_wait( service_cmd, &exit_status, OS_TRUE,
		0, 0u, out_buf, out_len, timeout );
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
#if !defined( __ANDROID__)
	snprintf( service_cmd, 255u, OS_SERVICE_STATUS_CMD, id );
#else /* if !defined(__ANDROID__) */
	snprintf( service_cmd, 255u, OS_SERVICE_STATUS_CMD, exe );
#endif /* else if !defined(__ANDROID__) */
	service_cmd[ 255u ] = '\0';
	result = os_system_run_wait( service_cmd, &exit_status, OS_TRUE,
		0, 0u, out_buf, out_len, timeout );
	if ( result == OS_STATUS_SUCCESS && exit_status != 0 )
		result = OS_STATUS_NOT_FOUND;
	if ( result != OS_STATUS_NOT_FOUND )
	{
		snprintf( service_cmd, 255u, OS_SERVICE_STOP_CMD, id );
		service_cmd[ 255u ] = '\0';
		result = os_system_run_wait( service_cmd, &exit_status,
			OS_TRUE, 0, 0u, out_buf, out_len, timeout );
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

#if !defined(__ANDROID__)
	const char *status_cmds[] = { "show", "is-active", "is-failed" };
	const char *operation_cmd = "systemctl %s %s";
#else /* if !defined(__ANDROID__) */
	const char *status_cmds[] = { "ps | grep" };
	const char *operation_cmd = "%s %s";
#endif /* else if !defined(__ANDROID__) */
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
			OS_FALSE, 0, 0u, out_buf, out_len, timeout );
#if !defined(__ANDROID__)
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
#else /* if !defined(__ANDROID__) */
		if ( result != OS_STATUS_SUCCESS || exit_status != 0 )
			result = OS_STATUS_NOT_INITIALIZED;
#endif /* else if !defined(__ANDROID__) */
	}
	return result;
}

#if !defined(__ANDROID__)
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
	snprintf( service_cmd, 255u, "systemctl restart %s", id );
	service_cmd[ 255u ] = '\0';
	result = os_system_run_wait( service_cmd, &exit_status, OS_TRUE, 0, 0u,
		out_buf, out_len, timeout );

	if ( result == OS_STATUS_SUCCESS && exit_status != 0 )
		result = OS_STATUS_FAILURE;
	return result;
}
#endif /* if !defined(__ANDROID__) */

os_status_t os_stream_echo_set(
	os_file_t stream, os_bool_t enable )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( stream )
	{
		struct termios termios;
		result = OS_STATUS_FAILURE;
		if ( tcgetattr( fileno( stream ), &termios ) == 0 )
		{
			if ( enable )
				termios.c_lflag |= ECHO;
			else
				termios.c_lflag &= (unsigned int)~ECHO;

			if ( tcsetattr( fileno( stream ), TCSAFLUSH, &termios ) == 0 )
				result = OS_STATUS_SUCCESS;
		}
	}
	return result;
}

os_status_t os_system_info(
	os_system_info_t *sys_info )
{
	os_status_t result = OS_STATUS_FAILURE;
	struct utsname uts_info;

	if ( sys_info )
		memset( sys_info, 0, sizeof( struct os_system_info ) );

	/* call uname to get necessary information, i.e. system arch(machine)
	 which can't be read from /etc/os-release*/
	if ( sys_info && uname( &uts_info ) == 0 )
	{
		FILE *fp;
#if !defined(__ANDROID__)
		const char *const build_info_file = "/etc/os-release";
#else /* if !defined(__ANDROID__) */
		const char *const build_info_file = "/system/build.prop";
#endif /* else if !defined(__ANDROID__) */

		strncpy( sys_info->host_name, uts_info.nodename,
			OS_SYSTEM_INFO_MAX_LEN );
		strncpy( sys_info->system_name, uts_info.sysname,
			OS_SYSTEM_INFO_MAX_LEN );
		strncpy( sys_info->system_platform, uts_info.machine,
			OS_SYSTEM_INFO_MAX_LEN );
		strncpy( sys_info->system_version, uts_info.version,
			OS_SYSTEM_INFO_MAX_LEN );
		strncpy( sys_info->kernel_version, uts_info.release,
			OS_SYSTEM_INFO_MAX_LEN );

		/* Read "ID" and "VERSION_ID" field from
		/etc/os-release if it exists */
		fp = fopen( build_info_file, "r" );
		if ( fp != NULL )
		{
			char *line = NULL;
			size_t len = 0;
#if !defined(__ANDROID__)
			const char *const id_field = "ID";
			const char *const variant_id_field = "VARIANT_ID";
			const char *const version_field = "VERSION_ID";
#else /* if !defined(__ANDROID__) */
			const char *const id_field = "ro.product.brand";
			const char *const variant_id_field = "ro.build.flavor";
			const char *const version_field = "ro.build.version.release";
#endif /* else if !defined(__ANDROID__) */
			while ( getline( &line, &len, fp ) != -1 )
			{
				/* Remove special characters in the field to
				adapt to different distribution */
				const char *id = NULL;
				const char *value = NULL;
				char *pos = line;
				while ( *pos != '\0' && len > 0u )
				{
					if( *pos == '\t' || *pos == '\n' || *pos == '"' )
						memmove( pos, pos + 1u, len - 1u );
					else
					{
						if ( !id )
							id = pos;
						else if ( !value && *pos == '=' )
						{
							*pos = '\0';
							value = pos + 1u;
						}
						++pos;
					}
					--len;
				}

				if ( value )
				{
					if ( strcmp( id, id_field ) == 0 )
#if !defined(__ANDROID__)
						strncpy( sys_info->system_name,
							value,
							OS_SYSTEM_INFO_MAX_LEN );
#else /* if !defined(__ANDROID__) */
						strncpy( sys_info->system_name,
							"Android",
							OS_SYSTEM_INFO_MAX_LEN );
#endif /* else if !defined(__ANDROID__) */
					else if ( strcmp( id, version_field ) == 0 )
						strncpy( sys_info->system_version,
							value,
							OS_SYSTEM_INFO_MAX_LEN );
					else if ( strcmp( id, variant_id_field ) == 0 )
						strncpy( sys_info->system_release,
							value,
							OS_SYSTEM_INFO_MAX_LEN );
				}
			}

			if ( line )
				free( line );
			fclose( fp );
		}
		if ( strncmp( sys_info->system_name, "wrlinux",
			OS_SYSTEM_INFO_MAX_LEN ) == 0 )
			strncpy( sys_info->vendor_name, "Wind River",
				OS_SYSTEM_INFO_MAX_LEN );
		else
		{
			strncpy( sys_info->vendor_name, sys_info->system_name,
				OS_SYSTEM_INFO_MAX_LEN );
		}
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_system_run(
	const char *command,
	int *exit_status,
	os_bool_t privileged,
	int priority,
	size_t stack_size,
	os_file_t pipe_files[2u] )
{
	size_t i;
	const int output_fd[2u] = { STDOUT_FILENO, STDERR_FILENO };
	int command_output_fd[2u] = { -1, -1 };
	os_status_t result = OS_STATUS_NOT_EXECUTABLE;
	os_timestamp_t start_time;
	pid_t pid;

	os_time( &start_time, NULL );

	for ( i = 0u; i < 2u; ++i )
		if( pipe_files[i] != NULL )
			command_output_fd[i] = fileno( pipe_files[i] );

	/* set a default exit status */
	if ( exit_status )
		*exit_status = -1;

	pid = fork();
	if ( pid != -1 )
	{
		/* pid = 0, for child... child_process_id returned to parent */
		if ( pid == 0 )
		{
			/* Create a new session for the child process.
			 */
			pid_t sid = setsid();
			if ( sid < 0 )
				exit( errno );
			/* redirect child stdout/stderr to the pipe */
			for ( i = 0u; i < 2u; ++i )
				dup2( command_output_fd[i], output_fd[i] );

			/* set priority */
			if ( priority != 0 )
			{
				int cur_priority = 0;
				cur_priority = getpriority( PRIO_PROCESS, (id_t)sid );
				priority += cur_priority;
				if ( priority < -20 )
					priority = -20;
				else if ( priority > 20 )
					priority = 20;
				setpriority( PRIO_PROCESS, (id_t)sid, priority );
			}

			/* set stack size */
			if ( stack_size > 0 )
			{
				struct rlimit lim;
				lim.rlim_cur = stack_size;
				lim.rlim_max = stack_size;
				setrlimit(RLIMIT_STACK, &lim);
			}

			if ( privileged == OS_FALSE )
				execl( OS_COMMAND_SH, command, (char *)NULL );
			else
				execl( OS_COMMAND_SH, OS_COMMAND_PREFIX,
					command, (char *)NULL );

			/* Process failed to be replaced, return failure */
			exit( errno );
		}

		for ( i = 0u; i < 2u; ++i )
			close( command_output_fd[i] );

		result = OS_STATUS_INVOKED;
	}
	return result;
}

os_status_t os_system_run_wait(
	const char *command,
	int *exit_status,
	os_bool_t privileged,
	int priority,
	size_t stack_size,
	char *out_buf[2u],
	size_t out_len[2u],
	os_millisecond_t max_time_out )
{
	int command_output_fd[2u][2u] =
		{ { -1, -1 }, { -1, -1 } };
	size_t i;
	const int output_fd[2u] = { STDOUT_FILENO, STDERR_FILENO };
	os_status_t result = OS_STATUS_SUCCESS;
	os_timestamp_t start_time;
	int system_result = -1;
	os_millisecond_t time_elapsed;

	os_time( &start_time, NULL );

	/* set a default exit status */
	if ( exit_status )
		*exit_status = -1;

	/* capture the stdout & stderr of the command and send it back
	 * as the response */
	for ( i = 0u; i < 2u && result == OS_STATUS_SUCCESS; ++i )
		if ( pipe( command_output_fd[i] ) != 0 )
			result = OS_STATUS_IO_ERROR;

	if ( result == OS_STATUS_SUCCESS )
	{
		const pid_t pid = fork();
		result = OS_STATUS_NOT_EXECUTABLE;
		if ( pid != -1 )
		{
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
				if ( priority != 0 )
				{
					int cur_priority = 0;
					cur_priority = getpriority( PRIO_PROCESS, (id_t)sid );
					priority += cur_priority;
					if ( priority < -20 )
						priority = -20;
					else if ( priority > 20 )
						priority = 20;
					setpriority( PRIO_PROCESS, (id_t)sid, priority );
				}

				/* set stack size */
				if ( stack_size > 0)
				{
					struct rlimit lim;
					lim.rlim_cur = stack_size;
					lim.rlim_max = stack_size;
					setrlimit(RLIMIT_STACK, &lim);
				}

				if ( privileged == OS_FALSE )
					execl( OS_COMMAND_SH, command, (char *)NULL );
				else
					execl( OS_COMMAND_SH, OS_COMMAND_PREFIX,
						command, (char *)NULL );

				/* Process failed to be replaced, return failure */
				exit( errno );
			}

			for ( i = 0u; i < 2u; ++i )
				close( command_output_fd[i][1] );

			errno = 0;
			do {
				waitpid( pid, &system_result, WNOHANG );
				os_time_elapsed( &start_time, &time_elapsed );
				os_time_sleep( LOOP_WAIT_TIME, OS_FALSE );
			} while ( ( errno != ECHILD ) &&
				( !WIFEXITED( system_result ) ) &&
				( !WIFSIGNALED( system_result ) ) &&
				( max_time_out == 0u || time_elapsed < max_time_out ) );

			if ( ( errno != ECHILD ) &&
				!WIFEXITED( system_result ) &&
				!WIFSIGNALED( system_result ) )
			{
				kill( pid, SIGTERM );
				waitpid( pid, &system_result, WNOHANG );
				result = OS_STATUS_TIMED_OUT;
			}
			else
				result = OS_STATUS_SUCCESS;

			fflush( stdout );
			fflush( stderr );

			for ( i = 0u; i < 2u; ++i )
			{
				if ( out_buf[i] && out_len[i] > 0u )
				{
					out_buf[i][0] = '\0';
					/* if we are able to read from pipe */
					if ( command_output_fd[i][0] != -1 )
					{
						const ssize_t output_size =
							read( command_output_fd[i][0],
							out_buf[i], out_len[i] - 1u );
						if ( output_size >= 0 )
							out_buf[i][ output_size ] = '\0';
					}
				}
			}

			if ( WIFEXITED( system_result ) )
				system_result = WEXITSTATUS( system_result );
			else if ( WIFSIGNALED( system_result ) )
				system_result = WTERMSIG( system_result );
			else
				system_result = WIFEXITED( system_result );
			if ( exit_status )
				*exit_status = system_result;
		}
	}
	return result;
}

os_status_t os_system_shutdown(
	os_bool_t reboot, unsigned int delay)
{
	char cmd[ PATH_MAX ];
	os_file_t out_files[2] = { NULL, NULL };

	if ( reboot == OS_FALSE )
		os_snprintf( cmd, PATH_MAX, "%s %d", OS_SHUTDOWN_CMD, delay );
	else
		os_snprintf( cmd, PATH_MAX, "%s %d", OS_REBOOT_CMD, delay );

	return os_system_run( cmd, NULL, OS_FALSE, 0, 0u, out_files );
}


/* thread support */
#if defined(OSAL_THREAD_SUPPORT) && OSAL_THREAD_SUPPORT
os_status_t os_thread_rwlock_create(
	os_thread_rwlock_t *lock )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( lock )
	{
		result = OS_STATUS_FAILURE;
		if ( pthread_rwlock_init( lock, NULL ) == 0 )
			result = OS_STATUS_SUCCESS;
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
		if ( pthread_rwlock_rdlock( lock ) == 0 )
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
		if ( pthread_rwlock_unlock( lock ) == 0 )
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
		if ( pthread_rwlock_wrlock( lock ) == 0 )
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
		if ( pthread_rwlock_unlock( lock ) == 0 )
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
		if ( pthread_rwlock_destroy( lock ) == 0 )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}
#endif /* if defined(OSAL_THREAD_SUPPORT) && OSAL_THREAD_SUPPORT */

#pragma clang diagnostic pop

