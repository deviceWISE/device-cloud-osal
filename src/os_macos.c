/**
 * @file
 * @brief source file defining functions for mac osx systems
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "os_posix_private.h"

/* service functions */
os_status_t os_service_run(
	const char *UNUSED(id),
	os_service_main_t UNUSED(service_function),
	int UNUSED(argc),
	char *UNUSED(argv[]),
	int UNUSED(remove_argc),
	const char *UNUSED(remove_argv[]),
	os_sighandler_t UNUSED(handler),
	const char *UNUSED(logdir) )
{
	return OS_STATUS_NOT_SUPPORTED;
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
	const char *UNUSED(id),
	os_millisecond_t UNUSED(timeout)
)
{
	return OS_STATUS_NOT_SUPPORTED;
}

os_status_t os_service_stop(
	const char *UNUSED(id),
	const char *UNUSED(exe),
	os_millisecond_t UNUSED(timeout)
)
{
	return OS_STATUS_NOT_SUPPORTED;
}

os_status_t os_service_query(
	const char *UNUSED(id),
	os_millisecond_t UNUSED(timeout)
)
{
	return OS_STATUS_NOT_SUPPORTED;
}

os_status_t os_service_restart(
	const char *UNUSED(id),
	const char *UNUSED(exe),
	os_millisecond_t UNUSED(timeout)
)
{
	return OS_STATUS_NOT_SUPPORTED;
}

