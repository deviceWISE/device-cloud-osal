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
#include <sys/stat.h>    /* for stat */
#include <sys/statvfs.h> /* for struct statvfs */
#include <sys/utsname.h> /* for struct utsname */
#include <sys/wait.h>    /* for waitpid */
#include <termios.h>     /* for terminal input */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"

/**
 * @brief Operating system reboot command
 */
#define OS_REBOOT_CMD                  "/sbin/shutdown -r"
/**
 * @brief Operating system shutdown command
 */
#define OS_SHUTDOWN_CMD                "/sbin/shutdown -h"

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
	os_system_run_args_t args = OS_SYSTEM_RUN_ARGS_INIT;
	os_status_t result;
	char service_cmd[ 256u ];

	snprintf( service_cmd, 255u, OS_SERVICE_START_CMD, id );
	service_cmd[ 255u ] = '\0';

	args.cmd = service_cmd;
	args.privileged = OS_TRUE;
	args.block = OS_TRUE;
	args.opts.block.max_wait_time = timeout;
	result = os_system_run( &args );
	if ( result == OS_STATUS_SUCCESS && args.return_code != 0 )
		result = OS_STATUS_FAILURE;
	return result;
}

os_status_t os_service_stop(
	const char *id,
	const char *exe,
	os_millisecond_t timeout
)
{
	os_system_run_args_t args = OS_SYSTEM_RUN_ARGS_INIT;
	os_status_t result;
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
	args.cmd = service_cmd;
	args.block = OS_TRUE;
	args.privileged = OS_TRUE;
	args.opts.block.max_wait_time = timeout;
	result = os_system_run( &args );
	if ( result == OS_STATUS_SUCCESS && args.return_code != 0 )
		result = OS_STATUS_NOT_FOUND;
	if ( result != OS_STATUS_NOT_FOUND )
	{
		snprintf( service_cmd, 255u, OS_SERVICE_STOP_CMD, id );
		service_cmd[ 255u ] = '\0';
		result = os_system_run( &args );
		if ( result == OS_STATUS_SUCCESS && args.return_code != 0 )
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
		os_system_run_args_t args = OS_SYSTEM_RUN_ARGS_INIT;
		char service_cmd[ 256u ];

		snprintf( service_cmd, 255u, operation_cmd,
			status_cmds[i], id );
		service_cmd[ 255u ] = '\0';
		args.cmd = service_cmd;
		args.block = OS_TRUE;
		args.opts.block.max_wait_time = timeout;
		result = os_system_run( &args );
#if !defined(__ANDROID__)
		if ( result == OS_STATUS_SUCCESS )
		{
			/* is-failed returns 0 if it's failed */
			if ( ( args.return_code != 0 && i != 2u ) ||
			     ( args.return_code == 0 && i == 2u ) )
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
		if ( result != OS_STATUS_SUCCESS || args.return_code != 0 )
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
	os_system_run_args_t args = OS_SYSTEM_RUN_ARGS_INIT;
	os_status_t result;
	char service_cmd[ 256u ];

	snprintf( service_cmd, 255u, "systemctl restart %s", id );
	service_cmd[ 255u ] = '\0';
	args.cmd = service_cmd;
	args.block = OS_TRUE;
	args.privileged = OS_TRUE;
	args.opts.block.max_wait_time = timeout;

	result = os_system_run( &args );
	if ( result == OS_STATUS_SUCCESS && args.return_code != 0 )
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

os_status_t os_system_shutdown(
	os_bool_t reboot, unsigned int delay )
{
	os_system_run_args_t args = OS_SYSTEM_RUN_ARGS_INIT;
	char cmd[ PATH_MAX ];

	if ( reboot == OS_FALSE )
		os_snprintf( cmd, PATH_MAX, "%s %d", OS_SHUTDOWN_CMD, delay );
	else
		os_snprintf( cmd, PATH_MAX, "%s %d", OS_REBOOT_CMD, delay );

	args.cmd = cmd;
	return os_system_run( &args );
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

