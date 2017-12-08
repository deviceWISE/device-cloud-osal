/**
 * @file
 * @brief
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
#ifndef OS_VXWORKS_H
#define OS_VXWORKS_H

/*
 * This file provides the pieces necessary for the OS abstraction layer to
 * function on VxWorks. Most of the functionality is provided by POSIX, but
 * some things are provided by the VxWorks kernel.
 */

#ifndef OS_H
#error "This file must be included only by os.h"
#endif /* ifndef OS_H */

#include <vxWorks.h>

#include <uuid.h>        /* for libuuid functions + uuid_t */
#include <semLib.h>      /* for SEM_ID */
#include <sockLib.h>     /* for socket() */
#include <selectLib.h>   /* for select() */

/* Maximum host name length string */
#define OS_HOST_MAX_LEN 256

/*
 * NOTE: For a POSIX RW semaphore there is no limit on the number of
 *       readers. Use a value of 255 as it should be larger than we
 *       need.
 */
#define VX_RW_SEM_MAX_READERS 255

typedef SEM_ID os_thread_rwlock_t;

#endif /* ifndef OS_VXWORKS_H */

