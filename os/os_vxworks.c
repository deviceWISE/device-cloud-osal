/**
 * @file  os_vxworks.c
 * @brief VxWorks OS adaptation layer
 *
 * @copyright Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#ifdef _WRS_KERNEL
#include <vxWorks.h>
#include "../os_.h"
#include "../os/os_posix.h"
#include <uuid.h>
#include <semLib.h>
#include <kernelLib.h>
#include <version.h>
#include <string.h>
#include <agent_config.h>
#include <taskLib.h>
#include <types.h>

extern char *sysModel (void);
extern STATUS sysToMonitor (int startType);

iot_status_t os_system_info(
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
		strncpy( sys_info->system_name, "VxWorks", OS_SYSTEM_INFO_MAX_LEN );
		strncpy( sys_info->system_version, runtimeVersion, OS_SYSTEM_INFO_MAX_LEN );
		strncpy( sys_info->system_release, _WRS_CONFIG_CORE_KERNEL_VERSION, OS_SYSTEM_INFO_MAX_LEN );
		strncpy( sys_info->system_platform, sysModel(), OS_SYSTEM_INFO_MAX_LEN );
		strncpy( sys_info->host_name, "", OS_SYSTEM_INFO_MAX_LEN );
		sys_info->system_flags = 0;
	}
	return IOT_STATUS_SUCCESS;
}

iot_lib_handle_t os_library_open(
	const char *path )
{
	return 0;
}

iot_status_t os_library_close(
	iot_lib_handle_t lib )
{
	return IOT_STATUS_FAILURE;
}

#ifndef IOT_API_ONLY
void *os_library_find(
	iot_lib_handle_t lib,
	const char *function )
{
	return NULL;
}

iot_uint64_t os_directory_free_space( const char* path )
{
	return 0;
}

iot_status_t os_file_temp(
	char *prototype,
	size_t suffix_len)
{
	return IOT_STATUS_FAILURE;
}

iot_status_t os_process_cleanup( void )
{
	return 0;
}

#endif /* IOT_API_ONLY */

iot_status_t os_file_chown(
	const char *path,
	const char *user )
{
	return 0;
}

/** @todo fix later once it needs in vxWorks */
iot_status_t os_directory_delete(
	const char *path, const char *regex, iot_bool_t recursive )
{
	return 0;
}

/** @todo fix later once it needs in vxWorks */
iot_status_t os_stream_echo_set(
	os_file_t stream, iot_bool_t enable )
{
	return 0;
}

/* NOTE: All API library clients share the pthread_attr structure. */ 
static pthread_attr_t pthread_attr;
static pthread_attr_t *pPthread_attr = NULL;

iot_status_t os_thread_create(
	os_thread_t *thread,
	os_thread_main_t main,
	void *arg )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;

	if ( main )
	{
		result = IOT_STATUS_FAILURE;

		if ( NULL == pPthread_attr )
		{
			/* Should setup the structure the first time through */
			if ( ( 0 == pthread_attr_init( &pthread_attr ) ) &&
				( 0 == pthread_attr_setstacksize( &pthread_attr, get_hdc_agent_worker_thread_stack_size() ) ) )
				pPthread_attr = &pthread_attr;
			else
			return result;
		}

		if ( pthread_create( thread, pPthread_attr, main, arg ) == 0 )
			result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t os_thread_rwlock_create(
	os_thread_rwlock_t *lock )
{
	SEM_ID semId;
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;      
    
	if ( lock )
	{
		result = IOT_STATUS_FAILURE;
		semId = semRWCreate( 0, VX_RW_SEM_MAX_READERS );
		if ( SEM_ID_NULL != semId )
		{
			*lock = semId;
			result = IOT_STATUS_SUCCESS;
		}
	}
    
	return result;
}

iot_status_t os_thread_rwlock_read_lock(
	os_thread_rwlock_t *lock )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;

	if ( lock )
	{
		result = IOT_STATUS_FAILURE;
		if ( OK == semRTake( *lock, WAIT_FOREVER ) )
			result = IOT_STATUS_SUCCESS;
	}

	return result;
}

iot_status_t os_thread_rwlock_read_unlock(
	os_thread_rwlock_t *lock )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;

	if ( lock )
	{
		result = IOT_STATUS_FAILURE;
		if (  OK == semGive( *lock ) )
			result = IOT_STATUS_SUCCESS;
	}

	return result;
}

iot_status_t os_thread_rwlock_write_lock(
	os_thread_rwlock_t *lock )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
    
	if ( lock )
	{
		result = IOT_STATUS_FAILURE;
		if ( OK == semWTake( *lock, WAIT_FOREVER ) )
			result = IOT_STATUS_SUCCESS;
	}
    
	return result;
}

iot_status_t os_thread_rwlock_write_unlock(
	os_thread_rwlock_t *lock )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
    
	if ( lock )
	{
		result = IOT_STATUS_FAILURE;
		if (  OK == semGive( *lock ) )
			result = IOT_STATUS_SUCCESS;
	}
    
	return result;
}

iot_status_t os_thread_rwlock_destroy(
	os_thread_rwlock_t *lock )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;

	if ( lock )
	{
		result = IOT_STATUS_FAILURE;
		if ( OK == semDelete( *lock ) )
			result = IOT_STATUS_SUCCESS;
	}

	return result;
}

iot_status_t os_system_run(
	const char *command,
	int *exit_status,
	char *out_buf[2u],
	size_t out_len[2u],
	iot_millisecond_t UNUSED(max_time_out) )
{
	/*
	 * Go through list of supported commands
	 */

	if (strncmp (command, "reboot", 6) == 0)
		sysToMonitor(2);

	*exit_status = 0;
	return IOT_STATUS_SUCCESS;
}

/* uuid support */
#ifndef IOT_API_ONLY
iot_status_t os_uuid_generate(
	os_uuid_t *uuid )
{
	uint32_t status;
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;

	if ( uuid )
	{
		uuid_create( uuid, &status );
		if ( uuid_s_ok == status )
			result = IOT_STATUS_SUCCESS;
	}
    
	return result;
}

iot_status_t os_uuid_to_string_lower(
	os_uuid_t *uuid,
	char *dest,
	size_t len )
{
	uint32_t status;
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
    
	if ( uuid && dest && ( len >= 37 ) )
        {
		uuid_to_string( uuid, dest, &status );
		if ( uuid_s_ok == status )
			result = IOT_STATUS_SUCCESS;
	}
	return result;
    }
#endif /* ifndef IOT_API_ONLY */

iot_uint32_t os_system_pid( void )
{
	/* 
	* VxWorks TASK_ID is a 64-bit pointer on a 64-bit host... We need to get the
	* ID and convert it to a non-pointer and then cast to the return type to
	* avoid a compiler warning.
 	*/
	return (iot_uint32_t) (ULONG) taskIdSelf();
}

#endif /* _WRS_KERNEL */
