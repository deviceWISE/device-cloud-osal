/**
 * @file
 * @brief header file declaring types for Windows systems
 *
 * @copyright Copyright (C) 2016-2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */
#ifndef OS_WIN32_H
#define OS_WIN32_H

#ifndef OS_H
#error "This file must be included only by os_.h"
#endif /* ifndef OS_H */

/** @brief Don't support advanced Windows kernel calls */
#define WIN32_LEAN_AND_MEAN
#pragma warning( push, 1 )
#include <Windows.h>
#include <Winsock2.h>
#include <wincrypt.h>

#include <rpc.h>        /* for uuid functions */
	/** @brief Universally unique id type */
	typedef struct _GUID os_uuid_t;
#pragma warning( pop )

/* Define missing signals on Windows */
/** @brief Interruption signal */
#define SIGINT  2
/** @brief User defined signal 1 */
#define SIGUSR1 10
/** @brief Termination signal */
#define SIGTERM 15
/** @brief Child process exit signal */
#define SIGCHLD 17
/** @brief Continue signal */
#define SIGCONT 18
/** @brief Stop signal */
#define SIGSTOP 19

/** @brief Maximum length of a host name on Windows systems */
#define OS_HOST_MAX_LEN 255u

/* missing types in Windows */
/** @brief Signed string length type */
#if !defined(ssize_t)
#define ssize_t SSIZE_T
#endif
/** @brief Socket port type */
typedef u_short in_port_t;

/**
 * @brief Character used to split environment variables
 */
#define OS_ENV_SPLIT               ';'

/**
 * @brief Character sequence for a line break
 */
#define OS_FILE_LINE_BREAK         "\r\n"
/**
 * @brief Seek from start of file
 * @see os_file_fseek
 */
#define OS_FILE_SEEK_START         FILE_BEGIN
/**
 * @brief Seek from current position in file
 * @see os_file_fseek
 */
#define OS_FILE_SEEK_CURRENT       FILE_CURRENT
/**
 * @brief Seek from end of file
 * @see os_file_fseek
 */
#define OS_FILE_SEEK_END           FILE_END

/**
 * @brief Invalid socket symbol
 */
#define OS_SOCKET_INVALID INVALID_SOCKET
/**
 * @brief Symbol to use when thread linking
 */
#define OS_THREAD_LINK   __stdcall
/**
 * @brief Return type for a thread main function
 */
#define OS_THREAD_RETURN DWORD

/**
 * @brief Handle to an open file
 */
typedef HANDLE os_file_t;
/**
 * @brief Defines type for invalid file handle
 */
#define OS_FILE_INVALID  INVALID_HANDLE_VALUE
/**
 * @brief Defines the symbol for standard error
 */
#define OS_STDERR        GetStdHandle( STD_ERROR_HANDLE )
/**
 * @brief Defines the symbol for standard in
 */
#define OS_STDIN         GetStdHandle( STD_INPUT_HANDLE )
/**
 * @brief Defines the symbol for standard out
 */
#define OS_STDOUT        GetStdHandle( STD_OUTPUT_HANDLE )

#pragma warning( push, 1 )
#include <iphlpapi.h>
#include <strsafe.h>
#include <ws2tcpip.h>
#pragma warning( pop )
	/**
	 * @def PATH_MAX
	 * @brief Maximum path length
	 */
#	ifndef PATH_MAX
#		define PATH_MAX 4096
#	endif /* ifndef PATH_MAX */

	/**
	 * @brief Structure holding internal adapter list
	 */
	typedef struct os_adapters
	{
		/** @brief Current adapter */
		IP_ADAPTER_ADDRESSES *adapter_current;
		/** @brief First adapter */
		IP_ADAPTER_ADDRESSES *adapter_first;
		/** @brief Current unicast address */
		IP_ADAPTER_UNICAST_ADDRESS *current;
	} os_adapters_t;

	/**
	 * @brief Directory seperator character
	 */
#	define OS_DIR_SEP '\\'
	/**
	 * @brief Structure holding directory list information
	 */
	typedef struct os_dir
	{
		/** @brief Handle to the open directory */
		HANDLE dir;
		/** @brief Last error result */
		os_status_t last_result;
		/** @brief Currently found file within the directory */
		WIN32_FIND_DATA wfd;
		/** @brief Path to the directory */
		char path[PATH_MAX];
	} os_dir_t;

	/**
	 * @brief Handle to an open dynamically-linked library
	 */
	typedef HMODULE os_lib_handle;

	/**
	 * @brief Handle to a thread
	 */
	typedef HANDLE os_thread_t;
	/**
	 * @brief Thread condition lock
	 */
	typedef CONDITION_VARIABLE os_thread_condition_t;
	/**
	 * @brief Thread mutually exclusive (mutex) lock
	 */
	typedef CRITICAL_SECTION os_thread_mutex_t;
	/**
	 * @brief Thread read/write lock
	 */
	typedef SRWLOCK os_thread_rwlock_t;

#endif /* ifndef OS_WIN32_H */

