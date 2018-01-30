/**
 * @file  os_vxworks.c
 * @brief VxWorks OS adaptation layer
 *
 * @copyright Copyright (C) 2016-2017 Wind River Systems, Inc. All Rights Reserved.
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

#if defined(__VXWORKS__)
#include <vxWorks.h>
#include <ioLib.h>
#include "os.h"
#include <semLib.h>
#include <version.h>
#include <string.h>
#include <taskLib.h>
#include <rtpLib.h>
#include <pthread.h>
#include <wait.h>
#ifdef _WRS_KERNEL
#include <sysLib.h>
#include <vsbConfig.h>
#include <bootLib.h>
#ifdef _WRS_CONFIG_SYS_PWR_OFF
#include <powerOffLib.h>
#endif /* _WRS_CONFIG_SYS_PWR_OFF */
extern BOOT_PARAMS sysBootParams;
#endif /* _WRS_KERNEL */

#define VX_RW_SEM_MAX_READERS (255)

#ifdef _WRS_KERNEL
extern const char *deviceCloudRtpDirGet (void);
extern unsigned int deviceCloudPriorityGet (void);
extern unsigned int deviceCloudStackSizeGet (void);
#else
static char config_dir[PATH_MAX] = "/bd0:1/etc/iot";
static char runtime_dir[PATH_MAX] = "/bd0:1/var/lib/iot";
static char rtp_dir[PATH_MAX] = "/bd0:1/bin";
static int priority = 100;
static int stack_size = 0x10000;

void deviceCloudConfigDirSet (char *str)
    {
    strncpy(config_dir, str, PATH_MAX);
    }

void deviceCloudRuntimeDirSet (char *str)
    {
    strncpy(runtime_dir, str, PATH_MAX);
    }

void deviceCloudRtpDirSet (char *str)
    {
    strncpy(rtp_dir, str, PATH_MAX);
    }

void deviceCloudPrioritySet (char *str)
    {
    priority = atoi(str);
    }

void deviceCloudStackSizeSet (char *str)
    {
    stack_size = atoi(str);
    }

const char *deviceCloudConfigDirGet (void)
    {
    return config_dir;
    }

const char *deviceCloudRuntimeDirGet (void)
    {
    return runtime_dir;
    }

const char *deviceCloudRtpDirGet (void)
    {
    return rtp_dir;
    }

unsigned int deviceCloudPriorityGet (void)
    {
    return priority;
    }

unsigned int deviceCloudStackSizeGet (void)
    {
    return stack_size;
    }
#endif /* _WRS_KERNEL */


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

		strncpy( sys_info->vendor_name, "Wind River Systems", OS_SYSTEM_INFO_MAX_LEN );
		strncpy( sys_info->system_name, RUNTIME_NAME, OS_SYSTEM_INFO_MAX_LEN );
		strncpy( sys_info->system_version, VXWORKS_VERSION, OS_SYSTEM_INFO_MAX_LEN );
		strncpy( sys_info->system_release, _WRS_CONFIG_CORE_KERNEL_VERSION, OS_SYSTEM_INFO_MAX_LEN );
		strncpy( sys_info->system_platform, sysModel(), OS_SYSTEM_INFO_MAX_LEN );
#ifdef _WRS_KERNEL
		strncpy( sys_info->host_name, sysBootParams.targetName, OS_SYSTEM_INFO_MAX_LEN );
#else
		strncpy( sys_info->host_name, "", OS_SYSTEM_INFO_MAX_LEN );
#endif /* _WRS_KERNEL */
		sys_info->system_flags = 0;
	}
	return OS_STATUS_SUCCESS;
}

os_status_t os_file_copy(
	const char *old_path,
	const char *new_path )
{
	os_status_t result = OS_STATUS_FAILURE;

	if (copy(old_path, new_path) == OK)
	{
		sleep(5);
		result = OS_STATUS_SUCCESS;
	}

	return result;
}

