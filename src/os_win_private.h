/**
 * @file
 * @brief header file declaring functions & symbols for windows systems
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
#ifndef OS_WIN_PRIVATE_H
#define OS_WIN_PRIVATE_H

#include <os.h>
#pragma warning( push, 1 )
#	include <Winsock2.h>
#	include <wincrypt.h> /* for HCRYPTPROV, PROV_RSA_FULL */
#	include <iphlpapi.h>
#	include <strsafe.h>
#	include <ws2tcpip.h>

#	include <rpc.h>        /* for uuid functions */
	/** @brief Universally unique id type */
	typedef struct _GUID os_uuid_t;
#pragma warning( pop )

/**
 * @brief contains information about a socket
 */
struct os_socket
{
	/** @brief Contains the address host address */
	struct sockaddr addr;
	/** @brief Contains the host port */
	os_uint16_t port;
	/** @brief File descriptor to the open socket */
	int fd;
	/** @brief Socket type */
	int type;
	/** @brief Socket protocol */
	int protocol;
};

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
	const char *path;
} os_dir_t;

#endif /* ifndef OS_WIN_PRIVATE_H */

