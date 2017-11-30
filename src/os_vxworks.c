/**
 * @file  os_vxworks.c
 * @brief VxWorks OS adaptation layer
 *
 * @copyright Copyright (C) 2016-2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#ifdef __vxworks
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
#endif
extern BOOT_PARAMS sysBootParams;
#endif

#define VX_RW_SEM_MAX_READERS (255)

#ifdef _WRS_KERNEL
extern const char *deviceCloudRtpDirGet (void);
extern unsigned int deviceCloudPriorityGet (void);
extern unsigned int deviceCloudStackSizeGet (void);
#else
#define deviceCloudRtpDirGet()    "/bd0:1/bin"
#define deviceCloudPriorityGet()  100
#define deviceCloudStackSizeGet() 0x10000
#endif

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
#endif
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
#endif
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

static void os_vxworks_reboot(void)
{
#ifdef _WRS_KERNEL
	/* Wait 5 seconds for messages to propagate */

	sleep (5);

	/* Force a cold reboot - We do not return */

	sysToMonitor(2);
#endif
}

#if defined(_WRS_KERNEL) && defined(_WRS_CONFIG_SYS_PWR_OFF)
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
#endif

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

	if (strncmp (argv[0], "reboot", sizeof("reboot")) == 0) {
		if (taskSpawn ("tReboot", 10, 0, 0x1000,
			(FUNCPTR) os_vxworks_reboot,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == TASK_ID_ERROR) {
			return OS_STATUS_FAILURE;
		}
	} else if (strncmp (argv[0], "shutdown", sizeof("shutdown")) == 0) {
#if defined(_WRS_KERNEL) && defined(_WRS_CONFIG_SYS_PWR_OFF)
		if (taskSpawn ("tShutdown", 10, 0, 0x1000,
			(FUNCPTR) os_vxworks_shutdown,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == TASK_ID_ERROR) {
			return OS_STATUS_FAILURE;
		}
#else
		return OS_STATUS_FAILURE;
#endif
	} else if (strncmp (argv[0], "decommission", sizeof("decommission")) == 0) {
#if defined(_WRS_KERNEL) && defined(_WRS_CONFIG_SYS_PWR_OFF)
		if (taskSpawn ("tDecommission", 10, 0, 0x1000,
			(FUNCPTR) os_vxworks_decommission,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == TASK_ID_ERROR) {
			return OS_STATUS_FAILURE;
		}
#else
		return OS_STATUS_FAILURE;
#endif
	} else if (strncmp (argv[0], "iot-update-copy", sizeof("iot-update-copy")) == 0) {
		if ( chdir ( deviceCloudRtpDirGet() ) != 0 )
			return OS_STATUS_FAILURE;

		argv[0] = "iot-update-copy";
		if (rtpSpawn (argv[0], argv, NULL,
			deviceCloudPriorityGet(),
			deviceCloudStackSizeGet(),
			RTP_LOADED_WAIT, VX_FP_TASK) == RTP_ID_ERROR) {
			return OS_STATUS_FAILURE;
		}
	} else if (strncmp (argv[0], "iot-relay", sizeof("iot-relay")) == 0) {
		if ( chdir ( deviceCloudRtpDirGet() ) != 0 )
			return OS_STATUS_FAILURE;

		if (rtpSpawn (argv[0], argv, NULL,
			deviceCloudPriorityGet(),
		 	deviceCloudStackSizeGet(),
			RTP_LOADED_WAIT, VX_FP_TASK) == RTP_ID_ERROR) {
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
	return os_system_run(command, exit_status, pipes);
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

#endif /* __vxworks */