os_status_t os_path_executable(
	char *path,
	size_t size )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;
	if ( path )
	{
		strncpy(path, deviceCloudRtpDirGet(), size);
		result = OS_STATUS_SUCCESS;
	}
	return result;
}

os_status_t os_process_cleanup( void )
{
	os_status_t result = OS_STATUS_FAILURE;
#ifndef _WRS_KERNEL
	if ( waitpid( -1, NULL, WNOHANG ) > 0 )
		result = OS_STATUS_SUCCESS;
#endif /* _WRS_KERNEL */
	return result;
}

os_status_t os_directory_delete(
	const char *path, const char *regex, os_bool_t recursive )
{
	os_status_t result = OS_STATUS_FAILURE;

	if (rmdir (path) == OK)
		result = OS_STATUS_SUCCESS;

	return result;
}

/* NOTE: All API library clients share the pthread_attr structure. */
static pthread_attr_t pthread_attr;
static pthread_attr_t *pPthread_attr = NULL;

os_status_t os_thread_create(
	os_thread_t *thread,
	os_thread_main_t main,
	void *arg )
{
	os_status_t result = OS_STATUS_BAD_PARAMETER;

	if ( main )
	{
		result = OS_STATUS_FAILURE;

		if ( NULL == pPthread_attr )
		{
			/* Should setup the structure the first time through */
			if ( ( 0 == pthread_attr_init( &pthread_attr ) ) &&
-			       ( 0 == pthread_attr_setstacksize( &pthread_attr, deviceCloudStackSizeGet() ) ) )
				pPthread_attr = &pthread_attr;
			else
				return result;
		}

		if ( pthread_create( thread, pPthread_attr, main, arg ) == 0 )
			result = OS_STATUS_SUCCESS;
	}
	return result;
}

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
			result = OS_STATUS_SUCCESS;
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

#ifdef _WRS_KERNEL
static void os_vxworks_reboot(void)
{
	/* Wait 5 seconds for messages to propagate */

	sleep (5);

	/* Force a cold reboot - We do not return */

	sysToMonitor(2);
}

#if defined(_WRS_CONFIG_SYS_PWR_OFF)
static void os_vxworks_shutdown(void)
{
	/* Wait 5 seconds for messages to propagate */

	sleep (5);

	powerOff();
}

static void os_vxworks_decommission(void)
{
	/* Wait 5 seconds for messages to propagate */

	sleep (5);

	powerOff();
}
#endif /* _WRS_CONFIG_SYS_PWR_OFF */
#endif /* _WRS_KERNEL */

static status_t os_vxworks_script(
	char * script )
{
        os_status_t result = OS_STATUS_FAILURE;
	char * shellTaskName;
	int fd;

	if ((fd = open(script, O_RDONLY, 0)) == ERROR) {
		printf("Error opening script at [%s] [%s]!\n", scriptPath,
				strerror(errnoGet()));
		return OS_STATUS_FAILURE;
	}

	if (shellGenericInit("INTERPRETER=Cmd", 0, NULL, &shellTaskName, FALSE,
		FALSE, fd, STD_OUT, STD_ERR) == OK) {
		result = OS_STATUS_SUCCESSFUL;
	}

	do {
		taskDelay(sysClkRateGet());
	} while (taskNameToId(shellTaskName) != TASK_ID_ERROR);

	close(fd);

	return result;
}

