/**
 * @file
 * @brief header file declaring functions & symbols for POSIX systems
 *
 * @copyright Copyright (C) 2016-2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */
#ifndef OS_POSIX_H
#define OS_POSIX_H

#ifndef OS_H
#error "This file must be included only by os.h"
#endif /* ifndef OS_H */

#include <limits.h>      /* for PATH_MAX */
#include <stdio.h>       /* for FILE* */

#ifndef _WRS_KERNEL
#include <sys/utsname.h> /* for struct utsname */
#endif

#ifndef _WRS_KERNEL
#	include <uuid/uuid.h>   /* for libuuid functions + uuid_t */
#endif
#define os_uuid_t uuid_t

#ifndef _WRS_KERNEL
/** @brief Maximum length of a host name on POSIX systems */
#ifndef _POSIX_HOST_NAME_MAX
#	define _POSIX_HOST_NAME_MAX 64
#endif
#define OS_HOST_MAX_LEN _POSIX_HOST_NAME_MAX
#endif

/**
 * @brief Character used to split environment variables
 */
#define OS_ENV_SPLIT               ':'

/**
 * @brief Character sequence for a line break
 */
#define OS_FILE_LINE_BREAK         "\n"
/**
 * @brief Seek from start of file
 * @see os_file_fseek
 */
#define OS_FILE_SEEK_START         SEEK_SET
/**
 * @brief Seek from current position in file
 * @see os_file_fseek
 */
#define OS_FILE_SEEK_CURRENT       SEEK_CUR
/**
 * @brief Seek from end of file
 * @see os_file_fseek
 */
#define OS_FILE_SEEK_END           SEEK_END

/**
 * @brief Invalid socket symbol
 */
#define OS_SOCKET_INVALID -1
/**
 * @brief Symbol to use when thread linking
 */
#define OS_THREAD_LINK
/**
 * @brief Return type for a thread main function
 */
#define OS_THREAD_RETURN void*

/**
 * @brief Handle to an open file
 */
typedef FILE *os_file_t;
/**
 * @brief Defines type for invalid file handle
 */
#define OS_FILE_INVALID  NULL
/**
 * @brief Defines the symbol for standard error
 */
#define OS_STDERR        stderr
/**
 * @brief Defines the symbol for standard in
 */
#define OS_STDIN         stdin
/**
 * @brief Defines the symbol for standard out
 */
#define OS_STDOUT        stdout

#	include <arpa/inet.h>  /* for inet_ntoa, inet_htons */
#	include <dirent.h>     /* for DIR*, opendir, closedir */
#	include <fcntl.h>      /* for open, O_WRONLY, O_CREAT, O_EXCL, S_IRUSR,
	                          S_IWUSR, S_IRGRP, S_IROTH */
#	include <limits.h>     /* for PATH_MAX */
#	include <netdb.h>      /* for struct addrinfo */
#	include <pthread.h>    /* for threading support */
#	include <signal.h>     /* for siginfo_t */
#	include <sys/socket.h> /* for struct addrinfo */

#	if defined(__linux__) || defined (_WRS_KERNEL)
#		include <net/if.h>    /* for struct ifconf */
#	endif
#	include <ifaddrs.h>

	/**
	 * @brief Structure holding internal adapter list
	 */
	typedef struct os_adapters
	{
		/** @brief Current adapter information (used to obtain mac) */
		struct addrinfo *addr;
		/** @brief Current adapter */
		struct ifaddrs *current;
		/** @brief First adapter */
		struct ifaddrs *first;
	} os_adapters_t;

	/**
	 * @brief Directory seperator character
	 */
#	define OS_DIR_SEP '/'
	/**
	 * @brief Structure holding directory list information
	 */
	typedef struct os_dir
	{
		/** @brief Handle to the open directory */
		DIR *dir;
		/** @brief Path to the directory */
		char path[PATH_MAX];
	} os_dir_t;

	/**
	 * @brief Handle to an open shared library
	 */
	typedef void *iot_lib_handle_t;

	/**
	 * @brief Handle to a thread
	 */
	typedef pthread_t os_thread_t;
	/**
	 * @brief Thread condition lock
	 */
	typedef pthread_cond_t os_thread_condition_t;
	/**
	 * @brief Thread mutually exclusive (mutex) lock
	 */
	typedef pthread_mutex_t os_thread_mutex_t;

#ifndef _WRS_KERNEL
	/**
	 * @brief Thread read/write lock
	 */
	typedef pthread_rwlock_t os_thread_rwlock_t;
#endif

#define os_vsnprintf                   vsnprintf

#endif /* ifndef OS_POSIX_H */