os_status_t os_system_run(
	const char *command,
	int *exit_status,
	os_file_t pipe_files[2u] )
{
	const char * argv[10];
	int argc = 0;

	/* set a default exit status */

	if ( exit_status )
		*exit_status = -1;

	/* tokenize the command */

	argv[argc] = strtok (command, " ");

	while ((argv[argc] != NULL) && (++argc < 9)) {
		argv[argc] = strtok (NULL, " ");
	}
	argv[9] = NULL;

	/*
	 * Go through list of supported commands
	 */

        if (strstr (argv[0], "iot-control") != NULL) {
                if ( chdir ( deviceCloudRtpDirGet() ) != 0 )
                        return OS_STATUS_FAILURE;

                argv[0] = "iot-control";
                if (rtpSpawn (argv[0], argv, NULL,
                        deviceCloudPriorityGet(),
                        deviceCloudStackSizeGet(),
                        RTP_LOADED_WAIT, VX_FP_TASK) == RTP_ID_ERROR) {
                        return OS_STATUS_FAILURE;
                }
	} else if (strstr (argv[0], "iot-update") != NULL) {
		if ( chdir ( deviceCloudRtpDirGet() ) != 0 )
			return OS_STATUS_FAILURE;

		argv[0] = "iot-update";
		if (rtpSpawn (argv[0], argv, NULL,
			deviceCloudPriorityGet(),
			deviceCloudStackSizeGet(),
			RTP_LOADED_WAIT, VX_FP_TASK) == RTP_ID_ERROR) {
			return OS_STATUS_FAILURE;
		}
	} else if (strstr (argv[0], "iot-relay") != NULL) {
		if ( chdir ( deviceCloudRtpDirGet() ) != 0 )
			return OS_STATUS_FAILURE;

		if (rtpSpawn (argv[0], argv, NULL,
			deviceCloudPriorityGet(),
		 	deviceCloudStackSizeGet(),
			RTP_LOADED_WAIT, VX_FP_TASK) == RTP_ID_ERROR) {
			return OS_STATUS_FAILURE;
		}
#if defined(_WRS_KERNEL)
        } else if (strncmp (argv[0], "reboot", sizeof("reboot")) == 0) {
                if (taskSpawn ("tReboot", 10, 0, 0x1000,
                        (FUNCPTR) os_vxworks_reboot,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == TASK_ID_ERROR) {
                        return OS_STATUS_FAILURE;
                }
#if defined(_WRS_CONFIG_SYS_PWR_OFF)
        } else if (strncmp (argv[0], "shutdown", sizeof("shutdown")) == 0) {
                if (taskSpawn ("tShutdown", 10, 0, 0x1000,
                        (FUNCPTR) os_vxworks_shutdown,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == TASK_ID_ERROR) {
                        return OS_STATUS_FAILURE;
                }
        } else if (strncmp (argv[0], "decommission", sizeof("decommission")) == 0) {
                if (taskSpawn ("tDecommission", 10, 0, 0x1000,
                        (FUNCPTR) os_vxworks_decommission,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == TASK_ID_ERROR) {
                        return OS_STATUS_FAILURE;
                }
#endif /* _WRS_CONFIG_SYS_PWR_OFF */
#endif /* _WRS_KERNEL */
        } else if (strncmp (argv[0], "sh", sizeof("sh")) == 0) {
		if (os_vxworks_script(argv[1]) == OS_STATUS_FAILURE) {
			return OS_STATUS_FAILURE;
		}
	} else {
		printf("Invalid command:%s\n", command);
		return OS_STATUS_FAILURE;
	}

	if ( exit_status )
		*exit_status = 0;

	return OS_STATUS_SUCCESS;
}

os_status_t os_system_run_wait(
	const char *command,
	int *exit_status,
	char *out_buf[2u],
	size_t out_len[2u],
	os_millisecond_t UNUSED(max_time_out) )
{
	os_file_t pipes[2u] = {NULL, NULL};
	os_status_t result = os_system_run(command, exit_status, pipes);
	return result;
}

os_uint32_t os_system_pid( void )
{
	/*
	* VxWorks TASK_ID is a 64-bit pointer on a 64-bit host... We need to get the
	* ID and convert it to a non-pointer and then cast to the return type to
	* avoid a compiler warning.
 	*/
	return (os_uint32_t) (ULONG) taskIdSelf();
}

#endif /* __VXWORKS__ */
